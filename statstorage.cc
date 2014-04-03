#include "statstorage.hh"
#include <string>
#include "metromisc.hh"
#include <sys/stat.h>
#include <algorithm>
#include <iostream>
using namespace std;

StatStorage::StatStorage(const string& fname) : d_root(fname)
{
}


void StatStorage::store(const string& name, uint32_t timestamp, float value)
{
  if(name.find("/") != string::npos)
    return;

  string fname=d_root+"/"+name;

  FILE* fp=fopen(fname.c_str(), "a");
  if(!fp)
    unixDie("Opening '"+fname+"'");
  StatStorage::Val val({timestamp, value});  
  if(fwrite(&val, 1, sizeof(val), fp) != sizeof(val)) {
    fclose(fp);
    throw runtime_error("Failed to store datum in "+fname+", may be corrupted now");
  }
  fclose(fp);
}

static uint64_t filesize(int fd)
{
  struct stat buf;
  if(!fstat(fd, &buf)) {
    return buf.st_size;
  }
  return 0;
}

vector<StatStorage::Val> StatStorage::retrieveVals(const std::string& name)
{
  vector<StatStorage::Val> values;

  if(name.find("/") != string::npos)
    return values;

  string fname=d_root+"/"+name;
  FILE* fp=fopen(fname.c_str(), "r");
  if(!fp) {
    if(errno!=ENOENT)
      unixDie("Opening '"+fname+"'");
    return values;
  }
  auto size = filesize(fileno(fp));

  values.resize(size/sizeof(StatStorage::Val));
  //  cerr<<"Filesize: "<<size<<", "<<values.size()<<endl;
  if(fread(&values[0], sizeof(StatStorage::Val), values.size(), fp) != values.size()) {
    fclose(fp);
    unixDie("Reading from '"+fname+"'");
  }
  fclose(fp);

  sort(values.begin(), values.end());
  return values;
}

vector<StatStorage::Datum> StatStorage::retrieve(const std::string& name)
{
  auto vals = retrieveVals(name);
  vector<Datum> ret;
  for(const auto& val : vals) {
    ret.push_back({val.timestamp, val.value});
  }
  return ret;
}

vector<StatStorage::Datum> StatStorage::retrieve(const std::string& name, time_t begin, time_t end, int number)
{
  vector<Val> values=retrieveVals(name);
  vector<Datum> ret;
  if(values.empty())
    return ret;
  auto beginIter = lower_bound(values.begin(), values.end(), (int64_t)begin);
  auto endIter = lower_bound(values.begin(), values.end(), (int64_t)end);
  
  ret.reserve(endIter- beginIter);
  for(auto iter = beginIter; iter != endIter; ++iter) {
    ret.push_back({iter->timestamp, iter->value});
  }
  return ret;
}
