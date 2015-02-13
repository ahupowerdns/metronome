CXX2011FLAGS=-std=c++11 -stdlib=libc++ -I/usr/local/include/ -I/usr/local/include/eigen3
LIBRARIES=-L/usr/local/lib -lboost_program_options -lm
STATICFLAGS=$(LIBRARIES) -lc++
CXX=clang++
