#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

/** Size of receive buffer*/
#define RX_BUFFER_SIZE  32
/** Size of transmit buffer*/
#define TX_BUFFER_SIZE  32

/** Size of command buffer*/
#define CMDBUF_SIZE     64

#define TRANSMIT_BYTE(x) while( !(UCSR0A & (1 << UDRE0)));\
        			UDR0=(x);

typedef struct{
        char *commandString;
        uint8_t (*processor)(uint8_t nargs, char *commandLine); 

} CommandProcessor;
extern const char nl[];
extern CommandProcessor Commands[];
extern void platformSerialWrite(const char *buf, uint16_t len);
int strcmp_pn(const char *a, char *b);
void waitTransmissionIdle();
void copyReverse(volatile char *dst, volatile const char *src, uint8_t len);
void copyReverseP(volatile char *dst, volatile const char *src, uint8_t len);
void copy(volatile char *dst, volatile const char *src, uint8_t len);
//SIGNAL(USART_RX_vect);
//SIGNAL(USART_TX_vect);
void addCharToBuffer(char c);
void transmit(const char *msg, uint16_t num);
void transmitP(const char *msg, uint16_t num);
void transmitString(const char *msg);
void transmitStringP(const char *msg);
void initUsart();
void transmitInt(const int32_t i);
uint8_t executeCommand(char *cmd, uint8_t n);
void parseCommandLine(char *cmd);
int main_loop(void);
void commandLine(void);
int32_t getIntParameter(char *cmdLine, uint8_t num);
char * getTokenPointer(char *cmdLine, uint8_t num);

