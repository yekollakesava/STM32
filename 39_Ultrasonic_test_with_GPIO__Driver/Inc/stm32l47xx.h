#ifndef INC_STM32L47XX_H_
#define INC_STM32L47XX_H_

#include <stdint.h>

#define __vo volatile

/**************************************
 * Generic macros
 **************************************/
#define ENABLE                  1
#define DISABLE                 0
#define SET                     ENABLE
#define RESET                   DISABLE

#define GPIO_PIN_SET            SET
#define GPIO_PIN_RESET          RESET

#define FLAG_RESET              0
#define FLAG_SET                1

#define NO_PR_BITS_IMPLEMENTED  4

/**************************************
 * Base addresses
 **************************************/
#define FLASH_BASEADDR          0x08000000U
#define SRAM1_BASEADDR          0x20000000U
#define ROM_BASEADDR            0x1FFF0000U

#define PERIPH_BASEADDR         0x40000000U
#define APB1PERIPH_BASEADDR     PERIPH_BASEADDR
#define APB2PERIPH_BASEADDR     0x40010000U
#define AHB1PERIPH_BASEADDR     0x40020000U
#define AHB2PERIPH_BASEADDR     0x48000000U

/**************************************
 * GPIO base addresses
 **************************************/
#define GPIOA_BASEADDR          (AHB2PERIPH_BASEADDR + 0x0000U)
#define GPIOB_BASEADDR          (AHB2PERIPH_BASEADDR + 0x0400U)
#define GPIOC_BASEADDR          (AHB2PERIPH_BASEADDR + 0x0800U)
#define GPIOD_BASEADDR          (AHB2PERIPH_BASEADDR + 0x0C00U)
#define GPIOE_BASEADDR          (AHB2PERIPH_BASEADDR + 0x1000U)
#define GPIOF_BASEADDR          (AHB2PERIPH_BASEADDR + 0x1400U)
#define GPIOG_BASEADDR          (AHB2PERIPH_BASEADDR + 0x1800U)

/**************************************
 * RCC / EXTI / SYSCFG / SPI / USART / I2C base addresses
 **************************************/
#define RCC_BASEADDR            (AHB1PERIPH_BASEADDR + 0x1000U)
#define EXTI_BASEADDR           (APB2PERIPH_BASEADDR + 0x0400U)
#define SYSCFG_BASEADDR         (APB2PERIPH_BASEADDR + 0x0000U)

#define SPI1_BASEADDR           (APB2PERIPH_BASEADDR + 0x3000U)
#define SPI2_BASEADDR           (APB1PERIPH_BASEADDR + 0x3800U)
#define SPI3_BASEADDR           (APB1PERIPH_BASEADDR + 0x3C00U)

#define USART2_BASEADDR         (APB1PERIPH_BASEADDR + 0x4400U)

#define I2C1_BASEADDR           (APB1PERIPH_BASEADDR + 0x5400U)
#define I2C2_BASEADDR           (APB1PERIPH_BASEADDR + 0x5800U)
#define I2C3_BASEADDR           (APB1PERIPH_BASEADDR + 0x5C00U)

/**************************************
 * Peripheral register definition structures
 **************************************/
typedef struct
{
    __vo uint32_t MODER;
    __vo uint32_t OTYPER;
    __vo uint32_t OSPEEDR;
    __vo uint32_t PUPDR;
    __vo uint32_t IDR;
    __vo uint32_t ODR;
    __vo uint32_t BSRR;
    __vo uint32_t LCKR;
    __vo uint32_t AFR[2];
    __vo uint32_t BRR;
    __vo uint32_t ASCR;
} GPIO_RegDef_t;

