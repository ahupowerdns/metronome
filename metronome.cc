#include "yahttp.hpp"
#include "iputils.hh"
#include "statstorage.hh"
#include <thread>
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include "interpolate.hh"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
po::variables_map g_vm;
bool g_verbose;
bool g_console;

using namespace std;

void startCarbonThread(int sock, ComboAddress remote)
try
{
  StatStorage ss(g_vm["stats-directory"].as<string>());
  infolog("Got carbon connection from %s", remote.toStringWithPort());
  string line;

  int numStored=0;
  while(sockGetLine(sock, &line)) {
    // format: name value timestamp
//    cout<<"Got: "<<line;
    vector<string> parts;
    stringtok(parts, line, " \t\r\n");
    if(parts.size()!=3) {
      writen(sock, "ERR Wrong number of parts to line");
      break;
    }	
    ss.store(parts[0], atoi(parts[2].c_str()), atof(parts[1].c_str()));
    numStored++;
  }
  infolog("Closing connection with %s, stored %d data", remote.toStringWithPort() % numStored);
  close(sock);
}
catch(exception& e)
{
  errlog("Exception: %s", e.what());
  writen(sock, string("Error: ")+e.what()+"\n");
  close(sock);
}

void dumpRequest(const YaHTTP::Request& req)
{
  cout<<"Headers: \n";
  for(auto h : req.headers) {
    cout << h.first << " -> "<<h.second<<endl;
  }
  cout<<"Parameters: \n";
  for(auto h : req.parameters) {
    cout << h.first << " -> "<<h.second<<endl;
  }
  cout<<"URL: "<<req.url<<endl;
  cout<<"Body: "<<req.body<<endl;
  cout<<"Method: "<<req.method<<endl;
}

static void daemonize(void)
{
  if(fork())
    _exit(0); // bye bye
  
  setsid(); 

  int i=open("/dev/null",O_RDWR); /* open stdin */
  if(i < 0) 
    ; // L<<Logger::Critical<<"Unable to open /dev/null: "<<stringerror()<<endl;
  else {
    dup2(i,0); /* stdin */
    dup2(i,1); /* stderr */
    dup2(i,2); /* stderr */
    close(i);
  }
}


pair<double,double> smooth(const vector<StatStorage::Datum>& vals, double timestamp, int window)
{
  auto from = upper_bound(vals.begin(), vals.end(), timestamp-window/2.0);
  auto to = upper_bound(vals.begin(), vals.end(), timestamp+window/2.0);

  if(from == vals.end())
    return {0,0};
  if(from != vals.begin())
    --from;

  if(to != vals.end())
    ++to;
  if(to != vals.end())
    ++to;

  // cout.setf(std::ios::fixed);    
  //  cout<<"Desired timestamp:   "<<timestamp<<endl;
  if(to - from == 1) {
    return {from->value, 0};
  }
  vector<InterpolateDatum> id;
  id.reserve(to-from);
  for(auto iter = from ; iter != to; ++iter) {
    id.push_back({(double)iter->timestamp, iter->value});
  }
  return interpolate(id, 3, timestamp);
}

