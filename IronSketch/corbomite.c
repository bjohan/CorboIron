#include <avr/pgmspace.h>
#include "corbomite.h"
#include "uart_command_lib.h"
uint8_t countTokens(char *str, char *tokenizers, 
			uint8_t toknum, uint8_t *start, uint8_t *stop);
uint8_t tokenToString(char *tokenLine, char *tokenizers, uint8_t n, 
			char *strDest, uint8_t destLength);

/**Command token for labels*/
const char labelCmd[] PROGMEM = "label ";

/**Command token for buttons*/

const char digitalOutCmd[] PROGMEM = "dout ";
const char digitalInCmd[] PROGMEM = "din ";
const char analogOutCmd[] PROGMEM = "aout ";
const char analogInCmd[] PROGMEM = "ain ";
const char traceInCmd[] PROGMEM = "tin ";
const char eventOutCmd[] PROGMEM = "eout ";
const char eventInCmd[] PROGMEM = "ein ";
const char stringOutCmd[] PROGMEM = "sout ";
const char stringInCmd[] PROGMEM = "sin ";
const char textIoCmd[] PROGMEM = "tio ";
const char hintInformationCmd[] PROGMEM = "info ";
const char hintWeightCmd[] PROGMEM = "w";
const char hintContinueLineCmd[] PROGMEM = "cont";
const char hintPlotCmd[] PROGMEM = "plot ";
const char internalName[] PROGMEM = "internal";
const char internalId[] PROGMEM = "info";

/**Order in this array has to correspond to the enum #CorbomiteType*/
const char * layoutCommands[] = { digitalOutCmd, digitalInCmd, analogOutCmd, 
	analogInCmd, traceInCmd, eventOutCmd, eventInCmd, stringOutCmd, 
	stringInCmd, textIoCmd, hintInformationCmd, hintWeightCmd, 
	hintContinueLineCmd,	hintPlotCmd};

const char sp[] PROGMEM = " ";
const char fs[] PROGMEM = "#";
const char fe[] PROGMEM = "\r\n";
const char busy[] PROGMEM = "#busy\r\n";
const char idle[] PROGMEM = "#idle\r\n";
CorbomiteEntry tmpEntry;
CorbomiteData tmpData;
void reportAnalogInData(const CorbomiteData *d)
{
	readCorbomiteDataP(d, &tmpData);
	transmitStringP(tmpData.analogInData.unit);
	transmitStringP(sp);
	transmitStringP(tmpData.analogInData.minUnit);
	transmitStringP(sp);
	transmitStringP(tmpData.analogInData.maxUnit);
	transmitStringP(sp);
	transmitInt(tmpData.analogInData.minRaw);
	transmitStringP(sp);
	transmitInt(tmpData.analogInData.maxRaw);

}

void reportAnalogOutData(const CorbomiteData *d)
{
	readCorbomiteDataP(d, &tmpData);
	transmitStringP(tmpData.analogOutData.unit);
	transmitStringP(sp);
	transmitStringP(tmpData.analogOutData.minUnit);
	transmitStringP(sp);
	transmitStringP(tmpData.analogOutData.maxUnit);
	transmitStringP(sp);
	transmitInt(tmpData.analogOutData.minRaw);
	transmitStringP(sp);
	transmitInt(tmpData.analogOutData.maxRaw);
}

void reportTraceInData(const CorbomiteData *d)
{
	readCorbomiteDataP(d, &tmpData);
	transmitStringP(tmpData.traceInData.xUnit);
	transmitStringP(sp);
	transmitStringP(tmpData.traceInData.xMinUnit);
	transmitStringP(sp);
	transmitStringP(tmpData.traceInData.xMaxUnit);
	transmitStringP(sp);
	transmitInt(tmpData.traceInData.xMinRaw);
	transmitStringP(sp);
	transmitInt(tmpData.traceInData.xMaxRaw);
	transmitStringP(sp);
	transmitStringP(tmpData.traceInData.yUnit);
	transmitStringP(sp);
	transmitStringP(tmpData.traceInData.yMinUnit);
	transmitStringP(sp);
	transmitStringP(tmpData.traceInData.yMaxUnit);
	transmitStringP(sp);
	transmitInt(tmpData.traceInData.yMinRaw);
	transmitStringP(sp);
	transmitInt(tmpData.traceInData.yMaxRaw);
}