typedef struct
{
    __vo uint32_t CR;
    __vo uint32_t ICSCR;
    __vo uint32_t CFGR;
    __vo uint32_t PLLCFGR;
    __vo uint32_t PLLSAI1CFGR;
    __vo uint32_t PLLSAI2CFGR;
    __vo uint32_t CIER;
    __vo uint32_t CIFR;
    __vo uint32_t CICR;
    uint32_t RESERVED0;
    __vo uint32_t AHB1RSTR;
    __vo uint32_t AHB2RSTR;
    __vo uint32_t AHB3RSTR;
    uint32_t RESERVED1;
    __vo uint32_t APB1RSTR1;
    __vo uint32_t APB1RSTR2;
    __vo uint32_t APB2RSTR;
    uint32_t RESERVED2;
    __vo uint32_t AHB1ENR;
    __vo uint32_t AHB2ENR;
    __vo uint32_t AHB3ENR;
    uint32_t RESERVED3;
    __vo uint32_t APB1ENR1;
    __vo uint32_t APB1ENR2;
    __vo uint32_t APB2ENR;
    uint32_t RESERVED4;
    __vo uint32_t AHB1SMENR;
    __vo uint32_t AHB2SMENR;
    __vo uint32_t AHB3SMENR;
    uint32_t RESERVED5;
    __vo uint32_t APB1SMENR1;
    __vo uint32_t APB1SMENR2;
    __vo uint32_t APB2SMENR;
    uint32_t RESERVED6;
    __vo uint32_t CCIPR;
    uint32_t RESERVED7;
    __vo uint32_t BDCR;
    __vo uint32_t CSR;
    __vo uint32_t CRRCR;
    __vo uint32_t CCIPR2;
} RCC_RegDef_t;

typedef struct
{
    __vo uint32_t IMR1;
    __vo uint32_t EMR1;
    __vo uint32_t RTSR1;
    __vo uint32_t FTSR1;
    __vo uint32_t SWIER1;
    __vo uint32_t PR1;
    __vo uint32_t IMR2;
    __vo uint32_t EMR2;
    __vo uint32_t RTSR2;
    __vo uint32_t FTSR2;
    __vo uint32_t SWIER2;
    __vo uint32_t PR2;
} EXTI_RegDef_t;

typedef struct
{
    __vo uint32_t MEMRMP;
    __vo uint32_t CFGR1;
    __vo uint32_t EXTICR[4];
    uint32_t RESERVED1;
    __vo uint32_t SCSR;
    __vo uint32_t CFGR2;
    __vo uint32_t SWPR;
    __vo uint32_t SKR;
} SYSCFG_RegDef_t;

typedef struct
{
    __vo uint32_t CR1;
    __vo uint32_t CR2;
    __vo uint32_t SR;
    __vo uint32_t DR;
    __vo uint32_t CRCPR;
    __vo uint32_t RXCRCR;
    __vo uint32_t TXCRCR;
} SPI_RegDef_t;

typedef struct
{
    __vo uint32_t CR1;
    __vo uint32_t CR2;
    __vo uint32_t CR3;
    __vo uint32_t BRR;
    __vo uint32_t GTPR;
    __vo uint32_t RTOR;
    __vo uint32_t RQR;
    __vo uint32_t ISR;
    __vo uint32_t ICR;
    __vo uint32_t RDR;
    __vo uint32_t TDR;
} USART_RegDef_t;

typedef struct
{
    __vo uint32_t CR1;       /* 0x00 */
    __vo uint32_t CR2;       /* 0x04 */
    __vo uint32_t OAR1;      /* 0x08 */
    __vo uint32_t OAR2;      /* 0x0C */
    __vo uint32_t TIMINGR;   /* 0x10 */
    __vo uint32_t TIMEOUTR;  /* 0x14 */
    __vo uint32_t ISR;       /* 0x18 */
    __vo uint32_t ICR;       /* 0x1C */
    __vo uint32_t PECR;      /* 0x20 */
    __vo uint32_t RXDR;      /* 0x24 */
    __vo uint32_t TXDR;      /* 0x28 */
} I2C_RegDef_t;

/**************************************
 * Peripheral definitions
 **************************************/
