#include <p33FJ128MC802.h>
#include <string.h>			// For string functions

//UART-1 Baud rate settings
void uart1_init(int baud_rate)
{
  	// set baud rate
  	U1BRG = baud_rate;
  	// enable uarts, 8N1, low speed brg
  	U1MODE = 0x8000;
  	// enable tx and rx
  	U1STA = 0x0400;
  	// Interrupt after one TX character is transmitted
  	U1STAbits.UTXISEL1 = 1;	
	U1STAbits.UTXISEL0 = 0;
	// Enable UART TX interrupt
	IEC0bits.U1TXIE = 1;	
  	// Clear Interrupt Flag
  	IFS0bits.U1TXIF = 0;	
}
/*------------------------------------------*/

// Receive character from UART1
char serial_getc()
{
  	// clear out any overflow error condition
  	if (U1STAbits.OERR == 1)
  		U1STAbits.OERR = 0;
  	// wait until character is ready
  	while(!U1STAbits.URXDA);
  	return U1RXREG;
}
/*------------------------------------------*/

// Transmit string to UART1
void uart1_txString(char str1[])
{
  	int i;
  	for (i = 0; i < strlen(str1); i++)
  	{
    	// make sure buffer is empty
    	while(U1STAbits.UTXBF);
    	// write character
    	U1TXREG = str1[i];
  	}
}
/*------------------------------------------*/

//Transmit Character to UART1
void uart1_txChar(char s)
{
   	// make sure buffer is empty
   	while(U1STAbits.UTXBF);
   	// write character
   	U1TXREG = s;
}
/*------------------------------------------*/

//UART-2 Baud rate settings
void uart2_init(int baud_rate)
{
  	// set baud rate
  	U2BRG = baud_rate;
  	// enable uarts, 8N2, low speed brg
  	U2MODE = 0x8001;
  	// enable tx and rx
  	U2STA = 0x0400;
}
/*------------------------------------------*/

// Receive character from UART1
char uart2_getc()
{
  	// clear out any overflow error condition
  	if (U2STAbits.OERR == 1)
  		U2STAbits.OERR = 0;
  	// wait until character is ready
  	while(!U2STAbits.URXDA);
  	return U2RXREG;
}
/*------------------------------------------*/

// Transmit string to UART2
void uart2_txString(char str1[])
{
  	int i;
  	for (i = 0; i < strlen(str1); i++)
  	{
    	// make sure buffer is empty
    	while(U2STAbits.UTXBF);
    	// write character
    	U2TXREG = str1[i];
  	}
}
/*------------------------------------------*/

// Transmit Character to UART2
void uart2_txChar(char s)
{
   	// make sure buffer is empty
   	while(U2STAbits.UTXBF);
   	// write character
   	U2TXREG = s;
}

