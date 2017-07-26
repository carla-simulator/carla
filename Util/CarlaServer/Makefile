PROTOBUF_SRC_DIR=source/carla
PROTOBUF_CPP_OUT_DIR=source/carla/server
PROTOBUF_PY_OUT_DIR=source/carla/client

PROTOBUF_FILES= \
		$(PROTOBUF_CPP_OUT_DIR)/carla_protocol.pb.h \
		$(PROTOBUF_CPP_OUT_DIR)/carla_protocol.pb.cc \
		$(PROTOBUF_PY_OUT_DIR)/carla_protocol.pb2.py

MY_CMAKE_FLAGS=-H. -B$(BUILD_FOLDER)

ifeq ($(OS),Windows_NT)
DEFAULT_RULE=release
BUILD_RULE=build_windows
else
DEFAULT_RULE=debug
BUILD_RULE=build_linux
endif

default: $(DEFAULT_RULE)

### Build ######################################################################

debug: BUILD_FOLDER=build/debug
debug: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Debug
debug: $(BUILD_RULE)

release: BUILD_FOLDER=build/release
release: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Release
release: $(BUILD_RULE)

build_linux: protobuf
	cmake $(MY_CMAKE_FLAGS) -G "Ninja"
	cd $(BUILD_FOLDER) && ninja

build_windows: protobuf
	cmake $(MY_CMAKE_FLAGS) -G "NMake Makefiles"
	cd $(BUILD_FOLDER) && nmake

vsproject: BUILD_FOLDER=build/visualstudio
vsproject: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Debug
vsproject: protobuf
	cmake $(MY_CMAKE_FLAGS) -G "Visual Studio 14 2015 Win64"

protobuf: $(PROTOBUF_FILES)

$(PROTOBUF_FILES): $(PROTOBUF_SRC_DIR)/carla_protocol.proto
	protoc -I=$(PROTOBUF_SRC_DIR) --cpp_out=$(PROTOBUF_CPP_OUT_DIR) --python_out=$(PROTOBUF_PY_OUT_DIR) $<

### Clean ######################################################################

clean:
	rm -f $(PROTOBUF_FILES)
	rm -Rf build CMakeFiles bin bin_debug lib

### Test #######################################################################

check: debug launch_test_clients run_test_debug kill_test_clients

check_release: release launch_test_clients run_test_release kill_test_clients

run_test_debug:
	@-./bin_debug/test_carla_server --gtest_shuffle

run_test_release:
	@-./bin/test_carla_server --gtest_shuffle

launch_test_clients:
	@echo "Launch echo_client.py"
	@python source/carla/client/echo_client.py -p 4000 & echo $$! > echo_client.pid
	@echo "Launch carla_client.py"
	@python source/carla/client/carla_client.py -p 2000 & echo $$! > carla_client.pid

kill_test_clients:
	@echo "Kill echo_client.py"
	@kill `cat echo_client.pid` && rm echo_client.pid
	@echo "Kill carla_client.py"
	@kill `cat carla_client.pid` && rm carla_client.pid