#define GPIOA                   ((GPIO_RegDef_t*)GPIOA_BASEADDR)
#define GPIOB                   ((GPIO_RegDef_t*)GPIOB_BASEADDR)
#define GPIOC                   ((GPIO_RegDef_t*)GPIOC_BASEADDR)
#define GPIOD                   ((GPIO_RegDef_t*)GPIOD_BASEADDR)
#define GPIOE                   ((GPIO_RegDef_t*)GPIOE_BASEADDR)
#define GPIOF                   ((GPIO_RegDef_t*)GPIOF_BASEADDR)
#define GPIOG                   ((GPIO_RegDef_t*)GPIOG_BASEADDR)

#define RCC                     ((RCC_RegDef_t*)RCC_BASEADDR)
#define EXTI                    ((EXTI_RegDef_t*)EXTI_BASEADDR)
#define SYSCFG                  ((SYSCFG_RegDef_t*)SYSCFG_BASEADDR)

#define SPI1                    ((SPI_RegDef_t*)SPI1_BASEADDR)
#define SPI2                    ((SPI_RegDef_t*)SPI2_BASEADDR)
#define SPI3                    ((SPI_RegDef_t*)SPI3_BASEADDR)

#define USART2                  ((USART_RegDef_t*)USART2_BASEADDR)

#define I2C1                    ((I2C_RegDef_t*)I2C1_BASEADDR)
#define I2C2                    ((I2C_RegDef_t*)I2C2_BASEADDR)
#define I2C3                    ((I2C_RegDef_t*)I2C3_BASEADDR)

/**************************************
 * GPIO clock enable macros
 **************************************/
#define GPIOA_PCLK_EN()         (RCC->AHB2ENR |= (1U << 0))
#define GPIOB_PCLK_EN()         (RCC->AHB2ENR |= (1U << 1))
#define GPIOC_PCLK_EN()         (RCC->AHB2ENR |= (1U << 2))
#define GPIOD_PCLK_EN()         (RCC->AHB2ENR |= (1U << 3))
#define GPIOE_PCLK_EN()         (RCC->AHB2ENR |= (1U << 4))
#define GPIOF_PCLK_EN()         (RCC->AHB2ENR |= (1U << 5))
#define GPIOG_PCLK_EN()         (RCC->AHB2ENR |= (1U << 6))

#define GPIOA_PCLK_DI()         (RCC->AHB2ENR &= ~(1U << 0))
#define GPIOB_PCLK_DI()         (RCC->AHB2ENR &= ~(1U << 1))
#define GPIOC_PCLK_DI()         (RCC->AHB2ENR &= ~(1U << 2))
#define GPIOD_PCLK_DI()         (RCC->AHB2ENR &= ~(1U << 3))
#define GPIOE_PCLK_DI()         (RCC->AHB2ENR &= ~(1U << 4))
#define GPIOF_PCLK_DI()         (RCC->AHB2ENR &= ~(1U << 5))
#define GPIOG_PCLK_DI()         (RCC->AHB2ENR &= ~(1U << 6))

/**************************************
 * SPI clock enable macros
 **************************************/
#define SPI1_PCLK_EN()          (RCC->APB2ENR |= (1U << 12))
#define SPI2_PCLK_EN()          (RCC->APB1ENR1 |= (1U << 14))
#define SPI3_PCLK_EN()          (RCC->APB1ENR1 |= (1U << 15))

#define SPI1_PCLK_DI()          (RCC->APB2ENR &= ~(1U << 12))
#define SPI2_PCLK_DI()          (RCC->APB1ENR1 &= ~(1U << 14))
#define SPI3_PCLK_DI()          (RCC->APB1ENR1 &= ~(1U << 15))

/**************************************
 * USART clock enable macros
 **************************************/
#define USART2_PCLK_EN()        (RCC->APB1ENR1 |= (1U << 17))
#define USART2_PCLK_DI()        (RCC->APB1ENR1 &= ~(1U << 17))

/**************************************
 * I2C clock enable macros
 **************************************/
#define I2C1_PCLK_EN()          (RCC->APB1ENR1 |= (1U << 21))
#define I2C2_PCLK_EN()          (RCC->APB1ENR1 |= (1U << 22))
#define I2C3_PCLK_EN()          (RCC->APB1ENR1 |= (1U << 23))

