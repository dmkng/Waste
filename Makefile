NAME := Waste
TMP := tmp
SRC := src
INCLUDE := include
CC := gcc
CCFLAGS := -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-write-strings -std=c11
LDFLAGS :=
LDLIBS := -lSDL2 -lSDL2_ttf -lSDL2_mixer
ARGS :=

ifeq ($(BUILD), release)
	# Release build - optimization and no debugging symbols
	CCFLAGS += -O3 -s -flto -ffunction-sections -fdata-sections -DNDEBUG
	LDFLAGS += -O3 -s -flto -Wl,--gc-sections
	TAG := -release
else
	# Debug build - no optimization and debugging symbols
	CCFLAGS += -Og -ggdb3 -fno-inline -fno-omit-frame-pointer
	LDFLAGS += -Og -ggdb3
	TAG := -debug
endif

ifeq ($(OS), Windows_NT)
	BD := $(NAME)_Windows
	EXE := $(NAME).exe
	WIN_BD := $(subst /,\\,$(BD))
	WIN_TMP := $(subst /,\\,$(TMP))
	TEST := start "$(NAME) " cmd /c "cd $(WIN_BD) & $(EXE) $(ARGS) & echo. & pause"
	LDFLAGS += -mwindows
	ifeq ($(MSYSTEM), MINGW32)
		TEST := cmd //c $(subst /,//,$(TEST))
		DEL := rm -rf $(BD)/$(EXE) $(TMP)/*
		TIME := echo "        Compile Time: `date +%T`"
		NL := echo ""
	else
		DEL := del /f /q $(WIN_BD)\\$(EXE) $(WIN_TMP)\\* >nul 2>nul
		TIME := echo:        Compile Time: %time:~0,8%
		NL := echo:
		CCFLAGS += -IC:/MinGW/include
		LDFLAGS += -LC:/MinGW/lib
	endif
else
	BD := $(NAME)_Linux
	EXE := $(NAME)
	TEST := cd $(BD) && cp -f $(EXE) /tmp/$(EXE) && chmod +x /tmp/$(EXE) && x-terminal-emulator -T $(NAME) -e "/tmp/$(EXE) $(ARGS)" && rm -f /tmp/$(EXE)
	DEL := rm -f $(BD)/$(EXE) $(TMP)/*
	TIME := echo "        Compile Time: `date +%T`"
	NL := echo ""
endif

.PHONY: all
all: info $(BD)/$(EXE)

.PHONY: info
info:
	@echo ======================================
	@$(TIME)
	@echo ======================================
	@$(NL)

$(TMP)/main$(TAG).o: $(SRC)/main.c
	$(CC) $(CCFLAGS) $< -c -o $@

$(TMP)/%$(TAG).o: $(SRC)/%.c $(wildcard $(INCLUDE)/*.h)
	$(CC) $(CCFLAGS) $< -c -o $@

.PHONY: $(BD)/$(EXE)
$(BD)/$(EXE): $(patsubst $(SRC)/%.c,$(TMP)/%$(TAG).o,$(wildcard $(SRC)/*.c))
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

.PHONY: clean
clean:
	-@$(DEL)

.PHONY: run test
run test:
	@$(TEST)
