# Created from https://github.com/davisjp1822/stm32_nucleo_linux

CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar
OBJCOPY=arm-none-eabi-objcopy
OPENOCD=openocd
SCREEN=screen
OPENOCD_PREFIX=/usr
LINKER_RAM=linker/STM32F401RE_FLASH.ld
LIBDIR=./lib
OS=$(shell uname -s)
ARCH=$(shell uname -m)

LD_FLAGS=-L${LIBDIR}		\
	-lstm32f4xxbsp	\
	-lstm32f4xxhal	\
    -Wl,--gc-sections

CFLAGS=-Wall		\
	-mcpu=cortex-m4 \
	-mlittle-endian \
	-mthumb		\
	-O3		\
	-DSTM32F401xE   \
	-DNUCLEO

INCLUDES=-I./include							\
	-I./ciphers/arch                                                \
	-I./STM32Cube_FW/Drivers/CMSIS/Device/ST/STM32F4xx/include	\
	-I./STM32Cube_FW/Drivers/CMSIS/include			\
	-I./STM32Cube_FW/Drivers/BSP/STM32F4xx-Nucleo			\
	-I./STM32Cube_FW/Drivers/STM32F4xx_HAL_Driver/include

DEFINES=
FORCE_RESULT=
TEST_VECTOR=
BITSLICE=

.PHONY: src/main.o

ifeq ($(BITSLICE), 1)
DEFINES+= -DBITSLICE
endif


ifeq ($(TEST_VECTOR), 3)
DEFINES+= -DTEST_VECTOR_3
else ifeq ($(TEST_VECTOR), 2)
DEFINES+= -DTEST_VECTOR_2
else 
DEFINES+= -DTEST_VECTOR_1
endif


ifeq ($(FORCE_RESULT), 1)
DEFINES+= -DFORCE_RESULT
endif


################################################################
# Global setup

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
%.o: %.s
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

src/main.o: src/main.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c -o $@ $<

# .PRECIOUS: %.o

################################################################
# HAL library

HAL_LIB_FILES = $(wildcard STM32Cube_FW/Drivers/STM32F4xx_HAL_Driver/src/*.c)
HAL_LIB_OBJS = $(patsubst %.c,%.o, $(HAL_LIB_FILES))

lib/libstm32f4xxhal.a: $(HAL_LIB_OBJS)
	$(AR) rcs $@ $^

################################################################
# BSP library

BSP_LIB_FILES = $(wildcard STM32Cube_FW/Drivers/BSP/STM32F4xx-Nucleo/*.c)
BSP_LIB_OBJS = $(patsubst %.c,%.o, $(BSP_LIB_FILES))

lib/libstm32f4xxbsp.a: $(BSP_LIB_OBJS)
	$(AR) rcs $@ $^

################################################################
# Benchmarks

SRC_FILES = $(wildcard src/*.c src/*.s)
SRC_OBJS = $(patsubst %.s,%.o, $(patsubst %.c,%.o, $(SRC_FILES)))

CIPHERS_FILES = $(wildcard ciphers/*.c ciphers/*.s)
CIPHERS_OBJS = $(patsubst %.s,%.o, $(patsubst %.c,%.o, $(CIPHERS_FILES)))



%.elf: %.o $(SRC_OBJS) $(CIPHERS_OBJS) lib/libstm32f4xxhal.a lib/libstm32f4xxbsp.a
	$(CC) $(CFLAGS) $(DEFINES) -T$(LINKER_RAM) $<					\
		src/stm32f4xx_it.o 			\
		src/syscalls.o src/system_stm32f4xx.o				\
		src/startup_stm32f401xe.o				\
		src/main.o -o $@ $(LD_FLAGS)


################################################################
# Cleaning

clean:
	rm -f lib/libstm32f4xxhal.a		\
	      lib/libstm32f4xxbsp.a		\
	      $(BSP_LIB_OBJS) $(HAL_LIB_OBJS)	\
	      $(SRC_OBJS)			\
	      $(CIPHERS_BINS)			\

clean-res:
	find ciphers -type f -name '*.elf' -delete
	find ciphers -type f -name '*.o' -delete
	find ciphers -type f -name '*.log' -delete
	find ciphers -type f -name '*.cycles' -delete
	find ciphers -type f -name '*.cipher' -delete

clean-all:
	make clean
	make clean-res

################################################################
# Interactions with the board

# Restart the board
reboot:
	$(OPENOCD) -f board/st_nucleo_f4.cfg \
	                -c "init; reset; exit"

# Load .hex file to the board
%.upload: %.hex
	$(OPENOCD) -f board/st_nucleo_f4.cfg \
	                -c "init; reset halt; flash write_image erase $<; reset run; exit"

# Save serial input to the given file
%.cycles: %.hex force
	sh ./bin/serial.sh -b $*.elf -x $< -o $@

%.cipher: %.hex force
	sh ./bin/serial.sh -b $*.elf -x $< -o $*.cycles -c $@

%.hex: %.elf
	$(OBJCOPY) -Oihex $*.elf $*.hex

force:
	true
