

TOOL_PREFIX=aarch64-linux-musl-
# TOOL_PREFIX=aarch64-none-elf-

INCLUDE = -I $(realpath ./include)

BUILD=build

# arguments
GUEST=y
SMP=1

CFLAGS= -g -c -O0 -fno-pie -mgeneral-regs-only -DT_SMP_NUM=$(SMP)


ifeq ($(GUEST),y)
GUEST_LABEL = "[guest:0] "
LOAD_ADDR = 0x70200000
CFLAGS += '-DGUEST_LABEL="$(GUEST_LABEL)"'
else
LOAD_ADDR = 0x40080000
endif

.PHONY: all clean debug run $(BUILD)


all: 
	$(TOOL_PREFIX)gcc  $(CFLAGS) t_dmos.S $(INCLUDE) -o $(BUILD)/dmos.s.o
	$(TOOL_PREFIX)gcc  $(CFLAGS) t_dmos.c $(INCLUDE) -o $(BUILD)/dmos.o
	$(TOOL_PREFIX)gcc  $(CFLAGS) t_gic.c $(INCLUDE) -o $(BUILD)/gic.o
	$(TOOL_PREFIX)gcc  $(CFLAGS) t_exception.c $(INCLUDE) -o $(BUILD)/exception.o
	$(TOOL_PREFIX)gcc  $(CFLAGS) t_exception.S $(INCLUDE) -o $(BUILD)/exception.s.o
	$(TOOL_PREFIX)gcc  $(CFLAGS) t_string.c $(INCLUDE) -o $(BUILD)/string.o
	$(TOOL_PREFIX)gcc  $(CFLAGS) t_print.c $(INCLUDE) -o $(BUILD)/print.o
	$(TOOL_PREFIX)gcc  $(CFLAGS) t_uart.c $(INCLUDE) -o $(BUILD)/uart.o
	$(TOOL_PREFIX)gcc  $(CFLAGS) t_spinlock.S $(INCLUDE) -o $(BUILD)/spinlock.s.o
	$(TOOL_PREFIX)gcc  $(CFLAGS) t_task.c $(INCLUDE) -o $(BUILD)/task.o
	$(TOOL_PREFIX)gcc  $(CFLAGS) t_list.c $(INCLUDE) -o $(BUILD)/list.o

	$(TOOL_PREFIX)ld -T link.lds --defsym=__LOAD_ADDR__=$(LOAD_ADDR) -o $(BUILD)/kernel.elf \
		$(BUILD)/dmos.s.o\
		$(BUILD)/spinlock.s.o\
		$(BUILD)/print.o\
		$(BUILD)/string.o\
		$(BUILD)/uart.o\
		$(BUILD)/dmos.o\
		$(BUILD)/gic.o\
		$(BUILD)/exception.o\
		$(BUILD)/exception.s.o\
		$(BUILD)/task.o\
		$(BUILD)/list.o

	
	$(TOOL_PREFIX)objdump -x -d -S $(BUILD)/kernel.elf > $(BUILD)/dis.txt
	$(TOOL_PREFIX)readelf -a $(BUILD)/kernel.elf  > $(BUILD)/elf.txt
	$(TOOL_PREFIX)objcopy -O binary $(BUILD)/kernel.elf $(BUILD)/kernel.bin

debug:
	qemu-system-aarch64 -m 4G -smp $(SMP) -cpu cortex-a72 -M virt -M gic_version=2 -nographic -kernel $(BUILD)/kernel.elf -s -S

run:
	qemu-system-aarch64 -m 4G -smp $(SMP) -cpu cortex-a72 -M virt -M gic_version=2 -nographic -kernel $(BUILD)/kernel.elf 


clean:
	rm -f $(BUILD)/*.o
	rm -f $(BUILD)/*.bin
	rm -f $(BUILD)/*.txt
	rm -f $(BUILD)/*.elf