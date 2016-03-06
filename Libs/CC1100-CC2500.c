/* --COPYRIGHT--,BSD
 * Copyright (c) 2011, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//------------------------------------------------------------------------------
//  Description:  This file contains functions that configure the CC1100/2500
//  device.
//
//  Demo Application for MSP430/CC1100-2500 Interface Code Library v1.0
//
//  K. Quiring
//  Texas Instruments, Inc.
//  July 2006
//  IAR Embedded Workbench v3.41
//------------------------------------------------------------------------------


#include "include.h"
#include "TI_CC_CC1100-CC2500.h"

#define TI_CC_RF_FREQ  868                 // 315, 433, 868, 915, 2400



//------------------------------------------------------------------------------
//  void writeRFSettings(void)
//
//  DESCRIPTION:
//  Used to configure the CCxxxx registers.  There are five instances of this
//  function, one for each available carrier frequency.  The instance compiled
//  is chosen according to the system variable TI_CC_RF_FREQ, assigned within
//  the header file "TI_CC_hardware_board.h".
//
//  ARGUMENTS:
//      none
//------------------------------------------------------------------------------


#if TI_CC_RF_FREQ == 868                          // 868 MHz
// 868MHz -- 600 bauds GFSK -- CRC -- no addr check -- Optimised for power consumption
// GDO0 signal selection = (6) Asserts when sync word has been sent/received, and de-asserts at the end of the packet
// GDO2 signal selection = (7) CRC OK
void writeRFSettings(void)
{
    // Write register settings
    TI_CC_SPIWriteReg(TI_CCxxx0_IOCFG2,	  		0x07); // GDO2 output pin config.
    TI_CC_SPIWriteReg(TI_CCxxx0_IOCFG0,	  		0x06); // GDO0 output pin config.
    TI_CC_SPIWriteReg(TI_CCxxx0_FIFOTHR,		0x47); // RX FIFO and TX FIFO Thresholds
    TI_CC_SPIWriteReg(TI_CCxxx0_PKTCTRL1,		0x0C); // Packet Automation Control
    // TI_CC_SPIWriteReg(TI_CCxxx0_PKTCTRL0,		0x05); // Packet Automation Control
    TI_CC_SPIWriteReg(TI_CCxxx0_FSCTRL1,		0x06); // Frequency Synthesizer Control
    TI_CC_SPIWriteReg(TI_CCxxx0_FREQ2,			0x21); // Frequency Control Word, High Byte
    TI_CC_SPIWriteReg(TI_CCxxx0_FREQ1,			0x62); // Frequency Control Word, Middle Byte
    TI_CC_SPIWriteReg(TI_CCxxx0_FREQ0,			0x76); // Frequency Control Word, Low Byte
    TI_CC_SPIWriteReg(TI_CCxxx0_MDMCFG4,		0xF4); // Modem Configuration
    TI_CC_SPIWriteReg(TI_CCxxx0_MDMCFG3,		0x83); // Modem Configuration
    TI_CC_SPIWriteReg(TI_CCxxx0_MDMCFG2,		0x93); // Modem Configuration
    TI_CC_SPIWriteReg(TI_CCxxx0_DEVIATN,		0x15); // Modem Deviation Setting
    TI_CC_SPIWriteReg(TI_CCxxx0_MCSM0,			0x18); // Main Radio Control State Machine Configuration
    TI_CC_SPIWriteReg(TI_CCxxx0_FOCCFG,			0x16); // Frequency Offset Compensation Configuration
    TI_CC_SPIWriteReg(TI_CCxxx0_RESERVED_0X20,	0xFB); // Use setting from SmartRF Studio
    TI_CC_SPIWriteReg(TI_CCxxx0_FSCAL3,			0xE9); // Frequency Synthesizer Calibration
    TI_CC_SPIWriteReg(TI_CCxxx0_FSCAL2,			0x2A); // Frequency Synthesizer Calibration
    TI_CC_SPIWriteReg(TI_CCxxx0_FSCAL1,			0x00); // Frequency Synthesizer Calibration
    TI_CC_SPIWriteReg(TI_CCxxx0_FSCAL0,			0x1F); // Frequency Synthesizer Calibration
    TI_CC_SPIWriteReg(TI_CCxxx0_TEST2,			0x81); // Various Test Settings
    TI_CC_SPIWriteReg(TI_CCxxx0_TEST1,			0x35); // Various Test Settings
    TI_CC_SPIWriteReg(TI_CCxxx0_TEST0,			0x09); // Various Test Settings
}

// PATABLE (0 dBm output power)
//extern char paTable[] = {0x60};
//extern char paTableLen = 1;

extern char paTable[] = {0xC0, 0xC5, 0xCD, 0x86, 0x50, 0x37, 0x26, 0x1D, 0x17}; //+12,10,7,5,0,-6,-10,-15 Dbm default 10Dbm
extern char paTableLen = 9;

#endif


void RF_init()
{
  
	TI_CC_SPISetup();                         // Initialize SPI port
	TI_CC_PowerupResetCCxxxx();               // Reset CCxxxx
	writeRFSettings();                        // Write RF settings to config reg

	TI_CC_SPIWriteBurstReg(TI_CCxxx0_PATABLE, paTable, paTableLen);//Write PATABLE
	//TI_CC_SPIStrobe(TI_CCxxx0_SIDLE); // set IDLE
	TI_CC_SPIWriteReg (TI_CCxxx0_PATABLE,paTable[4]); // init at 0 dBm

	TI_CC_SPIStrobe(TI_CCxxx0_SRX);           // Initialize CCxxxx in RX mode.
                                            // When a pkt is received, it will
                                            // signal on GDO0 and wake CPU

}

void RF_change_Power(char power)
{
 if (power<paTableLen)
  {TI_CC_SPIStrobe(TI_CCxxx0_SIDLE); // set IDLE
   TI_CC_SPIWriteReg (TI_CCxxx0_PATABLE,paTable[power]);
   //Out_Payload.Status.power=TI_CC_SPIReadReg(TI_CCxxx0_PATABLE);
  }
}



//-----------------------------------------------------------------------------
//  void RFSendPacket(char *txBuffer, char size)
//
//  DESCRIPTION:
//  This function transmits a packet with length up to 63 bytes.  To use this
//  function, GD00 must be configured to be asserted when sync word is sent and
//  de-asserted at the end of the packet, which is accomplished by setting the
//  IOCFG0 register to 0x06, per the CCxxxx datasheet.  GDO0 goes high at
//  packet start and returns low when complete.  The function polls GDO0 to
//  ensure packet completion before returning.
//
//  ARGUMENTS:
//      char *txBuffer
//          Pointer to a buffer containing the data to be transmitted
//
//      char size
//          The size of the txBuffer
//-----------------------------------------------------------------------------
void RFSendPacket(char *txBuffer, char size)
{
//  TI_CC_SPIWriteBurstReg(TI_CCxxx0_TXFIFO, txBuffer, size); // Write TX data
//  TI_CC_SPIStrobe(TI_CCxxx0_STX);           // Change state to TX, initiating
//                                            // data transfer
//
//  while (!(TI_CC_GDO0_PxIN&TI_CC_GDO0_PIN));
//                                            // Wait GDO0 to go hi -> sync TX'ed
//  while (TI_CC_GDO0_PxIN&TI_CC_GDO0_PIN);
//                                            // Wait GDO0 to clear -> end of pkt
//  TI_CC_GDO0_PxIFG &= ~TI_CC_GDO0_PIN;      // After pkt TX, this flag is set.
//                                            // Has to be cleared before existing
  
    // added 20120123 based on ANAREN code 
    TI_CC_SPIStrobe(TI_CCxxx0_SIDLE); // set to IDLE
    TI_CC_SPIStrobe(TI_CCxxx0_SFRX); // Flush RX
    TI_CC_SPIStrobe(TI_CCxxx0_SFTX); // Flush TX
    // end new 20120123
    
    TI_CC_SPIWriteBurstReg(TI_CCxxx0_TXFIFO, txBuffer, size); // Write TX packet structure data
    
    TI_CC_SPIStrobe(TI_CCxxx0_STX); // Change state to TX, initiating data transfer
    
    while (!(TI_CC_GDO0_PxIN&TI_CC_GDO0_PIN)); // Wait GDO0 to go hi -> sync TX'ed
    while (TI_CC_GDO0_PxIN&TI_CC_GDO0_PIN); // Wait GDO0 to clear -> end of pkt
    
    TI_CC_SPIStrobe(TI_CCxxx0_SIDLE); // set to IDLE after sending
    
    TI_CC_GDO0_PxIFG &= ~TI_CC_GDO0_PIN; // After pkt TX, this interrupt flag is set.
    // Has to be cleared before exiting
}



//-----------------------------------------------------------------------------
//  char RFReceivePacket(char *rxBuffer, char *length)
//
//  DESCRIPTION:
//  Receives a packet of variable length (first byte in the packet must be the
//  length byte).  The packet length should not exceed the RXFIFO size.  To use
//  this function, APPEND_STATUS in the PKTCTRL1 register must be enabled.  It
//  is assumed that the function is called after it is known that a packet has
//  been received; for example, in response to GDO0 going low when it is
//  configured to output packet reception status.
//
//  The RXBYTES register is first read to ensure there are bytes in the FIFO.
//  This is done because the GDO signal will go high even if the FIFO is flushed
//  due to address filtering, CRC filtering, or packet length filtering.
//
//  ARGUMENTS:
//      char *rxBuffer
//          Pointer to the buffer where the incoming data should be stored
//      char *length
//          Pointer to a variable containing the size of the buffer where the
//          incoming data should be stored. After this function returns, that
//          variable holds the packet length.
//
//  RETURN VALUE:
//      char
//          0x80:  CRC OK
//          0x00:  CRC NOT OK (or no pkt was put in the RXFIFO due to filtering)
//-----------------------------------------------------------------------------
char RFReceivePacket(char *rxBuffer, char *length, char *status)
{
  //char status[2];
  char pktLen;

  if ((TI_CC_SPIReadStatus(TI_CCxxx0_RXBYTES) & TI_CCxxx0_NUM_RXBYTES))
  {
    pktLen = TI_CC_SPIReadReg(TI_CCxxx0_RXFIFO); // Read length byte

    if (pktLen <= *length)                  // If pktLen size <= rxBuffer
    {
      TI_CC_SPIReadBurstReg(TI_CCxxx0_RXFIFO, rxBuffer, pktLen); // Pull data
      *length = pktLen;                     // Return the actual size
      TI_CC_SPIReadBurstReg(TI_CCxxx0_RXFIFO, status, 2);
                                            // Read appended status bytes
      return (char)(status[TI_CCxxx0_LQI_RX]&TI_CCxxx0_CRC_OK);
    }                                       // Return CRC_OK bit
    else
    {
      *length = pktLen;                     // Return the large size
      TI_CC_SPIStrobe(TI_CCxxx0_SFRX);      // Flush RXFIFO
      return 0;                             // Error
    }
  }
  else
      return 0;                             // Error
}


// Contributed by Cor
void Radio_GotoSleep()
{
 TI_CC_SPIStrobe(TI_CCxxx0_SIDLE); // set IDLE
 TI_CC_SPIStrobe(TI_CCxxx0_SPWD); // power down.
}

// Contributed by Cor
void Radio_WakeUp()
{
  TI_CC_CSn_PxOUT &= ~TI_CC_CSn_PIN;        // /CS low
  while (TI_CC_SPI_USCIB0_PxIN & TI_CC_SPI_USCIB0_SOMI); //wait till P1.6 goes low
  TI_CC_CSn_PxOUT |= TI_CC_CSn_PIN;         // /CS high

  // write some test setting back;
  TI_CC_SPIWriteReg(TI_CCxxx0_TEST2, 0x88); // Various test settings.
  TI_CC_SPIWriteReg(TI_CCxxx0_TEST1, 0x31); // Various test settings.
  TI_CC_SPIWriteReg(TI_CCxxx0_TEST0, 0x0B); // Various test settings.
    
  TI_CC_SPIStrobe(TI_CCxxx0_SFRX);          // flush the receive FIFO of any residual data
  TI_CC_SPIStrobe(TI_CCxxx0_SIDLE); // set IDLE
}
