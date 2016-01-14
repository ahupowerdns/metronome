#pragma once
#include <string>
#include <vector>
#include <limits>
#include <tuple>
#include <ctime>
#include <regex.h>

//! make your own instance, not thread safe
class StatStorage
{
public:
  StatStorage(const std::string& root);
  ~StatStorage();
  void store(const std::string& name, uint32_t timestamp, float value);

  struct Datum
  {
    uint32_t timestamp;
    float value;
    bool operator<(double t) const
    {
      return timestamp < t;
    }
    bool operator==(const Datum &rhs) const
    {
      return std::tie(rhs.timestamp, rhs.value) == std::tie(timestamp, value);
    }
  };
  void store(const std::string& name, const std::vector<Datum>& data);
  std::vector<Datum> retrieve(const std::string& name, time_t begin, time_t end, int number=-1);
  std::vector<Datum> retrieve(const std::string& name);
  std::vector<std::string> getMetrics();
private:
  std::string d_root;
  regex_t d_preg;
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

  unsigned int getWeekNum(uint32_t t);
  std::string makeFilename(const std::string& name, uint32_t timestamp);
  std::vector<Val> retrieveVals(const std::string& name, uint32_t begin, uint32_t end);
  std::vector<Val> retrieveVals(const std::string& name);
  void retrieveAllFromFile(const std::string& fname, std::vector<StatStorage::Val>* values);
};

inline bool operator<(double t, const StatStorage::Datum& d)
{
  return t < d.timestamp;
}
