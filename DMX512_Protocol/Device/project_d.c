// Zankar, Mobin, Toshant

//----------------------------------------------------------------------------
// Device includes and assembler directives             
//----------------------------------------------------------------------------
#include <p33FJ128MC802.h>
#define FCY 40000000UL            // instruction cycle rate (ignore)
#include <libpic30.h>             // __delay_ms and __delay_us
#include <stdio.h>
#include <string.h>				  // For string functions

#include "serial/serial_d.h"	  // UART-1 and UART-2 Funtions

volatile int green_count,red_count,green_count2;

//----------------------------------------------------------------------------
// Subroutines                
//----------------------------------------------------------------------------

// Initialize Hardware
void init_hw()
{
  	LATBbits.LATB4 = 0;          // write 0 into output latches
  	LATBbits.LATB5 = 0;
  	TRISBbits.TRISB4 = 0;        // make green led pin an output
  	TRISBbits.TRISB5 = 0;        // make red led pin an output
  	AD1PCFGLbits.PCFG5 = 1;		 // Pin 7 Analog to Digital Conversion
  	// Orange LED
  	AD1PCFGLbits.PCFG4 = 1;	     // Pin 6 Analog to Digital Conversion
  	LATBbits.LATB2 = 0;
  	TRISBbits.TRISB2 = 0;        // make orange led pin an output  	
  	RPINR19bits.U2RXR = 7;       // assign U2RX to RP7
	RPOR3bits.RP6R = 5;          // assign U2TX to RP6	
	LATBbits.LATB8 = 0;			 // DEN = 0
  	TRISBbits.TRISB8 = 0;		 // Make RB8 as digital output	
  	TRISBbits.TRISB15 = 1;       // input pin 26
  	TRISBbits.TRISB14 = 1;       // input pin 25
  	TRISBbits.TRISB13 = 1;       // input pin 24
  	TRISBbits.TRISB12 = 1;       // input pin 23
  	TRISBbits.TRISB11 = 1;       // input pin 22
  	TRISBbits.TRISB10 = 1;       // input pin 21
  	TRISBbits.TRISB9 = 1;        // input pin 18
  	TRISAbits.TRISA4 = 1;        // input pin 12
  	TRISBbits.TRISB3 = 1;        // input pin 7
  	CNPU1bits.CN11PUE = 1;       // enable pull-up for pin 26
  	CNPU1bits.CN12PUE = 1;       // enable pull-up for pin 25
  	CNPU1bits.CN13PUE = 1;       // enable pull-up for pin 24
  	CNPU1bits.CN14PUE = 1;       // enable pull-up for pin 23
  	CNPU1bits.CN15PUE = 1;       // enable pull-up for pin 22
  	CNPU2bits.CN16PUE = 1;       // enable pull-up for pin 21
	CNPU2bits.CN21PUE = 1;       // enable pull-up for pin 18
  	CNPU1bits.CN0PUE = 1;        // enable pull-up for pin 12
  	CNPU1bits.CN7PUE = 1;        // enable pull-up for pin 7  
  	RPOR1bits.RP2R = 18;         // connect OC1 to RP2 (PWM)
  	PLLFBDbits.PLLDIV = 38;      // pll feedback divider = 40;
  	CLKDIVbits.PLLPRE = 0;       // pll pre divider = 2
  	CLKDIVbits.PLLPOST = 0;      // pll post divider = 2 
}
//----------------------------------------------------------------------------

void pwm_init()
{
  	T2CONbits.TCS = 0;			// Internal Clock
  	T2CONbits.TCKPS = 0;		// Prescalar = 1
  	T2CONbits.TON = 1;			// Turn ON Timer 2
  	PR2 = 255;					// Value set to compare with data value
  	
  	OC1CON = 0;					// Turn off OC1
  	OC1R = 0;					// set first cycle d.c.
  	OC1RS = 0;
  	OC1CONbits.OCTSEL = 0;		// Timer 2 select
  	OC1CONbits.OCM = 6;			// PWM mode with fault pin disabled
}
//----------------------------------------------------------------------------

//Address read
int read_address()
{
 	int addr = 0;
 	int b15=0,b14=0,b13=0,b12=0,b11=0,b10=0,b9=0,a4=0,b3=0;
 
 	if(PORTBbits.RB15 == 0)		// Set address bit for pin 26
 	{ b15 = 256; }
 	if(PORTBbits.RB14 == 0)		// Set address bit for pin 25
 	{ b14 = 128; }
 	if(PORTBbits.RB13 == 0)		// Set address bit for pin 24
 	{ b13 = 64; }
 	if(PORTBbits.RB12 == 0)		// Set address bit for pin 23
 	{ b12 = 32; }
 	if(PORTBbits.RB11 == 0)		// Set address bit for pin 22
 	{ b11 = 16; }
 	if(PORTBbits.RB10 == 0)		// Set address bit for pin 21
 	{ b10 = 8; }
 	if(PORTBbits.RB9 == 0)		// Set address bit for pin 18
 	{ b9 = 4; }
 	if(PORTAbits.RA4 == 0)		// Set address bit for pin 12
 	{ a4 = 2; }
 	if(PORTBbits.RB3 == 0)		// Set address bit for pin 7
 	{ b3 = 1; }
 
 	addr = b15 + b14 +	b13 + b12 +	b11 + b10 +	b9 + a4 + b3 + 1;

 	return addr;
}
//----------------------------------------------------------------------------

