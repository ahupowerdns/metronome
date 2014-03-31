#include "iputils.hh"
/** these functions provide a very lightweight wrapper to the Berkeley sockets API. Errors -> exceptions! */

using namespace std;

static void RuntimeError(const boost::format& fmt)
{
  throw runtime_error(fmt.str());
}


int SSocket(int family, int type, int flags)
{
  int ret = socket(family, type, flags);
  if(ret < 0)
    RuntimeError(boost::format("creating socket of type %d: %s") % family % strerror(errno));
  return ret;
}

int SConnect(int sockfd, const ComboAddress& remote)
{
  int ret = connect(sockfd, (struct sockaddr*)&remote, remote.getSocklen());
  if(ret < 0)
    RuntimeError(boost::format("connecting socket to %s: %s") % remote.toStringWithPort() % strerror(errno));
  return ret;
}

int SBind(int sockfd, const ComboAddress& local)
{
  int ret = bind(sockfd, (struct sockaddr*)&local, local.getSocklen());
  if(ret < 0)
    RuntimeError(boost::format("binding socket to %s: %s") % local.toStringWithPort() % strerror(errno));
  return ret;
}

int SAccept(int sockfd, ComboAddress& remote)
{
  socklen_t remlen = remote.getSocklen();

  int ret = accept(sockfd, (struct sockaddr*)&remote, &remlen);
  if(ret < 0)
    RuntimeError(boost::format("accepting new connection on socket: %s") % strerror(errno));
  return ret;
}

int SListen(int sockfd, int limit)
{
  int ret = listen(sockfd, limit);
  if(ret < 0)
    RuntimeError(boost::format("setting socket to listen: %s") % strerror(errno));
  return ret;
}

int SSetsockopt(int sockfd, int level, int opname, int value)
{
  int ret = setsockopt(sockfd, level, opname, &value, sizeof(value));
  if(ret < 0)
    RuntimeError(boost::format("setsockopt for level %d and opname %d to %d failed: %s") % level % opname % value % strerror(errno));
  return ret;
}

int writen(int fd, const void *buf, size_t count)
{
  const char *ptr = (char*)buf;
  const char *eptr = ptr + count;
  
  int res;
  while(ptr != eptr) {
    res = ::write(fd, ptr, eptr - ptr);
    if(res < 0) {
      if (errno == EAGAIN)
        throw std::runtime_error("used writen2 on non-blocking socket, got EAGAIN");
      else
        unixDie("failed in writen2");
    }
    else if (res == 0)
      throw std::runtime_error("could not write all bytes, got eof in writen2");
    
    ptr += res;
  }
  
  return count;
}


int makeIPv6sockaddr(const std::string& addr, struct sockaddr_in6* ret)
{
  if(addr.empty())
    return -1;
  string ourAddr(addr);
  int port = -1;
  if(addr[0]=='[') { // [::]:53 style address
    string::size_type pos = addr.find(']');
    if(pos == string::npos || pos + 2 > addr.size() || addr[pos+1]!=':')
      return -1;
    ourAddr.assign(addr.c_str() + 1, pos-1);
    port = atoi(addr.c_str()+pos+2);  
  }
  
  struct addrinfo* res;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  
  hints.ai_family = AF_INET6;
  hints.ai_flags = AI_NUMERICHOST;
  
  int error;
  if((error=getaddrinfo(ourAddr.c_str(), 0, &hints, &res))) { // this is correct
    /*
    cerr<<"Error translating IPv6 address '"<<addr<<"': ";
    if(error==EAI_SYSTEM)
      cerr<<strerror(errno)<<endl;
    else
      cerr<<gai_strerror(error)<<endl;
    */
    return -1;
  }
  
  memcpy(ret, res->ai_addr, res->ai_addrlen);
  if(port >= 0)
    ret->sin6_port = htons(port);
  freeaddrinfo(res);
  return 0;
}

int makeIPv4sockaddr(const std::string& str, struct sockaddr_in* ret)
{
  if(str.empty()) {
    return -1;
  }
  struct in_addr inp;
  
  string::size_type pos = str.find(':');
  if(pos == string::npos) { // no port specified, not touching the port
    if(inet_aton(str.c_str(), &inp)) {
      ret->sin_addr.s_addr=inp.s_addr;
      return 0;
    }
    return -1;
  }
  if(!*(str.c_str() + pos + 1)) // trailing :
    return -1; 
    
  char *eptr = (char*)str.c_str() + str.size();
  int port = strtol(str.c_str() + pos + 1, &eptr, 10);
  if(*eptr)
    return -1;
  
  ret->sin_port = htons(port);
  if(inet_aton(str.substr(0, pos).c_str(), &inp)) {
    ret->sin_addr.s_addr=inp.s_addr;
    return 0;
  }
  return -1;
}