#define I2C1_PCLK_DI()          (RCC->APB1ENR1 &= ~(1U << 21))
#define I2C2_PCLK_DI()          (RCC->APB1ENR1 &= ~(1U << 22))
#define I2C3_PCLK_DI()          (RCC->APB1ENR1 &= ~(1U << 23))

/**************************************
 * SYSCFG clock macros
 **************************************/
#define SYSCFG_PCLK_EN()        (RCC->APB2ENR |= (1U << 0))
#define SYSCFG_PCLK_DI()        (RCC->APB2ENR &= ~(1U << 0))

/**************************************
 * GPIO reset macros
 **************************************/
#define GPIOA_REG_RESET()       do{ (RCC->AHB2RSTR |= (1U << 0)); (RCC->AHB2RSTR &= ~(1U << 0)); }while(0)
#define GPIOB_REG_RESET()       do{ (RCC->AHB2RSTR |= (1U << 1)); (RCC->AHB2RSTR &= ~(1U << 1)); }while(0)
#define GPIOC_REG_RESET()       do{ (RCC->AHB2RSTR |= (1U << 2)); (RCC->AHB2RSTR &= ~(1U << 2)); }while(0)
#define GPIOD_REG_RESET()       do{ (RCC->AHB2RSTR |= (1U << 3)); (RCC->AHB2RSTR &= ~(1U << 3)); }while(0)
#define GPIOE_REG_RESET()       do{ (RCC->AHB2RSTR |= (1U << 4)); (RCC->AHB2RSTR &= ~(1U << 4)); }while(0)
#define GPIOF_REG_RESET()       do{ (RCC->AHB2RSTR |= (1U << 5)); (RCC->AHB2RSTR &= ~(1U << 5)); }while(0)
#define GPIOG_REG_RESET()       do{ (RCC->AHB2RSTR |= (1U << 6)); (RCC->AHB2RSTR &= ~(1U << 6)); }while(0)

/**************************************
 * SPI reset macros
 **************************************/
#define SPI1_REG_RESET()        do{ (RCC->APB2RSTR |= (1U << 12)); (RCC->APB2RSTR &= ~(1U << 12)); }while(0)
#define SPI2_REG_RESET()        do{ (RCC->APB1RSTR1 |= (1U << 14)); (RCC->APB1RSTR1 &= ~(1U << 14)); }while(0)
#define SPI3_REG_RESET()        do{ (RCC->APB1RSTR1 |= (1U << 15)); (RCC->APB1RSTR1 &= ~(1U << 15)); }while(0)

/**************************************
 * USART reset macros
 **************************************/
#define USART2_REG_RESET()      do{ (RCC->APB1RSTR1 |= (1U << 17)); (RCC->APB1RSTR1 &= ~(1U << 17)); }while(0)

/**************************************
 * I2C reset macros
 **************************************/
#define I2C1_REG_RESET()        do{ (RCC->APB1RSTR1 |= (1U << 21)); (RCC->APB1RSTR1 &= ~(1U << 21)); }while(0)
#define I2C2_REG_RESET()        do{ (RCC->APB1RSTR1 |= (1U << 22)); (RCC->APB1RSTR1 &= ~(1U << 22)); }while(0)
#define I2C3_REG_RESET()        do{ (RCC->APB1RSTR1 |= (1U << 23)); (RCC->APB1RSTR1 &= ~(1U << 23)); }while(0)

/**************************************
 * NVIC register addresses
 **************************************/
#define NVIC_ISER0              ((__vo uint32_t*)0xE000E100U)
#define NVIC_ISER1              ((__vo uint32_t*)0xE000E104U)
#define NVIC_ISER2              ((__vo uint32_t*)0xE000E108U)

#define NVIC_ICER0              ((__vo uint32_t*)0xE000E180U)
#define NVIC_ICER1              ((__vo uint32_t*)0xE000E184U)
#define NVIC_ICER2              ((__vo uint32_t*)0xE000E188U)

