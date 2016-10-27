/************************************************************
************************************************************/
#include <unistd.h>
#include "th_VJ_ContentsChange.h"

/************************************************************
function
************************************************************/

/******************************
******************************/
THREAD__VJ_CONTENTS_CHANGE_TIMETABLE::THREAD__VJ_CONTENTS_CHANGE_TIMETABLE()
{
	for(int i = 0; i < NUM_BUFFERS; i++){
		TimeTable[i] = new TIME_N_DATASET__CONTENTS_CHANGE[NUM_SAMPLES_PER_BUFFER];
	}
}

/******************************
******************************/
THREAD__VJ_CONTENTS_CHANGE_TIMETABLE::~THREAD__VJ_CONTENTS_CHANGE_TIMETABLE()
{
	for(int i = 0; i < NUM_BUFFERS; i++){
		delete[] TimeTable[i];
	}
}

/******************************
******************************/
void THREAD__VJ_CONTENTS_CHANGE_TIMETABLE::set_LogFile_id()
{
	LogFile_id = THREAD_TIMETABLE__VJ_CONTENTS_CHANGE_TIMING;
}

/******************************
******************************/
FILE* THREAD__VJ_CONTENTS_CHANGE_TIMETABLE::open_ScenarioFile()
{
	FILE* fp = fopen("../../../data/VJSB_ContentsChangeTiming.txt", "r");
	return fp;
}

/******************************
******************************/
void THREAD__VJ_CONTENTS_CHANGE_TIMETABLE::SetTime_DataToCharge(int time)
{
	data_to_charge.time_ms = time;
}

/******************************
******************************/
void THREAD__VJ_CONTENTS_CHANGE_TIMETABLE::chargeTimeTable_byCopying(int BufferId_toCharge, int Charge_id)
{
	TimeTable[BufferId_toCharge][Charge_id] = data_to_charge;
}

/******************************
******************************/
void THREAD__VJ_CONTENTS_CHANGE_TIMETABLE::SetData_DataToCharge(FILE* fp)
{
}

/******************************
******************************/
void THREAD__VJ_CONTENTS_CHANGE_TIMETABLE::set_outputData(int BufferId, int id)
{
	data_to_output = TimeTable[BufferId][id];
}

/******************************
******************************/
void THREAD__VJ_CONTENTS_CHANGE_TIMETABLE::setOscMessage_for_draw(ofxOscMessage& m)
{
	m.setAddress("/VJContentsChange");
	m.addIntArg(1);
}

/******************************
******************************/
int THREAD__VJ_CONTENTS_CHANGE_TIMETABLE::get_TimeData_from_DataToCharge()
{
	return data_to_charge.time_ms;
}

/******************************
******************************/
int THREAD__VJ_CONTENTS_CHANGE_TIMETABLE::get_TimeData_from_TimeTable(int BufferId, int id)
{
	return TimeTable[BufferId][id].time_ms;
}



