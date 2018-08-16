ARGS=--all

default: help

help:
	@less ${CARLA_BUILD_TOOLS_FOLDER}/Linux.mk.help

launch: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.command --build --launch

launch-only:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.command --launch

package: CarlaUE4Editor PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/Package.command

docs:
	@doxygen
	@echo "Documentation index at ./Doxygen/html/index.html"

clean:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.command --clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.command --clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.command --clean

rebuild: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.command --rebuild
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.command --rebuild
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.command --rebuild

hard-clean: clean
	@rm -Rf ${CARLA_BUILD_FOLDER}
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.command --hard-clean

check: PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.command $(ARGS)

benchmark: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.command --benchmark
	@cat profiler.csv

CarlaUE4Editor: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.command --build

.PHONY: PythonAPI
PythonAPI: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.command --py2

.PHONY: LibCarla
LibCarla: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.command --server --client

setup:
	@${CARLA_BUILD_TOOLS_FOLDER}/Setup.command

pretty:
	@${CARLA_BUILD_TOOLS_FOLDER}/Prettify.sh $(ARGS)
