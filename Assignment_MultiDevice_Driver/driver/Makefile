# Kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# List of driver source files (without extensions)
DRIVERS = drv pressure flow volt

# Convert driver names to kernel object format
obj-m += $(DRIVERS:%=%.o)

# Default target: Build all kernel modules
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean target
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean

