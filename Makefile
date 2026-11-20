CC          ?= gcc
AR          ?= ar
override CPPFLAGS += -Iinclude -isystem third_party -MMD -MP -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE
override CFLAGS   += -std=gnu99 \
                    -Wall -Wextra -Werror \
                    -pedantic \
                    -Wshadow \
                    -Wconversion \
                    -Wstrict-prototypes \
                    -Wmissing-prototypes \
                    -Wformat=2 \
                    -Wundef \
                    -fno-common
LDFLAGS     +=

DEV_CFLAGS  := -g -O0 -DDEBUG
REL_CFLAGS  := -O3 -DNDEBUG -ffunction-sections -fdata-sections

LEAKS_CFLAGS  := -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined
LEAKS_LDFLAGS := -fsanitize=address -fsanitize=undefined

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
    override CPPFLAGS += -D_DARWIN_C_SOURCE
    DEV_CFLAGS  += -gdwarf-4
    REL_LDFLAGS := -Wl,-dead_strip
else ifneq (,$(filter $(UNAME_S),Linux Android))
    REL_LDFLAGS := -Wl,--gc-sections -s
else
    $(error Platform $(UNAME_S) not supported)
endif

BINDIR := bin
OBJDIR := obj
SRCDIR := src

TARGET       := $(BINDIR)/vlsx
TARGET_DEV   := $(BINDIR)/vlsx-dev
TARGET_LEAKS := $(BINDIR)/vlsx-leaks
TARGET_TEST  := $(BINDIR)/vlsx-tests

SRCS := $(shell find $(SRCDIR) -name '*.c')

CORE_SRCS := $(shell find $(SRCDIR)/core $(SRCDIR)/common -name '*.c')
CORE_OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/core/%.o,$(CORE_SRCS))
LIB_CORE  := $(OBJDIR)/libvlsx_core.a

TEST_SRCS := $(wildcard tests/*.c)

OBJS_REL   := $(foreach f,$(SRCS),$(OBJDIR)/release/$(patsubst $(SRCDIR)/%,%,$(f:.c=.o)))
OBJS_DEV   := $(foreach f,$(SRCS),$(OBJDIR)/dev/$(patsubst $(SRCDIR)/%,%,$(f:.c=.o)))
OBJS_LEAKS := $(foreach f,$(SRCS),$(OBJDIR)/leaks/$(patsubst $(SRCDIR)/%,%,$(f:.c=.o)))

DEPS := $(OBJS_REL:.o=.d) $(OBJS_DEV:.o=.d) $(OBJS_LEAKS:.o=.d)

.PHONY: all dev leaks test check-layers clean

all: check-layers $(TARGET)

dev: $(TARGET_DEV)

leaks: $(TARGET_LEAKS)

test: $(LIB_CORE) | $(BINDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEV_CFLAGS) $(TEST_SRCS) $(LIB_CORE) -o $(TARGET_TEST)
	./$(TARGET_TEST)

check-layers:
	@echo "Checking layer boundaries..."
	@if grep -rn --include='*.c' --include='*.h' 'termbox2\.h' $(SRCDIR) \
	    | grep -v '^$(SRCDIR)/tui/'; then \
	    echo "ERROR: termbox2.h outside $(SRCDIR)/tui"; exit 1; \
	fi
	@if grep -rn --include='*.c' --include='*.h' -E 'termbox2\.h|\btb_' $(SRCDIR)/screens $(SRCDIR)/app; \
	    then echo "ERROR: screens/app touch termbox2"; exit 1; \
	fi
	@if grep -rn --include='*.c' --include='*.h' \
	    -E '#include "(app|tui|screens)/|#include "(app|ui|render|input)\.h"' \
	    $(SRCDIR)/core $(SRCDIR)/common; \
	    then echo "ERROR: core/common depends on a upper layer"; exit 1; \
	fi
	@if grep -rn --include='*.c' --include='*.h' '\bapp\.' $(SRCDIR)/core $(SRCDIR)/common; \
	    then echo "ERROR: core/common touches app state"; exit 1; \
	fi
	@if [ "$$(grep -rl --include='*.c' --include='*.h' '^#define TB_IMPL' $(SRCDIR) | wc -l)" -ne 1 ]; then \
	    echo "ERROR: TB_IMPL must be defined exactly once"; exit 1; \
	fi

$(TARGET): $(OBJS_REL) | $(BINDIR)
	$(CC) $(CFLAGS) $(REL_CFLAGS) $^ -o $@ $(LDFLAGS) $(REL_LDFLAGS)

$(TARGET_DEV): $(OBJS_DEV) | $(BINDIR)
	$(CC) $(CFLAGS) $(DEV_CFLAGS) $^ -o $@ $(LDFLAGS)

$(TARGET_LEAKS): $(OBJS_LEAKS) | $(BINDIR)
	$(CC) $(CFLAGS) $(LEAKS_CFLAGS) $^ -o $@ $(LDFLAGS) $(LEAKS_LDFLAGS)

$(LIB_CORE): $(CORE_OBJS)
	$(AR) rcs $@ $^

$(OBJDIR)/release/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(REL_CFLAGS) -c $< -o $@

$(OBJDIR)/dev/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEV_CFLAGS) -c $< -o $@

$(OBJDIR)/leaks/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LEAKS_CFLAGS) -c $< -o $@

$(OBJDIR)/core/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(REL_CFLAGS) -c $< -o $@

$(BINDIR):
	mkdir -p $@

-include $(DEPS)

clean:
	rm -rf $(BINDIR) $(OBJDIR)
