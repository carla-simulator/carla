include Util/BuildTools/Vars.mk
ifeq ($(OS),Windows_NT)
include Util/BuildTools/Windows.mk
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        include Util/BuildTools/Mac.mk
    else
        include Util/BuildTools/Linux.mk
    endif
endif
