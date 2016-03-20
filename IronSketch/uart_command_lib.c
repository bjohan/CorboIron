#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "uart_command_lib.h"
#include <stdlib.h>
/** Receive buffer*/
volatile char rxBuffer[RX_BUFFER_SIZE];

/** Transmit buffer*/
volatile char txBuffer[TX_BUFFER_SIZE];
/** Transmit buffer position*/
volatile int8_t txByte = -1;
/** Receive buffer position*/
volatile int8_t rxByte  = 0;

volatile uint8_t dataOverrun = 0;

const char nl[] PROGMEM = "\r\n";

void waitTransmissionIdle()
{
	while(txByte >= 0);
	while( !(UCSR0A & (1 << UDRE0)));
}

int strcmp_pn(const char *a, char *b)
{
	while(1){
		if((pgm_read_byte(a) != *b) || (*b == 0)){
			return pgm_read_byte(a)-*b;
		}
		a++;b++;
	}
}

/** Copy and reverse data from src to dst.
 * \param dst pointer to the destination
 * \param src pointer to the source
 * \param len the number of bytes to be copied
 */
void copyReverse(volatile char *dst, volatile const char *src, uint8_t len)
{
        uint8_t i;
        for(i = 0 ; i < len ; i++)
                dst[i]=src[len-i-1];
}

/** Copy and reverse data from src (in program memory) to dst.
 * \param dst pointer to the destination
 * \param src pointer to the source data, must residde in the program memory.
 * \param len the number of bytes to be copied
 */
void copyReverseP(volatile char *dst, volatile const char *src, uint8_t len)
{
        uint8_t i;
        for(i = 0 ; i < len ; i++)
                dst[i]=pgm_read_byte(src+len-i-1);
}

/** strlen for strings stored in program memory.
\param str the string for which the length should be calculated.
\return the length of the string. Not including the terminating null character.
*/
uint16_t strlenP(const char *str)
{
	uint16_t len = 0;
	while(pgm_read_byte(str++))
		len++;
	return len;
}

/** Copy data from src to dst.
 * \param dst pointer to the destination
 * \param src pointer to the source
 * \param len the number of bytes to be copied
 */
void copy(volatile char *dst, volatile const char *src, uint8_t len)
{
        uint8_t i;
        for(i = 0 ; i < len ; i++)
                *(dst++)=*(src++);
}


/** Receive interrupt for uart. Reads a byte from the register and puts it in
 * the receive buffer if there is room. Otherwise nothing is done. Does not
 * check for data overrun in the receive register/buffer.
 */
/*SIGNAL(USART_RX_vect)
{
	cli();
        static int8_t chr;
        chr = UDR0;
        if(rxByte < RX_BUFFER_SIZE){
                rxBuffer[rxByte++]=chr;
		//transmit((char*)&chr, 1);
        } else {
		PORTB |= _BV(0);
		dataOverrun = 1;
		rxByte = 0;
	}
	if(dataOverrun != 0)
		rxByte = 0;
	sei();

}*/

void addCharToBuffer(char chr)
{
        if(rxByte < RX_BUFFER_SIZE){
                rxBuffer[rxByte++]=chr;
		//transmit((char*)&chr, 1);
        } else {
		PORTB |= _BV(0);
		dataOverrun = 1;
		rxByte = 0;
	}
	if(dataOverrun != 0)
		rxByte = 0;
}

/** Transmit interrupt for uart. If there is a byte in the transmit buffer, it will be sent using transmitByte. Otherwise nothing is done.
 */
/*SIGNAL(USART_TX_vect)
{
        if(txByte>=0){
                TRANSMIT_BYTE(txBuffer[txByte--]);
	}
}*/


/** Transmit num bytes of data from msg. Transmit num bytes from from msg 
 * pointer. If num is less then the free space in the transmit buffer, 
 * num bytes data is copied from msg to the buffer and the the function 
 * returns. 
 * Othwerwise the function continously fills the buffer untill all data is 
 * in the buffer, then it returns.
 * \param msg pointer to the data to be transmitted
 * \param num number of bytes to transmit
 * \todo fix bug which causes double transmission of some characters between
 * different calls to this function.
 */
/*void transmit(const char *msg, uint16_t num)
{
        uint16_t i=0;//number of bytes xmitd
        int16_t tmp;
        while(i < num){
                //how many bytes to send in this burst?
                tmp = (num -i);
                tmp = tmp > TX_BUFFER_SIZE ? TX_BUFFER_SIZE : tmp;
                //wait for earlier xmit to finnish
                while(txByte>=0);
                //copy to tx-buffer
		cli();
                copyReverse(txBuffer, msg+i,(unsigned) tmp);
                //tmp more bytes has been sent
                i+=tmp;
                //copyReverse(txBuffer, "1234567890123456", 16);
                //txByte = 15;
                txByte = tmp-1;
                TRANSMIT_BYTE(txBuffer[txByte--]);
		sei();
        }
}*/

void transmit(const char *msg, uint16_t num)
{
    platformSerialWrite(msg, num);
    //int i;
    //for(i = num ; i < num ; i++)
    //    TRANSMIT_BYTE(i+'0');
}

