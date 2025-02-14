# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# List of driver names (without extension)
DRIVERS = temp_dev flowm_dev pressure_dev

# Object files generated from driver source files
OBJS = $(DRIVERS:%=%.o)

# Default target builds all drivers
all: $(DRIVERS)

# Rule for linking a driver executable from its object file
$(DRIVERS): %: %.o $(CC) $(CFLAGS) -o $@ $^


# Rule for compiling each .c file into an object file
%.o: %.c $(CC) $(CFLAGS) -c $< -o $@


# Clean target to remove object files and executables
clean: rm -f $(OBJS) $(DRIVERS)


.PHONY: all clean

	

