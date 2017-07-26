PROTOBUF_SRC_DIR=source/carla/server

MY_CMAKE_FLAGS=-H. -B$(BUILD_FOLDER)

ifeq ($(OS),Windows_NT)
DEFAULT_RULE=release
BUILD_RULE=build_windows
else
DEFAULT_RULE=debug
BUILD_RULE=build_linux
endif

default: $(DEFAULT_RULE)

debug: BUILD_FOLDER=build/debug
debug: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Debug
debug: $(BUILD_RULE)

debug_png: BUILD_FOLDER=build/debug_png
debug_png: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=DebugPNG
debug_png: $(BUILD_RULE)

release: BUILD_FOLDER=build/release
release: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Release
release: $(BUILD_RULE)

release_png: BUILD_FOLDER=build/release_png
release_png: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=ReleasePNG
release_png: $(BUILD_RULE)

build_linux: protobuf
	cmake $(MY_CMAKE_FLAGS) -G "Ninja"
	cd $(BUILD_FOLDER) && ninja && ninja install

build_windows: protobuf
	cmake $(MY_CMAKE_FLAGS) -G "NMake Makefiles"
	cd $(BUILD_FOLDER) && nmake && nmake install

vsproject: BUILD_FOLDER=build/visualstudio
vsproject: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Debug
vsproject: protobuf
	cmake $(MY_CMAKE_FLAGS) -G "Visual Studio 14 2015 Win64"

protobuf: $(PROTOBUF_SRC_DIR)/carla_protocol.pb.cc

$(PROTOBUF_SRC_DIR)/%.pb.cc: $(PROTOBUF_SRC_DIR)/%.proto
	protoc -I=$(PROTOBUF_SRC_DIR) --cpp_out=$(PROTOBUF_SRC_DIR) $<

clean:
	rm -f $(PROTOBUF_SRC_DIR)/carla_protocol.pb.h
	rm -f $(PROTOBUF_SRC_DIR)/carla_protocol.pb.cc
	rm -Rf build CMakeFiles

clean-all: clean
	rm -Rf bin bin_debug lib include
