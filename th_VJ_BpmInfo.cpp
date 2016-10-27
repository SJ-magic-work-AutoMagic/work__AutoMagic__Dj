/************************************************************
************************************************************/
#include <unistd.h>
#include "th_VJ_BpmInfo.h"

/************************************************************
function
************************************************************/

/******************************
******************************/
THREAD__VJ_BPM_INFO::THREAD__VJ_BPM_INFO()
{
	for(int i = 0; i < NUM_BUFFERS; i++){
		TimeTable[i] = new TIME_N_DATASET__BPM_INFO[NUM_SAMPLES_PER_BUFFER];
	}
}

/******************************
******************************/
THREAD__VJ_BPM_INFO::~THREAD__VJ_BPM_INFO()
{
	for(int i = 0; i < NUM_BUFFERS; i++){
		delete[] TimeTable[i];
	}
}

/******************************
******************************/
void THREAD__VJ_BPM_INFO::set_LogFile_id()
{
	LogFile_id = THREAD_TIMETABLE__VJ_BPM_INFO;
}

/******************************
******************************/
FILE* THREAD__VJ_BPM_INFO::open_ScenarioFile()
{
	FILE* fp = fopen("../../../data/VJSB_BpmInfo.txt", "r");
	return fp;
}

/******************************
******************************/
void THREAD__VJ_BPM_INFO::SetTime_DataToCharge(int time)
{
	data_to_charge.time_ms = time;
}

/******************************
******************************/
void THREAD__VJ_BPM_INFO::chargeTimeTable_byCopying(int BufferId_toCharge, int Charge_id)
{
	TimeTable[BufferId_toCharge][Charge_id] = data_to_charge;
}

/******************************
******************************/
void THREAD__VJ_BPM_INFO::SetData_DataToCharge(FILE* fp)
{
	char buf[BUF_SIZE];
	
	fscanf(fp, "%s", buf);
	data_to_charge.BeatInterval_ms = atoi(buf);
}

/******************************
******************************/
void THREAD__VJ_BPM_INFO::set_outputData(int BufferId, int id)
{
	data_to_output = TimeTable[BufferId][id];
}

/******************************
******************************/
void THREAD__VJ_BPM_INFO::setOscMessage_for_draw(ofxOscMessage& m)
{
	m.setAddress("/VJ_BpmInfo");
	m.addIntArg(data_to_output.BeatInterval_ms);
}

/******************************
******************************/
int THREAD__VJ_BPM_INFO::get_TimeData_from_DataToCharge()
{
	return data_to_charge.time_ms;
}

/******************************
******************************/
int THREAD__VJ_BPM_INFO::get_TimeData_from_TimeTable(int BufferId, int id)
{
	return TimeTable[BufferId][id].time_ms;
}



