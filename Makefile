#==============================================================================
#
# This file serves TWO roles:
#   1. CONSUMER CONFIG — when included by the root Makefile, it provides
#      include paths, link libraries, defines, and system dependencies.
#   2. BUILD CONFIG — when invoked directly ($(MAKE) -f libs/<lib_name>/Makefile),
#      it additionally sets build-only flags and delegates to Makefile.rules.
#
# The MAKEFILE_LIST trick auto-detects which role we're in:
#   - Direct invocation: we're the first file in MAKEFILE_LIST
#   - Included by root: root's Makefile is the first file
#==============================================================================

# Capture our filename before any includes modify MAKEFILE_LIST
_LAST_MK := $(lastword $(MAKEFILE_LIST))
_INCLUDED_AS_CONFIG := $(filter $(_LAST_MK),$(firstword $(MAKEFILE_LIST)))

# When invoked directly, load the generic build infrastructure
ifneq ($(_INCLUDED_AS_CONFIG),)
include Makefile.vars
endif

EXEC_NAME := app
LIB_NAME := mylib

SRC_DIR        ?= src
EXTRA_SRCS_CXX ?=
EXTRA_SRCS_C   ?=
EXCLUDE_SRCS   ?=

INCLUDE_DIRS +=
LIBS_PATH    +=
DEFINES      +=

#==============================================================================
# BUILD-ONLY section — everything below only applies when building the
# lib itself, not when included by the root Makefile for consumer config.
#==============================================================================
ifneq ($(_INCLUDED_AS_CONFIG),)

# Compilation flags
ifeq ($(TARGET),WEB)
    LDLIBS   += 
    LDFLAGS  += 
    DEBUG_FLAGS   += 
    RELEASE_FLAGS += 
    WFLAGS   += 
    CPPFLAGS += 
    CXXFLAGS += 
    CFLAGS   += 
endif
ifeq ($(TARGET),ANDROID)
	ANDROID_ARCH           ?= arm64
	ANDROID_API_VERSION    ?= 29
    PROJECT_NAME           ?= raylib_game
    PROJECT_RESOURCES_PATH ?= resources
    APP_LABEL_NAME         ?= rGame
    APP_COMPANY_NAME       ?= raylib
    APP_PRODUCT_NAME       ?= rgame
    APP_VERSION_CODE       ?= 1
    APP_VERSION_NAME       ?= 1.0
    APP_ICON_LDPI          ?= assets/icons/raylib_36x36.png
    APP_ICON_MDPI          ?= assets/icons/raylib_48x48.png
    APP_ICON_HDPI          ?= assets/icons/raylib_72x72.png
    APP_SCREEN_ORIENTATION ?= landscape
    APP_KEYSTORE_PASS      ?= raylib

    LDLIBS   += 
    LDFLAGS  += 
    DEBUG_FLAGS   += 
    RELEASE_FLAGS += 
    WFLAGS   += 
    CPPFLAGS += 
    CXXFLAGS += 
    CFLAGS   += 
endif
ifeq ($(TARGET),WINDOWS)
ifdef MSVC
    LDLIBS   := 
    LDFLAGS  := 
    DEBUG_FLAGS   := 
    RELEASE_FLAGS := 
    WFLAGS   := 
    CPPFLAGS := 
    CXXFLAGS := 
    CFLAGS   := 
else
    LDLIBS   := 
    LDFLAGS  := 
    DEBUG_FLAGS   := -g -ggdb -O0
    RELEASE_FLAGS := -march=native -Ofast -s -DNDEBUG
    WFLAGS   := 
    CPPFLAGS := 
    CXXFLAGS := 
    CFLAGS   := 
endif
endif
ifeq ($(TARGET),LINUX)
    LDLIBS   := 
    LDFLAGS  := 
    DEBUG_FLAGS   := -g -ggdb -O0
    RELEASE_FLAGS := -march=native -Ofast -s -DNDEBUG
    WFLAGS   := 
    CPPFLAGS := 
    CXXFLAGS := 
    CFLAGS   := 
endif
ifeq ($(TARGET),OSX)
    LDLIBS   := 
    LDFLAGS  := 
    DEBUG_FLAGS   := 
    RELEASE_FLAGS := 
    WFLAGS   := 
    CPPFLAGS := 
    CXXFLAGS := 
    CFLAGS   := 
endif
# Add more targets

export

.PHONY: all lib run run_cgdb info clean cleanall build-libs clean-libs cleanall-libs info-libs

all:
	@$(MAKE) -f Makefile.rules all

lib:
	@$(MAKE) -f Makefile.rules lib

run:
	@$(MAKE) -f Makefile.rules run

run_valgrind:
	@$(MAKE) -f Makefile.rules run_valgrind

run_cgdb:
	@$(MAKE) -f Makefile.rules run_cgdb

build-libs:
	@$(MAKE) -f Makefile.rules build-libs

info-libs:
	@$(MAKE) -f Makefile.rules info-libs

info:
	@$(MAKE) -f Makefile.rules info

info-libs:
	@$(MAKE) -f Makefile.rules info-libs

clean:
	@$(MAKE) -f Makefile.rules clean

cleanall:
	@$(MAKE) -f Makefile.rules cleanall

clean-libs:
	@$(MAKE) -f Makefile.rules clean-libs

cleanall-libs:
	@$(MAKE) -f Makefile.rules cleanall-libs

endif # _INCLUDED_AS_CONFIG
