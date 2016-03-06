#include "../Libs/TI_CC_hardware_board_launchpad.h"
#include "../Libs/TI_CC_msp430.h"
#include "../Libs/TI_CC_spi.h"
#include "../Libs/TI_CC_CC1100-CC2500.h"
#include "../Libs/CC1100-CC2500.h"


#define RADIO_ID	1

// Return internal temperature in °C
short readInteralTemp();

// Return vBat in mV
short readVbat();


int main(void) {
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

	// CPU MCLK and SMBCLK 150kHz
	DCOCTL  = 0x60;		// DCOx = 3, MODx = 0
	BCSCTL1 = 0x81; 	// RSELx = 1
	BCSCTL2 = 0x00;		// MCLK = SMBCLK
	BCSCTL3 |= LFXT1S_2; // FAKE internal VLO ACLK :'( Real external -> LFXT1S_0 + XCAP_3;

	// Disable unused pins
	P1DIR |= 0x8F;
	P1OUT &= ~0x8F;
	P2DIR |= 0x3F;
	P2OUT &= ~0x3F;

	// 1ms @ 150kHz
	__delay_cycles(150);

	// Set SPI and Radio
	RF_init();

	// Radio Sleep
	Radio_GotoSleep();

	// Set Timer A - 56s (7.5s sans ID_3)
	// Watch for timer_demult in time_A
	CCTL0 = CCIE; // CCR0 interrupt enabled
	TA0CTL = TASSEL_1 + ID_3 + MC_2 + TAIE; // ACLK, ACLK/8, Continous up, interrupt enable (TAIV)

	// LPM3 with interrupt
	__enable_interrupt();
	LPM3;

	while(1) {

	}
	
	return 0;
}


static volatile int timer_demult = 0;

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	// "TAIV == 0x0A" => Timer_A overflow
	if(TAIV == 0x0A) {
		// TIMER_A x5
		if(++timer_demult < 5) return;
		timer_demult = 0;
		LPM3_EXIT;

		char txBuffer[16+1] = {0};

		//[0] Packet length -- NEEDED FOR CC110L
		txBuffer[0] = 16;

		//[1] Radio ID
		unsigned char radioID = RADIO_ID;
		memcpy(txBuffer+1, (char *)&radioID, sizeof(radioID));

		//[2][3] Internal temperature
		short temp = readInteralTemp();
		memcpy(txBuffer+2, (char *)&temp, sizeof(temp));

		//[4][5] VBat
		short vbat = readVbat();
		memcpy(txBuffer+4, (char *)&vbat, sizeof(vbat));

		// Wakeup radio
		Radio_WakeUp();

		// 1ms @ 150kHz
		__delay_cycles(150);

		// Send packet
		RFSendPacket(txBuffer, sizeof(txBuffer));

		// Radio go to sleep
		Radio_GotoSleep();

		__enable_interrupt();
		LPM3;
	}
}

short readInteralTemp()
{
	// Internal temp reading
	ADC10CTL0 = SREF_1 + REFON + ADC10ON + ADC10SHT_3; //1.5V ref, Ref on, 64 clocks for sample
	ADC10CTL1 = INCH_10 + ADC10DIV_3; //temp sensor is at 10 and clock/4, SMKCLK
    ADC10CTL0 |= ENC + ADC10SC;       //enable conversion and start conversion

    while(ADC10CTL1 & BUSY);         	//wait convertion
    unsigned short raw_temp = ADC10MEM; //store val in t

    // Disable ADC for power saving
    ADC10CTL0 &= ~ENC;
	ADC10CTL0 &= ~(REFON + ADC10ON);
	ADC10CTL0 =0;

    return(short) ((raw_temp - 673) * 423L) >> 10; //convert and pass

    //return(short) ((raw_temp - 673) * 423L) >> 10; //convert and pass
    //return(short) ((raw_temp * 27069L - 18169625L) >> 16); //convert and pass
}

short readVbat() {
	short vbat = 0;

	// Internal temp reading
	ADC10CTL0 = SREF_1 + REFON + ADC10ON + ADC10SHT_0; //1.5V ref,Ref on,4 clocks for sample
	ADC10CTL1 = INCH_11 + ADC10DIV_3; //vcc/2 is at chan 11 and clock/4
	ADC10CTL0 |= ENC + ADC10SC;       //enable conversion and start conversion

	// wait convertion
	while(ADC10CTL1 & BUSY);
	unsigned short raw_value = ADC10MEM;

	// Disable ADC for power saving
	// Next read fail if not stopped
	ADC10CTL0 &= ~ENC;
	ADC10CTL0 &= ~(REFON + ADC10ON);
	ADC10CTL0 =0;

	// check for overflow
	if (raw_value == 0x3ff) {
		// switch range - use 2.5V reference (Vcc >= 3V)
		ADC10CTL0 = SREF_1 + REF2_5V + REFON + ADC10ON + ADC10SHT_0;
		ADC10CTL1 = INCH_11 + ADC10DIV_3;
		ADC10CTL0 |= ENC + ADC10SC;

		while (ADC10CTL1 & ADC10BUSY);
		raw_value = ADC10MEM;

		// Disable ADC for power saving
		ADC10CTL0 &= ~ENC;
		ADC10CTL0 &= ~(REFON + ADC10ON);
		ADC10CTL0 =0;

		vbat = (raw_value * 5000L) / 1024;
	}
	else {
		vbat = (raw_value * 3000L) / 1024;
	}

	return vbat;
}
