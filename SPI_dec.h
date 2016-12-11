
/**
*	Author: Chetan Bornarkar
*	
*	Description: Header file for the SPI flash memory handler
* 				 Define all the opcodes for read/write operation
*				 Define all the pins of the processor used to perform 
*				 the read/write operation on SPI.
*/


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