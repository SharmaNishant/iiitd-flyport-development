unsigned char ow_reset(void);
double DS1820Read(void);
void DS1820Init();

#define DQ_PORT I5					// defines the port used for the data line of the DS1820 sensor
//#define DS1820_9bit				// defines 9bit mode documented in DS1820 data-sheet
//#define DS1820_higher_resolution	// defines higher resolution mode describes in data-sheet
#define DS18B20_9bit_resolution	// defines lower resolution mode  for DS18B20  as described in data-sheet
//#define DS18B20_higher_resolution	// defines higher resolution mode for DS18B20  as described in data-sheet

