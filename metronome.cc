#include "yahttp.hpp"
#include "iputils.hh"
#include "statstorage.hh"
#include <thread>

using namespace std;

bool sockGetLine(int sock, string* ret)
{
  ret->clear();
  char c;
  int err;
  for(;;) {
    err=read(sock, &c, 1);
    if(err < 0)
      throw runtime_error("Error reading from socket: "+string(strerror(errno)));
    if(!err)
      break;
    ret->append(1, c);
    if(c=='\n')
      break;
  }
  return !ret->empty();
}

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

vector<StatStorage::Datum> nonNegativeDerivative(const vector<StatStorage::Datum>& in)
{
  vector<StatStorage::Datum> ret;
  if(in.empty())
    return ret;

  for(auto iter = in.cbegin()+1; iter != in.cend(); ++iter) {
    ret.push_back({(iter-1)->timestamp, (iter->value - (iter-1)->value)/(iter->timestamp - (iter-1)->timestamp)});
  }
  ret.push_back({in.rbegin()->timestamp, ret.rbegin()->value});
  return ret;
}

double smooth(const vector<StatStorage::Datum>& vals, uint32_t timestamp, int window)
{
  auto from = upper_bound(vals.begin(), vals.end(), timestamp-window/2.0);
  auto to = upper_bound(vals.begin(), vals.end(), timestamp+window/2.0);
  double total=0;
  if(to==from)
    return 0;
  for(auto iter = from ; iter != to; ++iter) {
    total += iter->value; // we could so some weighing here if we wanted
  }
  
  return total/(to-from);
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
  cerr<<"Did not receive full request"<<endl;
  return;
 ok:;
  YaHTTP::Request req;
  istringstream str(input);
  req.load(str);

  YaHTTP::Response resp(req);

  if(req.parameters["do"]=="store") {
    StatStorage ss("./stats");
    ss.store(req.parameters["name"], atoi(req.parameters["timestamp"].c_str()), 
	     atof(req.parameters["value"].c_str()));
  }
  else if(req.parameters["do"]=="get-all") {  
    StatStorage ss("./stats");
    auto vals = ss.retrieve(req.parameters["name"]);
    ostringstream body;
    body.setf(std::ios::fixed);    
    for(const auto& v: vals) {
      body<<v.timestamp<<'\t'<<v.value<<'\t'<<smooth(vals, v.timestamp, 60)<<'\t'<<smooth(vals, v.timestamp, 512)<<'\n';
    }
    resp.body=body.str();
  }
  else if(req.parameters["do"]=="retrieve") {
      //    dumpRequest(req);
    StatStorage ss("./stats");
    vector<string> names;
    stringtok(names, req.parameters["name"], ",");
    resp.headers["Content-Type"]= "application/json";
    resp.headers["Access-Control-Allow-Origin"]= "*";
    ostringstream body;
    body.setf(std::ios::fixed);

    double begin = atoi(req.parameters["begin"].c_str());
    double end = atoi(req.parameters["end"].c_str());

    body<<req.parameters["callback"]<<"({ raw: {";
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
      for(double t = begin ; t < end; t+= step) {
	if(count) {
	  body<<',';
	  float val = (smooth(vals, t, 2*step)-smooth(vals, prevt, 2*step))/(step);
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
    resp.body=body.str();
  }


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
  int s = SSocket(localWeb.sin4.sin_family,
		 SOCK_STREAM, 0);

  SSetsockopt(s, SOL_SOCKET, SO_REUSEADDR, 1);
  SBind(s, localWeb);
  SListen(s, 10);
  cout<<"Launched webserver on "<<localWeb.toStringWithPort()<<endl;
  thread t1(webServerThread, s);
  t1.detach();
}

int main(int argc, char** argv)
{
  ComboAddress localCarbon("::", 2003);
  int s = SSocket(localCarbon.sin4.sin_family,
		 SOCK_STREAM, 0);

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
		 

  YaHTTP::Response resp;

}
