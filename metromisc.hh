#pragma once
#include <string>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <vector>
using std::string;

inline void unixDie(const string &why)
{
  throw std::runtime_error(why+": "+strerror(errno));
}

template <typename Container>
void
stringtok (Container &container, string const &in,
           const char * const delimiters = " \t\n")
{
  const string::size_type len = in.length();
  string::size_type i = 0;
  
  while (i<len) {
    // eat leading whitespace
    i = in.find_first_not_of (delimiters, i);
    if (i == string::npos)
      return;   // nothing left but white space
    
    // find the end of the token
    string::size_type j = in.find_first_of (delimiters, i);
    
    // push token
    if (j == string::npos) {
      container.push_back (in.substr(i));
      return;
    } else
      container.push_back (in.substr(i, j-i));
    
    // set up for next loop
    i = j + 1;
  }
}

template<typename T>
struct CSplineSignalInterpolator
{
  explicit CSplineSignalInterpolator(const std::vector<T>& obs)
    : d_obs(&obs), d_disabled(false)
  {
    if(d_obs->empty()) {
      d_disabled=true;
      return;
    }
    double p, qn, sig, un;

    int n=obs.size();
    std::vector<double> u(n-1);
    d_y2.resize(n);

    d_y2[0]=u[0]=0.0;
    for (int i=1;i<n-1;i++) {
      sig=(obs[i].timestamp - obs[i-1].timestamp)/(obs[i+1].timestamp - obs[i-1].timestamp);
      p=sig*d_y2[i-1]+2.0;
      d_y2[i]=(sig-1.0)/p;
      u[i]=(obs[i+1].value - obs[i].value)/(obs[i+1].timestamp-obs[i].timestamp) - 
	(obs[i].value - obs[i-1].value)/(obs[i].timestamp - obs[i-1].timestamp);
      u[i]=(6.0*u[i]/(obs[i+1].timestamp- obs[i-1].timestamp)-sig*u[i-1])/p;
    }
    
    qn=un=0.0;
    d_y2[n-1]=(un-qn*u[n-2])/(qn*d_y2[n-2]+1.0);
    for (int k=n-2;k>=0;k--)
      d_y2[k]=d_y2[k] * d_y2[k+1]+u[k];
  }

  double operator()(double t) const {
    if(d_disabled)
      return 0;
    auto hi=lower_bound(d_obs->begin(), d_obs->end(), t);
    if(hi == d_obs->begin())
      return d_obs->begin()->value;

    if(hi == d_obs->end())
      return d_obs->rbegin()->value;
    auto lo = hi - 1;
    if(lo == d_obs->begin()) {
      //      cout << t << '\t' << 0 << '\n';
      return d_obs->begin()->value;
    }

    double h=hi->timestamp - lo->timestamp;

    //    if (h == 0.0) nrerror("Bad xa input to routine splint");
    double a=(hi->timestamp - t)/h;
    double b=(t - lo->timestamp)/h;
    double ret= a*lo->value + b*hi->value +((a*a*a-a)* d_y2[lo - d_obs->begin()]
						    +(b*b*b-b)*d_y2[hi - d_obs->begin()])*(h*h)/6.0;
    // cout << t << '\t\ << ret << '\n';
    return ret;
  }

  const std::vector<T>* d_obs;
  std::vector<double> d_y2;
  bool d_disabled;
};
