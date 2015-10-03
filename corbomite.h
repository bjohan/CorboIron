#include <stdlib.h>
#include <string.h>
#include <avr/io.h>

#define ANA_OUT(id, unit, minUnit, maxUnit, minRaw, maxRaw, callback, varName)\
const char varName ## _id[] PROGMEM = id;\
const char varName ## _unit[] PROGMEM = unit;\
const char varName ## _minUnit[] PROGMEM = minUnit;\
const char varName ## _maxUnit[] PROGMEM = maxUnit;\
const AnalogOutData varName ## _data PROGMEM = {callback, varName ## _unit,\
					varName ## _minUnit, \
					varName ## _maxUnit,\
					minRaw, maxRaw};\
const CorbomiteEntry varName PROGMEM = { ANALOG_OUT, varName ## _id, \
					(CorbomiteData*)&varName ## _data}

#define ANA_IN(id, unit, minUnit, maxUnit, minRaw, maxRaw, varName)\
const char varName ## _id[] PROGMEM = id;\
const char varName ## _unit[] PROGMEM = unit;\
const char varName ## _minUnit[] PROGMEM = minUnit;\
const char varName ## _maxUnit[] PROGMEM = maxUnit;\
const AnalogInData varName ## _data PROGMEM = {varName ## _unit, \
					varName ## _minUnit, \
					varName ## _maxUnit,\
					minRaw, maxRaw};\
const CorbomiteEntry varName PROGMEM = { ANALOG_IN, varName ## _id, \
					(CorbomiteData*)&varName ## _data}


#define TRACE_IN(id, xUnit, xMinUnit, xMaxUnit, xMinRaw, xMaxRaw, yUnit, yMinUnit, yMaxUnit, yMinRaw, yMaxRaw, varName)\
const char varName ## _id[] PROGMEM = id;\
const char varName ## _x_unit[] PROGMEM = xUnit;\
const char varName ## _x_min_unit[] PROGMEM = xMinUnit;\
const char varName ## _x_max_unit[] PROGMEM = xMaxUnit;\
const char varName ## _y_unit[] PROGMEM = yUnit;\
const char varName ## _y_min_unit[] PROGMEM = yMinUnit;\
const char varName ## _y_max_unit[] PROGMEM = yMaxUnit;\
const TraceInData varName ## _data PROGMEM = {varName ## _x_unit, \
					varName ## _x_min_unit, \
					varName ## _x_max_unit, \
					xMinRaw, \
					xMaxRaw, \
					varName ## _y_unit,\
					varName ## _y_min_unit, \
					varName ## _y_max_unit, \
					yMinRaw, \
					yMaxRaw};\
