/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxNetwork.h"

#include "sj_common.h"
#include "thread_Base.h"

/************************************************************
class
************************************************************/

/**************************************************
**************************************************/
struct TIME_N_DATASET__CONTENTS_CHANGE{
	int time_ms;
};

/**************************************************
Singleton
	https://ja.wikipedia.org/wiki/Singleton_%E3%83%91%E3%82%BF%E3%83%BC%E3%83%B3
**************************************************/
class THREAD__VJ_CONTENTS_CHANGE_TIMETABLE : public THREAD_BASE_STEPOVER
{
private:
	/****************************************
	****************************************/
	TIME_N_DATASET__CONTENTS_CHANGE *TimeTable[NUM_BUFFERS]; 
	TIME_N_DATASET__CONTENTS_CHANGE data_to_output;
	TIME_N_DATASET__CONTENTS_CHANGE data_to_charge;
	
	/****************************************
	function
	****************************************/
	/********************
	singleton
	********************/
	THREAD__VJ_CONTENTS_CHANGE_TIMETABLE();
	~THREAD__VJ_CONTENTS_CHANGE_TIMETABLE();
	THREAD__VJ_CONTENTS_CHANGE_TIMETABLE(const THREAD__VJ_CONTENTS_CHANGE_TIMETABLE&); // Copy constructor. no define.
	THREAD__VJ_CONTENTS_CHANGE_TIMETABLE& operator=(const THREAD__VJ_CONTENTS_CHANGE_TIMETABLE&); // コピー代入演算子. no define.
	
	/********************
	********************/
	void set_LogFile_id();
	
	FILE* open_ScenarioFile();
	void SetTime_DataToCharge(int time);
	void chargeTimeTable_byCopying(int BufferId_toCharge, int Charge_id);
	void SetData_DataToCharge(FILE* fp);
	void set_outputData(int BufferId, int id);
	void setOscMessage_for_draw(ofxOscMessage& m);
	int get_TimeData_from_DataToCharge();
	int get_TimeData_from_TimeTable(int BufferId, int id);
	
public:
	/****************************************
	****************************************/
	/********************
	********************/
	static THREAD__VJ_CONTENTS_CHANGE_TIMETABLE* getInstance(){
		static THREAD__VJ_CONTENTS_CHANGE_TIMETABLE inst;
		return &inst;
	}
};

