#import <uart.h>

#define MAX_RETRIES 5


void PostTask();

enum sensor_index {
	sensor_temperature,
	sensor_pir,
	sensor_light
};

#ifdef uart 
void UpTask();
//void AppTask(char upir[10],char utemp[10],char ulight[10]);
void AppTask(char* upir,char* utemp,char* ulight);
#else
//void AppTask(int upir,float utemp,float ulight);
void AppTask();
#endif 
