include Makefile.vars

EXEC_NAME := ecs

SRC_DIR ?= oop-ecs

INCLUDE_DIRS += $(SRC_DIR)

# Compilation flags
ifndef MSVC
    LDFLAGS  += -flto
    LDLIBS   += -ltbb 
    DEBUG_FLAGS   += -g -ggdb -O0 
    RELEASE_FLAGS += -march=native -Ofast -flto
    WFLAGS   += -Weffc++ -Wpadded
    CXXFLAGS += -std=c++20 -fno-rtti -fno-exceptions 
else
    WFLAGS += /wd5026 /wd5027 /wd4626 /wd4625 /wd4668 /wd4820
    CXXFLAGS += /std:c++20 /EHsc
endif

export

.PHONY: all run run_cgdb info clean cleanall

all:
	$(MAKE) -f Makefile.options all

lib:
	$(MAKE) -f Makefile.options lib

run:
	$(MAKE) -f Makefile.options run

run_valgrind:
	$(MAKE) -f Makefile.options run_valgrind

run_cgdb:
	$(MAKE) -f Makefile.options run_cgdb

info:
	$(MAKE) -f Makefile.options info

clean:
	$(MAKE) -f Makefile.options clean

cleanall:
	$(MAKE) -f Makefile.options cleanall
