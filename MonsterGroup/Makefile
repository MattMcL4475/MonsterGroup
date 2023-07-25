# Specify the compiler
CXX = g++

# Specify the build flags
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread

# Specify the object files
OBJS = avx512.o main.o matrix.o monster.o utility.o

# Specify the output file name
OUTFILE = MonsterGroup

# Default target
all: $(OUTFILE)

$(OUTFILE): $(OBJS)
    $(CXX) -o $(OUTFILE) $(OBJS)

avx512.o: avx512.cpp avx512.h
    $(CXX) $(CXXFLAGS) -c avx512.cpp

main.o: main.cpp
    $(CXX) $(CXXFLAGS) -c main.cpp

matrix.o: matrix.cpp matrix.h
    $(CXX) $(CXXFLAGS) -c matrix.cpp

monster.o: monster.cpp
    $(CXX) $(CXXFLAGS) -c monster.cpp

utility.o: utility.cpp utility.h
    $(CXX) $(CXXFLAGS) -c utility.cpp

clean:
    rm -f $(OUTFILE) $(OBJS)
