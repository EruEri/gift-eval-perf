# GIFT EVAL PERF

A little benchmark to count the number of cycles used to run the fixsliced version of gift64.

We test 2 configurations:
- One with the code on the ram
- One with the code on the ccram

## How to run
```sh
    # The number of cycles will be outputed to ciphers/gift64.log file
    $ make ciphers/gift64.log
    # The number of cycles will be outputed to ciphers/gift64.ccram.log file
    $ make ciphers/gift64.ccram.log
```

## Results

| iteration     | 0   | 1   | 2   |
|----           |---- |---- |---- |
| ram   (cyles) | 876 | 870 | 870 |
| ccram (cyles) | 847 | 840 | 840 |


## Hardware

We run our test on a [STM32F407VG](https://www.st.com/en/microcontrollers-microprocessors/stm32f407vg.html) microcontroller.

Compiled with arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi on
Fedora 41, kernel-version 6.11.10

## GIFT64 version

We use the Fully unrolled ARM assembly implementation of the GIFT-64 block cipher of `Alexandre Adomnicai` 
> https://github.com/aadomn/gift/blob/master/crypto_bc/gift64/armcortexm_fast/gift64.s

## Data

We use the $3^{rd}$ test vector
> https://github.com/giftcipher/gift/blob/master/implementations/test%20vectors/GIFT64_test_vector_3.txt

## Requirements

You will need of:
- [open-ocd](https://openocd.org/)
- arm-gnu-toolchain-arm-none-eabi
- netcat
- make (GNU make)