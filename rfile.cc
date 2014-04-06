#include "rfile.hh"
#include <stdexcept>
#include <string>
#include <errno.h>
using namespace std;

RFile::RFile(const char* name, const char* mode)
{
	d_fp = fopen(name, mode);
	if(!d_fp)
		throw runtime_error("Error openinging file '"+string(name)+"': "+strerror(errno));
	d_fd = fileno(d_fp);
}