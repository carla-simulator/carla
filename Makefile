INSTALL_FOLDER=$(CURDIR)/Unreal/CarlaUE4/Plugins/Carla/CarlaServer
PYTHON_CLIENT_FOLDER=$(CURDIR)/PythonClient/test
BASE_BUILD_FOLDER=$(CURDIR)/Util/Build/carlaserver-build
MY_CMAKE_FOLDER=$(CURDIR)/Util/cmake
MY_CMAKE_FLAGS=-B"$(BUILD_FOLDER)" -DCMAKE_INSTALL_PREFIX="$(INSTALL_FOLDER)"

define log
	@echo -- LOG [$(1)] '$($1)'
endef

ifeq ($(OS),Windows_NT)
BUILD_RULE=build_windows
CLEAN_RULE=clean_windows
PROTOC_COMPILE=cmd.exe /k "cd Util & call Protoc.bat & exit"
PROTOC_CLEAN=cmd.exe /k "cd Util & call Protoc.bat --clean & exit"
RM=
else
BUILD_RULE=build_linux
PROTOC_COMPILE=./Util/Protoc.sh
PROTOC_CLEAN=./Util/Protoc.sh --clean
endif

default: release

### Build ######################################################################

debug: BUILD_FOLDER=$(BASE_BUILD_FOLDER)/debug
debug: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Debug
debug: $(BUILD_RULE)

release: BUILD_FOLDER=$(BASE_BUILD_FOLDER)/release
release: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Release
release: $(BUILD_RULE)

vsproject: BUILD_FOLDER=$(BASE_BUILD_FOLDER)/visualstudio
vsproject: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Debug
vsproject: MY_CMAKE_FLAGS+=-G "Visual Studio 14 2015 Win64"
vsproject: call_cmake

build_linux: MY_CMAKE_FLAGS+=-G "Ninja"
build_linux: call_cmake
	@cd $(BUILD_FOLDER) && ninja && ninja install

build_windows: MY_CMAKE_FLAGS+=-G "NMake Makefiles"
build_windows: call_cmake
	@cd $(BUILD_FOLDER) && nmake && nmake install

call_cmake: protobuf
ifeq ($(OS),Windows_NT)
	-@mkdir "$(BUILD_FOLDER)"
else
	@mkdir -p $(BUILD_FOLDER)
endif
	@cd $(BUILD_FOLDER) && cmake $(MY_CMAKE_FLAGS) "$(MY_CMAKE_FOLDER)"

protobuf:
	@$(PROTOC_COMPILE)

### Docs #######################################################################

docs: doxygen

doxygen:
	@doxygen
	@echo "Documentation index at ./Doxygen/html/index.html"

### Clean ######################################################################

clean:
ifeq ($(OS),Windows_NT)
	@rd /s /q "$(BASE_BUILD_FOLDER)" "$(INSTALL_FOLDER)"
else
	@rm -Rf $(BASE_BUILD_FOLDER) $(INSTALL_FOLDER) Doxygen
endif
	@$(PROTOC_CLEAN)

### Test #######################################################################

check: debug launch_test_clients run_test_debug kill_test_clients

check_release: release launch_test_clients run_test_release kill_test_clients

run_test_debug:
	@-LD_LIBRARY_PATH=$(INSTALL_FOLDER)/shared $(INSTALL_FOLDER)/bin/test_carlaserverd --gtest_shuffle $(GTEST_ARGS)

run_test_release:
	@-LD_LIBRARY_PATH=$(INSTALL_FOLDER)/shared $(INSTALL_FOLDER)/bin/test_carlaserver --gtest_shuffle $(GTEST_ARGS)

launch_test_clients:
	@echo "Launch echo client"
	@python3 $(PYTHON_CLIENT_FOLDER)/test_client.py --echo -p 4000 --log echo_client.log & echo $$! > echo_client.pid
	@echo "Launch carla client"
	@python3 $(PYTHON_CLIENT_FOLDER)/test_client.py -p 2000 --log carla_client.log & echo $$! > carla_client.pid

kill_test_clients:
	@echo "Kill echo client"
	@kill `cat echo_client.pid` && rm echo_client.pid
	@echo "Kill carla client"
	@kill `cat carla_client.pid` && rm carla_client.pid