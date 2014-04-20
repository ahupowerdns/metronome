#if __cplusplus > 199711L
#include <functional>
#define HAVE_CPP_FUNC_PTR
namespace funcptr = std;
#else
#ifdef HAVE_BOOST
#include <boost/function.hpp>
namespace funcptr = boost;
#define HAVE_CPP_FUNC_PTR
#endif
#endif

#include <fstream>

#ifndef YAHTTP_MAX_REQUEST_SIZE
#define YAHTTP_MAX_REQUEST_SIZE 2097152
#endif

#ifndef YAHTTP_MAX_RESPONSE_SIZE
#define YAHTTP_MAX_RESPONSE_SIZE 2097152
#endif

#define YAHTTP_TYPE_REQUEST 1
#define YAHTTP_TYPE_RESPONSE 2

namespace YaHTTP {
  typedef std::map<std::string,std::string> strstr_map_t;
  typedef std::map<std::string,Cookie> strcookie_map_t;

  class HTTPDocument {
  public:
#ifdef HAVE_CPP_FUNC_PTR
    class SendBodyRender {
    public:
      SendBodyRender() {};

      size_t operator()(const HTTPDocument *doc, std::ostream& os) const {
        os << doc->body;
        return doc->body.length();
      };
    };
    class SendFileRender {
    public:
      SendFileRender(const std::string& path) {
        this->path = path;
      };
  
      size_t operator()(const HTTPDocument *doc, std::ostream& os) const {
        char buf[4096];
        size_t n,k;

        std::ifstream ifs(path, std::ifstream::binary);
        n = 0;
        while(ifs && ifs.good()) {
          ifs.read(buf, sizeof buf);
          n += (k = ifs.gcount());
          if (k)
            os.write(buf, k);
        }

        return n;
      };

      std::string path;
    };
#endif
    HTTPDocument() {
#ifdef HAVE_CPP_FUNC_PTR
      renderer = SendBodyRender();
#endif
    };
    HTTPDocument(const HTTPDocument& rhs) {
      this->url = rhs.url; this->kind = rhs.kind;
      this->status = rhs.status; this->statusText = rhs.statusText;
      this->method = rhs.method; this->headers = rhs.headers;
      this->jar = rhs.jar; this->parameters = rhs.parameters;
      this->body = rhs.body;
      this->renderer = rhs.renderer;
    };
    URL url;
    int kind;
    int status;
    std::string statusText;
    std::string method;
    strstr_map_t headers;
    CookieJar jar;
    strstr_map_t parameters;
    std::string body;
     
#ifdef HAVE_CPP_FUNC_PTR
    funcptr::function<size_t(const HTTPDocument*,std::ostream&)> renderer;
#endif
    void write(std::ostream& os) const;
  };

  class Response: public HTTPDocument { 
  public:
    Response() { this->kind = YAHTTP_TYPE_RESPONSE; };
    Response(const HTTPDocument& rhs): HTTPDocument(rhs) {
      this->kind = YAHTTP_TYPE_RESPONSE;
    };
    friend std::ostream& operator<<(std::ostream& os, const Response &resp);
    friend std::istream& operator>>(std::istream& is, Response &resp);
  };

  class Request: public HTTPDocument {
  public:
    Request() { this->kind = YAHTTP_TYPE_REQUEST; };
    Request(const HTTPDocument& rhs): HTTPDocument(rhs) {
      this->kind = YAHTTP_TYPE_REQUEST;
    };
    friend std::ostream& operator<<(std::ostream& os, const Request &resp);
    friend std::istream& operator>>(std::istream& is, Request &resp);
  };

  template <class T>
  class AsyncLoader {
  public:
    T* target;
    int state;
    size_t pos;
    
    std::string buffer;
    bool chunked;
    int chunk_size;
    std::ostringstream bodybuf;
    long maxbody;
    void keyValuePair(const std::string &keyvalue, std::string &key, std::string &value);

    void initialize(T* target) {
      chunked = false; chunk_size = 0;
      bodybuf.str(""); maxbody = 0;
      pos = 0; state = 0; this->target = target; 
    };
    int feed(const std::string& somedata);
    bool ready() { return state > 1 && (maxbody < 0 || static_cast<unsigned long>(maxbody) >= bodybuf.str().size()); };
    void finalize() {
      bodybuf.flush();
      target->body = bodybuf.str();
      bodybuf.str("");
      this->target = NULL;
    };
  };

  class AsyncResponseLoader: public AsyncLoader<Response> {
  };

  class AsyncRequestLoader: public AsyncLoader<Request> {
  };

};