/** same as the transmit function but for when the msg data is stired
 * in program memory.
 * \param msg pointer to data in in program memory to be transmitted.
 * \param num number of bytes to transmit
*/
/*void transmitP(const char *msg, uint16_t num)
{
        uint16_t i=0;//number of bytes xmitd
        int16_t tmp;
        while(i < num){
                //how many bytes to send in this burst?
                tmp = (num -i);
                tmp = tmp > TX_BUFFER_SIZE ? TX_BUFFER_SIZE : tmp;
                //wait for earlier xmit to finnish
                while(txByte>=0);
                //copy to tx-buffer
		cli();
                copyReverseP(txBuffer, msg+i,(unsigned) tmp);
                //tmp more bytes has been sent
                i+=tmp;
                txByte = tmp-1;
                TRANSMIT_BYTE(txBuffer[txByte--]);
		sei();
        }
}*/

void transmitP(const char *msg, uint16_t num)
{
    char a[1];
    for(; num > 0 ; num--){
        a[0] = pgm_read_byte(msg++);
        platformSerialWrite(a,1);
    }
}
/** Transmit a string, determines string length by looking for null character
 * and then calls transmit the string.
 * \param msg poiner to the first byte in the string to be transmitted.
 */
void transmitString(const char *msg)
{
        transmit(msg, strlen(msg));
}

/** Transmit a string, determines string length by looking for null character
 * and then calls transmit the string. The string must be stored in program
 * memory
 * \param msg poiner to the first byte in the string to be transmitted.
 * The string must be located in program memory.
 */
void transmitStringP(const char *msg)
{
        transmitP(msg, strlenP(msg));
}

/** initialize the Universal serial asynchronous receiver/transcaiver.
 * 19200 bps 8N1, with both transmit and receive interrupts
 */
void initUsart()
{
	DDRD &= 0xFE;
	DDRD |= 0x02;
        DDRD = 2;
	UBRR0H = 0;
        UBRR0L = 35;
        //UBRR0L = 36*2+1;//576/8;
        //UBRR0 = 576/8;
        UCSR0B = _BV(RXEN0)  | _BV(RXCIE0) | _BV(TXCIE0) | _BV(TXEN0);
        UCSR0C = _BV(UCSZ00) | _BV(UCSZ01) ;
	sei();
}

/** Transmit an integer as its decimal representation in ascii.
 * The integer number must fit in 7 bytes.
 * \param i the integer to be transmitted as ascii text.
*/
void transmitInt(const int32_t i)
{
	static char m[16];
	ltoa(i, m, 10);
	transmitString(m);
}


/** Dual use function: count the number of tokens in a string 
 * (entered command line) or get the pointer to a specific token 
 * indicated by toknum.
 * 
 * Counts the number of sections in the string which are separated by the
 * characters given in the tokenizer array.
 * \param str pointer to a null terminated string in which the tokens 
 *            should be counted.
 * \param tokenizer pointer to anull terminated string containing the
 *                  tokenizing characters.
 * \param toknum return start and stop pointers to the toknum'th token.
 * \param start pointer to the start of the toknum'th token, relative to the
               start of str. Shall be NULL if function is used to count tokens.
 * \param stop pointer to the stop of the toknum'oth token, relative to the
 *            start of str. Shall be NULL if function  is used to count tokens.
 * \return the number of tokens in the string, except if toknum, start and stop
 *         is nonzero then toknum will be returned.
*/
uint8_t countTokens(char *str, char *tokenizers, 
			uint8_t toknum, uint8_t *start, uint8_t *stop)
{
	uint16_t n,m;
	uint16_t nTokenizers = strlen(tokenizers)+1; //use nulterm as nultok
	uint8_t isToken=0;
	uint8_t lastWasToken=1;
	uint8_t nTokens=0;
	for(n = 0 ; ; n++){
		isToken = 0;
		for( m = 0 ; m < nTokenizers ; m++)
			if (str[n] == tokenizers[m]){
				isToken = 1;
				break;
			}
		if (isToken == 1 && lastWasToken == 0){
			if((stop != 0) && (toknum == nTokens)){
				*stop = n;
				break;
			}
		}
		if (isToken == 0 && lastWasToken == 1){
			nTokens++;
			if((start != 0) && (toknum == nTokens))
				*start = n;
		}
		if (str[n] == '\0'){
			break;
		}
		lastWasToken = isToken;
	}
	return nTokens;	
}

/**Copy a token to a null-terminated string.
 * \param tokenLine the string of tokens from which the token shall be 
 *                  extracted.
 * \param tokenizers characters used to separate the tokens.
 * \param n the number of the token which should be copied to the string.
 * \param strDest pointer to where the string copy should be placed.
 * \param destLength length of the memory that can be used at strDest.
 * \return the number of bytes copied to the string. 
*/
uint8_t tokenToString(char *tokenLine, char *tokenizers, uint8_t n, 
			char *strDest, uint8_t destLength)
{
	uint8_t start, stop, length;
	start = stop = 0;
	countTokens(tokenLine, tokenizers, n, &start, &stop);
	length = stop-start;
	if (length == 0){
		transmitString("No such token\r\n");
	} else if ( (length+1) > destLength){
		transmitString("Token is too long\r\n");
	} else {
		for(; start < stop ; start++)
			*strDest++ = tokenLine[start];
		*strDest = '\0';
		return length; 
	}
	return 0;
}

