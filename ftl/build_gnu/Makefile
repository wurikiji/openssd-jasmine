FTL	= greedy
PREFIX 	= arm-none-eabi-
CC 	= $(PREFIX)gcc
AS 	= $(PREFIX)as
LD 	= $(PREFIX)ld
OBJCOPY	= $(PREFIX)objcopy
RM	= rm

INCLUDES = -I../include -I../ftl_$(FTL) -I../sata -I../target_spw -L./
CFLAGS 	= -mcpu=arm7tdmi-s -mthumb-interwork -ffreestanding -nostdlib -std=c99 -O3 -DPROGRAM_MAIN_FW -Wall
ASFLAGS	= -R -mcpu=arm7tdmi-s
LDFLAGS	= -static -nostartfiles -ffreestanding -T ld_script -Wl,-O1,-Map=list.txt
LIBS	= -lgcc 
VPATH	= ../ftl_$(FTL):../sata:..:../target_spw:../installer

SRCS 	= ftl.c sata_identify.c sata_cmd.c sata_isr.c sata_main.c sata_table.c initialize.c mem_util.c flash.c flash_wrapper.c misc.c uart.c
INITSRC	= ../target_spw/init_gnu.s
OBJS	= $(SRCS:.c=.o) init.o
DEPS	= $(SRCS:.c=.d) installer-linux.d
TARGET 	= firmware
TARGETELF = $(TARGET).elf
TARGETBIN = $(TARGET).bin

all: $(TARGETBIN) installer

$(TARGETBIN): $(TARGETELF)
	$(OBJCOPY) -O binary --strip-all --strip-debug $< $@

$(TARGETELF): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

%.d: %.c
	$(CC) -M $(CFLAGS) -c $(INCLUDES) $< > $@

.c.o:
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

init.o: $(INITSRC)
	$(AS) $(ASFLAGS) $< -o $@

installer: installer-linux.o
	gcc -Wall -W $(INCLUDES) $< -o $@ 

installer-linux.o:
	gcc -Wall -W -c $(INCLUDES) $< -o $@

installer-linux.d: installer-linux.c
	gcc -M -Wall -W -c $(INCLUDES) $< > $@

clean:
	$(RM) $(OBJS) $(DEPS) $(TARGETELF) $(TARGETBIN) installer-linux.o installer


-include $(DEPS)