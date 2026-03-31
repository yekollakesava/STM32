#include "main.h"
#include "gpio_hal_driver.h"

/* Function prototypes */
void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* Enable clocks for GPIOA and GPIOC */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Create GPIO handles */
    GPIO_Handle_t led;
    GPIO_Handle_t button;

    /* LED configuration: PA5 */
    led.GPIOx      = GPIOA;
    led.GPIO_Pin   = GPIO_PIN_5;
    led.GPIO_Mode  = GPIO_MODE_OUTPUT_PP;
    led.GPIO_Pull  = GPIO_NOPULL;
    led.GPIO_Speed = GPIO_SPEED_FREQ_LOW;

    /* Button configuration: PC13 */
    button.GPIOx      = GPIOC;
    button.GPIO_Pin   = GPIO_PIN_13;
    button.GPIO_Mode  = GPIO_MODE_INPUT;
    button.GPIO_Pull  = GPIO_NOPULL;
    button.GPIO_Speed = GPIO_SPEED_FREQ_LOW;

    /* Initialize GPIOs */
    GPIO_Driver_Init(&led);
    GPIO_Driver_Init(&button);

    /* Initially LED OFF */
    GPIO_Driver_Write(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    uint8_t ledState = 0;
    uint8_t lastButtonState = 1;

    while (1)
    {
        uint8_t currentButtonState = GPIO_Driver_Read(GPIOC, GPIO_PIN_13);

        /* Detect button press: released -> pressed */
        if ((lastButtonState == 1) && (currentButtonState == 0))
        {
            HAL_Delay(50);   /* debounce */

            if (GPIO_Driver_Read(GPIOC, GPIO_PIN_13) == 0)
            {
                ledState = !ledState;

                if (ledState)
                {
                    GPIO_Driver_Write(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                }
                else
                {
                    GPIO_Driver_Write(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                }

                /* wait until button released */
                while (GPIO_Driver_Read(GPIOC, GPIO_PIN_13) == 0);
                HAL_Delay(50);
            }
        }

        lastButtonState = currentButtonState;
    }
}
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Configure main internal regulator output voltage */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /* Initializes the RCC Oscillators */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Initializes CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
