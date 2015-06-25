/************************************************************************//**
 * @file    dac.c
 * @author  Noah Huetter
 * @version 
 * @date    2015-16-25
 * @brief   DAC-handling module
 ****************************************************************************/
 /*************************************************************************//*
 * INCLUDE FILES
 ****************************************************************************/
#include "dac.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <math.h>

#include "main.h" 

/****************************************************************************
 * MODULE NAME DEFINITONS
 -----------------------------------------------------------------------*//**
 * @brief  DAC handles basic access to the DAC Peripheral
 * @details 
 *    
 *************************************************************************//*
 * STRUCTURE DEFINITONS
 -----------------------------------------------------------------------*//**
 * @addtogroup DAC
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
#define PI 3.14159265


/** @endcond */
/****************************************************************************
 * PRIVATE DATA TYPES
 ****************************************************************************/
/** @cond *//* hide these definitions from document generator */



/** @endcond */
/****************************************************************************
 * PRIVATE FUNCTION PROTOTYPES (STATIC)
 ****************************************************************************/
static void InitTIM(void);
static void InitDMA(void);
static void InitDAC(void);


/****************************************************************************
 * PRIVATE DATA (STATIC)
 ****************************************************************************/
static DAC_HandleTypeDef m_hDac1;
static TIM_HandleTypeDef m_hTim6;
static DMA_HandleTypeDef m_hDMA_dac1;

static uint8_t m_DAC_Buf[DAC_OUTPUT_BUF_SIZE];

/****************************************************************************
 * PRIVATE FUNCTIONS (STATIC)
 ****************************************************************************/
/*=======================================================================*//**
  @brief  Inits the timer needed for the DAC Peripheral
  @details 
  @param 
  @retval 
*//*========================================================================*/
static void InitTIM(void)
{
  	TIM_MasterConfigTypeDef sMasterConfig;

  	/* TIM6 time base config */
  	__TIM6_CLK_ENABLE();
  	m_hTim6.Instance = TIM6;
	m_hTim6.Init.Prescaler         = 0; // 72MHz TIM6 clk
	m_hTim6.Init.Period            = 1632; // 44.08kHz trigger output TRGO
	m_hTim6.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	m_hTim6.Init.CounterMode       = TIM_COUNTERMODE_UP;
	m_hTim6.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&m_hTim6);

	/* TIM6 TRGO selection */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&m_hTim6, &sMasterConfig);

	__HAL_TIM_CLEAR_IT(&m_hTim6, TIM_FLAG_UPDATE);
  	HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
  	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  	__HAL_TIM_ENABLE_IT(&m_hTim6, TIM_FLAG_UPDATE);

	/* Start TIM6 */
 	HAL_TIM_Base_Start(&m_hTim6);
}

/*=======================================================================*//**
  @brief  Inits the DMA channel needed by the DAC Peripheral
  @details 
  @param 
  @retval 
*//*========================================================================*/
static void InitDMA(void)
{
  /* DMA1 clock enable */
  __DMA1_CLK_ENABLE();
  /* SYSCFG clock enable for DMA remapping */
  __SYSCFG_CLK_ENABLE();

  m_hDMA_dac1.Instance = DMA1_Channel3;
  m_hDMA_dac1.Init.Direction = DMA_MEMORY_TO_PERIPH;
  m_hDMA_dac1.Init.PeriphInc = DMA_PINC_DISABLE;
  m_hDMA_dac1.Init.MemInc = DMA_MINC_ENABLE;
  m_hDMA_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  m_hDMA_dac1.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  m_hDMA_dac1.Init.Mode = DMA_CIRCULAR;
  m_hDMA_dac1.Init.Priority = DMA_PRIORITY_HIGH;

  HAL_DMA_Init(&m_hDMA_dac1);

  /* Associate the initialized DMA handle to the the DAC handle */
  __HAL_LINKDMA(&m_hDac1, DMA_Handle1, m_hDMA_dac1);

  /* Enable the DMA1_Channel3 IRQ Channel */
  //HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 2, 0);
  //HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

  /* Configure the SYSCFG for DMA remapping */
  __HAL_REMAPDMA_CHANNEL_ENABLE(HAL_REMAPDMA_TIM6_DAC1_CH1_DMA1_CH3);
}

