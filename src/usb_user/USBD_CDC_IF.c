/************************************************************************//**
 * @file    USBD_CDC_IF.c
 * @author  Thomas Liebi
 * @version V0.1
 * @date    2014-10-15
 * @brief   single module body
 ****************************************************************************/

/****************************************************************************
 * MODULE MODIFICATION HISTORY
 ************************************************************************//**
 @page PAGE_HISTORY_USBD_CDC_IF History: USBD_CDC_IF
 |Version |Date       | Author       | Description of the modifications
 |--------|-----------|--------------|---------------------------------------
 |V0.1    |2015-10-15 | Thomas Liebi | initial
 
 *************************************************************************//*
 * INCLUDE FILES
 ****************************************************************************/
#include "main.h"
#include "usbd_cdc.h"
#include "USBD_CDC_IF.h"

/****************************************************************************
 * MODULE NAME DEFINITONS
 -----------------------------------------------------------------------*//**
 * @defgroup interface for usb_cdc_device (virtual com port)
 * @brief  This is the brief description of module USBD_CDC_IF
 * @details 
 *    This is a detailed description of module USBD_CDC_IF.
 *************************************************************************//*
 * STRUCTURE DEFINITONS
 -----------------------------------------------------------------------*//**
 * @addtogroup USBD_CDC_IF
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
/* buffer sizes */
#define APP_RX_DATA_SIZE 256
#define APP_TX_DATA_SIZE 256

/* exti-line used for sw-interrupt */
/* change exti-nbr in first 3 of the following definitions. rest is adopted */
#define CDC_EXTI_LINE GPIO_PIN_0
#define CDC_EXTI_IRQn EXTI0_IRQn
#define CDC_EXTI_IRQ_HANDLER EXTI0_IRQHandler
  #define CDC_EXTI_ENABLE_IT (EXTI->IMR |= (CDC_EXTI_LINE))
  #define CDC_EXTI_DISABLE_IT (EXTI->IMR &= ~(CDC_EXTI_LINE))
  #define CDC_EXTI_GENERATE_SWIT __HAL_GPIO_EXTI_GENERATE_SWIT(CDC_EXTI_LINE)
  #define CDC_EXTI_CLEAR_IT __HAL_GPIO_EXTI_CLEAR_IT(CDC_EXTI_LINE)


/** @endcond */
/****************************************************************************
 * PRIVATE DATA TYPES
 ****************************************************************************/
/*---*/

/****************************************************************************
 * PRIVATE DATA (STATIC)
 ****************************************************************************/
static USBD_CDC_LineCodingTypeDef LineCoding =
  {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
  };
  
/* receive buffer: Data received over USB are stored in this buffer */
static uint8_t UserRxBuffer[APP_RX_DATA_SIZE];/* Received Data over USB are stored in this buffer */
static uint32_t UserRxBufPtrIn = 0; 
static uint32_t UserRxBufPtrOut = 0;

/* transmitt buffer: Data to send over USB are stored in this buffer */
static uint8_t UserTxBuffer[APP_TX_DATA_SIZE];
static uint32_t UserTxBufPtrIn = 0;
static uint32_t UserTxBufPtrOut = 0; 
  
/* intermediate frame buffer: Data is copied from USB-packet to this buffer first */
/* this is done by HAL-Lib. No circular buffer handling there */
#define FRAMEBUFSIZE 64
static uint8_t FrameBuffer[FRAMEBUFSIZE];
static uint32_t FrameBufferLen = 0;
  
/* USB Device handler declaration */
static USBD_HandleTypeDef  hUSBDDevice;
  
/* USB device descriptor (defined in usb_desc.c) */
extern USBD_DescriptorsTypeDef VCP_Desc;

/* flag for transmission and reception */
static bool CDC_Transmission_Running = false;
static bool CDC_Reception_Running = false;


/****************************************************************************
 * PRIVATE FUNCTION PROTOTYPES (STATIC)
 ****************************************************************************/
