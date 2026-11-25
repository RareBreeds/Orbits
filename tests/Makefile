# Unit tests for the plugin code that doesn't depend on VCV Rack.
#
#   make -C tests              build and run the tests
#   make -C tests WERROR=1     also fail on compiler warnings, as CI does
#   make -C tests clean        remove the build output
#
# Unlike the plugin's Makefile this doesn't need RACK_DIR or the Rack SDK. The tests are
# built with AddressSanitizer and UndefinedBehaviorSanitizer, so memory errors and undefined
# behaviour fail the run as well as failed checks.

BUILD_DIR := build
TARGET := $(BUILD_DIR)/orbits-tests

# Plugin sources under test, then the tests.
SOURCES := ../src/OrbitsRhythm.cpp
SOURCES += main.cpp
SOURCES += test_rhythm.cpp

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

vpath %.cpp $(sort $(dir $(SOURCES)))
OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.cpp=.o)))

.PHONY: test clean

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d)
