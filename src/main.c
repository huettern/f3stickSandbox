/************************************************************************//**
 * @file    main.c
 * @author  Noah Huetter
 * @version 
 * @date    2015-07-01
 * @brief   main
 ****************************************************************************/
 /*************************************************************************//*
 * INCLUDE FILES
 ****************************************************************************/
#include "main.h"

#include "USBD_CDC_IF.h"  /* virtual com port */


/****************************************************************************
 * MODULE NAME DEFINITONS
 -----------------------------------------------------------------------*//**
 * @brief  main
 * @details 
 *    
 *************************************************************************//*
 * STRUCTURE DEFINITONS
 -----------------------------------------------------------------------*//**
 * @addtogroup main
 * @{
 ****************************************************************************/
/****************************************************************************
 * PRIVATE SECTION (definitions and members are only visible locally)
 -----------------------------------------------------------------------*//**
 * @privatesection
 ****************************************************************************/
/****************************************************************************
 * PRIVATE CONSTANT DEFINITIONS
 ****************************************************************************/
/** @cond *//* hide these definitions from document generator */



/** @endcond */
/****************************************************************************
 * PRIVATE DATA TYPES
 ****************************************************************************/
/** @cond *//* hide these definitions from document generator */



/** @endcond */
/****************************************************************************
 * PRIVATE FUNCTION PROTOTYPES (STATIC)
 ****************************************************************************/
static void SystemClock_Config(void);
static void Error_Handler(void);


/****************************************************************************
 * PRIVATE DATA (STATIC)
 ****************************************************************************/



/****************************************************************************
 * PRIVATE FUNCTIONS (STATIC)
 ****************************************************************************/

/*=======================================================================*//**
  @brief   Main program
  @details
  @param 
  @retval 
*//*========================================================================*/
int main(void)
{
  HAL_Init();

  /* Configure the system clock to 72 Mhz */
  SystemClock_Config();

  /* Enbale GPIOB clock */
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
  /* Configure GPIOB pin 5 as output */
  GPIOB->MODER |= 0x01;
  /* Configure GPIOB pin 5 in max speed */
  GPIOB->OSPEEDR |= 3;

  /* Toggle PB0 */  
  while (1)
  {
    GPIOB->ODR ^= GPIO_PIN_0; //toggle pin
    HAL_Delay (450);
    GPIOB->ODR ^= GPIO_PIN_0; //toggle pin
    HAL_Delay (450);
  }
}

/*=======================================================================*//**
  @brief  System Clock Configuration
          The system Clock is configured as follow : 
             System Clock source            = PLL (HSE)
             SYSCLK(Hz)                     = 72000000
             HCLK(Hz)                       = 72000000
             AHB Prescaler                  = 1
             APB1 Prescaler                 = 2
             APB2 Prescaler                 = 1
             HSE Frequency(Hz)              = 8000000
             HSE PREDIV                     = 1
             PLLMUL                         = RCC_PLL_MUL9 (9)
             Flash Latency(WS)              = 2
  @details
  @param 
  @retval 
*//*========================================================================*/
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    Error_Handler();
  }
}

/*=======================================================================*//**
  @brief   This function is executed in case of error occurrence.
  @details
  @param 
  @retval 
*//*========================================================================*/
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/*=======================================================================*//**
  @brief   Reports the name of the source file and the source line number
           where the assert_param error has occurred.
  @details
  @param   file: pointer to the source file name
  @param   line: assert_param error line source number
  @retval  None
*//*========================================================================*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/****************************************************************************
 * PRIVATE CALLBACK FUNCTIONS (STATIC)
 ****************************************************************************/



/****************************************************************************
 * INTERRUPT SERVICE FUNCTIONS
 ****************************************************************************/



/****************************************************************************
 * END OF COMPONENT STRUCTURE DEFINITONS
 -----------------------------------------------------------------------*//**
 * @}
 ****************************************************************************/
/****************************************************************************
 * END OF FILE
 ****************************************************************************/
