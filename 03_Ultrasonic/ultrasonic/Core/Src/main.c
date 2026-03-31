/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
void delay_us(volatile uint32_t t)
{
    while (t--);
}

/* UART transmit one character */
void uart_tx(char c)
{
    while (!(USART2->ISR & (1 << 7)));   // TXE
    USART2->TDR = c;
}

/* UART transmit string */
void uart_print(char *s)
{
    while (*s)
        uart_tx(*s++);
}

/* UART print number */
void uart_print_num(uint32_t num)
{
    char buf[10];
    int i = 0;

    if (num == 0)
    {
        uart_tx('0');
        return;
    }

    while (num)
    {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    while (i--)
        uart_tx(buf[i]);
}

/* Ultrasonic distance read (cm) */
uint32_t ultrasonic_read(void)
{
    uint32_t time = 0;

    /* TRIG pulse: PA1 */
    GPIOA->ODR &= ~(1 << 1);
    delay_us(20);
    GPIOA->ODR |=  (1 << 1);
    delay_us(50);
    GPIOA->ODR &= ~(1 << 1);

    /* wait for ECHO HIGH (PA0) */
    while (!(GPIOA->IDR & (1 << 0)));

    /* measure ECHO HIGH time */
    while (GPIOA->IDR & (1 << 0))
    {
        time++;
        delay_us(1);
    }
    return (time / 58);
}
int main(void)
{

	uint32_t distance;

	    /* Enable clocks */
	    RCC->AHB2ENR  |= (1 << 0);     // GPIOA
	    RCC->AHB2ENR  |= (1 << 2);     // GPIOC
	    RCC->APB1ENR1 |= (1 << 17);    // USART2

	    /* PA1 as OUTPUT (TRIG) */
	    GPIOA->MODER &= ~(3 << (1 * 2));
	    GPIOA->MODER |=  (1 << (1 * 2));

	    /* PA0 as INPUT (ECHO) */
	    GPIOA->MODER &= ~(3 << (0 * 2));

	    /* PA2, PA3 as Alternate Function (USART2) */
	    GPIOA->MODER &= ~(3 << (2 * 2));
	    GPIOA->MODER |=  (2 << (2 * 2));

	    GPIOA->MODER &= ~(3 << (3 * 2));
	    GPIOA->MODER |=  (2 << (3 * 2));

	    /* AF7 for USART2 */
	    GPIOA->AFR[0] &= ~(0xF << (2 * 4));
	    GPIOA->AFR[0] &= ~(0xF << (3 * 4));
	    GPIOA->AFR[0] |=  (7 << (2 * 4));
	    GPIOA->AFR[0] |=  (7 << (3 * 4));

	    /* PC13 as INPUT (Button) */
	    GPIOC->MODER &= ~(3 << (13 * 2));

	    /* Pull-up for PC13 */
	    GPIOC->PUPDR &= ~(3 << (13 * 2));
	    GPIOC->PUPDR |=  (1 << (13 * 2));

	    /* USART2: 9600 baud (default MSI ~4MHz) */
	    USART2->BRR = 4000000 / 9600;
	    USART2->CR1 = 0;
	    USART2->CR1 |= (1 << 3);   // TE
	    USART2->CR1 |= (1 << 2);   // RE
	    USART2->CR1 |= (1 << 0);   // UE

	    while (1)
	    {
	        /* Button pressed (ACTIVE LOW) */
	        if ((GPIOC->IDR & (1 << 13)) == 0)
	        {
	            delay_us(200000);   // debounce

	            distance = ultrasonic_read();

	            uart_print("Distance: ");
	            uart_print_num(distance);
	            uart_print(" cm\r\n");

	            /* wait till button released */
	            while ((GPIOC->IDR & (1 << 13)) == 0);
	            delay_us(200000);   // debounce
	        }
	    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
