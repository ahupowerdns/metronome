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

struct Val { 
  int64_t timestamp;
  double value;
  bool operator<(const Val& rhs) const
  {
    return timestamp < rhs.timestamp;
  }
  bool operator<(int64_t rhs) const
  {
    return timestamp < rhs;
  }
} __attribute__((packed));

void StatStorage::store(const string& name, time_t timestamp, double value)
{
  if(name.find("/") != string::npos)
    return;

  string fname=d_root+"/"+name;

  FILE* fp=fopen(fname.c_str(), "a");
  if(!fp)
    unixDie("Opening '"+fname+"'");
  Val val({(int64_t)timestamp, value});  
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


vector<StatStorage::Datum> StatStorage::retrieve(const std::string& name, time_t begin, time_t end, int number)
{

  vector<Datum> ret;
  if(name.find("/") != string::npos)
    return ret;

  string fname=d_root+"/"+name;
  FILE* fp=fopen(fname.c_str(), "r");
  if(!fp) {
    if(errno!=ENOENT)
      unixDie("Opening '"+fname+"'");
    return ret;
  }
  auto size = filesize(fileno(fp));

  vector<Val> values(size/sizeof(Val));
  //  cerr<<"Filesize: "<<size<<", "<<values.size()<<endl;
  if(fread(&values[0], sizeof(Val), values.size(), fp) != values.size()) {
    fclose(fp);
    unixDie("Reading from '"+fname+"'");
  }
  fclose(fp);

  sort(values.begin(), values.end());
 
  auto beginIter = lower_bound(values.begin(), values.end(), (int64_t)begin);
  auto endIter = lower_bound(values.begin(), values.end(), (int64_t)end);
  
  ret.reserve(endIter- beginIter);
  for(auto iter = beginIter; iter != endIter; ++iter) {
    ret.push_back({(time_t)iter->timestamp, iter->value});
  }
  return ret;
}
