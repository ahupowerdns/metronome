#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "yahttp.hpp"
#include "iputils.hh"
#include "statstorage.hh"
#include <fenv.h>
#include <thread>
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include "interpolate.hh"
#include <boost/program_options.hpp>
#ifdef HAVE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

namespace po = boost::program_options;
po::variables_map g_vm;
bool g_verbose;
bool g_console = false;
bool g_disableSyslog;
bool g_http10;
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
  infolog("Closing connection with %s, stored %d data", remote.toStringWithPort(), numStored);
  close(sock);
}
catch(exception& e)
{
  errlog("Exception: %s", e.what());
  try {
    writen(sock, string("Error: ")+e.what()+"\n");
  }catch(...){}
  close(sock);
}

void dumpRequest(const YaHTTP::Request& req)
{
  cout<<"Headers: \n";
  for(auto h : req.headers) {
    cout << h.first << " -> "<<h.second<<endl;
  }
  cout<<"getvars: \n";
  for(auto h : req.getvars) {
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

  //  cout <<"Initial find for timestamp "<<timestamp<<", left side of window at: "<< timestamp-window/2.0 <<endl;
  
  if(from == vals.end()) {
    return {0,0};
  }
  if(from != vals.begin()) {
    --from;
    //cout<<"Lowered left once to "<<from->timestamp<<endl;
  }

  if(to != vals.end()) {
    ++to;
    //    cout<<"Raised right once to "<<to->timestamp<<endl;
  }
  if(to != vals.end()) {
    ++to;
    //    cout<<"Raised right again to "<<to->timestamp<<endl;
  }

  // cout.setf(std::ios::fixed);    
  //  cout<<"Desired timestamp:   "<<timestamp<<endl;
  //  cout<<"Num considered: "<<(to-from)<<endl;
  if(to - from == 1) { 
    return {from->value, 0};
  }
  
  vector<InterpolateDatum> id;
  id.reserve(to-from);
  bool hadReset=false;
  for(auto iter = from ; iter != to; ++iter) {
    //    cout << '\t'<< iter->timestamp - timestamp<<endl;
    id.push_back({(double)iter->timestamp, iter->value});
    if(iter != from && iter->value < prev(iter)->value)
      hadReset=true;
  }
  if(hadReset) {
    auto ret = interpolate(id, 3, timestamp);
    return make_pair(ret.first > 0 ? ret.first : 0, 0);
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

      input.append(line);
      if(line.empty() || line=="\n" || line=="\r\n") // XXX NO
	goto ok;      
    }
    close(sock);
    if(input.size()) {
      warnlog("Did not receive full request, got %ld bytes", input.size());
    }
    else {
      infolog("EOF from %s after %d requests", remote.toStringWithPort(), numrequests);
    }
    return;
  ok:;
    YaHTTP::Request req;
    istringstream str(input);
    
    str >> req;
    YaHTTP::Response resp(req);
    ostringstream body;
//    dumpRequest(req);
    resp.status=200;
    if(req.getvars["do"]=="store") {
      StatStorage ss(g_vm["stats-directory"].as<string>());
      ss.store(req.getvars["name"], atoi(req.getvars["timestamp"].c_str()), 
	       atof(req.getvars["value"].c_str()));
    }
    else if(req.getvars["do"]=="get-metrics") {  
      StatStorage ss(g_vm["stats-directory"].as<string>());
      resp.headers["Content-Type"]= "application/json";
      resp.headers["Access-Control-Allow-Origin"]= "*";
      body<<req.getvars["callback"]<<"(";
      body<<"{ \"metrics\": [";
      auto metrics = ss.getMetrics();
      for(const auto& metric : metrics)  {
	if(&metric != &metrics[0]) 
	  body<<',';
	body<<'"'<<metric<<'"';
      }
      body << "]});";
    }
    else if(req.getvars["do"]=="get-all") {  
      StatStorage ss(g_vm["stats-directory"].as<string>());
      auto vals = ss.retrieve(req.getvars["name"]);
      
      body.setf(std::ios::fixed);    
      for(const auto& v: vals) {
	auto s = smooth(vals, v.timestamp, 60);
	body<<v.timestamp<<'\t'<<v.value<<'\t'<<s.first<<'\t'<<s.second<<'\n';
      }
    }
    else if(req.getvars["do"]=="retrieve") {
      //    
      StatStorage ss(g_vm["stats-directory"].as<string>());
      vector<string> names;
      stringtok(names, req.getvars["name"], ",");
      resp.headers["Content-Type"]= "application/json";
      resp.headers["Access-Control-Allow-Origin"]= "*";
      
      body.setf(std::ios::fixed);
      
      double begin = atoi(req.getvars["begin"].c_str());
      double end = atoi(req.getvars["end"].c_str());
      int datapoints = atoi(req.getvars["datapoints"].c_str());
      if(!datapoints)
	datapoints=100;
      body<<req.getvars["callback"]<<"(";
      body<<"{ raw: {";
      bool first=true;
      map<string,vector<StatStorage::Datum> > derivative;
      for(const auto& name : names) {
	// little bit of margin so interpolation has chance to work
	auto vals = ss.retrieve(name, begin - (end-begin)/20.0, end + (end-begin)/20.0);
	if(!first) 
	  body<<',';
	first=false;
	
	body<< '"' << name << "\": [";
	int count=0;
	vector<StatStorage::Datum> derived;

	double step = (end-begin)/datapoints;
	//cout<<"step: "<<step<<", "<<datapoints<<endl;
	for(double t = begin ; t < end ; t+= step) {
	  auto inst = smooth(vals, t, 1.5*step);

	  if(count) {
	    body<<',';
	  }
	  body<<"["<<(uint32_t)t<<','<<inst.first<<']';   
	  if(!vals.empty() && t + step >= vals.rbegin()->timestamp && t - vals.rbegin()->timestamp < 60 && !derived.empty()) {
	    derived.push_back({(uint32_t)t, derived.rbegin()->value});
	  }
	  else
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
    if(g_http10)
      resp.headers["Connection"]="Close"; 
    else
      resp.headers["Connection"]="Keep-Alive";
    ostringstream ostr;
    ostr << resp;
    
    writen(sock, ostr.str());
    if(g_http10)
      break;
  }
  close(sock);
}
catch(exception& e) {
  errlog("Web connection thread for %s terminated because of error: %s", remote.toStringWithPort(), e.what());
  close(sock);
}

void webServerThread(int sock, const ComboAddress& local)
try
{
  for(;;) {
    ComboAddress remote=local; // sets the family flag right
    int client=SAccept(sock, remote);
    if(client >= 0) {
      thread t1(startWebserverThread, client, remote);
      t1.detach();
    }
    else 
      errlog("Error from accept: %s", strerror(errno));
  }
}
catch(...)
{
  errlog("Webserver thread died because of exception");
}

void launchWebserver(int s, const ComboAddress& local)
{
  thread t1(webServerThread, s, local);
  t1.detach();
}

void processCommandLine(int argc, char **argv)
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "produce help message")
    ("carbon-address", po::value<string>()->default_value("[::]:2003"), "Accept carbon data on this address")
    ("webserver-address", po::value<string>()->default_value("[::]:8000"), "Provide HTTP service on this address")
    ("http1.0", "If set, use http 1.0 semantics for lighttpd proxy")
    ("quiet", po::value<bool>()->default_value(true), "don't be too noisy")
    ("daemon", po::value<bool>()->default_value(true), "run in background")
    ("disable-syslog", "don't log to syslog")
    ("stats-directory", po::value<string>()->default_value("./stats"), "Store/access statistics from this directory");

  try {
    po::store(po::command_line_parser(argc, argv).options(desc).run(), g_vm);
    po::notify(g_vm);
  }
  catch(std::exception& e) {
    cerr<<"Error parsing options: "<<e.what()<<endl;
    cout<<desc<<endl;
    exit(EXIT_SUCCESS);
  }
  if(g_vm.count("help")) {
    cout<<desc<<endl;
    exit(EXIT_SUCCESS);
  }
  if(g_vm.count("http1.0"))
    g_http10=true;
  if(g_vm.count("disable-syslog"))
    g_disableSyslog=true;
}


int makeAndBindSocket(const ComboAddress& local, string name) 
{
  int s = SSocket(local.sin4.sin_family, SOCK_STREAM, 0);
  SSetsockopt(s, SOL_SOCKET, SO_REUSEADDR, 1);
  SBind(s, local);
  SListen(s, 10);
  warnlog("Launched %s functionality on %s", name, local.toStringWithPort());
  return s;
}


int main(int argc, char** argv)
try
{
  signal(SIGPIPE, SIG_IGN);
#ifdef __linux__
  feenableexcept(FE_DIVBYZERO | FE_INVALID); 
#endif 

  openlog("metronome", LOG_PID, LOG_DAEMON);
  g_console=true;
  g_verbose=true;  
  processCommandLine(argc, argv);

  g_verbose=!g_vm["quiet"].as<bool>();

  ComboAddress carbonLocal{g_vm["carbon-address"].as<string>(), 2003};
  int s = makeAndBindSocket(carbonLocal, "carbon");
  ComboAddress wsLocal{g_vm["webserver-address"].as<string>(), 8000};
  int ws = makeAndBindSocket(wsLocal, "webserver");
  
  if(g_vm["daemon"].as<bool>())  {
    daemonize();
    warnlog("daemonizing as %d", getpid());
    g_console=false;
  }
  else {
    infolog("Running in the foreground");
  }

  launchWebserver(ws, wsLocal);

#ifdef HAVE_SYSTEMD
  sd_notify(0, "READY=1");
#endif

  int client;
  ComboAddress remote=carbonLocal;
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
