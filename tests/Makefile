# Unit tests for the plugin.
#
#   make -C tests                          check the SVGs, then build and run the tests that don't
#                                          need VCV Rack
#   make -C tests svg                      check that the SVGs are well-formed XML, with xmllint
#   make -C tests rack RACK_DIR=<path>     build and run the tests that use VCV Rack
#   make -C tests bench RACK_DIR=<path>    build and run the benchmarks, which use VCV Rack and
#                                          open a Rack window while they run
#   make -C tests compare RACK_DIR=<path>  compare the modules' behaviour in VCV Rack's engine with
#                                          the last release, see compare/compare.sh
#   make -C tests WERROR=1                 also fail on compiler warnings, as CI does
#   make -C tests clean                    remove the build output
#
# The default tests don't need RACK_DIR or the Rack SDK. The Rack tests are built from all of
# the plugin's sources and linked with libRack. RACK_DIR is a Rack SDK or a Rack source
# directory with libRack built; as `make -C tests` runs from this directory, use an absolute
# path, e.g. RACK_DIR=$HOME/Rack-SDK.
#
# All tests are built with AddressSanitizer and UndefinedBehaviorSanitizer, so memory errors and
# undefined behaviour fail the run as well as failed checks. The benchmarks and comparisons are
# built like the plugin instead, optimised and without sanitizers, so their timings and behaviour
# match a release build.

BUILD_DIR ?= build
TARGET := $(BUILD_DIR)/orbits-tests
RACK_TARGET := $(BUILD_DIR)/orbits-rack-tests
BENCH_TARGET := $(BUILD_DIR)/orbits-bench
COMPARE_TARGET := $(BUILD_DIR)/orbits-compare

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

# All of the plugin's sources, then the comparison scenarios. COMPARE_SRC_DIR lets
# compare/compare.sh build the scenarios against the plugin's sources from another revision.
COMPARE_SRC_DIR ?= ../src
COMPARE_PLUGIN_SOURCES := $(wildcard $(COMPARE_SRC_DIR)/*.cpp)
COMPARE_SOURCES := $(wildcard compare/*.cpp)

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
RELEASE_CXXFLAGS := -std=c++11 -g -MMD -MP -O3 -funsafe-math-optimizations -fno-omit-frame-pointer
ifeq ($(shell uname -m),x86_64)
RELEASE_CXXFLAGS += -march=nehalem
else ifneq ($(filter arm64 aarch64,$(shell uname -m)),)
RELEASE_CXXFLAGS += -march=armv8-a+fp+simd
endif
RELEASE_CXXFLAGS += -pthread -isystem $(RACK_DIR)/include -isystem $(RACK_DIR)/dep/include $(WARNINGS)
RELEASE_LDFLAGS = -pthread -L$(RACK_DIR) -lRack
ifeq ($(shell uname -s),Darwin)
RELEASE_LDFLAGS += -framework OpenGL
else
RELEASE_LDFLAGS += -lGL
endif

BENCH_CXXFLAGS = $(RELEASE_CXXFLAGS) -I$(BENCH_SRC_DIR) -Irack
# -iquote, as the plugin's plugin.hpp would hide Rack's from #include <plugin.hpp>.
COMPARE_CXXFLAGS = $(RELEASE_CXXFLAGS) -iquote $(COMPARE_SRC_DIR)
COMPARE_LDFLAGS = $(RELEASE_LDFLAGS)
# compare/compare.sh also checks the working tree with the sanitizers, in its own BUILD_DIR.
ifdef COMPARE_SANITIZE
COMPARE_CXXFLAGS += -fsanitize=address,undefined -fno-sanitize-recover=all
COMPARE_LDFLAGS += -fsanitize=address,undefined
endif
# The drawing benchmarks load Rack's fonts from RACK_DIR and the plugin's SVGs and fonts from here.
BENCH_PLUGIN_DIR ?= $(abspath ..)

# Rack's headers are system headers so the warnings above only apply to the plugin and tests.
RACK_CXXFLAGS = $(CXXFLAGS) -pthread -isystem $(RACK_DIR)/include -isystem $(RACK_DIR)/dep/include -DORBITS_TEST_RES_DIR=\"$(abspath ../res)\"
RACK_LDFLAGS = $(LDFLAGS) -pthread -L$(RACK_DIR) -lRack

ifneq ($(filter rack bench compare %/orbits-compare,$(MAKECMDGOALS)),)
ifndef RACK_DIR
$(error Set RACK_DIR to a Rack SDK or Rack source directory, e.g. make -C tests $(firstword $(filter rack bench compare,$(MAKECMDGOALS))) RACK_DIR=$$HOME/Rack-SDK)
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
COMPARE_OBJECTS := $(addprefix $(BUILD_DIR)/compare-src-,$(notdir $(COMPARE_PLUGIN_SOURCES:.cpp=.o)))
COMPARE_OBJECTS += $(addprefix $(BUILD_DIR)/compare-,$(notdir $(COMPARE_SOURCES:.cpp=.o)))

.PHONY: test svg rack bench compare clean

test: svg $(TARGET)
	$(abspath $(TARGET))

# Rack draws SVGs with nanosvg, which loads some malformed ones without an error and draws them
# wrongly, e.g. the unterminated attributes svgopt left in two of v2.0.4's SVGs (273c07f).
svg:
	xmllint --noout ../res/*.svg

# libRack's install name on macOS is just "libRack.dylib", so tell the loader where it is.
rack: $(RACK_TARGET)
	DYLD_LIBRARY_PATH="$(RACK_DIR)" LD_LIBRARY_PATH="$(RACK_DIR)" $(abspath $(RACK_TARGET))

bench: $(BENCH_TARGET)
	DYLD_LIBRARY_PATH="$(RACK_DIR)" LD_LIBRARY_PATH="$(RACK_DIR)" $(abspath $(BENCH_TARGET)) "$(RACK_DIR)" "$(BENCH_PLUGIN_DIR)"

compare:
	RACK_DIR="$(RACK_DIR)" compare/compare.sh

$(TARGET): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

$(RACK_TARGET): $(RACK_OBJECTS)
	$(CXX) $^ $(RACK_LDFLAGS) -o $@

$(BENCH_TARGET): $(BENCH_OBJECTS)
	$(CXX) $^ $(RELEASE_LDFLAGS) -o $@

$(COMPARE_TARGET): $(COMPARE_OBJECTS)
	$(CXX) $^ $(COMPARE_LDFLAGS) -o $@

$(BUILD_DIR)/bench-src-%.o: $(BENCH_SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(BENCH_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/bench-%.o: bench/%.cpp | $(BUILD_DIR)
	$(CXX) $(BENCH_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/compare-src-%.o: $(COMPARE_SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(COMPARE_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/compare-%.o: compare/%.cpp | $(BUILD_DIR)
	$(CXX) $(COMPARE_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/rack-%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(RACK_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d) $(RACK_OBJECTS:.o=.d) $(BENCH_OBJECTS:.o=.d) $(COMPARE_OBJECTS:.o=.d)
