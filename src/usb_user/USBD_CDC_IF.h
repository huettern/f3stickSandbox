/************************************************************************//**
 * @file    USBD_CDC_IF.h
 * @author  Thomas Liebi
 * @version V1.0
 * @date    2014-10-15
 * @brief   single module header 
 ****************************************************************************/
#ifndef USBD_CDC_IF_h
#define USBD_CDC_IF_h

/****************************************************************************
 * INCLUDE FILES
 ****************************************************************************/
#include <stdint.h>
#include <stdbool.h>


/****************************************************************************
 * STRUCTURE DEFINITONS
 -----------------------------------------------------------------------*//**
 * @addtogroup USBD_CDC_IF
 * @{
 ****************************************************************************/
/****************************************************************************
 * EXPORTED DEFINITIONS
 ****************************************************************************/
/*---*/

/***************************************************************************
 * EXPORTED TYPE DEFINITIONS
 ***************************************************************************/
/** exported data sructure */
/*---*/

/****************************************************************************
 * END OF GROUP STRUCTURE DEFINITONS
 -----------------------------------------------------------------------*//**
 * @}
 ****************************************************************************/
/****************************************************************************
 * EXPORTED DATA
 ****************************************************************************/
/*---*/

/***************************************************************************
 * EXPORTED FUNCTIONS
 ***************************************************************************/
/* communication interface to upper layer (loader) */
void SIO_Itf_fInit(void);
void SIO_Itf_fUnInit(void);
void SIO_Itf_fWriteByte(uint8_t byteToWrite);
uint8_t SIO_Itf_fReadByte(void);
bool SIO_Itf_fReadReady(void);


#endif
/****************************************************************************
 * END OF FILE
 ****************************************************************************/
