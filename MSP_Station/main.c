#include "../Libs/TI_CC_hardware_board_launchpad.h"
#include "../Libs/TI_CC_msp430.h"
#include "../Libs/TI_CC_spi.h"
#include "../Libs/TI_CC_CC1100-CC2500.h"
#include "../Libs/CC1100-CC2500.h"
#include "../PC_Relais/protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RXD 		BIT1
#define TXD 		BIT2
#define GDO0 		BIT6
#define GDO2 		BIT0
#define BUTTON		BIT3

#define CRC_OK			0x80

#define BUFFER_STD_LENGTH		64
#define BUFFER_NUMBERS_LENGTH	10


char UART_TX_BUFFER[BUFFER_STD_LENGTH];
unsigned UART_TX_BUFFER_POS = 0;
unsigned UART_TX_DATALENGTH = 0;

char UART_RX_BUFFER[BUFFER_STD_LENGTH];
unsigned UART_RX_BUFFER_POS = 0;


int main(void) {
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

	// CPU MCLK and SMBCLK 1MHz
	DCOCTL = CALDCO_1MHZ;
	BCSCTL1 = CALBC1_1MHZ;
	BCSCTL2 = 0x00;

	// 1ms @ 1MHz
	__delay_cycles(1000);

	// Set GDO2 interrut (CRC OK = packet received)
	P1IE |= GDO2;    	// P1.0 interrupt enabled
	P1IES &= ~GDO2;		// lo/Hi int
	P1IFG &= ~GDO2;  	// P1.0 IFG cleared

	// Set BUTTON interrut (Force read RXBuffer)
	P1IE |= BUTTON;     // P1.3 interrupt enabled
	P1REN |= BUTTON;	// Pull-up res on
	P1IES |= BUTTON;	// Hi/lo int
	P1IFG &= ~BUTTON;   // P1.3 IFG cleared

	// Set UART pins
	P1SEL |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
	P1SEL2 |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD

	// Set UART
	UCA0CTL1 |= UCSSEL_2; 		// SMCLK
	UCA0BR0 = 0x08; 			// 1MHz 115200
	UCA0BR1 = 0x00; 			// 1MHz 115200
	UCA0MCTL = UCBRS2 + UCBRS0; // Modulation UCBRSx = 5
	UCA0CTL1 &= ~UCSWRST; 		// **Initialize USCI state machine**
	UC0IE |= UCA0RXIE; 			// Enable USCI_A0 RX interrupt

	// Set Timer A
	//CCTL0 = CCIE; // CCR0 interrupt enabled
	//TACTL = TASSEL_2 + MC_2; // SMCLK, contmode

	// Set SPI and Radio
	RF_init();

	// 0 dBm
	RF_change_Power(0x50);

	// enable interrupt
	__enable_interrupt();

	while (1) {

	}

	return 0;
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {

}

/* RF RX */
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	char buffer[BUFFER_STD_LENGTH] = {0};
	char length = sizeof(buffer);
	char status[2] = {0};
	char SUCCESS = 0;

	if(RFReceivePacket(buffer, &length, status) == CRC_OK) {
		SUCCESS = 1;
	}

	if(SUCCESS) {
		// Read radio ID
		unsigned char radioID;
		memcpy((char *)&radioID, buffer, sizeof(radioID));

		// Read Temperature
		short temp = 0;
		memcpy((char *)&temp, buffer+sizeof(radioID), sizeof(temp));

		// Read VBat
		short vbat = 0;
		memcpy((char *)&vbat, buffer+sizeof(temp)+sizeof(radioID), sizeof(vbat));

		// Read RSSI
		short rssi = status[0];
		short RSSI_dBm = 0;
		if(rssi >= 128 )
			RSSI_dBm = (rssi - 256)/2 - 74;
		else
			RSSI_dBm = rssi/2 - 74;

		// Packet to send
		S_Packet pack;

		// Def of header
		unsigned char packetType = E_PacketType_VBatAndInternalTemperature;
		unsigned short dataLength = packetTypeDataLength(E_PacketType_VBatAndInternalTemperature);

		memcpy(pack.header.magicNumber, C_MAGIC_NUMBER, sizeof(pack.header.magicNumber));
		memcpy((char *)&pack.header.radioID, (char *)&radioID, sizeof(pack.header.radioID));
		memcpy((char *)&pack.header.packetType, (char *)&packetType, sizeof(pack.header.packetType));
		memcpy((char *)&pack.header.dataLength, (char *)&dataLength, sizeof(pack.header.dataLength));
		memcpy((char *)&pack.header.rssi, (char *)&RSSI_dBm, sizeof(pack.header.rssi));

		// Def of data
		pack.data = (unsigned char *) malloc(sizeof(unsigned char) * dataLength);
		memcpy((char *)&pack.data, (char *)&vbat, sizeof(vbat));
		memcpy((char *)&pack.data+sizeof(vbat), (char *)&temp, sizeof(temp));

		// Send data UART
		memcpy(UART_TX_BUFFER, (char *)&pack, sizeof(S_PacketHeader) + dataLength);
		UART_TX_DATALENGTH = sizeof(S_PacketHeader) + dataLength;

		if(pack.data) free(pack.data);

		// Enable USCI_A0 TX interrupt
		UC0IE |= UCA0TXIE;
	}
	else {
		// Packet to send
		S_Packet pack;

		// Def of header
		unsigned char packetType = E_PacketType_Empty;
		unsigned short dataLength = packetTypeDataLength(E_PacketType_Empty);
		unsigned char fakeRadioID = 0;
		short fakeRssi = 0;
		memcpy(pack.header.magicNumber, C_MAGIC_NUMBER, sizeof(pack.header.magicNumber));
		memcpy((char *)&pack.header.radioID, (char *)&fakeRadioID, sizeof(pack.header.radioID));
		memcpy((char *)&pack.header.packetType, (char *)&packetType, sizeof(pack.header.packetType));
		memcpy((char *)&pack.header.dataLength, (char *)&dataLength, sizeof(pack.header.dataLength));
		memcpy((char *)&pack.header.rssi, (char *)&fakeRssi, sizeof(pack.header.rssi));

		// Send data UART
		memcpy(UART_TX_BUFFER, (char *)&pack, (sizeof(S_PacketHeader) + dataLength));
		UART_TX_DATALENGTH = sizeof(S_PacketHeader) + dataLength;

		// Enable USCI_A0 TX interrupt
		UC0IE |= UCA0TXIE;
	}

	TI_CC_SPIStrobe(TI_CCxxx0_SRX);		// RX mode.

	P1IFG &= ~GDO2;  // P1.0 IFG cleared
	P1IFG &= ~BUTTON;  // P1.0 IFG cleared
}

/* UART RX */
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	if(UART_RX_BUFFER_POS < sizeof(UART_RX_BUFFER))
		UART_RX_BUFFER[UART_RX_BUFFER_POS++] = UCA0RXBUF;
}

/* UART TX */
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
	UCA0TXBUF = UART_TX_BUFFER[UART_TX_BUFFER_POS++]; // TX next character
	UART_TX_DATALENGTH--;

	// End of transmission ?
	if(UART_TX_BUFFER_POS >= sizeof(UART_TX_BUFFER) ||
			UART_TX_DATALENGTH == 0)
	{
		// Clear buffer
		memset(UART_TX_BUFFER, 0, BUFFER_STD_LENGTH);
		UART_TX_BUFFER_POS = 0;
		UART_TX_DATALENGTH = 0;

		// Disable USCI_A0 TX interrupt
		UC0IE &= ~UCA0TXIE;
	}
}
