#include "../include/main.h"
#include "../include/gift64.h"

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dst, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

volatile unsigned long bench_cycles[3] = {0, 0, 0};

#define ERROR_CODE 0x42

#ifdef TEST_VECTOR_3

// GIFT64 - TEST VECTOR 3

// https://github.com/giftcipher/gift/blob/master/implementations/test%20vectors/GIFT64_test_vector_3.txt

const u8 key_gift[KEY_SIZE] = {0xbd, 0x91, 0x73, 0x1e, 0xb6, 0xbc, 0x27, 0x13,
                               0xa1, 0xf9, 0xf6, 0xff, 0xc7, 0x50, 0x44, 0xe7};

const u8 block0[GIFT64_BLOCK_SIZE] = {0xc4, 0x50, 0xc7, 0x72,
                                      0x7a, 0x9b, 0x8a, 0x7d};

const u8 block1[GIFT64_BLOCK_SIZE] = {0xc4, 0x50, 0xc7, 0x72,
                                      0x7a, 0x9b, 0x8a, 0x7d};

const u8 cipher_expect[GIFT64_BLOCK_SIZE * 2] = {
    // e3 27 28 85 fa 94 ba 8b
    0xe3, 0x27, 0x28, 0x85, 0xfa, 0x94, 0xba, 0x8b,
    0xe3, 0x27, 0x28, 0x85, 0xfa, 0x94, 0xba, 0x8b};

#elif defined TEST_VECTOR_2

// GIFT64 - TEST VECTOR 2

// https://github.com/giftcipher/gift/blob/master/implementations/test%20vectors/GIFT64_test_vector_2.txt

// fe dc ba 98 76 54 32 10
const u8 key_gift[KEY_SIZE] = {
    // fe dc ba 98 76 54 32 10 fe dc ba 98 76 54 32 10
    0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
    0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};

// fe dc ba 98 76 54 32 10
const u8 block0[GIFT64_BLOCK_SIZE] = {0xfe, 0xdc, 0xba, 0x98,
                                      0x76, 0x54, 0x32, 0x10};

const u8 block1[GIFT64_BLOCK_SIZE] = {0xfe, 0xdc, 0xba, 0x98,
                                      0x76, 0x54, 0x32, 0x10};

const u8 cipher_expect[GIFT64_BLOCK_SIZE * 2] = {
    // c1 b7 1f 66 16 0f f5 87
    0xc1, 0xb7, 0x1f, 0x66, 0x16, 0x0f, 0xf5, 0x87,
    0xc1, 0xb7, 0x1f, 0x66, 0x16, 0x0f, 0xf5, 0x87};

#else

// GIFT64 - TEST VECTOR 1

// https://github.com/giftcipher/gift/blob/master/implementations/test%20vectors/GIFT64_test_vector_1.txt

const u8 key_gift[KEY_SIZE] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 1st key
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const u8 block0[GIFT64_BLOCK_SIZE] = {0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00}; // 1st
const u8 block1[GIFT64_BLOCK_SIZE] = {0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00};

const u8 cipher_expect[GIFT64_BLOCK_SIZE * 2] = {
    // f6 2b c3 ef 34 f7 75 ac
    0xf6, 0x2b, 0xc3, 0xef, 0x34, 0xf7, 0x75, 0xac,
    0xf6, 0x2b, 0xc3, 0xef, 0x34, 0xf7, 0x75, 0xac};
#endif

u8 ciphertexts[GIFT64_BLOCK_SIZE * 2] = {0};

static unsigned long get_cycle_count() {
    /* retrieve the CPU cycles using the Cortex-M DWT->CYCCNT register
     * avaialable only starting from CM3
     */

#if (__CORTEX_M >= 0x03U)
    static int dwt_started = 0;
    if (dwt_started == 0) {
        dwt_started = 1;
        /* Enable Tracing */
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#if (__CORTEX_M == 0x07U)
        /* in Cortex M7, the trace needs to be unlocked
         * via the DWT->LAR register with 0xC5ACCE55 value
         */
        DWT->LAR = 0xC5ACCE55;
#endif
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

        /* Reset counter */
        DWT->CYCCNT = 0;
    }

    return (unsigned long)DWT->CYCCNT;
#else
    return 0;
#endif
}

unsigned long gift(u8 *ciphertexts, const u8 key_gift[16], const u8 block0[8],
                   const u8 block1[8]) {
    u32 rkeys[56] = {0};
    gift64_rearrange_key(rkeys, key_gift);
    giftb64_keyschedule(rkeys);
    unsigned long t1 = get_cycle_count();
    gift64_encrypt_block(ciphertexts, rkeys, block0, block1);
    return get_cycle_count() - t1;
}

static void SystemClock_Config(void);
static void Error_Handler(void);
static unsigned long get_cycle_count();

static void SystemClock_Config(void) {
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the
       device is clocked below the maximum system frequency, to update the
       voltage scaling value regarding system frequency refer to product
       datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /* Enable HSI Oscillator and activate PLL with HSI as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 0x10;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

static void Error_Handler(void) {
    /* Turn LED2 on */
    BSP_LED_On(LED2);
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */

    /* Infinite loop */
    while (1) {
    }
}
#endif

int main(void) {
    /* STM32F4xx HAL library initialization:
         - Configure the Flash prefetch, instruction and Data caches
         - Configure the Systick to generate an interrupt each 1 msec
         - Set NVIC Group Priority to 4
         - Global MSP (MCU Support Package) initialization
       */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    for (int i = 0; i < 3; i++) {
        unsigned long cycles = gift(ciphertexts, key_gift, block0, block1);
        bench_cycles[i] = cycles;
#ifndef FORCE_RESULT
        if (memcmp(ciphertexts, cipher_expect, GIFT64_BLOCK_SIZE * 2) != 0) {
            bench_cycles[i] = ERROR_CODE;
        };
#endif
    }

    while (1) {
    }
}