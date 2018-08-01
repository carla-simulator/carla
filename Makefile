include Util/BuildTools/Vars.mk
ifeq ($(OS),Windows_NT)
include Util/BuildTools/Windows.mk
else
include Util/BuildTools/Linux.mk
endif
