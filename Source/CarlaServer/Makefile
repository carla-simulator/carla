BUILD_FOLDER=build
PROTOBUF_SRC_DIR=source/carla/server

default: build
	cd build && ninja && ninja install

build: protobuf
	cmake -H. -B$(BUILD_FOLDER) -G "Ninja"

vsproject: protobuf
	cmake -H. -B$(BUILD_FOLDER) -G "Visual Studio 14 2015 Win64"

protobuf: $(PROTOBUF_SRC_DIR)/carla_protocol.pb.cc

$(PROTOBUF_SRC_DIR)/%.pb.cc: $(PROTOBUF_SRC_DIR)/%.proto
	protoc -I=$(PROTOBUF_SRC_DIR) --cpp_out=$(PROTOBUF_SRC_DIR) $<

clean:
	rm -Rf build CMakeFiles

clean-all: clean
	rm -Rf bin lib include
