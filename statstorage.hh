#pragma once
#include <string>
#include <vector>

//! make your own, not thread safe
class StatStorage
{
public:
  StatStorage(const std::string& root);
  void store(const std::string& name, uint32_t timestamp, float value);
  
  struct Datum
  {
    uint32_t timestamp;
    float value;
    bool operator<(double t) const
    {
      return timestamp < t;
    }
  };
  
  std::vector<Datum> retrieve(const std::string& name, time_t begin, time_t end, int number=-1);

private:
  std::string d_root;
};
