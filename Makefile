# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Treat the Rack SDK headers as system headers so the stricter warnings below
# only report problems in this plugin's code.
FLAGS += -isystem $(RACK_DIR)/include -isystem $(RACK_DIR)/dep/include

# Stricter warnings than the Rack defaults.
FLAGS += -Wpedantic -Wundef -Wshadow -Wconversion -Wsign-conversion -Wimplicit-fallthrough
CXXFLAGS += -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual

# `make WERROR=1` fails the build on any warning. Off by default so that newer
# or different compilers don't break builds for people compiling from source.
ifdef WERROR
FLAGS += -Werror
endif

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += presets

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

# Rack targets macOS 10.9 (-mmacosx-version-min), which newer macOS SDKs' libc++
# reports with a #warning from its own headers. Keep it visible but non-fatal.
# ARCH_MAC is set by plugin.mk, so this must come after the include.
ifdef WERROR
ifdef ARCH_MAC
CFLAGS += -Wno-error=\#warnings
CXXFLAGS += -Wno-error=\#warnings
endif
endif
