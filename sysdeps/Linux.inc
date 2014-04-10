CXX2011FLAGS=-std=gnu++0x
LIBRARIES=-lboost_program_options
STATICFLAGS ?=-Wl,-Bstatic $(LIBRARIES) -lstdc++ -lgcc -Wl,-Bdynamic -static-libgcc -lm -lc

