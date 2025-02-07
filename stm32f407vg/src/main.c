/**
 ******************************************************************************
 * @file    UART/UART_Printf/Src/main.c
 * @author  MCD Application Team
 * @brief   This example shows how to retarget the C library printf function
 *          to the UART.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *notice, this list of conditions and the following disclaimer in the
 *documentation and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "../include/main.h"
#include "../include/gift64.h"

#include <stddef.h>
#include <stdint.h>

volatile unsigned long bench_cycles[3] = {-1, -1, -1};

const u8 key_gift[KEY_SIZE] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 1st key
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const u8 block0[GIFT64_BLOCK_SIZE] = {0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00}; // 1st plaintext
const u8 block1[GIFT64_BLOCK_SIZE] = {0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00};

const u8 expect_ciphertext0[GIFT64_BLOCK_SIZE * 2] = {
    // f6 2b c3 ef 34 f7 75 ac
    0xf6, 0x2b, 0xc3, 0xef, 0x34, 0xf7, 0x75, 0xac,
    0xf6, 0x2b, 0xc3, 0xef, 0x34, 0xf7, 0x75, 0xac};

// const u8 key_gift[KEY_SIZE] = {0xbd, 0x91, 0x73, 0x1e, 0xb6, 0xbc, 0x27,
// 0x13,
//                                0xa1, 0xf9, 0xf6, 0xff, 0xc7, 0x50, 0x44,
//                                0xe7};

// const u8 block0[GIFT64_BLOCK_SIZE] = {0xc4, 0x50, 0xc7, 0x72,
//                                       0x7a, 0x9b, 0x8a, 0x7d}; // 1st
//                                       plaintext
// const u8 block1[GIFT64_BLOCK_SIZE] = {0xc4, 0x50, 0xc7, 0x72,
//                                       0x7a, 0x9b, 0x8a, 0x7d};

u8 ciphertexts[GIFT64_BLOCK_SIZE * 2] = {0};

/**
 *
 * Portions COPYRIGHT 2018 STMicroelectronics
 * Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *
 ******************************************************************************
 * @file    timing_alt_template.c[
 * @author  MCD Application Team
 * @brief   mbedtls alternate timing functions implementation.
 *          mbedtls timing API is implemented using the CMSIS-RTOS v1/v2 API
 *          this file has to be reamed to timing_alt.c and copied under
 *          the project tree.
 ******************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

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

unsigned long gift() {
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

        // bench_lens[i] = len;
        unsigned long cycles = gift();
        bench_cycles[i] = cycles;
        if (!memeq(ciphertexts, expect_ciphertext0, GIFT64_BLOCK_SIZE * 2)) {
            bench_cycles[i] = 0;
        };
    }

    while (1) {
    }
}