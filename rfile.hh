#pragma once
#include <stdio.h>

class RFile
{
public:
	RFile(const char* name, const char* mode);

	FILE* d_fp;
	int d_fd;
};