ARGS=--all

default: help

# root of the project (makefile directory)
export ROOT_PATH=$(CURDIR)/

# dependecy install/build directory (rpclib, gtest, boost)
export INSTALLATION_DIR=$(ROOT_PATH)Build/

help:
	@type "${CARLA_BUILD_TOOLS_FOLDER}\Linux.mk.help"

launch: LibCarla
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --build --ue-version 4.19

launch-editor: LibCarla
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --launch-editor

package: PythonAPI
	@"${CARLA_BUILD_TOOLS_FOLDER}/Package.bat" --ue-version 4.19

docs:
	@doxygen
	@echo "Documentation index at ./Doxygen/html/index.html"

clean:
	@"${CARLA_BUILD_TOOLS_FOLDER}/Package.bat" --clean --ue-version 4.19
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --clean

	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --clean
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --clean

rebuild: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --rebuild
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --rebuild
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --rebuild --ue-version 4.19

check: PythonAPI
	@echo "Not implemented!"

benchmark: LibCarla
	@echo "Not implemented!"

CarlaUE4Editor: LibCarla
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.bat" --build-editor --ue-version 4.19

.PHONY: PythonAPI
PythonAPI: LibCarla
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.bat" --py3

.PHONY: LibCarla
LibCarla: setup
	@"${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.bat" --server --client

setup:
	@"${CARLA_BUILD_TOOLS_FOLDER}/Setup.bat" --boost-toolset msvc-14.1
