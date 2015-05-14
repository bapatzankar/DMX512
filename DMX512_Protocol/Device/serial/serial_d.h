#define BAUD_19200 129               // BRG for low-speed, 40 MHz clock
                                     // round((40000000/16/19200)-1)
#define BAUD_250k 9					 // BRG for UART-2 250 kHz
#define BAUD_92592 26				 // BREAK = 97.2us and MAB = 21.6us

// UART-2 funtions
void uart2_init(int baud_rate);		 // UART-2 Baud rate settings
void uart2_txString(char str1[]);	 // Transmit string to UART2
void uart2_txChar(char s);			 // Transmit Character to UART2
char uart2_getc();					 // To receive data at UART2

