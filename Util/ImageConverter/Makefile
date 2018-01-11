CXX=clang++
FLAGS=-Wall -Wextra -std=c++14 -fopenmp
LIBS=-lboost_system -lboost_filesystem -lboost_program_options -lpng -ljpeg -ltiff
HEADERS=*.h
SOURCES=main.cpp
EXE=image_converter

build: release

release: $(SOURCES) $(HEADERS)
	@mkdir -p bin
	$(CXX) $(FLAGS) -O3 -DNDEBUG -o bin/$(EXE) $(SOURCES) $(LIBS)

debug: $(SOURCES) $(HEADERS)
	@mkdir -p bin
	$(CXX) $(FLAGS) -O0 -g -D_DEBUG -o bin/$(EXE)_debug $(SOURCES) $(LIBS)

clean:
	rm -f $(EXE) $(EXE)_debug