char* getTokenPointer(char *tokenLine, uint8_t n)
{
	uint8_t start, stop;
	start = stop = 0;
	countTokens(tokenLine, " \r\n\t", n, &start, &stop);
	return tokenLine+start;
}





/**Get an integer parameter form a command line string.
 * \param cmdLine pointer to the command line from which the integer should
 *                be extracted.
 * \param num the number of the token which contains the integer.
 * \return a int32_t containing the value represented by the decimal
 *         string representation in token num. If the token is invalid
 *         -0x7FFFFFFF is returned.
 */
int32_t getIntParameter(char *cmdLine, uint8_t num)
{
	static char data[16];
	uint8_t l;
	l=tokenToString(cmdLine, " \r\n\t", num, data, 16);
	if(l > 0){
		return atol(data);
	}
	return -0x7FFFFFFF;
} 

/**Parse a received command and execute the command.
 * The first token on the line will be matche against the #Commands array.
 * \param cmdLine a pointer to the received command line.
 */
/*void parseCommandLine(char * cmdLine)
{
	uint8_t ntok, l,i;
	static char token[16];
	ntok=countTokens(cmdLine, " \r\n\t", 0, NULL, NULL);
	if(ntok > 0){
		l=tokenToString(cmdLine, " \r\n\t", 1, token, 16);
		if(l > 0){
			i=0;
        		while(strcmp(Commands[i].commandString, "last") != 0){
                		if(strcmp(Commands[i].commandString, token) == 0){
                        		l = Commands[i].processor(l, cmdLine);
					return;
				}
                		i++;
			}
			transmitString("ERROR: Command not found\r\n");
		}
        }
}*/

/**Command line funciton, call this function from your main loop!
 * This function reads data received by the receive interrupts and if a 
 * complete line is detected in the receive buffer, a copy of this will
 * be sent to the #parseCommandLine function and the receive buffer will
 * be emptied.
 * The commands are defined by the #Commands array. The arguments are
 * parsed by the function specified in the #Commands array.
 */

void corbomiteParse(char * line);

void commandLine(void)
{
	static char cmdBuf[CMDBUF_SIZE];
	static char rxTmpBuf[RX_BUFFER_SIZE];
	static uint16_t cmdBytes = 0;
	static uint16_t recvBytes = 0;
	//static uint8_t ignoreNext = 0;
	static uint8_t cmdBufPtr = 0;
	static char lastByte = 0;
	static uint8_t startRecvd;
	uint16_t n;
        
	//Read data from receive buffer
	cli();
        copy(rxTmpBuf, rxBuffer, rxByte);
        recvBytes=rxByte;
        rxByte = 0;
        sei();
	if(dataOverrun!=0){
		dataOverrun = 0;
		startRecvd = 0;
	}
	if(recvBytes == 0)
		return;		//No data received
	for(n=0 ; n < recvBytes ;n++){
		if(startRecvd != 0)
			cmdBuf[cmdBufPtr++] = rxTmpBuf[n];

		if(rxTmpBuf[n] == '#' && lastByte !='\\'){
			cmdBufPtr = 0;
			startRecvd = 1;
		}else if(rxTmpBuf[n] == '\r' && lastByte != '\\'){
			if(cmdBufPtr >= 3){
				cmdBuf[cmdBufPtr-1] = '\0';
				//transmitString("S:");
				//transmitString(cmdBuf);
				//transmitString(":E");
				corbomiteParse(cmdBuf);
				cmdBufPtr = 0;
			}
		}
		
		if(cmdBytes >= CMDBUF_SIZE){
			cmdBuf[0] = '\0';
			cmdBufPtr = 0;
		}
		lastByte = rxTmpBuf[n];
	}

	/*for( n = 0 ; n < recvBytes; n++){
		if(rxTmpBuf[n] == '\r' || rxTmpBuf[n] == '\n'){
			cmdBuf[cmdBytes] = '\0';
			//transmitString("\r\nnewline detected, parsing\r\n");
			transmitString("\r\n");
			//parseCommandLine(cmdBuf);
			if(dataOverrun == 0 && ignoreNext == 0){
				//transmitStringP(PSTR("##"));
				corbomiteParse(cmdBuf);
			}
			if(dataOverrun != 0)
				ignoreNext = 1;
			else
				ignoreNext = 0;
			cmdBuf[0] = '\0';
			cmdBytes = 0;
			dataOverrun = 0;
			transmitStringP(PSTR(">>"));
		} else {
			//transmit(rxTmpBuf+n,1);
			cmdBuf[cmdBytes++]=rxTmpBuf[n];
			if(cmdBytes >= CMDBUF_SIZE){
				cmdBuf[0] = '\0';
				cmdBytes = 0;
				transmitStringP(PSTR("\r\nERROR: command to long, buf reset\r\n"));
				transmitStringP(PSTR(">>"));
			}
		}
	}*/
}

