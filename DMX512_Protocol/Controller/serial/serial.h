#define BAUD_19200 129                       // brg for low-speed, 40 MHz clock
                                             // round((40000000/16/19200)-1)
#define BAUD_250k 9							 // BRG for UART-2 250 kHz
#define BAUD_92592 26						 // BREAK = 97.2us and MAB = 21.6us

// UART-1 funtions
void uart1_init(int baud_rate);				// UART-1 Baud rate settings
void uart1_txString(char str1[]);			// Transmit string to UART1
void uart1_txChar(char s);					// Transmit Character to UART1
char serial_getc();							// Receive character from UART1

// UART-2 funtions
void uart2_init(int baud_rate);				// UART-2 Baud rate settings
void uart2_txString(char str1[]);			// Transmit string to UART2
void uart2_txChar(char s);					// Transmit Character to UART2
char uart2_getc();							// Receive character from UART2