void reportWeightHintData(const CorbomiteData *d)
{
	readCorbomiteDataP(d, &tmpData);
	transmitInt((int16_t)tmpData.weightHintData.weightWidth);
	transmitStringP(sp);
	transmitInt((int16_t)tmpData.weightHintData.weightHeight);
}

void reportSignal(const CorbomiteEntry *e)
{
	if(e->t < LASTTYPE){
		transmitStringP(fs);
                transmitStringP(PSTR("info "));
		transmitStringP(layoutCommands[e->t]);
		transmitStringP(e->id);
		transmitStringP(sp);
		switch(e->t){
			case ANALOG_OUT:
				reportAnalogOutData(e->data);
			break;
				
			case ANALOG_IN:
				reportAnalogInData(e->data);
			break;
				
			case TRACE_IN:
				reportTraceInData(e->data);
			break;
				
			case HINT_INFORMATION:
				//Implemented using the id string.
			break;
				
			case HINT_WEIGHT:
				reportWeightHintData(e->data);
			break;
				
			default:
				break;
		}
	}
	transmitStringP(fe);
}

void pgm_copy(void const * src, uint8_t *dst, uint8_t len)
{
	while(len--){
		*(uint8_t *)(dst++) = pgm_read_byte(src++);
	}
}

void readEntryP(const CorbomiteEntry *src, CorbomiteEntry *dst)
{
	pgm_copy((void *) src, (uint8_t *)dst, sizeof(*dst));
}

void readEntryPP(const CorbomiteEntry * const *src, CorbomiteEntry *dst)
{
	CorbomiteEntry *w;
	w = (CorbomiteEntry*)pgm_read_word(src);
	readEntryP(w, dst);
}

void readCorbomiteDataP(const CorbomiteData *src, CorbomiteData *dst)
{
	pgm_copy((void *) src, (uint8_t *)dst, sizeof(*dst));
}
void registeredEntries()
{
	uint8_t n = 0;
	//transmitString("Registered entries:\r\n");	
	readEntryPP(&entries[n], &tmpEntry);
	while(tmpEntry.t != LASTTYPE){
		reportSignal(&tmpEntry);
		n++;
		readEntryPP(&entries[n], &tmpEntry);
	}
}

void processInfo(char *l)
{
}

void processCorbomiteCall(const CorbomiteEntry *e, char *l)
{
	//transmitStringP(PSTR("Entry found: "));
	//transmitStringP(e->id);
	//transmitStringP(nl);
	CorbomiteData data;
	if(e->t < LASTTYPE){
		readCorbomiteDataP(e->data, &data);
		switch(e->t){
			case DIGITAL_OUT:
				data.digitalData.callback(
					getIntParameter(l,2));
			break;

			case ANALOG_OUT:
				data.analogOutData.callback(
					getIntParameter(l,2));
			break;
				
			case EVENT_OUT:
				(data.eventData.callback)();
			break;
				
			case STRING_OUT:
				data.stringData.callback(getTokenPointer(l,2));
			break;
				
			case TEXT_IO:
				data.textIoData.callback(getTokenPointer(l,2));
			break;
			
			case INFO:
				processInfo(l);
			break;	
			default:
				break;
		}
	}
	
}

