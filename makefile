# use "gcc" to compile source files.
CC = gcc
# the linker is also "gcc".
LD = gcc

# Compiler flags
CFLAGS_COMMON  = -Wall -Wextra -Wpedantic
CFLAGS_DEBUG   = $(CFLAGS_COMMON) -O0 -g -fsanitize=address -fanalyzer
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2 -DNDEBUG
CFLAGS ?= $(CFLAGS_DEBUG)

# Linker flags
LDFLAGS_COMMON  = -lstrophe
LDFLAGS_DEBUG   = $(LDFLAGS_COMMON) -fsanitize=address
LDFLAGS_RELEASE = $(LDFLAGS_COMMON)
LDFLAGS ?= $(LDFLAGS_DEBUG)

# Remove command
RM = /bin/rm -f

# Objects and target
OBJS = nwws_client.o file_io.o alarm_timer.o text_utils.o xmpp_connect.o xmpp_ping.o log.o
PROG = nwws_client

# Default target
all: $(PROG)

# Debug build
debug:
	$(MAKE) CFLAGS="$(CFLAGS_DEBUG)" LDFLAGS="$(LDFLAGS_DEBUG)" all

# Release build
release:
	$(MAKE) CFLAGS="$(CFLAGS_RELEASE)" LDFLAGS="$(LDFLAGS_RELEASE)" all

# Link rule
$(PROG): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(PROG)

# Clean generated files
clean:
	$(RM) $(PROG) $(OBJS)