const CorbomiteEntry varName PROGMEM = { TRACE_IN, varName ## _id, \
					(CorbomiteData*)&varName ## _data}


#define DIG_IN(id, varName)\
const char varName ## _id[] PROGMEM = id;\
const CorbomiteEntry varName PROGMEM = { DIGITAL_IN, varName ## _id, \
					(CorbomiteData*)0}

#define DIG_OUT(id, callback, varName)\
const char varName ## _id[] PROGMEM = id;\
const DigitalData varName ## _data PROGMEM = {callback};\
const CorbomiteEntry varName PROGMEM = {DIGITAL_OUT, varName ## _id, \
					(CorbomiteData *)&varName ## _data}

#define EVENT_IN(id, varName)\
const char varName ## _id[] PROGMEM = id;\
const CorbomiteEntry varName PROGMEM = { EVENT_IN, varName ## _id, \
					(CorbomiteData*)0}

#define EVENT_OUT(id, callback, varName)\
const char varName ## _id[] PROGMEM = id;\
const EventData varName ## _data PROGMEM = {callback};\
const CorbomiteEntry varName PROGMEM = {EVENT_OUT, varName ## _id, \
					(CorbomiteData *)&varName ## _data}

#define STRING_IN(id, varName)\
const char varName ## _id[] PROGMEM = id;\
const CorbomiteEntry varName PROGMEM = { STRING_IN, varName ## _id, \
					(CorbomiteData*)0}

#define STRING_OUT(id, callback, varName)\
const char varName ## _id[] PROGMEM = id;\
const StringData varName ## _data PROGMEM = {callback};\
const CorbomiteEntry varName PROGMEM = {STRING_OUT, varName ## _id, \
					(CorbomiteData *)&varName ## _data}

#define TEXT_IO(id, callback, varName)\
const char varName ## _id[] PROGMEM = id;\
const TextIoData varName ## _data PROGMEM = {callback};\
const CorbomiteEntry varName PROGMEM = {TEXT_IO, varName ## _id, \
					(CorbomiteData *)&varName ## _data}

#define CONTINUE_LINE_HINT(varName)\
const char varName ## _id[] PROGMEM = "";\
const CorbomiteEntry varName PROGMEM = { HINT_CONTINUE_LINE, varName ## _id, \
					(CorbomiteData*)0}

#define PLOT_HINT(varName)\
const char varName ## _id[] PROGMEM = "";\
const CorbomiteEntry varName PROGMEM = { HINT_PLOT, varName ## _id, \
					(CorbomiteData*)0}

#define INFORMATION_HINT(information, varName)\
const char varName ## _id[] PROGMEM = information;\
const CorbomiteEntry varName PROGMEM = { HINT_INFORMATION, varName ## _id, \
					(CorbomiteData*)0}

#define WEIGHT_HINT(width, height, varName)\
const char varName ## _id[] PROGMEM = "";\
const WeightHintData varName ## _data PROGMEM = {width, height};\
const CorbomiteEntry varName PROGMEM = { HINT_WEIGHT, varName ## _id, \
					(CorbomiteData*)&varName ## _data}


extern const char internalName[9];
extern const char internalId[5];
extern const char fe[3];
extern const char fs[2];
typedef const enum {
	DIGITAL_OUT,
	DIGITAL_IN,
	ANALOG_OUT,
	ANALOG_IN,
	TRACE_IN,
	EVENT_OUT,
	EVENT_IN,
	STRING_OUT,
	STRING_IN,
	TEXT_IO,
	HINT_INFORMATION,
	HINT_WEIGHT,
	HINT_CONTINUE_LINE,
	HINT_PLOT,
	INFO,
	LASTTYPE,
} CorbomiteType;

typedef void (* const DigitalCallback)(uint8_t);
typedef struct{
	DigitalCallback callback;
	uint8_t *last;
}DigitalData;

typedef void (* const AnalogCallback)(int32_t);
typedef struct{
	AnalogCallback callback;
	const char *unit;
	const char *minUnit;
	const char *maxUnit;
	int32_t minRaw;
	int32_t maxRaw;
	int32_t *lastRaw;
}AnalogOutData;

typedef struct{
	const char *unit;
	const char *minUnit;
	const char *maxUnit;
	int32_t minRaw;
	int32_t maxRaw;
	int32_t *lastRaw;
}AnalogInData;

typedef struct{
	const char *xUnit;
	const char *xMinUnit;
	const char *xMaxUnit;
	int32_t xMinRaw;
	int32_t xMaxRaw;
	const char *yUnit;
	const char *yMinUnit;
	const char *yMaxUnit;
	int32_t yMinRaw;
	int32_t yMaxRaw;
}TraceInData;

typedef void (* const EventCallback)(void);
typedef void (* EventCallbackW)(void);
typedef struct{
	EventCallback callback;
}EventData;

typedef void (* const StringCallback)(const char *c);
typedef struct{
	StringCallback callback;
}StringData;

typedef void (* const TextIoCallback)(const char *c);
typedef struct{
	StringCallback callback;
}TextIoData;

typedef struct {
	const uint8_t weightWidth;
	const uint8_t weightHeight;
}WeightHintData;

typedef union{
	const DigitalData digitalData;
	const AnalogOutData analogOutData;
	const AnalogInData analogInData;
	const EventData eventData;
	const StringData stringData;
	const TextIoData textIoData;
	//const InformationHint informationHint;
	const WeightHintData weightHintData;
	const TraceInData traceInData;
} CorbomiteData;
/*Plot and newline hints does not need any data and are implemented by
using the id field in the corbomite entity.*/

typedef struct{
	const CorbomiteType t;
	const char *id;
	const CorbomiteData *data; 
} CorbomiteEntry;

void textBoxPrintStringP(const char *tbid, const char *str);
void textBoxPrintString(const char *tbid, const char *str);
void textBoxPrintInt(const char *tbid, int32_t i);
void textBoxClear(const char *tbid);
void transmitProgressbarValue(const char *pbid, int16_t value);
void transmitSeekbarValue(const char *pbid, int16_t value);
void setButtonLabel(const char *bid, const char *text);
void traceClear(const char *pbid, const char *trace);
void traceAddPoint(const char *pbid, const char *trace, int16_t x, int16_t y);
void traceLegend(const char *pid, const char *trace, const char *legend);
void traceColor(const char *pid, const char *trace, uint8_t r, uint8_t g, 
						uint8_t b);
void registeredEntries();
extern const CorbomiteEntry *const entries[];
extern CorbomiteEntry tmpEntry;
void readEntryP(const CorbomiteEntry *src, CorbomiteEntry *dst);
void readCorbomiteDataP(const CorbomiteData *src, CorbomiteData *dst);

void transmitEventIn(const CorbomiteEntry *e);
void transmitAnalogIn(const CorbomiteEntry *e, int32_t s);
void transmitDigitalIn(const CorbomiteEntry *e, int8_t s);
void transmitStringInP(const CorbomiteEntry *e, const char *s);
void transmitStringIn(const CorbomiteEntry *e, char *s);
void transmitStringInInt(const CorbomiteEntry *e, int32_t i);
void transmitTextIoP(const CorbomiteEntry *e, const char *s);
void transmitTextIo(const CorbomiteEntry *e, char *s);
void transmitTextIoInt(const CorbomiteEntry *e, int32_t i);
void transmitTraceIn(const CorbomiteEntry *e, int32_t x, int32_t y);
void transmitTraceClear(const CorbomiteEntry *e);
void transmitProlog(const CorbomiteEntry *e);
void transmitEpilog(const CorbomiteEntry *e);
void transmitBusy();
void transmitIdle();
