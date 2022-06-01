export WFLAGS   := -Wno-missing-braces -Wmissing-field-initializers
export CPPFLAGS := -I src -I external/tmpl
export CXXFLAGS := -std=c++17 -fno-rtti -fno-exceptions

all:
	$(MAKE) -f Makefile.gen
