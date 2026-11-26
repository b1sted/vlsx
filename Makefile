# SPDX-License-Identifier: GPL-3.0-or-later
#
# vlsx build system.

SHELL := /bin/sh

CC ?= gcc
AR ?= ar
CLANG_FORMAT ?= clang-format
CLANG_TIDY ?= clang-tidy
CPPCHECK ?= cppcheck

UNAME_S := $(shell uname -s)

VERSION ?= dev
VERSION_NUMBER := $(patsubst v%,%,$(VERSION))

CPPFLAGS ?=
override CPPFLAGS += -Iinclude -isystem third_party -MMD -MP \
	-D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE \
	-DVLSX_VERSION=\"$(VERSION_NUMBER)\"
CFLAGS ?=
LDFLAGS ?=

COMMON_CFLAGS := -std=gnu99 -Wall -Wextra -Werror -pedantic -Wshadow \
	-Wconversion -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 \
	-Wundef -fno-common
RELEASE_CFLAGS := $(COMMON_CFLAGS) -O3 -DNDEBUG -ffunction-sections \
	-fdata-sections
DEV_CFLAGS := $(COMMON_CFLAGS) -g -O0 -DDEBUG
LEAKS_CFLAGS := $(COMMON_CFLAGS) -g -O0 -DDEBUG -fsanitize=address \
	-fsanitize=undefined
LEAKS_LDFLAGS := -fsanitize=address -fsanitize=undefined

ifeq ($(UNAME_S),Darwin)
	override CPPFLAGS += -D_DARWIN_C_SOURCE
	DEV_CFLAGS += -gdwarf-4
	RELEASE_LDFLAGS := -Wl,-dead_strip
else ifneq (,$(filter $(UNAME_S),Linux Android))
	RELEASE_LDFLAGS := -Wl,--gc-sections -s
else
	$(error Platform $(UNAME_S) not supported)
endif

BIN_DIR := bin
OBJ_DIR := obj
SRC_DIR := src

RELEASE_OBJDIR := $(OBJ_DIR)/release
DEV_OBJDIR := $(OBJ_DIR)/dev
LEAKS_OBJDIR := $(OBJ_DIR)/leaks
TEST_CORE_OBJDIR := $(OBJ_DIR)/test_core
LEAKS_CORE_OBJDIR := $(OBJ_DIR)/leaks_core

TARGET := $(BIN_DIR)/vlsx
TARGET_DEV := $(BIN_DIR)/vlsx-dev
TARGET_LEAKS := $(BIN_DIR)/vlsx-leaks
TARGET_TEST := $(BIN_DIR)/vlsx-tests
TARGET_TEST_LEAKS := $(BIN_DIR)/vlsx-tests-leaks

