# Created from https://github.com/davisjp1822/stm32_nucleo_linux

CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar
OBJCOPY=arm-none-eabi-objcopy
OPENOCD=openocd
SCREEN=screen
OPENOCD_PREFIX=/usr
LINKER_CCRAM=linker/STM32F407IGHX_FLASH.ccram.ld
LINKER_RAM=linker/STM32F407IGHX_FLASH.ld
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
	-DSTM32F407xx   \
	-DNUCLEO

INCLUDES=-I./include							\
	  -I./ciphers/arch                                                \
	  -I./STM32Cube_FW/Drivers/CMSIS/Device/ST/STM32F4xx/include	\
	  -I./STM32Cube_FW/Drivers/CMSIS/include			\
	  -I./STM32Cube_FW/Drivers/BSP/STM32F4xx-Nucleo			\
	  -I./STM32Cube_FW/Drivers/STM32F4xx_HAL_Driver/include




ifeq ($(shell test $(OS) = FreeBSD), 0)
	OPENOCD_PREFIX=/usr/local
endif

ifeq ($(shell test $(OS) = Darwin), 0)
	ifeq ($(shell test $(ARCH) = arm64), 0)
		OPENOCD_PREFIX=/opt/homebrew
	else ifeq ($(shell test $(ARCH) = x86_64"), 0)
		OPENOCD_PREFIX=/usr/local/homebrew
	endif
	
endif


################################################################
# Global setup

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
%.o: %.s
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.PRECIOUS: %.o

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

CIPHERS=ciphers/gift.s

all: gift64 gift64.ccram

gift64: ciphers/gift64.elf

gift64.ccram: ciphers/gift64.ccram.elf



%.elf: %.o $(SRC_OBJS) $(CIPHERS_OBJS) lib/libstm32f4xxhal.a lib/libstm32f4xxbsp.a
	$(CC) $(CFLAGS) -T$(LINKER_RAM) $<					\
		src/stm32f4xx_it.o 			\
		src/syscalls.o src/system_stm32f4xx.o				\
		src/startup_stm32f407xx.o				\
		src/main.o -o $@ $(LD_FLAGS)

%.ccram.elf: %.o $(SRC_OBJS) $(CIPHERS_OBJS) lib/libstm32f4xxhal.a lib/libstm32f4xxbsp.a
	$(CC) $(CFLAGS) -T$(LINKER_CCRAM) $<					\
		src/stm32f4xx_it.o 			\
		src/syscalls.o src/system_stm32f4xx.o				\
		src/startup_stm32f407xx.o				\
		src/main.o -o $@ $(LD_FLAGS)

################################################################
# Cleaning

clean:
	rm -f lib/libstm32f4xxhal.a		\
	      lib/libstm32f4xxbsp.a		\
	      $(BSP_LIB_OBJS) $(HAL_LIB_OBJS)	\
	      $(SRC_OBJS)			\
	      $(CIPHERS_BINS)			\

clean-all:
	make clean

################################################################
# Interactions with the board

# Restart the board
reboot:
	sudo $(OPENOCD) -f $(OPENOCD_PREFIX)/share/openocd/scripts/board/st_nucleo_f4.cfg \
	                -c "init; reset; exit"

# Load .hex file to the board
%.upload: %.hex
	sudo $(OPENOCD) -f $(OPENOCD_PREFIX)/share/openocd/scripts/board/st_nucleo_f4.cfg \
	                -c "init; reset halt; flash write_image erase $<; reset run; exit"

# Save serial input to the given file
%.log: %.hex force
	sh ./bin/serial.sh -b $*.elf -x $< -o $@

%.hex: %.elf
	objcopy -Oihex $*.elf $*.hex

force:
	true
