#include "../yahttp/yahttp.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ext/stdio_filebuf.h>
#include <string.h>

/** Helper class for keeping single request/response pair */
class ServerRequestResponse {
public:
  YaHTTP::Request req;  
  YaHTTP::Response resp;
  YaHTTP::AsyncRequestLoader arl;

  int state;
  int id;
  time_t last;

  ServerRequestResponse() {
    state = 0;
    id = -1;
  };

  void initialize() {
    req.initialize();
    resp.initialize();
  }
};

/** Really basic simple server */
class Server {
public:
  /** Connection pool */
  ServerRequestResponse pool[100];
  int port;
  int lfd;

  Server(int port) {
    this->port = port;
  }

  void bind() {
    struct sockaddr_in sa;
    int val = 1;
    lfd = ::socket(AF_INET, SOCK_STREAM, 6); // tcp 
    // some modern options
    ::setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    ::setsockopt(lfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = 0;
    sa.sin_port = htons(port);
    ::bind(lfd, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa));
    ::listen(lfd, 100);
  }

  void accept() {
    struct sockaddr_in sa;
    socklen_t salen;
    int fd;
    int val=1;
    char ipaddr[INET_ADDRSTRLEN];
    salen = sizeof(sa);
    memset(&sa,0,sizeof(sa));
    fd = ::accept(lfd, reinterpret_cast<struct sockaddr*>(&sa), &salen);
    if (fd < 0) return; // false alarm
    inet_ntop(AF_INET, &sa.sin_addr, ipaddr, sizeof ipaddr);
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
    val = 0;
    ::setsockopt(fd, SOL_SOCKET, SO_LINGER, &val, sizeof(val));
    pool[fd].initialize();
    pool[fd].id = fd;
    std::cout << "Connection from " << ipaddr << ":" << ntohs(sa.sin_port) << " accepted on fd=" << fd << std::endl;
  }

  // handle a request
  void handle(ServerRequestResponse &rr) {
    try {
      if (rr.state == 0) {
        rr.arl.initialize(&rr.req);
        rr.state = 1;
      }
  
      if (rr.state == 1) {
        char buf[4096];
        size_t r;
        r = ::read(rr.id, buf, sizeof(buf));

        if (r>0) {
          rr.arl.feed(std::string(buf,r)); 
        }

        if (rr.arl.ready()) {
          rr.arl.finalize();
          rr.state = 2; 
        } else {
          if (r == 0) {
            // EOF
            ::close(rr.id);
            rr.state = 0;
            rr.id = -1;
          }
          return;
        }
      }
    } catch (YaHTTP::Error &err) {
      ::close(rr.id);
      rr.state = 0;
      rr.id = -1;
      std::cout << "YaHTTP::Error: " << err.what() << std::endl;
      return;
    }

    rr.resp.headers["content-type"] = "text/html; charset=utf-8";
    rr.resp.headers["connection"] = "close";

    if (rr.req.url.path == "/") {
      rr.resp.url = rr.req.url;
      rr.resp.status = 200; 
      rr.resp.body = "<!DOCTYPE html>\n<html lang=\"en\"><head><title>Hello, world</title><link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\" /></head><body><h1>200 OK</h1><p>Hello, world</p></body></html>";
    } else if (rr.req.url.path == "/style.css") {
      rr.resp.url = rr.req.url;
      rr.resp.status = 200;
      rr.resp.headers["content-type"] = "text/css; charset=utf-8";
      rr.resp.renderer = YaHTTP::HTTPBase::SendFileRender("style.css");
    } else if (rr.req.url.path == "/bg.jpg") {
      rr.resp.url = rr.req.url;
      rr.resp.status = 200;
      rr.resp.headers["content-type"] = "image/jpeg";
//      rr.resp.headers["content-length"] = "810816";
      rr.resp.renderer = YaHTTP::HTTPBase::SendFileRender("bg.jpg");
    } else {
      rr.resp.url = rr.req.url;
      rr.resp.status = 404;
      rr.resp.body = "<!DOCTYPE html>\n<html lang=\"en\"><head><title>404 Not Found</title><link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\" /></head><body><h1>404 Not Found</h1><p>Requested URL not found</p></body></html>";
    }

    std::cout << "Sending " << rr.resp.status << " for " << rr.resp.url.path << std::endl;

    std::ostringstream tmp;
    tmp << rr.resp;
    ::write(rr.id, tmp.str().c_str(), tmp.str().size());
    ::close(rr.id);
    rr.id = -1;
    rr.state = 0;
  }

  void run() {
    int idx;
    struct timeval tv;
    int maxfd;
    fd_set rfd;
 
    bind();

    std::cout << "Listening on 0.0.0.0:" << port << std::endl;

    while(true) {
      FD_ZERO(&rfd);
      maxfd = lfd;
      FD_SET(lfd, &rfd); 
      for(idx = lfd+1; idx < 100; idx++) {
        if (pool[idx].id>-1) { FD_SET( idx, &rfd ); maxfd = idx; }
      }
      tv.tv_sec = 0;
      tv.tv_usec = 100;
      if (select(maxfd+1, &rfd, NULL, NULL, &tv)>0) {
         // we have a winner
         if (FD_ISSET( lfd, &rfd )) {
           accept();
         }
         for(idx = lfd+1; idx < 100; idx++) {
           if (FD_ISSET( idx, &rfd )) {
             handle(pool[idx]);
           }
         }
      }
    }
  }
};

int main(void) {
  Server(2828).run();
}
