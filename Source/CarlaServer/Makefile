BUILD_FOLDER=build

vsproject:
	cmake -H. -B$(BUILD_FOLDER) -G "Visual Studio 14 2015 Win64"

stproject:
	cmake -H. -B$(BUILD_FOLDER) -G "Sublime Text 2 - Ninja"

clean:
	rm -Rf build CMakeFiles

clean-all: clean
	rm -Rf bin lib
