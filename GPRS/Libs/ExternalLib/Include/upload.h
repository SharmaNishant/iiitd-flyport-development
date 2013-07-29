extern int AppTaskStart;
/*extern int alarmcount;
extern int alarmread;

extern char pirstr[50];
extern char pseudostr[50];
extern char timestamp[50];*/


void AppTask();

void PostTask();

void SampleTask();

enum sensor_index {
	sensor_pir,
	sensor_pseudo,
	sensor_END
};

//extern char sensdata[sensor_END][600];
