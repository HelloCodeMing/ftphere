CPP = clang++
DEBUGCPP = g++
CPPFLAGS = -std=c++14 -pthread -lboost_system -lboost_filesystem

TARGET = ftphere

debug: ftphere.cc
	$(DEBUGCPP) -o $(TARGET) -g $(CPPFLAGS) $(TARGET).cc

all: ftphere.cc
	$(CPP) -o $(TARGET) $(CPPFLAGS) $(TARGET).cc

test: test.cc
	$(CPP) -g $(CPPFLAGS) -o test test.cc 
	./test

clean:
	-rm ./test
	-rm ftphere
	-rm *.log