#define NVIC_PR_BASEADDR        ((__vo uint32_t*)0xE000E400U)
#define NVIC_PR_BASE_ADDR       NVIC_PR_BASEADDR

/**************************************
 * IRQ Numbers
 **************************************/
#define IRQ_NO_EXTI0            6
#define IRQ_NO_EXTI1            7
#define IRQ_NO_EXTI2            8
#define IRQ_NO_EXTI3            9
#define IRQ_NO_EXTI4            10
#define IRQ_NO_EXTI9_5          23
#define IRQ_NO_I2C1_EV          31
#define IRQ_NO_I2C1_ER          32
#define IRQ_NO_SPI1             35
#define IRQ_NO_SPI2             36
#define IRQ_NO_USART2           38
#define IRQ_NO_EXTI15_10        40
#define IRQ_NO_I2C2_EV          33
#define IRQ_NO_I2C2_ER          34
#define IRQ_NO_SPI3             51

/**************************************
 * SPI_CR1 bit positions
 **************************************/
#define SPI_CR1_CPHA            0
#define SPI_CR1_CPOL            1
#define SPI_CR1_MSTR            2
#define SPI_CR1_BR              3
#define SPI_CR1_SPE             6
#define SPI_CR1_LSBFIRST        7
#define SPI_CR1_SSI             8
#define SPI_CR1_SSM             9
#define SPI_CR1_RXONLY          10
#define SPI_CR1_CRCL            11
#define SPI_CR1_CRCNEXT         12
#define SPI_CR1_CRCEN           13
#define SPI_CR1_BIDIOE          14
#define SPI_CR1_BIDIMODE        15

/**************************************
 * SPI_CR2 bit positions
 **************************************/
#define SPI_CR2_RXDMAEN         0
#define SPI_CR2_TXDMAEN         1
#define SPI_CR2_SSOE            2
#define SPI_CR2_NSSP            3
#define SPI_CR2_FRF             4
#define SPI_CR2_ERRIE           5
#define SPI_CR2_RXNEIE          6
#define SPI_CR2_TXEIE           7
#define SPI_CR2_DS              8
#define SPI_CR2_FRXTH           12
#define SPI_CR2_LDMA_RX         13
#define SPI_CR2_LDMA_TX         14

/**************************************
 * SPI_SR bit positions
 **************************************/
#define SPI_SR_RXNE             0
#define SPI_SR_TXE              1
#define SPI_SR_CRCERR           4
#define SPI_SR_MODF             5
#define SPI_SR_OVR              6
#define SPI_SR_BSY              7
#define SPI_SR_FRE              8

/**************************************
 * SPI flag macros
 **************************************/
#define SPI_TXE_FLAG            (1U << SPI_SR_TXE)
#define SPI_RXNE_FLAG           (1U << SPI_SR_RXNE)
#define SPI_BSY_FLAG            (1U << SPI_SR_BSY)
#define SPI_OVR_FLAG            (1U << SPI_SR_OVR)

/**************************************
 * SPI data size macros for STM32L4
 * DS[3:0] field values
 **************************************/
#define SPI_DATASIZE_4BIT       3U
#define SPI_DATASIZE_5BIT       4U
#define SPI_DATASIZE_6BIT       5U
#define SPI_DATASIZE_7BIT       6U
#define SPI_DATASIZE_8BIT       7U
#define SPI_DATASIZE_9BIT       8U
#define SPI_DATASIZE_10BIT      9U
#define SPI_DATASIZE_11BIT      10U
#define SPI_DATASIZE_12BIT      11U
#define SPI_DATASIZE_13BIT      12U
#define SPI_DATASIZE_14BIT      13U
#define SPI_DATASIZE_15BIT      14U
#define SPI_DATASIZE_16BIT      15U

/**************************************
 * USART_CR1 bit positions
 **************************************/
#define USART_CR1_UE            0
#define USART_CR1_RE            2
#define USART_CR1_TE            3

/**************************************
 * USART_ISR bit positions
 **************************************/
#define USART_ISR_RXNE          5
#define USART_ISR_TC            6
#define USART_ISR_TXE           7

#endif