SRCS := $(shell find $(SRC_DIR) -name '*.c')
CORE_SRCS := $(shell find $(SRC_DIR)/core $(SRC_DIR)/common -name '*.c')
TEST_SRCS := $(wildcard tests/*.c)

OBJS_REL := $(patsubst $(SRC_DIR)/%.c,$(RELEASE_OBJDIR)/%.o,$(SRCS))
OBJS_DEV := $(patsubst $(SRC_DIR)/%.c,$(DEV_OBJDIR)/%.o,$(SRCS))
OBJS_LEAKS := $(patsubst $(SRC_DIR)/%.c,$(LEAKS_OBJDIR)/%.o,$(SRCS))

CORE_OBJS_TEST := $(patsubst $(SRC_DIR)/%.c,$(TEST_CORE_OBJDIR)/%.o,$(CORE_SRCS))
CORE_OBJS_LEAKS := $(patsubst $(SRC_DIR)/%.c,$(LEAKS_CORE_OBJDIR)/%.o,$(CORE_SRCS))

LIB_CORE_TEST := $(OBJ_DIR)/libvlsx_core_test.a
LIB_CORE_LEAKS := $(OBJ_DIR)/libvlsx_core_leaks.a

DEPS := $(OBJS_REL:.o=.d) $(OBJS_DEV:.o=.d) $(OBJS_LEAKS:.o=.d) \
	$(CORE_OBJS_TEST:.o=.d) $(CORE_OBJS_LEAKS:.o=.d)

FORMAT_FILES := $(shell find src include tests -type f \
	\( -name '*.c' -o -name '*.h' \) 2>/dev/null | LC_ALL=C sort)

# `STATIC=1` links the release binary statically. Only Linux ships a static
# libc; elsewhere the flag is ignored with a warning.
ifeq ($(STATIC),1)
	ifeq ($(UNAME_S),Linux)
		RELEASE_LDFLAGS += -static
	else
		$(warning STATIC=1 is supported only on Linux; ignoring)
	endif
endif

.PHONY: all dev leaks test lint format format-check check-layers clean

all: check-layers $(TARGET)

dev: $(TARGET_DEV)

$(TARGET): $(OBJS_REL)
	@mkdir -p $(dir $@)
	$(CC) $(RELEASE_CFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(RELEASE_LDFLAGS)

$(TARGET_DEV): $(OBJS_DEV)
	@mkdir -p $(dir $@)
	$(CC) $(DEV_CFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TARGET_LEAKS): $(OBJS_LEAKS)
	@mkdir -p $(dir $@)
	$(CC) $(LEAKS_CFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LEAKS_LDFLAGS)

$(TARGET_TEST_LEAKS): $(LIB_CORE_LEAKS) $(TEST_SRCS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(LEAKS_CFLAGS) $(CFLAGS) $(TEST_SRCS) $(LIB_CORE_LEAKS) \
		-o $@ $(LDFLAGS) $(LEAKS_LDFLAGS)

$(LIB_CORE_TEST): $(CORE_OBJS_TEST)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(LIB_CORE_LEAKS): $(CORE_OBJS_LEAKS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(RELEASE_OBJDIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(RELEASE_CFLAGS) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(DEV_OBJDIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(DEV_CFLAGS) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(LEAKS_OBJDIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(LEAKS_CFLAGS) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(TEST_CORE_OBJDIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(DEV_CFLAGS) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(LEAKS_CORE_OBJDIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(LEAKS_CFLAGS) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

test: $(LIB_CORE_TEST) $(TEST_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(DEV_CFLAGS) $(CFLAGS) $(TEST_SRCS) $(LIB_CORE_TEST) \
		-o $(TARGET_TEST) $(LDFLAGS)
	./$(TARGET_TEST)

leaks: $(TARGET_TEST_LEAKS)
	ASAN_OPTIONS=detect_leaks=1 \
	UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
	./$(TARGET_TEST_LEAKS)

lint: check-layers format-check
	@command -v $(CPPCHECK) >/dev/null 2>&1 || { \
		echo "error: $(CPPCHECK) is required for linting" >&2; \
		exit 1; \
	}
	$(CPPCHECK) --enable=warning,performance,portability \
		--error-exitcode=1 \
		--suppress=missingIncludeSystem \
		--suppress='*:*third_party/*' \
		-Iinclude -Ithird_party \
		src tests

format:
	@if [ -z "$(FORMAT_FILES)" ]; then \
		echo "format: no C sources or headers found" >&2; \
		exit 1; \
	fi
	$(CLANG_FORMAT) -i $(FORMAT_FILES)

format-check:
	@if [ -z "$(FORMAT_FILES)" ]; then \
		echo "format-check: no C sources or headers found" >&2; \
		exit 1; \
	fi
	$(CLANG_FORMAT) --dry-run --Werror $(FORMAT_FILES)

check-layers:
	@echo "Checking layer boundaries..."
	@if grep -rn --include='*.c' --include='*.h' 'termbox2\.h' $(SRC_DIR) \
	    | grep -v '^$(SRC_DIR)/tui/'; then \
	    echo "ERROR: termbox2.h outside $(SRC_DIR)/tui"; exit 1; \
	fi
	@if grep -rn --include='*.c' --include='*.h' -E 'termbox2\.h|\btb_' $(SRC_DIR)/screens $(SRC_DIR)/app; \
	    then echo "ERROR: screens/app touch termbox2"; exit 1; \
	fi
	@if grep -rn --include='*.c' --include='*.h' \
	    -E '#include "(app|tui|screens)/|#include "(app|ui|render|input)\.h"' \
	    $(SRC_DIR)/core $(SRC_DIR)/common; \
	    then echo "ERROR: core/common depends on a upper layer"; exit 1; \
	fi
	@if grep -rn --include='*.c' --include='*.h' '\bapp\.' $(SRC_DIR)/core $(SRC_DIR)/common; \
	    then echo "ERROR: core/common touches app state"; exit 1; \
	fi
	@if [ "$$(grep -rl --include='*.c' --include='*.h' '^#define TB_IMPL' $(SRC_DIR) | wc -l)" -ne 1 ]; then \
	    echo "ERROR: TB_IMPL must be defined exactly once"; exit 1; \
	fi

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

-include $(DEPS)