void corbomiteParse(char * line)
{
	//CorbomiteEntry *w;
	uint8_t ntok, l,i;
	static char token[16];
	transmitStringP(busy);
	ntok=countTokens(line, " \r\n\t", 0, NULL, NULL);
	if(ntok > 0){
		l=tokenToString(line, " \r\n\t", 1, token, 16);
		if(l > 0){
			i=0;
			//pgm_copy(&(entries[0]), (uint8_t *)&w, sizeof(w));
			//transmitStringP(PSTR("entries place "));
			//transmitInt( (int32_t)((uint16_t)entries));
			//transmitStringP(nl);
			//w = (CorbomiteEntry *)pgm_read_word(entries);
			readEntryPP(&entries[0], &tmpEntry);
			//readEntryP((CorbomiteEntry *)w, &tmpEntry);
			//transmitStringP(PSTR("Widget loaded: "));
			//transmitStringP(tmpEntry.id);
			//transmitStringP(nl);
			while(tmpEntry.t != LASTTYPE){
				i++;
				if(strcmp_pn(tmpEntry.id, token)==0){
					//transmitString("Found command\n\r");
					processCorbomiteCall(&tmpEntry, line);
					transmitStringP(idle);
					return;
				}
				//pgm_copy(&entries[i], (uint8_t *)&w, sizeof(w));
				readEntryPP(&entries[i], &tmpEntry);
				//transmitStringP(PSTR("Widget loaded: "));
				//transmitStringP(tmpEntry.id);
				//transmitStringP(nl);
			}
			transmitStringP(PSTR("ERROR: Command not found\r\n"));
			transmitString(line);
			transmitStringP(nl);
		}
        }
	transmitStringP(idle);
}


void textBoxPrintInt(const char *tbid, int32_t i)
{
	transmitStringP(fs);
	transmitStringP(tbid);
	transmitStringP(PSTR(" app"));
	transmitInt(i);
	transmitStringP(fe);
}


void transmitProgressbarValue(const char *pbid, int16_t value)
{
	transmitStringP(fs);
	transmitStringP(pbid);
	transmitStringP(sp);
	transmitInt(value);
	transmitStringP(fe);
}

void transmitSeekbarValue(const char *pbid, int16_t value)
{
	transmitProgressbarValue(pbid, value);
}

void traceLegend(const char *pid, const char *trace, const char *legend)
{
	transmitStringP(fs);
	transmitStringP(pid);
	transmitStringP(sp);
	transmitStringP(trace);
	transmitStringP(PSTR(" leg "));
	transmitStringP(legend);
	transmitStringP(fe);
}

void traceColor(const char *pid, const char *trace, uint8_t r, uint8_t g, 
						uint8_t b)
{
	transmitStringP(fs);
	transmitStringP(pid);
	transmitStringP(sp);
	transmitStringP(trace);
	transmitStringP(PSTR(" col "));
	transmitInt((int32_t) r);
	transmitStringP(sp);
	transmitInt((int32_t) g);
	transmitStringP(sp);
	transmitInt((int32_t) b);
	transmitStringP(fe);
}

void traceClear(const char *pid, const char *trace)
{
	transmitStringP(fs);
	transmitStringP(pid);
	transmitStringP(sp);
	transmitStringP(trace);
	transmitStringP(sp);
	transmitStringP(PSTR("clr"));
	transmitStringP(fe);
}

void traceAddPoint(const char *pid, const char *trace, int16_t x, int16_t y)
{
	transmitStringP(fs);
	transmitStringP(pid);
	transmitStringP(sp);
	transmitStringP(trace);
	transmitStringP(PSTR(" add "));
	transmitInt((int32_t)x);
	transmitStringP(sp);
	transmitInt((int32_t)y);
	transmitStringP(fe);
	
}

void textBoxPrintStringP(const char *tbid, const char *text)
{
	transmitStringP(fs);
	transmitStringP(tbid);
	transmitStringP(PSTR(" app"));
	transmitStringP(text);
	transmitStringP(fe);
}

void textBoxClear(const char *tbid)
{
	transmitStringP(fs);
	transmitStringP(tbid);
	transmitStringP(PSTR(" set"));
	transmitStringP(fe);
}

