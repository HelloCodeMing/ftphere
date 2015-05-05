CPP = clang++
CPPFLAGS = -std=c++11 -pthread -lboost_system
TARGET = ftphere

all: ftphere.cc
	$(CPP) -o $(TARGET) $(CPPFLAGS) $(TARGET).cc

test: 
	$(CPP) $(CPPFLAGS) -o test test.cc 
	./test

clean:
	-rm ./test
	-rm ftphere
	-rm *.log
