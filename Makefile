CC          ?= gcc
override CPPFLAGS += -Iinclude -Ithird_party -MMD -MP -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE
override CFLAGS   += -Wall -Wextra -std=gnu99
LDFLAGS     +=

DEV_CFLAGS  := -g -O0 -DDEBUG
REL_CFLAGS  := -O3 -DNDEBUG -ffunction-sections -fdata-sections

LEAKS_CFLAGS  := -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined
LEAKS_LDFLAGS := -fsanitize=address -fsanitize=undefined

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
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

SRCS := $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/screens/*.c)

OBJS_REL   := $(foreach f,$(SRCS),$(OBJDIR)/release/$(patsubst $(SRCDIR)/%,%,$(f:.c=.o)))
OBJS_DEV   := $(foreach f,$(SRCS),$(OBJDIR)/dev/$(patsubst $(SRCDIR)/%,%,$(f:.c=.o)))
OBJS_LEAKS := $(foreach f,$(SRCS),$(OBJDIR)/leaks/$(patsubst $(SRCDIR)/%,%,$(f:.c=.o)))

DEPS := $(OBJS_REL:.o=.d) $(OBJS_DEV:.o=.d) $(OBJS_LEAKS:.o=.d)

.PHONY: all dev leaks clean

all: $(TARGET)

dev: $(TARGET_DEV)

leaks: $(TARGET_LEAKS)

$(TARGET): $(OBJS_REL) | $(BINDIR)
	$(CC) $(CFLAGS) $(REL_CFLAGS) $^ -o $@ $(LDFLAGS) $(REL_LDFLAGS)

$(TARGET_DEV): $(OBJS_DEV) | $(BINDIR)
	$(CC) $(CFLAGS) $(DEV_CFLAGS) $^ -o $@ $(LDFLAGS)

$(TARGET_LEAKS): $(OBJS_LEAKS) | $(BINDIR)
	$(CC) $(CFLAGS) $(LEAKS_CFLAGS) $^ -o $@ $(LDFLAGS) $(LEAKS_LDFLAGS)

$(OBJDIR)/release/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(REL_CFLAGS) -c $< -o $@

$(OBJDIR)/dev/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEV_CFLAGS) -c $< -o $@

$(OBJDIR)/leaks/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LEAKS_CFLAGS) -c $< -o $@

$(BINDIR):
	mkdir -p $@

-include $(DEPS)

clean:
	rm -rf $(BINDIR) $(OBJDIR)
