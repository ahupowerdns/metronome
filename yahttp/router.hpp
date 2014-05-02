#ifndef _YAHTTP_ROUTER_HPP
#define _YAHTTP_ROUTER_HPP 1

#ifdef HAVE_CXX11
#include <functional>
#include <tuple>
#define HAVE_CPP_FUNC_PTR
#define IGNORE std::ignore
namespace funcptr = std;
#else
#ifdef HAVE_BOOST
#include <boost/function.hpp>
#include <boost/tuple/tuple.hpp>
#define IGNORE boost::tuples::ignore
namespace funcptr = boost;
#define HAVE_CPP_FUNC_PTR
#else
#warn "You need to configure with boost or have C++11 capable compiler for router"
#endif
#endif

#ifdef HAVE_CPP_FUNC_PTR
#include <vector>
#include <utility>

namespace YaHTTP {
  typedef funcptr::function <bool(Request* req, Response* resp)> THandlerFunction;
  typedef funcptr::tuple<std::string, std::string, THandlerFunction, std::string> TRoute;
  typedef std::vector<TRoute> TRouteList;

  class Router {
  private:
    Router() {};
    static Router router;
  public:
    ~Router() {};
    void map(const std::string& method, const std::string& url, THandlerFunction hander, const std::string& name);
    void route(Request *req, Response *resp);
    void printRoutes(std::ostream &os);
    std::pair<std::string, std::string> urlFor(const std::string &name, const strstr_map_t& arguments);

    static void Map(const std::string& method, const std::string& url, THandlerFunction handler, const std::string& name = "") { router.map(method, url, handler, name); };
    static void Get(const std::string& url, THandlerFunction handler, const std::string& name = "") { router.map("GET", url, handler, name); };
    static void Post(const std::string& url, THandlerFunction handler, const std::string& name = "") { router.map("POST", url, handler, name); };
    static void Put(const std::string& url, THandlerFunction handler, const std::string& name = "") { router.map("PUT", url, handler, name); };
    static void Delete(const std::string& url, THandlerFunction handler, const std::string& name = "") { router.map("DELETE", url, handler, name); };

    static void Route(Request *req, Response *resp) { router.route(req, resp); };
    static void PrintRoutes(std::ostream &os) { router.printRoutes(os); };

    static std::pair<std::string, std::string> URLFor(const std::string &name, const strstr_map_t& arguments) { return router.urlFor(name,arguments); }; 

    TRouteList routes;
  };
};
#endif

#endif
