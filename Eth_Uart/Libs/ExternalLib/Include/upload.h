
#define MAX_RETRIES 5


void PostTask();

enum sensor_index {
	sensor_temperature,
	sensor_pir,
	sensor_light//,sensor_tag
};

#ifdef uart 
void UpTask();
void AppTask(int upir,float utemp,float ulight);
#else
void AppTask();
#endif 