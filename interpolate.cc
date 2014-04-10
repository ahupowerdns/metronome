#include "interpolate.hh"
#include <Eigen/SVD>
#include <fstream>
#include <iostream>
using namespace std;
using namespace Eigen;

namespace {
  // returns (1, x, x*x, x*x*x)
  VectorXd func(double x, int order)
  {
    VectorXd ret(order);
    if(!order)
      return ret;
    
    ret(0)=1;
    for(int i = 1 ; i < order; ++i)
      ret(i)=x*ret(i-1);
    return ret;
  }
  
  // returns (0, 1, 2*x, 3*x*x, 4*x*x*x)
  VectorXd deriv(double x, int order)
  {
    VectorXd ret(order);
    if(!order)
      return ret;
    
    ret(0)=0;
    if(order <2)
      return ret;

    ret(1)=1;
    for(int i = 2 ; i < order; ++i) {
      ret(i)=i*x;
      x *= x;
    }
    return ret;
  }


  void plotSolution(const vector<InterpolateDatum>& input, const VectorXd& res, int order)
  {
    ofstream plot("plot");
    for(double x=-1; x < 1; x+=0.02) {
      VectorXd f = func(x, order);
      plot<<x<<'\t' << res.dot(f) <<'\n';
      
    }
    ofstream r("real");
    for(const auto& i : input) 
      r<<i.x<<'\t'<<i.y<<endl;
  }
  
  vector<InterpolateDatum> normalize(const vector<InterpolateDatum>& input, double* x, double *factor)
  {
    auto ret = input;
    sort(ret.begin(), ret.end());
    double low = ret.begin()->x, high = ret.rbegin()->x;
    if(low == high) {
      for(auto& d : ret) {
	d.x = 0;
      }
      *x=0;
      *factor=0;
      return ret;
    }
    
    for(auto& d : ret) {
      d.x = -1.0 + 2.0*(d.x-low)/(high-low);
    }
    *x = (*x - low )/ (high-low);
    *factor = 2/(high-low);
    return ret;
  }
}

pair<double, double> interpolate(const vector<InterpolateDatum>& input, unsigned int order, double x)
{
  /*

  cerr<<"Input: ";
  for(const auto& f : input) {
    cerr<<"("<<f.x<<", "<<f.y<<") ";
  }
  cerr<<endl;
  */
  double factor;
  auto norm = normalize(input, &x, &factor);

  if(input.size() < order) {
    order = input.size()-1;
  }

  /*
  cerr<<"Normalized: ";
  for(auto f : norm) {
    cerr<<"("<<f.x<<", "<<f.y<<") ";
  }
  cerr<<endl;
  */
  MatrixXd aa(norm.size(), order);
  VectorXd b(norm.size()); 
  for(unsigned int i=0;i < norm.size(); ++i) {
    auto res = func(norm[i].x, order); // evaluate all functions on our x values
    double sig= cos(1.0*norm[i].x);    // inverse weight it appears, optional
    for(unsigned int j=0; j < order; ++j)
      aa(i,j)=res[j]*sig;
    b(i)=norm[i].y*sig;
  }
  //  cout<<"aa:\n"<<aa<<endl;
  VectorXd res = aa.jacobiSvd(ComputeThinU | ComputeThinV).solve(b);
  //cout<<"Got res: "<<res<<endl;
  //   plotSolution(norm, res, order);
  double y = res.dot(func(x, order));
  double dydx = res.dot(deriv(x, order));
  return {y, dydx*factor};
}
