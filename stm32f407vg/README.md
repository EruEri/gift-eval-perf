# GIFT-EVAL-PERF - STM32F407VG

## Hardware:
The benchmark are run on on [STM32F407VG](https://www.st.com/en/microcontrollers-microprocessors/stm32f407vg.html) microcontroller

We test 2 implementations of the gift64 ciphers of `Alexandre Adomnicai`:
- [Fully unrolled](https://github.com/aadomn/gift/blob/master/crypto_bc/gift64/armcortexm_fast/gift64.s)
- [Compact (code size)](https://github.com/aadomn/gift/blob/master/crypto_bc/gift64/armcortexm_compact/gift64.s) 

On two different memory area
- the ram
- the ccmram

## How to run
```sh
    # The number of cycles will be outputed to ciphers/gift64.unroll.cycles file
    $ make ciphers/gift64.unroll.cycles
    # The number of cycles will be outputed to stm32f407vg/ciphers/gift64.ccram.log file
    $ make  ciphers/gift64.unroll.ccmram.cycles
    # The number of cycles will be outputed to ciphers/gift64.unroll.cycles file
    $ make ciphers/gift64.compact.cycles
    # The number of cycles will be outputed to stm32f407vg/ciphers/gift64.ccram.log file
    $ make  ciphers/gift64.compact.ccmram.cycles
```

By default the benchmark runs the [gift64 test-vector 1](https://github.com/giftcipher/gift/blob/master/implementations/test%20vectors/GIFT64_test_vector_1.txt), but by changing the Makefile variable `TEST_VECTOR`, you can change the test-vector.
Any others value than `1`, `2` or `3` default to the test-vector 1.

```sh
    $ make TEST_VECTOR=2 ciphers/gift64.compact.cycles
```

By default, if the encrypted cipher doesn't match the expected one, an error code of `0x42` is outputted instead of the actual cycles.
To force the output to cycles, set the Makefile variable `FORCE_RESULT` to `1`.

```sh
    $ make FORCE_RESULT=1 ciphers/gift64.unroll.cycles
```

To also write the encrypted cipher.

```sh
    $ make ciphers/gift64.compact.cipher
```

## Results


| iteration      | 0   | 1   | 2   |
|----            |---- |---- |---- |
| ram   (cycles) | 875 | 868 | 868 |
| ccram (cycles) | 844 | 837 | 837 |

: Number of cycles used to encrypt two blocks on STM32F407VG using gift64 unroll implementation

| iteration      | 0   | 1   | 2   |
|----            |---- |---- |---- |
| ram   (cycles) | 944 | 930 | 930 |
| ccram (cycles) | 912 | 897 | 897 |

: Number of cycles used to encrypt two blocks on STM32F407VG using gift64 compact implementation

Compiled with arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi


## Requirements

You will need of:
- [open-ocd](https://openocd.org/)
- arm-gnu-toolchain-arm-none-eabi
- netcat
- make (GNU make)