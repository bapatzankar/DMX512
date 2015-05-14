// Zankar, Mobin, Toshant

//----------------------------------------------------------------------------
// Device includes and assembler directives             
//----------------------------------------------------------------------------
#include <p33FJ128MC802.h>
#define FCY 40000000UL                // instruction cycle rate (ignore)
#include <libpic30.h>                 // __delay_ms and __delay_us
#include <stdio.h>
#include <stdlib.h>
#include <string.h>					  // For string functions
#include "serial/serial.h"			  // UART-1 and UART-2 functions

#define MAX_LENGTH 21				  // Maximum length of input string
#define DEL '\0'
#define buffer_size 256				  // Size of ring buffer
//----------------------------------------------------------------------------
// Variables                
//----------------------------------------------------------------------------
//Global variables
  char str[MAX_LENGTH];			  // Input string (maxLength + 1)
  char type[3];
  int pos[3];
  int field_count, addr, data, maxAddr, tx_on, count;
  int poll_flag, dev_present, FULL, EMPTY;
  unsigned char data512[513], poll_data[513];
  unsigned char buffer[buffer_size];
  volatile int green_count, red_count, rd_index, wr_index;
//----------------------------------------------------------------------------
// Subroutines                
//----------------------------------------------------------------------------
// Initialize Hardware
void init_hw()
{
  	LATBbits.LATB4 = 0;			// write 0 into output latches
  	LATBbits.LATB5 = 0;
  	TRISBbits.TRISB4 = 0;		// make green led pin an output
  	TRISBbits.TRISB5 = 0;		// make red led pin an output
  	RPINR18bits.U1RXR = 11;		// assign U1RX to RP11
  	RPOR5bits.RP10R = 3;		// assign U1TX to RP10
  	RPINR19bits.U2RXR = 7;		// assign U2RX to RP7
	RPOR3bits.RP6R = 5;			// assign U2TX to RP6
  	LATBbits.LATB8 = 1;			// Transmitter ON (DEN = 1)
  	TRISBbits.TRISB8 = 0;		// Make RB8 as digital output
	
	PLLFBDbits.PLLDIV = 38;		// pll feedback divider = 40;
  	CLKDIVbits.PLLPRE = 0;		// pll pre divider = 2
  	CLKDIVbits.PLLPOST = 0;		// pll post divider = 2 
}
//----------------------------------------------------------------------------
// Initialize Timer-1
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
// Timer-1 ISR
 void __attribute__((interrupt, no_auto_psv)) _T1Interrupt (void)
{
	if(green_count > 0)
		green_count--;			// Counter for Green LED
	if(red_count > 0)
		red_count--;			// Counter for Red LED
	
	if(green_count == 0)
	{
		LATBbits.LATB4 = 0;		// Turn off green LED
	}
	if(red_count == 0)
	{
		LATBbits.LATB5 = 0;		// Turn off red LED
	}
  	IFS0bits.T1IF = 0;			// clear Interrupt flag
}	
//----------------------------------------------------------------------------
// Function to blink Green LED
void blink_greenLED()
{
	LATBbits.LATB4 = 1;			// Turn off green LED
	green_count = 250;			// Counter for 250 ms
}
//----------------------------------------------------------------------------
// Function to blink Red LED
void blink_redLED()
{
	LATBbits.LATB5 = 1;			// Turn off Red LED
	red_count = 250;			// Counter for 250 ms
}
//----------------------------------------------------------------------------
// Function to check status of Ring Buffer
void check_bufferStatus()
{
	// Check if Buffer is Empty
	if(rd_index == wr_index)	
	{ EMPTY = 1; }
	else
	{ EMPTY = 0; }
	
	// Check if Buffer is Full
	if(((wr_index+1)%buffer_size) == rd_index)	
	{ FULL = 1;}
	else
	{ FULL = 0; }
}
//----------------------------------------------------------------------------
// Function to send the string to buffer
void send_string(char *str)
{
	int i;
	for(i = 0; i < strlen(str); i++)
	{
		// Write to Buffer if it is Empty or Not Full
		if(!FULL || EMPTY)
		{
			buffer[wr_index++] = str[i];
			if(wr_index > 255)		//Roll back to starting index
				wr_index = 0;
			check_bufferStatus();
		}
		else
		{ break; }
	}
	// Check if the transmit buffer is empty 
	if(U1STAbits.UTXBF == 0)
	{
		U1TXREG = buffer[rd_index++];
		if(rd_index > 255)		//Roll back to starting index
			rd_index = 0;
	}				
}
//----------------------------------------------------------------------------
// Function to send a character to buffer
void send_char(char c)
{
	check_bufferStatus();

	// Write to Buffer if it is Empty or Not Full
	if(!FULL || EMPTY)
	{
		buffer[wr_index++] = c;
		if(wr_index > 255)		//Roll back to starting index
			wr_index = 0;
	}

	// Check if the transmit buffer is empty 
	if(U1STAbits.UTXBF == 0)
	{
		U1TXREG = buffer[rd_index++];	
		if(rd_index > 255)		//Roll back to starting index
			rd_index = 0;
	}
}
//----------------------------------------------------------------------------
// UART-1 ISR
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void)
{
	// Read buffer if buffer is not Empty
	if(rd_index != wr_index)
	{
		U1TXREG = buffer[rd_index++];
		if(rd_index > 255)		//Roll back to starting index
			rd_index = 0;
	}
	IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag
}
//----------------------------------------------------------------------------
// Function to check whether character is alpha
int isAlpha(char c)
{
 	if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
 	{ return 1; }
	return 0;
}
//----------------------------------------------------------------------------
// Function to check whether character is numeric
int isNumeric(char c)
{
 	if(c >= '0' && c <= '9')
 	{ return 1; }
 	return 0;
}
//----------------------------------------------------------------------------
// Function to display error message
void displayError()
{
	send_string("This is an invalid string!\r\n");
	send_string("------------------------------------\r\n");
}
//----------------------------------------------------------------------------
//Function to process the command
int parseCmd()
{
	int i;
 	field_count = 0;

 	for (i = 0; i < strlen(str); i++)
 	{
 	//------------Check if input character is alpha------------
  		if(isAlpha(str[i]))
  		{
  			if(i == 0)		// If start of string
   			{
				pos[field_count] =  i;
				type[field_count] = 'a';
				field_count++;
			}
   			else if(i > 0)
   			{
				// Transition a->n
				if(isNumeric(str[i+1]))
				{
					displayError();		// Display Error Message
					return 0;			// Invalid String
				}
				else 
					// To check it is the first alpha in string
					if(!isAlpha(str[i-1]) && !isNumeric(str[i-1]))	
					{
	 					pos[field_count] =  i;
	 					type[field_count] = 'a';
	 					field_count++;
					}
   			}
  		}
  		else 
			//--------Check if input character is numeric---------
  			if (isNumeric(str[i]))
  			{
   				// Transition n->a
    			if(isAlpha(str[i+1]))
				{
					displayError();		// Display Error Message
					return 0;			// Invalid String
				}
				else 
					// Transition d->n
					if(!isAlpha(str[i-1]) && !isNumeric(str[i-1]))
					{
	 					pos[field_count] =  i;
	 					type[field_count] = 'n';
	 					field_count++;
					}
			}
 	}

	// Replace delimiters by '\0'--null character
 	for (i = strlen(str)-1; i >=0 ; i--)
 	{
  		if (!(isAlpha(str[i]) || isNumeric(str[i])))
  		{ str[i] = DEL; }
 	}

	return 1;
}
//----------------------------------------------------------------------------
//To check argument type number
int isArgNumber(int index)
{
	if (isNumeric(str[pos[index]]))
	{ return 1; }
	return 0;
}
//----------------------------------------------------------------------------
//To check argument type alpha
int isArgAlpha(int index)
{
	if (isAlpha(str[pos[index]]))
	{ return 1; }
	return 0;
}
//----------------------------------------------------------------------------
//To get the required argument
int getArgNumber(int index)
{
	int argNum = 0;
	argNum = atoi(&str[pos[index]]);
	return argNum;
}
//----------------------------------------------------------------------------
//To check if the command is correct
int isCmd(char *v_str, int count)
{
	return (strcmp(v_str,&str[pos[0]]) == 0 && count == field_count);
}
//----------------------------------------------------------------------------
// To set data array to Zero
void clearData()
{
	int i = 0;
	for(i = 0; i <= 512; i++)
	{
		data512[i] = 0x00;
	}
}
//----------------------------------------------------------------------------
// To set Poll data array
void set_pollData()
{
	int i;
	for(i = 0; i <= 512; i++)
	{
		poll_data[i] = 0;
	}
}
//----------------------------------------------------------------------------
// To poll devices present
int poll_command(int start_index, int end_index)
{
	int i, mid_index;
	int response = 0;			// Clear response flag

	while(!U2STAbits.TRMT);		// To check the completion of previous transmission
	U2BRG = BAUD_92592;    		// configure uart-2 for Break and MAB
	uart2_txChar(0x00);			// Send Break and MAB
	
	while(!U2STAbits.TRMT);		// To check the completion of previous transmission
	U2BRG = BAUD_250k;         	// configure uart-2 for start_code and data bytes
	uart2_txChar(0xF0);			// Send Start code for poll command
	
	// Send 512 data bytes----------------------
	if(start_index > 1)
	{
		for(i = 1; i < start_index; i++)
		{
		  	uart2_txChar(0);
		}
	}
	
	for(i = start_index; i <= end_index; i++)
	{
	  	uart2_txChar(1);
	}
	
	if(end_index < 512)
	{
		for(i = end_index+1; i <= 512; i++)
		{
	  		uart2_txChar(0);
		}
	}	
	//-----------------------------------------	
	 	
 	// Check for the response (Break from device)
 	while(!U2STAbits.TRMT);		// To check previous transmission
 	LATBbits.LATB8 = 0;			// Transmitter OFF
 	
 	for(i=0; i<50; i++)
 	{
 		__delay_us(1);
 		if(U2STAbits.URXDA)
 		{
			if(U2STAbits.FERR)	// Check Frame Error
			{
				if(uart2_getc() == 0)	// Check for break
				{
					blink_redLED();		//Blink Red LED
					response = 1;
				}
			}
			else uart2_getc();
 		}
 	}	
 	__delay_us(50);
 	while(!U2STAbits.RIDLE);	// Check if receiver is IDLE
 	LATBbits.LATB8 = 1;			// Transmitter ON
	//--------------------------------------------
			
	if(response == 1)
	{
		dev_present = 1;				// Set flag if any device is present
		if(start_index == end_index)
		{
			poll_data[start_index] = 1;	// Flag the device found
			return 1;
		}
		
		mid_index = (start_index + end_index - 1) / 2;
		// Recursive call for first half
		poll_command(start_index, mid_index);	
		// Recursive call for second half
		poll_command((mid_index+1), end_index);	
	}
	else
	{
		// device absent
		return 1;
	}
	return 1;
}
//----------------------------------------------------------------------------
// Command processing
void processCommands()
{
	char str1[220];		// For general conversion from integer to string

	if(parseCmd())
   	{
    	if(isCmd("set",3))		// Validate Set command
		{
 			if(isArgNumber(1) && isArgNumber(2))
 			{
				addr = getArgNumber(1);		// Get the value of address
				data = getArgNumber(2);		// Get the value of data
				if(addr > maxAddr || addr <= 0 || data > 255 || data < 0)
			  	{
					sprintf(str1,"Address limit is 1 to %d and value limit is 0 to 255 \r\n",maxAddr);
 					send_string(str1);
 					send_string("------------------------------------\r\n");
					
  				}
  				else
  				{
	  				send_string("Set command entered.\r\n");
					sprintf(str1,"Address : %d\r\n",addr);
					send_string(str1);
					sprintf(str1,"Data : %d\r\n",data);
					send_string(str1);
					send_string("------------------------------------\r\n");
					data512[addr] = data;
	  				blink_greenLED();
				}
 			}
 			else 
 			{	displayError();	}
		}
		else 
			if (isCmd("get",2))		// Validate Get command
			{
 				if(isArgNumber(1))
 				{
					addr = getArgNumber(1);	// Get the first argument
					if(addr > maxAddr || addr <= 0)
				  	{
						sprintf(str1,"Address limit is 1 to %d\r\n",maxAddr);
 						send_string(str1);
 						send_string("------------------------------------\r\n");
  					}
  					else
  					{	
	 					sprintf(str1,"Data at given addr is : %d\r\n",data512[addr]);
	 					send_string(str1);
	 					send_string("------------------------------------\r\n");
	  					blink_greenLED();	// Blink Green LED
	 				}
 				}
				else 
				{	displayError();	}
			}
			else 
				if (isCmd("clear",1))	// Validate Clear Command
				{
					clearData();
					blink_greenLED();	
					send_string("Clear the values of all the addresses.\r\n");
					send_string("------------------------------------\r\n");
						
				}
				else 
					if (isCmd("on",1))	// Validate ON command
					{
						tx_on = 1;
						send_string("DMX stream sent continuously.\r\n");
						send_string("------------------------------------\r\n");
						blink_greenLED();	// Blink Green LED
					}
					else 
						if (isCmd("off",1))	// Validate OFF command
						{
							tx_on = 0;
							send_string("DMX stream will not be sent.\r\n");
							send_string("------------------------------------\r\n");
							blink_greenLED();	//Blink Green LED
						}
						else 
							if (isCmd("max",2))	// Validate MAX command
							{
								if(isArgNumber(1))
 								{
  									if(getArgNumber(1) <= 512 && getArgNumber(1) >= 1)
									{
										maxAddr = getArgNumber(1);
										sprintf(str1,"The maximum addresses are set to %d\r\n",maxAddr);
										send_string("------------------------------------\r\n");
										send_string(str1);	
										blink_greenLED();	// Blink Green LED
									}
  									else 
  									{ 
  										send_string("Maximum value of adress should be <= 512\r\n");
  										send_string("------------------------------------\r\n");
  									}
 								}
								else 
								{ displayError(); }
							}
							else
								if (isCmd("poll",1))	// Validate Poll Command
								{
 									send_string("Poll command entered.\r\n");
 									poll_flag = 1;
									blink_greenLED();
 								}
								else
								{	displayError();	}
   	}   			
}


