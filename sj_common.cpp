/************************************************************
************************************************************/
#include "ofMain.h"
#include "sj_common.h"


/************************************************************
************************************************************/
OSC_TARGET OscTarget[] = {
	// OSC_TARGET("10.0.0.2", 12346, 12345), // OSC_DMX_LIGHT,
	OSC_TARGET("127.0.0.1", 12346, 12345), // OSC_DMX_LIGHT,
	
	// OSC_TARGET("10.0.0.3", 12349, 12348), // OSC_VJ,
	OSC_TARGET("127.0.0.1", 12349, 12348), // OSC_VJ,
};

/************************************************************
************************************************************/
static FILE* fp_Log[NUM_THREAD_TIMETABLE];

/************************************************************
************************************************************/

/******************************
******************************/
void printMessage(const char* message)
{
	printf("\n> %s\n", message);
}

/******************************
******************************/
void fopen_LogFile()
{
	for(int i = 0; i < NUM_THREAD_TIMETABLE; i++){
		char FileName[BUF_SIZE];
		sprintf(FileName, "../../../data/Log_%d.csv", i);
		
		fp_Log[i] = fopen(FileName, "w");
		if(fp_Log == NULL)	{ ERROR_MSG(); std::exit(1); }
	}
}

/******************************
******************************/
void fclose_LogFile()
{
	for(int i = 0; i < NUM_THREAD_TIMETABLE; i++){
		if(fp_Log[i]) fclose(fp_Log[i]);
	}
}

/******************************
******************************/
void fprint_debug_Log(char* message, int FileId)
{
	if(NUM_THREAD_TIMETABLE <= FileId)	{ ERROR_MSG(); std::exit(1); }
	
	
	if(FileId == -1){
		for(int i = 0; i < NUM_THREAD_TIMETABLE; i++){
			fprintf(fp_Log[i], "%s", message);
		}
	}else{
		fprintf(fp_Log[FileId], "%s", message);
	}
}