void setButtonLabel(const char *bid, const char *text)
{
	transmitStringP(fs);
	transmitStringP(bid);
	transmitStringP(PSTR(" "));
	transmitStringP(text);
	transmitStringP(fe);
}


void transmitEventIn(const CorbomiteEntry *e)
{
	readEntryP(e, &tmpEntry);
	transmitStringP(fs);
	transmitStringP(tmpEntry.id);
	transmitStringP(fe);
}

void transmitAnalogIn(const CorbomiteEntry *e, int32_t s)
{
	//readEntryP(e, &tmpEntry);
	//transmitStringP(fs);
	//transmitStringP(tmpEntry.id);
	//transmitStringP(sp);
	*e->data->analogInData.lastRaw = s;
	transmitProlog(e);
	transmitInt(s);
	transmitStringP(fe);
}

void transmitDigitalIn(const CorbomiteEntry *e, int8_t s)
{
	s = s == 0 ? 0 : 1;
	//readEntryP(e, &tmpEntry);
	//transmitStringP(fs);
	//transmitStringP(tmpEntry.id);
	//transmitStringP(sp);
	transmitProlog(e);
	transmitInt(s);
	transmitStringP(fe);
}

void transmitStringInP(const CorbomiteEntry *e, const char *s)
{
	//readEntryP(e, &tmpEntry);
	//transmitStringP(fs);
	//transmitStringP(tmpEntry.id);
	//transmitStringP(sp);
	transmitProlog(e);
	transmitStringP(s);
	transmitStringP(fe);
}

void transmitStringIn(const CorbomiteEntry *e, char *s)
{
	//readEntryP(e, &tmpEntry);
	//transmitStringP(fs);
	//transmitStringP(tmpEntry.id);
	//transmitStringP(sp);
	transmitProlog(e);
	transmitString(s);
	transmitStringP(fe);
}

void transmitStringInInt(const CorbomiteEntry *e, int32_t i)
{
	transmitProlog(e);
	transmitInt(i);
	transmitEpilog(e);
}

void transmitTextIoPOp(const CorbomiteEntry *e, const char *op, const char *s)
{
	transmitProlog(e);
	transmitStringP(op);
	transmitStringP(s);
	transmitEpilog(e);
}

void transmitTextIoIntOp(const CorbomiteEntry *e, const char *op, int32_t i)
{
	transmitProlog(e);
	transmitStringP(op);
	transmitInt(i);
	transmitEpilog(e);
}

void transmitTextIoP(const CorbomiteEntry *e, const char *s)
{
	transmitTextIoPOp(e, PSTR("app "), s);
}

void transmitTextIo(const CorbomiteEntry *e, char *s)
{
	transmitStringIn(e, s);
}

void transmitTextIoInt(const CorbomiteEntry *e, int32_t i)
{
	transmitTextIoIntOp(e, PSTR("app ") ,i);
}

void transmitTraceIn(const CorbomiteEntry *e, int32_t x, int32_t y)
{
	//readEntryP(e, &tmpEntry);
	//transmitStringP(fs);
	//transmitStringP(tmpEntry.id);
	//transmitStringP(sp);
	transmitProlog(e);
	transmitInt(x);
	transmitStringP(sp);
	transmitInt(y);
	transmitStringP(fe);
}

void transmitTraceClear(const CorbomiteEntry *e)
{
	transmitProlog(e);
	transmitStringP(PSTR("clr"));
	transmitStringP(fe);
}

void transmitProlog(const CorbomiteEntry *e)
{
	readEntryP(e, &tmpEntry);
	transmitStringP(fs);
	transmitStringP(tmpEntry.id);
	transmitStringP(sp);
}

void transmitEpilog(const CorbomiteEntry *e)
{
	transmitStringP(fe);
}

void transmitIdle()
{
    transmitStringP(PSTR("#idle\r\n"));
}

void transmitBusy()
{
    transmitStringP(PSTR("#busy\r\n"));
}
