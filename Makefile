#updated by kobe, 2007,4,25
#---------------------------------------------------------------------------------------------
OUTPUTFILES=hooktest libkobehook.so
CXXFLAGS=
LIBS=-L/usr/local/lib/ -ldl
CXX=g++ -g -Wall -O2 -fno-strict-aliasing -fPIC
CC=gcc -g -Wall -O2 -fno-strict-aliasing -fPIC -shared

all: $(OUTPUTFILES)

.SUFFIXES: .o .cpp .hpp .h
#---------------------------------------------------------------------------------------------

hooktest: main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
libkobehook.so: hook.o
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(LIBS)
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<
.c.o:
	$(CC) $(CXXFLAGS) -c $<
#---------------------------------------------------------------------------------------------

clean:
	rm -rf $(OUTPUTFILES) *.o *.so *.a *~
