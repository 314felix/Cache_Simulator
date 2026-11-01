# Authors: Øyvind Nohr

CC = gcc
CFLAGS = -Wall -Werror -Wno-unused -g -O2
OBJDIR = obj

PROGRAM = cachesim
TEST_PROGRAM = cachetest

OBJS = cpu.o memory.o cache.o
TEST_OBJS = test.o memory.o cache.o

HEADERS = memory.h cache.h

all: $(PROGRAM)

.PHONY: all test clean dirs

dirs:
	@mkdir -p $(OBJDIR)

# Main program
$(PROGRAM): $(patsubst %, $(OBJDIR)/%, $(OBJS))
	$(CC) $(CFLAGS) $^ -o $@ -lm

# Test program
$(TEST_PROGRAM): $(patsubst %, $(OBJDIR)/%, $(TEST_OBJS))
	$(CC) $(CFLAGS) $^ -o $@ -lm

# Compile test program
test: $(TEST_PROGRAM)

# Compile object files
$(OBJDIR)/%.o: %.c $(HEADERS) | dirs
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o *~ $(PROGRAM) $(TEST_PROGRAM) $(OBJDIR)