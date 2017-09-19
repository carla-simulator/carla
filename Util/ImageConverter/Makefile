CXX=g++
FLAGS=-Wall -Wextra -std=c++14 -fopenmp
LIBS=-lboost_system -lboost_filesystem -lboost_program_options -lpng -ljpeg -ltiff
HEADERS=*.h
SOURCES=main.cpp
EXE=image_converter

build: release

release: $(SOURCES) $(HEADERS)
	$(CXX) $(FLAGS) -O3 -DNDEBUG -o $(EXE) $(SOURCES) $(LIBS)

debug: $(SOURCES) $(HEADERS)
	$(CXX) $(FLAGS) -O0 -g -D_DEBUG -o $(EXE)_debug $(SOURCES) $(LIBS)

clean:
	rm -f $(EXE) $(EXE)_debug
