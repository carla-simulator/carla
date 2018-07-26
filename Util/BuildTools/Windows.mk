ARGS=--all

default: help

help:
	@type "${CARLA_BUILD_TOOLS_FOLDER}\Linux.mk.help"

launch: LibCarla
	@echo "Not implemented!"

launch-only:
	@echo "Not implemented!"

package: CarlaUE4Editor PythonAPI
	@echo "Not implemented!"

docs:
	@echo "Not implemented!"

clean:
	@echo "Not implemented!"

rebuild: setup
	@echo "Not implemented!"

hard-clean: clean
	@echo "Not implemented!"

check: PythonAPI
	@echo "Not implemented!"

benchmark: LibCarla
	@echo "Not implemented!"

CarlaUE4Editor: LibCarla
	@echo "Not implemented!"

.PHONY: PythonAPI
PythonAPI: LibCarla
	@echo "Not implemented!"

.PHONY: LibCarla
LibCarla: setup
	@${CARLA_BUILD_TOOLS_FOLDER}\BuildLibCarla.bat --server --client

setup:
	@${CARLA_BUILD_TOOLS_FOLDER}\Setup.bat -j 8 --boost-toolset msvc-14.1

pretty:
	@echo "Not implemented!"
