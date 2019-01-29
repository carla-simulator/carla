default: help

help:
	@less ${CARLA_BUILD_TOOLS_FOLDER}/Linux.mk.help

launch: LibCarla.server
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --build --launch

launch-only:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --launch

package: CarlaUE4Editor PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/Package.sh $(ARGS)

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

hard-clean:
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --hard-clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --clean
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --clean
	@echo "To force recompiling dependencies run: rm -Rf ${CARLA_BUILD_FOLDER}"

export-maps:
	@${CARLA_BUILD_TOOLS_FOLDER}/ExportMaps.sh ${ARGS}

check: LibCarla PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --all $(ARGS)

check.LibCarla: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --libcarla-debug --libcarla-release $(ARGS)

check.LibCarla.debug: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --libcarla-debug $(ARGS)

check.LibCarla.release: LibCarla
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --libcarla-release $(ARGS)

check.PythonAPI: PythonAPI
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --python-api-2 --python-api-3 $(ARGS)

check.PythonAPI.2: PythonAPI.2
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --python-api-2 $(ARGS)

check.PythonAPI.3: PythonAPI.3
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --python-api-3 $(ARGS)

benchmark: LibCarla.server
	@${CARLA_BUILD_TOOLS_FOLDER}/Check.sh --benchmark $(ARGS)
	@cat profiler.csv

CarlaUE4Editor: LibCarla.server
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildCarlaUE4.sh --build

.PHONY: PythonAPI
PythonAPI: LibCarla.client
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --py2 --py3

PythonAPI.2: LibCarla.client
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --py2

PythonAPI.3: LibCarla.client
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildPythonAPI.sh --py3

.PHONY: LibCarla
LibCarla: LibCarla.server LibCarla.client

LibCarla.server: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --server

LibCarla.client: setup
	@${CARLA_BUILD_TOOLS_FOLDER}/BuildLibCarla.sh --client

setup:
	@${CARLA_BUILD_TOOLS_FOLDER}/Setup.sh

pretty:
	@${CARLA_BUILD_TOOLS_FOLDER}/Prettify.sh $(ARGS)
