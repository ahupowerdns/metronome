#pragma once
#include <vector>
#include <utility>
struct InterpolateDatum
{
  double x;
  double y;
  bool operator<(const InterpolateDatum& rhs) const
  {
    return x < rhs.x;
  }
};

std::pair<double, double> interpolate(const std::vector<InterpolateDatum>& input, int order, double x);
