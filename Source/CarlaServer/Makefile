BUILD_FOLDER=build

vsproject:
	cmake -H. -B$(BUILD_FOLDER) -G "Visual Studio 14 2015 Win64"

clean:
	rm -Rf build CMakeFiles

clean-all: clean
	rm -Rf bin lib