/* cdc-interface to lower layer (callbacks) */
static int8_t CDC_Itf_Init     (void);
static int8_t CDC_Itf_DeInit   (void);
static int8_t CDC_Itf_Control  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Itf_Receive  (uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_Itf_TransmitCplt  (uint8_t* pbuf, uint32_t *Len);

/* interface control block for cdc  (assigned to cdc) */
static USBD_CDC_ItfTypeDef USBD_CDC_fops = 
{
  CDC_Itf_Init,
  CDC_Itf_DeInit,
  CDC_Itf_Control,
  CDC_Itf_Receive,
  CDC_Itf_TransmitCplt,
};


/****************************************************************************
 * PRIVATE FUNCTIONS (STATIC)
 ****************************************************************************/
/*---*/

/****************************************************************************
 * PRIVATE CALLBACK FUNCTIONS (STATIC)
 ****************************************************************************/
/*=======================================================================*//**
  @brief Initializes the CDC media low layer
  @details 
  @param none
  @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
*//*========================================================================*/
static int8_t CDC_Itf_Init(void)
{
  /* config exti-line interrupt (sw-interrupt only) */
  /* disable interupt and clear pending flag */
  CDC_EXTI_DISABLE_IT;
  CDC_EXTI_CLEAR_IT;

  /* Set EXTI priority (m u s t  have the same prio as USB!) */
  HAL_NVIC_SetPriority(CDC_EXTI_IRQn, USB_PRIO, 0);
  
  /* clear pending interrupt */
  HAL_NVIC_ClearPendingIRQ(CDC_EXTI_IRQn);
  
  /* Enable EXTI Interrupt in NVIC*/
  HAL_NVIC_EnableIRQ(CDC_EXTI_IRQn);  /* in NVIC */
  CDC_EXTI_ENABLE_IT;                 /* EXTI-Line */
  
  /* Set Application Buffers for cdc */
  USBD_CDC_SetTxBuffer(&hUSBDDevice, UserTxBuffer, 0);
  USBD_CDC_SetRxBuffer(&hUSBDDevice, FrameBuffer);

  return (USBD_OK);
} /* end CDC_Itf_Init */


/*=======================================================================*//**
  @brief Deinitializes the CDC media low layer
  @details 
  @param none
  @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
*//*========================================================================*/
static int8_t CDC_Itf_DeInit(void)
{
  /* deinit exti-line */
  /* disable interupt and clear pending flag */
  CDC_EXTI_DISABLE_IT;
  CDC_EXTI_CLEAR_IT;
  
  /* Disable EXTI Interrupt in NVIC*/
  HAL_NVIC_DisableIRQ(CDC_EXTI_IRQn);  /* in NVIC */

  /* clear pending interrupt */
  HAL_NVIC_ClearPendingIRQ(CDC_EXTI_IRQn);
  
  return (USBD_OK);
} /* end CDC_Itf_DeInit */


/*=======================================================================*//**
  @brief Data received over USB OUT endpoint are sent over CDC interface 
         through this function.
  @details 
  @param Buf: Buffer of data
  @param  Len: Number of data received (in bytes)
  @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
*//*========================================================================*/
static int8_t CDC_Itf_Receive(uint8_t* Buf, uint32_t *Len)
{
  uint32_t i; 
  uint32_t freeBufSize; 

  freeBufSize = ((uint32_t)(UserRxBufPtrOut - UserRxBufPtrIn - 1)) % APP_RX_DATA_SIZE;
 
  if(freeBufSize < *Len)
  {
    /* not enough room in buffer, stop reception */
    FrameBufferLen = *Len;
    CDC_Reception_Running = false;
  }
  else
  {
    /* copy received data to buffer and restart reception */
    for(i=0; i < *Len; i++)
    {
      UserRxBuffer[UserRxBufPtrIn] = Buf[i];
      UserRxBufPtrIn = (UserRxBufPtrIn+1) % APP_RX_DATA_SIZE;
    }
    FrameBufferLen = 0;
    USBD_CDC_ReceivePacket(&hUSBDDevice);
  }  
  return (USBD_OK);
}



/*=======================================================================*//**
  @brief CDC_Itf_TransmitCplt
  @details transmission over USB IN endpoint completed
  @param  Buf: Buffer of data that were transmitted
  @param  Len: Number of data transmitted (in bytes)
  @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
*//*========================================================================*/
static int8_t CDC_Itf_TransmitCplt(uint8_t* Buf, uint32_t *Len)
{
  uint32_t buffsize;
  uint32_t buffptr;
 
  /* free buffer */
  UserTxBufPtrOut = (UserTxBufPtrOut + *Len) % APP_TX_DATA_SIZE;

  /* start a new transmission, if new data available */
  if(UserTxBufPtrIn != UserTxBufPtrOut)
  {
    if(UserTxBufPtrOut > UserTxBufPtrIn) 
    {
      buffsize = APP_TX_DATA_SIZE - UserTxBufPtrOut; // no rollover in lower layer!
    }
    else 
    {
      buffsize = UserTxBufPtrIn - UserTxBufPtrOut;
    }
    
    buffptr = UserTxBufPtrOut;

    USBD_CDC_SetTxBuffer(&hUSBDDevice, (uint8_t*)&UserTxBuffer[buffptr], buffsize);

    if(USBD_CDC_TransmitPacket(&hUSBDDevice) != USBD_OK)
    {
      __NOP(); // for breakpoint
    }
  }
  else
  {
    CDC_Transmission_Running = false;  // transmission is done
  }

  return (USBD_OK);
}


/*=======================================================================*//**
  @brief Manage the CDC class requests
  @details e.g. line coding, control-lines
  @param Cmd: Command code
  @param Buf: Buffer containing command data (request parameters)
  @param Len: Number of data to be sent (in bytes)
  @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
*//*========================================================================*/
static int8_t CDC_Itf_Control (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{ 
  switch (cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:
    /* Add your code here */
    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:
    /* Add your code here */
    break;

  case CDC_SET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_GET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_CLEAR_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_SET_LINE_CODING:
    LineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |\
                            (pbuf[2] << 16) | (pbuf[3] << 24));
    LineCoding.format     = pbuf[4];
    LineCoding.paritytype = pbuf[5];
    LineCoding.datatype   = pbuf[6];
    break;

  case CDC_GET_LINE_CODING:
    pbuf[0] = (uint8_t)(LineCoding.bitrate);
    pbuf[1] = (uint8_t)(LineCoding.bitrate >> 8);
    pbuf[2] = (uint8_t)(LineCoding.bitrate >> 16);
    pbuf[3] = (uint8_t)(LineCoding.bitrate >> 24);
    pbuf[4] = LineCoding.format;
    pbuf[5] = LineCoding.paritytype;
    pbuf[6] = LineCoding.datatype;     
    break;

  case CDC_SET_CONTROL_LINE_STATE:
    break;

  case CDC_SEND_BREAK:
    break;    
    
  default:
    break;
  }
  
  return (USBD_OK);
} /* end CDC_Itf_Control */



/****************************************************************************
 * INTERRUPT SERVICE FUNCTIONS
 ****************************************************************************/

/*=======================================================================*//**
  @brief EXTIx handles asynch access to USB. EXTIx must have same prio as
         USB!!!
  @details waits until data available
  @param none
  @retval byte read
*//*========================================================================*/
void CDC_EXTI_IRQ_HANDLER(void)
{
  uint32_t buffsize;
  uint32_t buffptr;
  uint32_t i;

  CDC_EXTI_CLEAR_IT; /* clear pending flag */
  
  /* check if reception has to be started */
  if(CDC_Reception_Running == false)
  {
    CDC_Reception_Running = true;
    /* copy data from last reception into user-buffer if any */
    for(i = 0; i < FrameBufferLen; i++)
    {
      UserRxBuffer[UserRxBufPtrIn] = FrameBuffer[i];
      UserRxBufPtrIn = (UserRxBufPtrIn+1) % APP_RX_DATA_SIZE;
    }
    USBD_CDC_ReceivePacket(&hUSBDDevice);
  }

  /* check if transmission has to be started */
  if(CDC_Transmission_Running == false)
  {
  /* start a new transmission, if new data available */
    if(UserTxBufPtrIn != UserTxBufPtrOut)
    {
      CDC_Transmission_Running = true;    

      if(UserTxBufPtrOut > UserTxBufPtrIn) 
      {
        buffsize = APP_TX_DATA_SIZE - UserTxBufPtrOut; // no rollover in PCD!
      }
      else 
      {
        buffsize = UserTxBufPtrIn - UserTxBufPtrOut;
      }
      
      buffptr = UserTxBufPtrOut;

      USBD_CDC_SetTxBuffer(&hUSBDDevice, (uint8_t*)&UserTxBuffer[buffptr], buffsize);

      if(USBD_CDC_TransmitPacket(&hUSBDDevice) != USBD_OK)
      {
        __NOP(); // for breakpoint
      }
    }
  }
}


/****************************************************************************
 * PUBLIC SECTION (this section must corresponds with header file)
 -----------------------------------------------------------------------*//**
 * @publicsection
 ****************************************************************************/

/****************************************************************************
 * EXPORTED DATA
 ****************************************************************************/
/*---*/

/****************************************************************************
 * EXPORTED FUNCTIONS
 ****************************************************************************/
/*=======================================================================*//**
  @brief init the virtual comport for loader (SIO)
  @details initialize and start usb-device
  @param[in,out]  none
  @retval none
*//*========================================================================*/
void SIO_Itf_fInit(void)
{
  /* init usb */
  /* Init Device Library */
  USBD_Init(&hUSBDDevice, &VCP_Desc, 0);
  
  /* Register the CDC class */
  USBD_RegisterClass(&hUSBDDevice, &USBD_CDC);
  
  /* Register the CDC interface class */
  USBD_CDC_RegisterInterface(&hUSBDDevice, &USBD_CDC_fops);

  /* Start Device Process */
  USBD_Start(&hUSBDDevice);
} /* end SIO_Itf_fInit */

/*=======================================================================*//**
  @brief deinit the virtual comport for loader (SIO)
  @details stop and disconnect the usb-device
  @param[in,out]  none
  @retval none
*//*========================================================================*/
void SIO_Itf_fUnInit(void)
{
  USBD_Stop(&hUSBDDevice);
} /* end SIO_Itf_fUnInit */

/*=======================================================================*//**
  @brief write one byte tx buffer
  @details waits if no connection or no space in buffer
           starts transmission if not running 
  @param[in] byteToWrite
  @retval none
*//*========================================================================*/
void SIO_Itf_fWriteByte(uint8_t byteToWrite)
{
  uint32_t newWritePos;

  /* Loop until usb-device configured */
  while(hUSBDDevice.dev_state != USBD_STATE_CONFIGURED)
  {
  }

  /* wait for free space in buffer */
  newWritePos = (UserTxBufPtrIn + 1) % APP_TX_DATA_SIZE;
  while(UserTxBufPtrOut == newWritePos)
  {
    /* check if transmission running. start it if not */
    if(CDC_Transmission_Running == false)
    {
      CDC_EXTI_GENERATE_SWIT; //exti-irq to balance irq-prio with usb
    }
  }

  /* data into buffer */  
  UserTxBuffer[UserTxBufPtrIn] = byteToWrite;
  UserTxBufPtrIn = newWritePos;
    
  /* start transmission if not running */
  if(CDC_Transmission_Running == false)
  {
    CDC_EXTI_GENERATE_SWIT; //exti-irq to balance irq-prio with usb
  }
} /* end SIO_Itf_fWriteByte */

/*=======================================================================*//**
  @brief read one byte from rx-buffer
  @details waits until data available
           restarts reception if not running 
  @param none
  @retval byte read
*//*========================================================================*/
uint8_t SIO_Itf_fReadByte(void)
{
  uint8_t data;

  /* wait for data in buffer */
  while(UserRxBufPtrIn == UserRxBufPtrOut)
  {
    /* start reception if not running */
    if(CDC_Reception_Running == false)
    {
      CDC_EXTI_GENERATE_SWIT; //exti-irq to balance irq-prio with usb
    }
  }
    
  data = UserRxBuffer[UserRxBufPtrOut];
  UserRxBufPtrOut = (UserRxBufPtrOut+1) % APP_RX_DATA_SIZE;
  
  /* check if buffer became empty */
  if(UserRxBufPtrIn == UserRxBufPtrOut)
  {
    /* start reception if not running */
    if(CDC_Reception_Running == false)
    {
      CDC_EXTI_GENERATE_SWIT; //exti-irq to balance irq-prio with usb
    }
  }
  return data;
} /* end SIO_Itf_fReadByte */

/*=======================================================================*//**
  @brief check if data available in rx-buf
  @details restarts reception if not running
  @param none
  @retval true if data available, else false
*//*========================================================================*/
bool SIO_Itf_fReadReady(void)
{
  if(UserRxBufPtrIn == UserRxBufPtrOut)
  {
    /* start reception if not running */
    if(CDC_Reception_Running == false)
    {
      CDC_EXTI_GENERATE_SWIT; //exti-irq to balance irq-prio with usb
    }
    return false;
  }
  else
  {
    return true;
  }    
} /* end SIO_Itf_fReadReady */


/****************************************************************************
 * END OF MODULE STRUCTURE DEFINITONS
 -----------------------------------------------------------------------*//**
 * @}
 ****************************************************************************/
/****************************************************************************
 * END OF FILE
 ****************************************************************************/
