#include "../include/main.h"
#include "../include/gift64.h"

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dst, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

volatile unsigned long bench_cycles[3] = {0, 0, 0};

#define ERROR_CODE 0x42

#ifdef TEST_VECTOR_3

const u8 key_gift[KEY_SIZE] = {0xbd, 0x91, 0x73, 0x1e, 0xb6, 0xbc, 0x27, 0x13,
                               0xa1, 0xf9, 0xf6, 0xff, 0xc7, 0x50, 0x44, 0xe7};

const u8 block0[GIFT64_BLOCK_SIZE] = {0xc4, 0x50, 0xc7, 0x72,
                                      0x7a, 0x9b, 0x8a, 0x7d}; // 1st

const u8 block1[GIFT64_BLOCK_SIZE] = {0xc4, 0x50, 0xc7, 0x72,
                                      0x7a, 0x9b, 0x8a, 0x7d};

const u8 cipher_expect[GIFT64_BLOCK_SIZE * 2] = {
    // e3 27 28 85 fa 94 ba 8b
    0xe3, 0x27, 0x28, 0x85, 0xfa, 0x94, 0xba, 0x8b,
    0xe3, 0x27, 0x28, 0x85, 0xfa, 0x94, 0xba, 0x8b};

#elif defined TEST_VECTOR_2

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

#define is_bit_set(bytes, shift) (((bytes) & (1 << (shift))) == (1 << (shift)))

void bit_set(u8 *dst, const u8 *slice, size_t dst_b, size_t src_b) {
    u8 byte_src = src_b / 8;
    u8 byte_dst = dst_b / 8;
    u8 shift_dst = dst_b % 8;
    u8 src_bit = is_bit_set(slice[byte_src], (src_b % 8));
    dst[byte_dst] =
        (dst[byte_dst] & ~(1 << shift_dst)) | (src_bit << shift_dst);
}

// Work in progress
void bitslice(u8 *slice) {
    u8 dst[GIFT64_BLOCK_SIZE * 2];
    u8 permutations[128] = {
        114, 38,  82, 6,  115, 39,  83, 7,  112, 36,  80, 4,  113, 37,  81, 5,
        50,  34,  18, 2,  51,  35,  19, 3,  48,  32,  16, 0,  49,  33,  17, 1,
        122, 46,  90, 14, 123, 47,  91, 15, 120, 44,  88, 12, 121, 45,  89, 13,
        58,  42,  26, 10, 59,  43,  27, 11, 56,  40,  24, 8,  57,  41,  25, 9,
        118, 102, 86, 70, 119, 103, 87, 71, 116, 100, 84, 68, 117, 101, 85, 69,
        54,  98,  22, 66, 55,  99,  23, 67, 52,  96,  20, 64, 53,  97,  21, 65,
        126, 110, 94, 78, 127, 111, 95, 79, 124, 108, 92, 76, 125, 109, 93, 77,
        62,  106, 30, 74, 63,  107, 31, 75, 60,  104, 28, 72, 61,  105, 29, 73};

    for (size_t i = 0; i < 128; i += 1) {
        bit_set(dst, slice, permutations[i], i);
    }
    memcpy(slice, dst, sizeof(dst));
}

void unbitslice(u8 *slice) {
    u8 dst[GIFT64_BLOCK_SIZE * 2];
    u8 permutations[128] = {
        25, 29, 17, 21, 89, 93, 81, 85, 57,  61,  49,  53,  121, 125, 113, 117,
        24, 28, 16, 20, 8,  12, 0,  4,  56,  60,  48,  52,  40,  44,  32,  36,
        9,  13, 1,  5,  73, 77, 65, 69, 41,  45,  33,  37,  105, 109, 97,  101,
        88, 92, 80, 84, 72, 76, 64, 68, 120, 124, 112, 116, 104, 108, 96,  100,
        27, 31, 19, 23, 91, 95, 83, 87, 59,  63,  51,  55,  123, 127, 115, 119,
        26, 30, 18, 22, 10, 14, 2,  6,  58,  62,  50,  54,  42,  46,  34,  38,
        11, 15, 3,  7,  75, 79, 67, 71, 43,  47,  35,  39,  107, 111, 99,  103,
        90, 94, 82, 86, 74, 78, 66, 70, 122, 126, 114, 118, 106, 110, 98,  102};
    for (size_t i = 0; i < 128; i += 1) {
        bit_set(dst, slice, permutations[i], i);
    }
    memcpy(slice, dst, sizeof(dst));
}

// Work in progress
unsigned long giftb(u8 *ciphertexts, const u8 *key_gift, const u8 block0[8],
                    const u8 block1[8]) {
    u32 rkeys[56] = {0};
    u8 block[16] = {0};
    memcpy(block, block0, 8);
    memcpy(block + 8, block1, 8);
    unbitslice(block);
    gift64_rearrange_key(rkeys, key_gift);
    giftb64_keyschedule(rkeys);
    unsigned long start = get_cycle_count();
    giftb64_encrypt_block(ciphertexts, rkeys, block, block + 8);
    unsigned long end = get_cycle_count();
    unbitslice(ciphertexts);
    return end - start;
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

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSI)
 *            SYSCLK(Hz)                     = 84000000
 *            HCLK(Hz)                       = 84000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 2
 *            APB2 Prescaler                 = 1
 *            HSI Frequency(Hz)              = 16000000
 *            PLL_M                          = 16
 *            PLL_N                          = 336
 *            PLL_P                          = 4
 *            PLL_Q                          = 7
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale2 mode
 *            Flash Latency(WS)              = 2
 * @param  None
 * @retval None
 */
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

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
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

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/* Private functions ---------------------------------------------------------*/

// Returns 0 if [lhs] != [rhs] for the n bytes
int memeq(const void *lhs, const void *rhs, size_t n) {
    for (size_t i = 0; i < n; i += 1) {
        if (((char *)lhs)[i] != ((char *)rhs)[i]) {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {
    /* STM32F4xx HAL library initialization:
         - Configure the Flash prefetch, instruction and Data caches
         - Configure the Systick to generate an interrupt each 1 msec
         - Set NVIC Group Priority to 4
         - Global MSP (MCU Support Package) initialization
       */
    HAL_Init();

    /* Configure the system clock to 84 MHz */
    SystemClock_Config();

    for (int i = 0; i < 3; i++) {

#ifdef BITSLICE
        unsigned long cycles = giftb(ciphertexts, key_gift, block0, block1);
#else
        unsigned long cycles = gift(ciphertexts, key_gift, block0, block1);
#endif
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