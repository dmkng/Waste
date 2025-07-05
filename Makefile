# Project name used for generating build directories
NAME := Waste

# Project directories
BUILD_DIR := build
SRC_DIR := src
INCLUDE_DIR := include

# Compile settings
CC := gcc
CXX := g++
CFLAGS := -std=c11
CXXFLAGS := -std=c++11
CPPFLAGS := -Wall -Wextra -Wpedantic
LDFLAGS :=
LDLIBS := -lSDL2 -lSDL2_mixer -lSDL2_ttf

# Test run arguments
ARGS :=

# Additional objects to link to the final executable
OBJS :=

# Multiple choices
ifeq ($(filter $(RELEASE),1 y yes true),)
	# Debug build - no optimization and debugging symbols
	CPPFLAGS += -Og -ggdb3 -fno-inline -fno-omit-frame-pointer
	LDFLAGS += -Og -ggdb3
	TAG := -debug
else
	# Release build - optimization and no debugging symbols
	CPPFLAGS += -O3 -s -flto -ffunction-sections -fdata-sections -DNDEBUG
	LDFLAGS += -O3 -s -flto -Wl,--gc-sections
	TAG := -release
endif

ifeq ($(OS), Windows_NT)
	EXE := $(NAME).exe
	OUT_DIR := $(NAME)$(TAG)-windows
	TEST := cmd /c start "test: $(NAME)" cmd /c "cd $(OUT_DIR) && $(EXE) $(ARGS) && pause"

	LDFLAGS += -mwindows
	OBJS += $(BUILD_DIR)/resources.o

	ifeq ($(findstring .exe,$(SHELL)),)
		# MSYS2
		TEST := $(subst /,//,$(TEST))
		CLEAN := rm -f $(OUT_DIR)/$(EXE) $(BUILD_DIR)/*
		TIME := echo "        Compile Time: `date +%T`"
	else
		# Not MSYS2
		CLEAN := attrib +r $(BUILD_DIR)\.keep && del /f /q $(OUT_DIR)\$(EXE) $(BUILD_DIR)\*
		TIME := echo         Compile Time: %time:~0,8%

		# Some MinGW builds need those, tweak them according to your MinGW installation path
		CPPFLAGS += -IC:/MinGW/include
		LDFLAGS += -LC:/MinGW/lib
	endif
else
	EXE := $(NAME)
	OUT_DIR := $(NAME)-linux
	TEST := cd $(OUT_DIR) && cp -f $(EXE) /tmp/$(EXE) && chmod +x /tmp/$(EXE) && xterm -T $(NAME) -e "/tmp/$(EXE) $(ARGS)" && rm -f /tmp/$(EXE)
	CLEAN := rm -f $(OUT_DIR)/$(EXE) $(BUILD_DIR)/*
	TIME := echo "        Compile Time: `date +%T`"
endif

.PHONY: all
all: $(OUT_DIR)/$(EXE)

.PHONY: clean
clean:
	$(CLEAN)

.PHONY: run test
run test:
	$(TEST)

$(OUT_DIR)/$(EXE): $(OBJS) $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%$(TAG).c.o,$(wildcard $(SRC_DIR)/*.c)) $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%$(TAG).cpp.o,$(wildcard $(SRC_DIR)/*.cpp))
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@echo ======================================
	@$(TIME)
	@echo ======================================

$(BUILD_DIR)/resources.o: $(SRC_DIR)/resources.rc
	windres $^ -o $@

$(BUILD_DIR)/%$(TAG).cpp.o: $(SRC_DIR)/%.cpp $(wildcard $(INCLUDE_DIR)/*.hpp)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $< -c -o $@

$(BUILD_DIR)/%$(TAG).c.o: $(SRC_DIR)/%.c $(wildcard $(INCLUDE_DIR)/*.h)
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@
