#include <p33FJ128MC802.h>
#include <string.h>				  // For string functions

// Receive character from UART2
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
