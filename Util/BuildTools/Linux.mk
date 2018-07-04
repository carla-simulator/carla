ARGS=--all

default: help

help:
	@less ${CARLA_BUILD_TOOLS_FOLDER}/Linux.mk.help

launch: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --build --launch

launch-only:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --launch

package: CarlaUE4Editor PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/Package.sh

docs:
	@doxygen
	@echo "Documentation index at ./Doxygen/html/index.html"

clean:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --clean

rebuild: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --rebuild
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --rebuild
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --rebuild

hard-clean: clean
	@rm -Rf ${CARLA_BUILD_FOLDER}
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --hard-clean

check: PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh $(ARGS)

benchmark: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --benchmark
	@cat profiler.csv

CarlaUE4Editor: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --build

.PHONY: PythonAPI
PythonAPI: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --py2 --py3

.PHONY: LibCarla
LibCarla: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --server --client

setup:
	@${CARLA_BUILD_TOOLS_FOLDER}/Setup.sh

pretty:
	@${CARLA_BUILD_TOOLS_FOLDER}/Prettify.sh $(ARGS)