void init_timer()
{
	T1CON = 0;
	T1CONbits.TCS = 0;			// Internal clock
	T1CONbits.TCKPS = 2;		// Prescale by 64 (40M/64 = 625000)
	T1CONbits.TON = 1;			// Start the timer
	PR1 = 625;					// Set period for 1ms
  	IEC0bits.T1IE = 1;			// Enable timer 1 interrupts
}
//----------------------------------------------------------------------------

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt (void)
{
	if(green_count2 > 0)
		green_count2--;			// Counter-2 Green LED for data change
	if(green_count > 0)
		green_count--;			// Counter for Green LED
	if(red_count > 0)
		red_count--;			// Counter for Red LED
	
	if(green_count == 0)
	{
		LATBbits.LATB4 = 0;		// Turn off green LED
	}
	if(green_count2 == 0)
	{
		LATBbits.LATB4 = 1;		// Turn on green LED
	}
	if(red_count == 0)
	{
		LATBbits.LATB5 = 0;		// Turn off red LED
	}
  	IFS0bits.T1IF = 0;			// clear Interrupt flag
}	
//----------------------------------------------------------------------------

void blink_greenLED()
{
	LATBbits.LATB4 = 1;		// Turn ON Green LED
	green_count = 1000;		// Counter for 1 sec
}
//----------------------------------------------------------------------------

void blink_redLED()
{
	LATBbits.LATB5 = 1;		// Turn ON Red LED
	red_count = 250;		// Counter for 250 ms
}
//----------------------------------------------------------------------------

//main function
int main(void)
{
  	int addr,i;
  	unsigned char data,start_code,break_mab,prev_data;
  	init_hw();                   // initialize hardware
  	uart2_init(BAUD_250k);       // configure uart-2
 	init_timer();				 // Initialize timer 1
  	LATBbits.LATB4 = 1;		 	 // Green LED ON
  	__delay_ms(500);			 // Wait for 500ms
  	LATBbits.LATB4 = 0;			 // Green LED OFF
  	prev_data = 0;				 // Initialize previous data
  	pwm_init();					 // Initialize pwm
  	while(1)
  	{
   		addr = read_address();
   		if(U2STAbits.URXDA)
   		{
	   		if(U2STAbits.FERR)	// Check Frame Error
	   		{
		   		break_mab = uart2_getc();
		   		if (break_mab == 0)		// Chek for break and MAB
		   		{
			   		if(green_count2 <= 0)
			   			blink_greenLED();
			   		start_code = uart2_getc();	// Stop bit received
			   		if(start_code == 0)
			   		{
			   			for(i = 1; i <= 512; i++)
				   		{
				   			data = uart2_getc();// Get the data bytes
				   			if (i == addr)	// Check for device address
				   			{
					   			// Check for change in device data
					   			if (data != prev_data)
					   			{
						   			green_count2 = 250;
						   			prev_data = data;
						   			LATBbits.LATB4 = 0;
						   		}	
				   				break;
				   			}
			   			}
   						OC1RS = data;	// Set PWM value for Orange LED
			   		}
			   		else 
			   		// Detection of poll command
			   		if(start_code == 0xF0)		
			   		{
				   		for(i = 1; i <= 512; i++)
				   		{
				   			if (i == addr)	//Check for data at device address
				   			data = uart2_getc();
				   			else
				   			uart2_getc();
			   			}
			   			__delay_us(1);
   						if(data > 0)
   						{
	   						// Transmitter ON (DEN = 1)
	   						LATBbits.LATB8 = 1;		
   							// To check previous transmission
   							while(!U2STAbits.TRMT);	
							// Slowdown baud rate to 92592 for break
							U2BRG = BAUD_92592;
							// Transmit break		
							uart2_txChar(0x00);	
							// To check break transmission	
							while(!U2STAbits.TRMT);	
							// Transmitter OFF (DEN = 0)
							LATBbits.LATB8 = 0;	
							// Baudrate set to 250k	
   							U2BRG = BAUD_250k;
   							// Blink Red LED		
   							blink_redLED();				
				   		}
   			   		}	
		   		}
	   		}
   			else uart2_getc();	   		
   		}
   		else
   		{
	   		LATBbits.LATB4 = 0;		// If no transmission from controller
   		}   		
  	}
  	return 0;
}
