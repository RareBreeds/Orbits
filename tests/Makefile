# Unit tests for the plugin.
#
#   make -C tests                          build and run the tests that don't need VCV Rack
#   make -C tests rack RACK_DIR=<path>     build and run the tests that use VCV Rack
#   make -C tests WERROR=1                 also fail on compiler warnings, as CI does
#   make -C tests clean                    remove the build output
#
# The default tests don't need RACK_DIR or the Rack SDK. The Rack tests are built from all of
# the plugin's sources and linked with libRack. RACK_DIR is a Rack SDK or a Rack source
# directory with libRack built; as `make -C tests` runs from this directory, use an absolute
# path, e.g. RACK_DIR=$HOME/Rack-SDK.
#
# All tests are built with AddressSanitizer and UndefinedBehaviorSanitizer, so memory errors and
# undefined behaviour fail the run as well as failed checks.

BUILD_DIR := build
TARGET := $(BUILD_DIR)/orbits-tests
RACK_TARGET := $(BUILD_DIR)/orbits-rack-tests

# Plugin sources under test, then the tests.
SOURCES := ../src/OrbitsRhythm.cpp
SOURCES += TestMain.cpp
SOURCES += OrbitsRhythmTests.cpp

# All of the plugin's sources, then the Rack tests.
RACK_SOURCES := $(wildcard ../src/*.cpp)
RACK_SOURCES += $(wildcard rack/*.cpp)

CXXFLAGS += -std=c++11 -g -O1 -MMD -MP
CXXFLAGS += -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer
CXXFLAGS += -I../src -isystem doctest

# The same warnings as the plugin's Makefile.
CXXFLAGS += -Wall -Wextra -Wpedantic -Wundef -Wshadow -Wconversion -Wsign-conversion -Wimplicit-fallthrough
CXXFLAGS += -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual
ifdef WERROR
CXXFLAGS += -Werror
endif

LDFLAGS += -fsanitize=address,undefined

# Rack's headers are system headers so the warnings above only apply to the plugin and tests.
RACK_CXXFLAGS = $(CXXFLAGS) -pthread -isystem $(RACK_DIR)/include -isystem $(RACK_DIR)/dep/include
RACK_LDFLAGS = $(LDFLAGS) -pthread -L$(RACK_DIR) -lRack

ifneq ($(filter rack,$(MAKECMDGOALS)),)
ifndef RACK_DIR
$(error Set RACK_DIR to a Rack SDK or Rack source directory, e.g. make -C tests rack RACK_DIR=$$HOME/Rack-SDK)
endif
endif

vpath %.cpp $(sort $(dir $(SOURCES) $(RACK_SOURCES)))
OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.cpp=.o)))
# Prefixed so they don't clash with the objects above, which are built without Rack.
RACK_OBJECTS := $(addprefix $(BUILD_DIR)/rack-,$(notdir $(RACK_SOURCES:.cpp=.o)))

.PHONY: test rack clean

test: $(TARGET)
	./$(TARGET)

# libRack's install name on macOS is just "libRack.dylib", so tell the loader where it is.
rack: $(RACK_TARGET)
	DYLD_LIBRARY_PATH="$(RACK_DIR)" LD_LIBRARY_PATH="$(RACK_DIR)" ./$(RACK_TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

$(RACK_TARGET): $(RACK_OBJECTS)
	$(CXX) $^ $(RACK_LDFLAGS) -o $@

$(BUILD_DIR)/rack-%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(RACK_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d) $(RACK_OBJECTS:.o=.d)
