#include "yahttp.hpp"
#include "iputils.hh"
#include "statstorage.hh"
#include <thread>
#include <mutex>

using namespace std;


void startCarbonThread(int sock, ComboAddress remote)
try
{
  StatStorage ss("./stats");
  cout<<"Got connection from "<<remote.toStringWithPort()<<endl;
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
  cout<<"Closing connection, stored "<<numStored<<" data"<<endl;
  close(sock);
}
catch(exception& e)
{
  cerr<<"Exception: "<<e.what()<<endl;
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

double smooth(const vector<StatStorage::Datum>& vals, double timestamp, int window)
{
  auto from = upper_bound(vals.begin(), vals.end(), timestamp-window/2.0);
  auto to = upper_bound(vals.begin(), vals.end(), timestamp+window/2.0);

  if(from == vals.end())
    return 0;
  if(from != vals.begin())
    --from;

  if(to != vals.end())
    ++to;
  if(to != vals.end())
    ++to;

  double xySum=0, xSum=0, ySum=0, x2Sum=0; 
  int n=0;
  
  // cout.setf(std::ios::fixed);    
  //  cout<<"Desired timestamp:   "<<timestamp<<endl;
  if(to - from == 1) {
    return from->value;
  }
  for(auto iter = from ; iter != to; ++iter) {
    //    cout<<"\tConsidering: "<<(iter->timestamp)<<"\t"<<iter->value<<endl;
    double adjT = iter->timestamp - timestamp;
    xySum += (adjT) * iter-> value;
    xSum += (adjT);
    ySum += iter->value;
    x2Sum += adjT* adjT;
    n++;
  }
  
  double beta = (xySum - (xSum*ySum)/n)   /  (x2Sum - xSum*xSum/n);
  double alpha = ySum / n - beta*xSum/n;

  double ret= alpha; // + beta*timestamp;
  //  cout<<n<<", "<<alpha<<", "<<beta<<" -> "<<ret<<endl;

  return ret;
}

void startWebserverThread(int sock, ComboAddress remote)
try
{
  string line;
  //  cout<<"Got web connection from "<<remote.toStringWithPort()<<endl;

  string input;
  while(sockGetLine(sock, &line)) {
    if(line.empty() || line=="\n" || line=="\r\n") // XXX NO
      goto ok;
    input.append(line);
  }
  close(sock);
  cerr<<"Did not receive full request, got "<<input.size()<<" bytes"<<endl;
  return;
 ok:;
  YaHTTP::Request req;
  istringstream str(input);
  req.load(str);

  YaHTTP::Response resp(req);
  ostringstream body;
  
  if(req.parameters["do"]=="store") {
    StatStorage ss("./stats");
    ss.store(req.parameters["name"], atoi(req.parameters["timestamp"].c_str()), 
	     atof(req.parameters["value"].c_str()));
  }
  else if(req.parameters["do"]=="get-metrics") {  
    StatStorage ss("./stats");
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
    StatStorage ss("./stats");
    auto vals = ss.retrieve(req.parameters["name"]);

    body.setf(std::ios::fixed);    
    for(const auto& v: vals) {
      body<<v.timestamp<<'\t'<<v.value<<'\t'<<smooth(vals, v.timestamp, 60)<<'\t'<<smooth(vals, v.timestamp, 512)<<'\n';
    }
  }
  else if(req.parameters["do"]=="retrieve") {
      //    dumpRequest(req);
    StatStorage ss("./stats");
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
      uint32_t prevt=0;
      double step = (end-begin)/100.0;
      //      cout<<"step: "<<step<<endl;
      for(double t = begin ; t < end; t+= step) {
	if(count) {
	  body<<',';
	  float val = (smooth(vals, t, 2*step)-smooth(vals, prevt, 2*step))/(step);
	  if(val < 0)
	    val=0;
	  derived.push_back({prevt, val});
	}
	body<<"["<<(uint32_t)t<<','<<smooth(vals, t, 2*step)<<']';   
	count++; 
	prevt=t;
      }
      body<<"]";
      if(!derived.empty())
        derived.push_back({prevt, derived.rbegin()->value});
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
  ostringstream ostr;
  ostr << resp;
  
  writen(sock, ostr.str());

  close(sock);
}
catch(exception& e) {
  cerr<<"Dying because of error: "<<e.what()<<endl;
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
      cerr<<"Error from accept: "<<strerror(errno)<<endl;
  }
}

void launchWebserver()
{
  ComboAddress localWeb("::", 8000);
  int s = SSocket(localWeb.sin4.sin_family, SOCK_STREAM, 0);

  SSetsockopt(s, SOL_SOCKET, SO_REUSEADDR, 1);
  SBind(s, localWeb);
  SListen(s, 10);
  cout<<"Launched webserver on "<<localWeb.toStringWithPort()<<endl;
  thread t1(webServerThread, s);
  t1.detach();
}

int main(int argc, char** argv)
try
{
  ComboAddress localCarbon("::", 2003);
  int s = SSocket(localCarbon.sin4.sin_family, SOCK_STREAM, 0);

  SSetsockopt(s, SOL_SOCKET, SO_REUSEADDR, 1);
  SBind(s, localCarbon);
  SListen(s, 10);
  cout<<"Launched Carbon functionality on "<<localCarbon.toStringWithPort()<<endl;

  launchWebserver();
  
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
  cerr<<"Fatal error: "<<e.what()<<endl;
  exit(EXIT_FAILURE);
}
