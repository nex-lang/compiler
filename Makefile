CC = gcc

SRC_DIR = $(CURDIR)/src
GEN_DIR = $(CURDIR)/src/gen
INCLUDE_DIR = $(CURDIR)/include
TEST_DIR = $(CURDIR)/tests
DEPS_DIR = $(CURDIR)/deps
CRITERION_DIR = $(DEPS_DIR)/criterion-v2.3.3
BUILD_DIR = $(CURDIR)/build

LIB_SOURCES = \
    $(INCLUDE_DIR)/tmp/alphadev.c \
    $(SRC_DIR)/io.c \
    $(SRC_DIR)/symtbl.c \
    $(SRC_DIR)/lexer.c \
    $(SRC_DIR)/ast.c \
    $(SRC_DIR)/parser.c \
    $(SRC_DIR)/sao.c \
    $(SRC_DIR)/gen.c \
    $(GEN_DIR)/arth.c \
    $(GEN_DIR)/class.c \
    $(GEN_DIR)/control.c \
    $(GEN_DIR)/enum.c \
    $(GEN_DIR)/err.c \
    $(GEN_DIR)/func.c \
    $(GEN_DIR)/imports.c \
    $(GEN_DIR)/mem.c \
    $(GEN_DIR)/stack.c \
    $(GEN_DIR)/struct.c

EXE_SOURCES = $(SRC_DIR)/main.c

HEADERS = \
    $(INCLUDE_DIR)/tmp/alphadev.h \
    $(INCLUDE_DIR)/utils/bit128.h \
    $(INCLUDE_DIR)/p_info.h \
    $(INCLUDE_DIR)/io.h \
    $(INCLUDE_DIR)/token.h \
    $(INCLUDE_DIR)/symtbl.h \
    $(INCLUDE_DIR)/lexer.h \
    $(INCLUDE_DIR)/ast.h \
    $(INCLUDE_DIR)/parser.h \
    $(INCLUDE_DIR)/sao.h \
    $(INCLUDE_DIR)/gen.h \
    $(INCLUDE_DIR)/gen/arth.h \
    $(INCLUDE_DIR)/gen/class.h \
    $(INCLUDE_DIR)/gen/control.h \
    $(INCLUDE_DIR)/gen/enum.h \
    $(INCLUDE_DIR)/gen/err.h \
    $(INCLUDE_DIR)/gen/func.h \
    $(INCLUDE_DIR)/gen/import.h \
    $(INCLUDE_DIR)/gen/mem.h \
    $(INCLUDE_DIR)/gen/stack.h \
    $(INCLUDE_DIR)/gen/struct.h

TARGET = $(BUILD_DIR)/nex
LIB_TARGET = $(BUILD_DIR)/nex_library
TEST_TARGET = $(BUILD_DIR)/test

GREEN=$(shell tput setaf 2)
LIME_YELLOW=$(shell tput setaf 190)
MAGENTA=$(shell tput setaf 5)
BRIGHT=$(shell tput bold)
NORMAL=$(shell tput sgr0)
UNDERLINE=$(shell tput smul)

BUILD_TYPE ?= debug

ifeq ($(BUILD_TYPE), release)
	CFLAGS = -Wall -std=c99 -O3 -I$(INCLUDE_DIR) -I$(GEN_DIR)
else
	CFLAGS = -Wall -std=c99 -g -I$(INCLUDE_DIR) -I$(GEN_DIR)
endif

LDFLAGS =

# Determine OS
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	CRITERION_URL = https://github.com/Snaipe/Criterion/releases/download/v2.3.3/criterion-v2.3.3-linux-x86_64.tar.bz2
	CRITERION_ARCHIVE = criterion-v2.3.3-linux-x86_64.tar.bz2
	CRITERION_EXTRACT_CMD = tar -xjf
else
	CRITERION_URL = https://github.com/Snaipe/Criterion/releases/download/v2.3.3/criterion-v2.3.3-win64-mingw.zip
	CRITERION_ARCHIVE = criterion-v2.3.3-win64-mingw.zip
	CRITERION_EXTRACT_CMD = powershell -Command "Expand-Archive -Path '$(CRITERION_ARCHIVE)' -DestinationPath '$(DEPS_DIR)'"
endif

all: help

build: $(BUILD_DIR) $(TARGET)
	@echo "$(MAGENTA)$(BRIGHT)[BUILD]$(NORMAL) BUILT TARGET $(GREEN)$(BRIGHT)NEX$(NORMAL)"

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(LIB_SOURCES) $(EXE_SOURCES)
	@echo "$(MAGENTA)$(BRIGHT)[BUILD]$(NORMAL) Compiling $(TARGET)..."
	@cd $(BUILD_DIR)
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(LIB_SOURCES) $(EXE_SOURCES)


clean:
	@echo "$(MAGENTA)$(BRIGHT)[CLEAN]$(NORMAL) Cleaning build files..."
	rm -f $(TARGET) $(TEST_TARGET)
	rm -rf $(BUILD_DIR)
	rm -rf $(DEPS_DIR)/criterion-v2.3.3/$(CRITERION_ARCHIVE)

BUILD_TESTS = 0
ifeq ($(BUILD_TESTS), 1)
test: $(BUILD_DIR) $(TEST_DIR)/lexer_test.c $(TEST_DIR)/symtbl_test.c
	@echo "$(MAGENTA)$(BRIGHT)[TEST]$(NORMAL) Building tests..."
	# Ensure Criterion library is present
	@if [ ! -d "$(CRITERION_DIR)" ]; then \
		echo "$(MAGENTA)$(BRIGHT)[TEST]$(NORMAL) Criterion not found. Downloading and extracting..."; \
		curl -LJO $(CRITERION_URL) || exit 1; \
		$(CRITERION_EXTRACT_CMD) $(CRITERION_ARCHIVE) -C $(DEPS_DIR) || exit 1; \
		rm $(CRITERION_ARCHIVE); \
	fi
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -I$(CRITERION_DIR)/include -o $(TEST_TARGET) $(TEST_DIR)/lexer_test.c $(TEST_DIR)/symtbl_test.c $(LDFLAGS) -L$(CRITERION_DIR)/lib -lcriterion

else
test:
	@echo "$(MAGENTA)$(BRIGHT)[TEST]$(NORMAL) Tests are not built. Set BUILD_TESTS to 1 to enable."

endif

help:
	@echo "$(MAGENTA)$(BRIGHT)[HELP]$(NORMAL) Usage: make [target] [options]"
	@echo "${UNDERLINE}Targets:${NORMAL}"
	@echo "  ${BRIGHT}all, help${NORMAL}            Display this help message"
	@echo "  ${BRIGHT}clean${NORMAL}                Remove build artifacts"
	@echo "  ${BRIGHT}test${NORMAL}                 Build tests if BUILD_TESTS=1"
	@echo "  ${BRIGHT}build${NORMAL}                Builds the project BUILD_TYPE defaults to debug"
	@echo "${UNDERLINE}Options:${NORMAL}"
	@echo "  ${BRIGHT}BUILD_TYPE=${LIME_YELLOW}[release|debug]${NORMAL}    Build type (default: release)"
	@echo "  ${BRIGHT}BUILD_TESTS=${LIME_YELLOW}[0|1]${NORMAL}             Build tests (default: 0)"

.PHONY: all clean test help
