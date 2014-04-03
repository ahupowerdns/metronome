#pragma once
#include <string>
#include <vector>
#include <limits>

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
  std::vector<Datum> retrieve(const std::string& name);

private:
  std::string d_root;
  struct Val { 
    uint32_t timestamp;
    float value;
    bool operator<(const Val& rhs) const
    {
      return timestamp < rhs.timestamp;
    }
    bool operator<(int64_t rhs) const
    {
      return timestamp < rhs;
    }
  } __attribute__((packed));

  std::vector<Val> retrieveVals(const std::string& name);
};

inline bool operator<(double t, const StatStorage::Datum& d)
{
  return t < d.timestamp;
}