void startWebserverThread(int sock, ComboAddress remote)
try
{
  infolog("Got web connection from %s", remote.toStringWithPort());

  for(int numrequests=0;;++numrequests) {
    string input, line;
    while(sockGetLine(sock, &line)) {
      if(line.empty() || line=="\n" || line=="\r\n") // XXX NO
	goto ok;
      input.append(line);
    }
    close(sock);
    if(input.size()) {
      warnlog("Did not receive full request, got %ld bytes", input.size());
    }
    else {
      infolog("EOF from %s after %d requests", remote.toStringWithPort() % numrequests);
    }
    return;
  ok:;
    YaHTTP::Request req;
    istringstream str(input);
    req.load(str);
    
    YaHTTP::Response resp(req);
    ostringstream body;
    
    if(req.parameters["do"]=="store") {
      StatStorage ss(g_vm["stats-directory"].as<string>());
      ss.store(req.parameters["name"], atoi(req.parameters["timestamp"].c_str()), 
	       atof(req.parameters["value"].c_str()));
    }
    else if(req.parameters["do"]=="get-metrics") {  
      StatStorage ss(g_vm["stats-directory"].as<string>());
      resp.headers["Content-Type"]= "application/json";
      resp.headers["Access-Control-Allow-Origin"]= "*";
      body<<req.parameters["callback"]<<"(";
      body<<"{ \"metrics\": [";
      auto metrics = ss.getMetrics();
      for(const auto& metric : metrics)  {
	if(&metric != &metrics[0]) 
	  body<<',';
	body<<'\''<<metric<<'\'';
      }
      body << "]});";
    }
    else if(req.parameters["do"]=="get-all") {  
      StatStorage ss(g_vm["stats-directory"].as<string>());
      auto vals = ss.retrieve(req.parameters["name"]);
      
      body.setf(std::ios::fixed);    
      for(const auto& v: vals) {
	auto s = smooth(vals, v.timestamp, 60);
	body<<v.timestamp<<'\t'<<v.value<<'\t'<<s.first<<'\t'<<s.second<<'\n';
      }
    }
    else if(req.parameters["do"]=="retrieve") {
      //    dumpRequest(req);
      StatStorage ss(g_vm["stats-directory"].as<string>());
      vector<string> names;
      stringtok(names, req.parameters["name"], ",");
      resp.headers["Content-Type"]= "application/json";
      resp.headers["Access-Control-Allow-Origin"]= "*";
      
      body.setf(std::ios::fixed);
      
      double begin = atoi(req.parameters["begin"].c_str());
      double end = atoi(req.parameters["end"].c_str());
      
      body<<req.parameters["callback"]<<"(";
      body<<"{ raw: {";
      bool first=true;
      map<string,vector<StatStorage::Datum> > derivative;
      for(const auto& name : names) {
	auto vals = ss.retrieve(name, begin, end);
	if(!first) 
	  body<<',';
	first=false;
	
	body<< '"' << name << "\": [";
	int count=0;
	vector<StatStorage::Datum> derived;

	double step = (end-begin)/100.0;
	//      cout<<"step: "<<step<<endl;
	for(double t = begin ; t < end; t+= step) {
	  auto inst = smooth(vals, t, 1.5*step);

	  if(count) {
	    body<<',';
	  }
	  body<<"["<<(uint32_t)t<<','<<inst.first<<']';   
	  derived.push_back({(uint32_t)t, inst.second > 0 ? (float)inst.second : 0});
	  count++; 
	}
	body<<"]";
	derivative[name]=derived;
      }
      body<<"}, derivative: {  ";
      first=true;
      for(const auto& deriv: derivative) {
	if(!first)
	  body<<',';
	first=false;
	body<< '"' << deriv.first << "\": [";
	int count=0;
	for(auto iter = deriv.second.begin(); iter !=deriv.second.end(); ++iter) {
	  if(count) 
	    body<<',';
	  body<<"["<<iter->timestamp<<','<<iter->value<<']';   
	  count++; 
	}
	body<<"]";
      }
      body <<"}});";
    }
    else {
      resp.status=404;
      body<<"404 File not found"<<endl;
    }
    resp.body=body.str();
    resp.headers["Content-Length"]=boost::lexical_cast<string>(resp.body.length());
    resp.headers["Connection"]="Keep-Alive";
    ostringstream ostr;
    ostr << resp;
    
    writen(sock, ostr.str());
  }
  close(sock);
}
catch(exception& e) {
  errlog("Web connection thread terminated because of error: %s", e.what());
  close(sock);
}

void webServerThread(int sock)
{
  for(;;) {
    ComboAddress remote("::");
    int client=SAccept(sock, remote);
    if(client >= 0) {
      thread t1(startWebserverThread, client, remote);
      t1.detach();
    }
    else 
      errlog("Error from accept: %s", strerror(errno));
  }
}

void launchWebserver()
{
  ComboAddress localWeb(g_vm["webserver-address"].as<string>(), 8000);
  int s = SSocket(localWeb.sin4.sin_family, SOCK_STREAM, 0);

  SSetsockopt(s, SOL_SOCKET, SO_REUSEADDR, 1);
  SBind(s, localWeb);
  SListen(s, 10);
  warnlog("Launched webserver on %s", localWeb.toStringWithPort());
  thread t1(webServerThread, s);
  t1.detach();
}

void processCommandLine(int argc, char **argv)
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "produce help message")
    ("carbon-address", po::value<string>()->default_value("[::]:2003"), "Accept carbon data on this address")
    ("webserver-address", po::value<string>()->default_value("[::]:8000"), "Provide HTTP service on this address")
    ("quiet", po::value<bool>()->default_value(true), "don't be too noisy")
    ("daemon", po::value<bool>()->default_value(true), "run in background")
    ("stats-directory", po::value<string>()->default_value("./stats"), "Store/access statistics from this directory");

  po::store(po::command_line_parser(argc, argv).options(desc).run(), g_vm);
  po::notify(g_vm);
  if(g_vm.count("help")) {
    cout<<desc<<endl;
    exit(EXIT_SUCCESS);
  }
}

int main(int argc, char** argv)
try
{
  signal(SIGPIPE, SIG_IGN);
  openlog("metronome", LOG_PID, LOG_DAEMON);
  
  processCommandLine(argc, argv);
  g_console=true;
  g_verbose=!g_vm["quiet"].as<bool>();
  ComboAddress localCarbon(g_vm["carbon-address"].as<string>(), 2003);
  int s = SSocket(localCarbon.sin4.sin_family, SOCK_STREAM, 0);

  SSetsockopt(s, SOL_SOCKET, SO_REUSEADDR, 1);
  SBind(s, localCarbon);
  SListen(s, 10);
  warnlog("Launched Carbon functionality on %s", localCarbon.toStringWithPort());

  launchWebserver();
  
  if(g_vm["daemon"].as<bool>())  {
    daemonize();
    warnlog("daemonizing as %d", getpid());
    g_console=false;
  }
  else {
    infolog("Running in the %s", "foreground");

  }

  
  int client;
  ComboAddress remote=localCarbon;
  for(;;) {
    client=SAccept(s, remote);
    if(client >= 0) {
      thread t1(startCarbonThread, client, remote);
      t1.detach();
    }
  }		 
}
catch(exception& e) {
  errlog("Fatal error: %s", e.what());
  exit(EXIT_FAILURE);
}
