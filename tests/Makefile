# Unit tests for the plugin.
#
#   make -C tests                          build and run the tests that don't need VCV Rack
#   make -C tests rack RACK_DIR=<path>     build and run the tests that use VCV Rack
#   make -C tests bench RACK_DIR=<path>    build and run the benchmarks, which use VCV Rack and
#                                          open a Rack window while they run
#   make -C tests WERROR=1                 also fail on compiler warnings, as CI does
#   make -C tests clean                    remove the build output
#
# The default tests don't need RACK_DIR or the Rack SDK. The Rack tests are built from all of
# the plugin's sources and linked with libRack. RACK_DIR is a Rack SDK or a Rack source
# directory with libRack built; as `make -C tests` runs from this directory, use an absolute
# path, e.g. RACK_DIR=$HOME/Rack-SDK.
#
# All tests are built with AddressSanitizer and UndefinedBehaviorSanitizer, so memory errors and
# undefined behaviour fail the run as well as failed checks. The benchmarks are built like the
# plugin instead, optimised and without sanitizers, so their timings match a release build.

BUILD_DIR ?= build
TARGET := $(BUILD_DIR)/orbits-tests
RACK_TARGET := $(BUILD_DIR)/orbits-rack-tests
BENCH_TARGET := $(BUILD_DIR)/orbits-bench

# Plugin sources under test, then the tests.
SOURCES := ../src/OrbitsRhythm.cpp
SOURCES += TestMain.cpp
SOURCES += OrbitsRhythmTests.cpp
SOURCES += OrbitsStepCounterTests.cpp

# All of the plugin's sources, then the Rack tests.
RACK_SOURCES := $(wildcard ../src/*.cpp)
RACK_SOURCES += $(wildcard rack/*.cpp)

# All of the plugin's sources, then the benchmarks. BENCH_SRC_DIR lets bench/compare.sh build the
# benchmarks against the plugin's sources from another revision.
BENCH_SRC_DIR ?= ../src
BENCH_PLUGIN_SOURCES := $(wildcard $(BENCH_SRC_DIR)/*.cpp)
BENCH_SOURCES := $(wildcard bench/*.cpp)

# The same warnings as the plugin's Makefile.
WARNINGS := -Wall -Wextra -Wpedantic -Wundef -Wshadow -Wconversion -Wsign-conversion -Wimplicit-fallthrough
WARNINGS += -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual
ifdef WERROR
WARNINGS += -Werror
endif

CXXFLAGS += -std=c++11 -g -O1 -MMD -MP
CXXFLAGS += -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer
CXXFLAGS += -I../src -isystem doctest $(WARNINGS)

LDFLAGS += -fsanitize=address,undefined

# The optimisation flags from Rack's compile.mk, which the plugin is built with.
BENCH_CXXFLAGS := -std=c++11 -g -MMD -MP -O3 -funsafe-math-optimizations -fno-omit-frame-pointer
ifeq ($(shell uname -m),x86_64)
BENCH_CXXFLAGS += -march=nehalem
else ifneq ($(filter arm64 aarch64,$(shell uname -m)),)
BENCH_CXXFLAGS += -march=armv8-a+fp+simd
endif
BENCH_CXXFLAGS += -I$(BENCH_SRC_DIR) -Irack -pthread -isystem $(RACK_DIR)/include -isystem $(RACK_DIR)/dep/include $(WARNINGS)
BENCH_LDFLAGS = -pthread -L$(RACK_DIR) -lRack
ifeq ($(shell uname -s),Darwin)
BENCH_LDFLAGS += -framework OpenGL
else
BENCH_LDFLAGS += -lGL
endif
# The drawing benchmarks load Rack's fonts from RACK_DIR and the plugin's SVGs and fonts from here.
BENCH_PLUGIN_DIR ?= $(abspath ..)

# Rack's headers are system headers so the warnings above only apply to the plugin and tests.
RACK_CXXFLAGS = $(CXXFLAGS) -pthread -isystem $(RACK_DIR)/include -isystem $(RACK_DIR)/dep/include
RACK_LDFLAGS = $(LDFLAGS) -pthread -L$(RACK_DIR) -lRack

ifneq ($(filter rack bench,$(MAKECMDGOALS)),)
ifndef RACK_DIR
$(error Set RACK_DIR to a Rack SDK or Rack source directory, e.g. make -C tests $(filter rack bench,$(MAKECMDGOALS)) RACK_DIR=$$HOME/Rack-SDK)
endif
endif

vpath %.cpp $(sort $(dir $(SOURCES) $(RACK_SOURCES)))
OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.cpp=.o)))
# Prefixed so they don't clash with the objects above, which are built without Rack.
RACK_OBJECTS := $(addprefix $(BUILD_DIR)/rack-,$(notdir $(RACK_SOURCES:.cpp=.o)))
# The benchmarks' objects have their own rules rather than using vpath, so that the plugin's
# sources always come from BENCH_SRC_DIR.
BENCH_OBJECTS := $(addprefix $(BUILD_DIR)/bench-src-,$(notdir $(BENCH_PLUGIN_SOURCES:.cpp=.o)))
BENCH_OBJECTS += $(addprefix $(BUILD_DIR)/bench-,$(notdir $(BENCH_SOURCES:.cpp=.o)))

.PHONY: test rack bench clean

test: $(TARGET)
	$(abspath $(TARGET))

# libRack's install name on macOS is just "libRack.dylib", so tell the loader where it is.
rack: $(RACK_TARGET)
	DYLD_LIBRARY_PATH="$(RACK_DIR)" LD_LIBRARY_PATH="$(RACK_DIR)" $(abspath $(RACK_TARGET))

bench: $(BENCH_TARGET)
	DYLD_LIBRARY_PATH="$(RACK_DIR)" LD_LIBRARY_PATH="$(RACK_DIR)" $(abspath $(BENCH_TARGET)) "$(RACK_DIR)" "$(BENCH_PLUGIN_DIR)"

$(TARGET): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

$(RACK_TARGET): $(RACK_OBJECTS)
	$(CXX) $^ $(RACK_LDFLAGS) -o $@

$(BENCH_TARGET): $(BENCH_OBJECTS)
	$(CXX) $^ $(BENCH_LDFLAGS) -o $@

$(BUILD_DIR)/bench-src-%.o: $(BENCH_SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(BENCH_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/bench-%.o: bench/%.cpp | $(BUILD_DIR)
	$(CXX) $(BENCH_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/rack-%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(RACK_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d) $(RACK_OBJECTS:.o=.d) $(BENCH_OBJECTS:.o=.d)
