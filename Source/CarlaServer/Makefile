BUILD_FOLDER=build

vsproject:
	cmake -H. -B$(BUILD_FOLDER) -G "Visual Studio 14 2015 Win64"

stproject:
	cmake -H. -B$(BUILD_FOLDER) -G "Sublime Text 2 - Ninja"
	
	cd protobuf-2.6.1
	./autogen.sh
	./configure --prefix= ${PROJECT_SOURCE_DIR}/lib
	make
	make check
	sudo make install
	sudo ldconfig # refresh shared library cache

clean:
	rm -Rf build CMakeFiles

clean-all: clean
	rm -Rf bin lib
