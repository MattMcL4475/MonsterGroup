# Specify the compiler
CXX = g++

# Specify the build flags
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread

# Specify the object files
OBJS = main.o matrix.o utility.o

# Specify the output file name
OUTFILE = monster

# Default target
all: $(OUTFILE)

$(OUTFILE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(OUTFILE) $(OBJS) -lpthread

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

matrix.o: matrix.cpp matrix.h
	$(CXX) $(CXXFLAGS) -c matrix.cpp

utility.o: utility.cpp utility.h
	$(CXX) $(CXXFLAGS) -c utility.cpp

clean:
	rm -f $(OUTFILE) $(OBJS)