/*=======================================================================*//**
  @brief  Inits the DAC Peripheral
  @details 
  @param 
  @retval 
*//*========================================================================*/
static void InitDAC(void)
{
	GPIO_InitTypeDef gpio;
	DAC_ChannelConfTypeDef dac_ch;

	/* Init Port */
	__GPIOA_CLK_ENABLE();
	gpio.Pin = GPIO_PIN_4;
	gpio.Mode = GPIO_MODE_ANALOG;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_HIGH;
	gpio.Alternate = 0;
	HAL_GPIO_Init(GPIOA, &gpio);

	/* Init DAC1 */
	__HAL_DAC_RESET_HANDLE_STATE(&m_hDac1);
	m_hDac1.Instance = DAC1;
	m_hDac1.Lock = HAL_UNLOCKED;
	m_hDac1.DMA_Handle1 = &m_hDMA_dac1;
	m_hDac1.DMA_Handle2 = 0;
	m_hDac1.ErrorCode = 0;
	HAL_DAC_Init(&m_hDac1);
	__DAC1_CLK_ENABLE();

	dac_ch.DAC_Trigger = DAC_TRIGGER_T6_TRGO;;
	dac_ch.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	HAL_DAC_ConfigChannel(&m_hDac1, &dac_ch, DAC1_CHANNEL_1);

	//__HAL_DAC_ENABLE(&m_hDac1, DAC1_CHANNEL_1);
	//HAL_DAC_Start(&m_hDac1, DAC1_CHANNEL_1);
	HAL_DAC_Start_DMA(&m_hDac1, DAC1_CHANNEL_1, 
		(uint32_t *)m_DAC_Buf, DAC_OUTPUT_BUF_SIZE, DAC_ALIGN_8B_R);
}


/****************************************************************************
 * PRIVATE CALLBACK FUNCTIONS (STATIC)
 ****************************************************************************/



/****************************************************************************
 * INTERRUPT SERVICE FUNCTIONS
 ****************************************************************************/
void TIM6_DAC1_IRQHandler (void)
{
	__HAL_TIM_CLEAR_IT(&m_hTim6, TIM_FLAG_UPDATE);

}


/****************************************************************************
 * PUBLIC SECTION (this section must corresponds with header file)
 -----------------------------------------------------------------------*//**
 * @publicsection
 ****************************************************************************/
/****************************************************************************
 * EXPORTED DATA
 ****************************************************************************/
/* it is usually not recommendable to export data  */

/** This data definition is only used for demonstration purpose! 
    (This text will be used for the automated documentation generation) 	*/



/****************************************************************************
 * EXPORTED FUNCTIONS
 ****************************************************************************/
/*=======================================================================*//**
  @brief  DAC Init Function
  @details initializes the DAC
  @param 
  @retval 
*//*========================================================================*/
void DAC_fInit(void)
{
	uint32_t ctr;

    GPIOB->ODR ^= GPIO_PIN_0; //toggle pin
	for(ctr = 0; ctr < 1024; ctr++)
	{
		m_DAC_Buf[ctr] = 127.5*sin(ctr*((2*PI)/1024))+127.5;
		//m_DAC_Buf[ctr] = 126;
	}
    GPIOB->ODR ^= GPIO_PIN_0; //toggle pin

	InitTIM();
	InitDMA();
	InitDAC();
	//HAL_DAC_SetValue(&m_hDac1, DAC1_CHANNEL_1, DAC_ALIGN_12B_R, 0);
	//HAL_DACEx_TriangleWaveGenerate(&m_hDac1, DAC_CHANNEL_1, DAC_TRIANGLEAMPLITUDE_4095);
	//DAC1->DHR12R1 = 4000;
}


/****************************************************************************
 * END OF COMPONENT STRUCTURE DEFINITONS
 -----------------------------------------------------------------------*//**
 * @}
 ****************************************************************************/
/****************************************************************************
 * END OF FILE
 ****************************************************************************/
