

/**
*	Author: Chetan Bornarkar
*	
*	Description: Header file for the SPI flash memory handler
* 				 Define all the opcodes for read/write operation
*				 Define all the pins of the processor used to perform 
*				 the read/write operation on SPI.
*/

/////////////////////////////////////HEADER FILE/////////////////////////////////////////////////////
#ifndef _SPI_DEC_H_
#define _SPI_DEC_H_

#define CS_ENABLE  (FIO0CLR0 |= (1 << 6))
#define CS_DISABLE (FIO0SET0 |= (1 << 6))
#define SPI_RDOP_E8

#define SPI_WROP_WE         0x88
#define SPI_WROP_WE_BUF2    0x89    
#define SPI_WROP_BUF2       0x85           // WRITE OPCODE FOR BUF 2
#define BUF2_WR             0x87

#define PAGE_LOC            0x3FF          // used for identification of the byte location within the page
#define SPI_BUF_SZ          524            // spi_buffer size for storing of the data   
#define FLSH_RD_OFF     8              // After the read operation from the flash mem the first 8 bytes consist garbage value.     

#define SPIRD   1
#define SPIWR   2
#define FLSH_STATOP          0xD7      // STATUS OPCODE
#define FLSH_RDOP            0xE8      // READ OPCODE  
#define FLSH_IDOP            0x9F      // Flahs Id read opcode
#define FLSH_WROP            0X82      // WRITE OPCODE
#define FLSH_EROP           0x81      // Erase opcode

typedef struct 
{
  unsigned char tx[SPI_BUF_SZ];  //spi buffer 512 data + 4 opcode bytes + 4 dnt care bytes during read operation 
  unsigned short wrptr;            //ptr to locate the position where the data is written
  unsigned short rdptr;            //ptr to locate the position from where the data is to be read during tx 
  unsigned short r_wrptr;          //ptr to locate the position where the data is read after rec  
}__spi;

void SpiHandler();
void spi_init();

#endif

///////////////////////////////////////////////// HEADER FILE END/////////////////////////////////////////////////////

/**
*	Author: Chetan Bornarkar
*
*	Description: SPI device driver code for flash memory read and write operation
*				 The flash memory is read from the address specified or the memory
*				 location is writen to the locaion specified. 
*				 The read/write depends on the opcode of the flash memory.
*				 Only the first 512 bytes of memory of every page is used to read/write.
*/

#include "includes.h"

extern __store store;

__spi spi = {.tx = {0}, .wrptr = 0, .rdptr = 0, .r_wrptr = 0};

unsigned char SpiWrBuf[516];    // buffer to store the data to be written
unsigned char flsh_buf[520];    // store the data of flash read or write
unsigned char SpiHdlrOpr = 0;   // to identify the operation of read or write to flash
unsigned char temp_store[512] = {0};    
unsigned int temp_wrptr, temp_rptr  = 0;

/*************************************************************************
 * Function Name  : SPC_INIT

 * Parameters     : ARRAYS

 * Return         : none
 
 * Description    : SPC Initialzation.
 
 *************************************************************************/
void spi_init()
{
  volatile unsigned int dummy_spi;
  PCONP_bit.PCSSP1 = 1;                 // Enable the SSP1 
  PCLKSEL0_bit.PCLK_SSP1 = 3;           // Divide the Cclk/8 -> 48Mhz/8 = 6Mhz
  SSP1CR0 = 0X0C7;                      // 8bit, spi, cpol & cpha =1 
  SSP1CR1_bit.SSE = 0;                  // disable sse of CR1 
  SSP1CR1_bit.MS = 0;                   // master mode enable 
  SSP1CR1_bit.LBM = 0;                  // loop back disabled 
  SSP1CPSR = 0X02;                      // 6Mhz/2(even value) = 3Mhz 
  SSP1CR1_bit.SSE = 1;                      // enable sse 
  SSP1IMSC = 0x0F;                      // Enable the intr when Rx timeout, Rec over run, Rx fifo half empty, Tx FIFO half empty       
  CS_ENABLE;
  __no_operation();
  __no_operation();
  __no_operation();
  CS_DISABLE;
}

void SSP1_IRQHandler(void)
{
  // Transmit function 
  for(unsigned int i = 0 ; (SSP1SR_bit.TNF == 1) && (spi.rdptr != spi.wrptr) && i < 8 ; i++)  //tx successive 8 bytes
    SSP1DR = spi.tx[spi.rdptr++];                 // Tx until it's empty 
  // rec function 
  for(unsigned int i = 0 ; (SSP1SR_bit.RNE && (i < 8)) ; i++) //receive 8 bytes
  {
    spi.tx[spi.r_wrptr++] = SSP1DR;             // store the data rec in the spi buffer
  }
  if(spi.r_wrptr == spi.wrptr)
    NVIC_IntDisable(NVIC_SSP1);                 // Disable the intr for spi  Flag indication of the end of spi 
  return;
}

/*************************************************************************
 * Function Name: SpiFlshHandler
 * Parameters: void
 * Return: void
 *
 * Description: Used to read the data from the memory according to the addr given
 *              Write the data in the flash memory
 
 *************************************************************************/
void SpiFlshHandler()
{
  switch(SpiHdlrOpr)
  {
  case 1 : // Read flsh
    {
      NVIC_IntEnable(NVIC_SSP1);  // enable the intr for UART 
      NVIC_IntPri(NVIC_SSP1,17);  // set the priority to the intr gen
      SpiHdlrOpr += 1;             // Wait for intr opr to be complete
      break;
    }
    
  case 2 :  // case used to read the data 
    {
      if(spi.r_wrptr == spi.wrptr)  // the intr is over, wrptr - ptr indicate the data written and r_wrptr indicate the data read after write
      {
        unsigned short rptr, wptr;
        rptr = 0; 
        wptr = spi.wrptr;   
        for(store.wptr = 0 ; wptr != store.wptr ; )  // Store the data in the store buffer.
          store.data[store.wptr++] = spi.tx[rptr++]; // read 
        SpiHdlrOpr = 0;
      }
      break;
    }
    
  case 3 : // Write flash
    {
      NVIC_IntEnable(NVIC_SSP1);   // enable intr 
      SpiHdlrOpr += 1;  // Wait for intr opr to be complete
      break;
    }
    
  case 4 : 
    {
      if(spi.r_wrptr == spi.wrptr)  // indicate the intr is over 
      {
        SpiHdlrOpr = 0; 
      }
      break;
    }
  }
}