//----------------------------------------------------------------------------
// Function to get input string from user
void getInputString()
{
	char charRx;						// Character received
  	
	if(count < MAX_LENGTH-1)
	{
   		charRx =  serial_getc(); 		// Receive character
   		
   		if(charRx == '\b')				// Check for backspace
		{
 			if(count > 0)
 			{
	 			count--; 
				str[count] = ' ';
   				send_char(' ');
   				send_char('\b');
 			}
		}
   		else
		{
 			if(charRx == '\r')		// Check for carriage return
 			{ 
				send_char('\n');
  				str[count] = '\0';	// Padd null at the end of the string
   				count = 0;
   				processCommands();	// Process command if Carriage Return
 			}
 			else
  				if(charRx >= ' ')
  				{
	  				// Increment count and store char in string
  					str[count++] = charRx;	
  				}
		} 
	}
	else
	{
 		send_char('\r');
		send_char('\n');
   		str[count] = '\0'; 
 		count = 0;
 		processCommands();		// Process command if exceed max length
	}
}
//----------------------------------------------------------------------------
//main function
int main(void)
{
	int i;
    pos[0] = 0,pos[1] = 0,pos[2] = 0;	// Clear the position vector
	rd_index = 0, wr_index = 0;			// Initialize buffer Index
  	EMPTY = 1, FULL = 0;
    tx_on = 1;    						// Transmitter ON by default
  	maxAddr = 512;						// Default Maz addresses set to 512
  	init_hw();                      	// initialize hardware
  	uart1_init(BAUD_19200);         	// configure uart-1	with 19200 bps
  	uart2_init(BAUD_250k);				// Configure UART-2 with 250k bps
	LATBbits.LATB4 = 1;					// Green LED ON
  	__delay_ms(500);					// Wait for 500ms
  	LATBbits.LATB4 = 0;					// Green LED OFF
    init_timer();						// Initialize timer 1
  	clearData();						// Clear the data512 array
    count = 0;							// Initialize the count
    poll_flag = 0;						// Clear the Poll flag
    send_string("Ready !\r\n");
  	while(1)
  	{
	  	if(tx_on)
	  	{
			while(!U2STAbits.TRMT);		// To check previous transmission
			U2BRG = BAUD_92592;         // configure uart-2 for Break and MAB
			uart2_txChar(0x00);			// Send Break and MAB
			
			while(!U2STAbits.TRMT);		// To check previous transmission
			U2BRG = BAUD_250k;      	// configure uart-2 for Data Slots
	
		  	for(i = 0; i <= maxAddr; i++)
		  	{
			  	//Start code at i=0
			  	uart2_txChar(data512[i]);	// Send data slots 0 to N
			  	if(U1STAbits.URXDA)
			  	{
			  		getInputString();	// To get Input String frm user
			  	}
		  	}
	  	}
	  	else
	  	{
	   		getInputString();			// To get Input String from user
	   	}
	   	if(poll_flag ==  1)				// Check for poll command
	   	{
		   	int i;
		   	dev_present = 0;
		   	set_pollData();				// Set the poll_data array to Zero
		   	poll_command(1,512);		// Poll all the devices
		   	
		   	if(dev_present)
		   	{
 				send_string("The devices present: "); 				
 				for(i = 1; i <= 512; i++)
 				{
	 				if(poll_data[i] == 1)
	 				{
		 				char c[3];
		 				itoa(c,i,10);		// Get present device address
		 				send_string(c);		// Print device address
		 				send_char(' ');
	 				}
 				}
 				send_string("\r\n------------------------------------\r\n");
 			}
 			else
 			{
	 			// If no device ever responded
	 			send_string("No device present!\r\n");	
	 			send_string("------------------------------------\r\n");
 			}
 			
 			poll_flag = 0;
	   	}
  	}
  return 0;
}
