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
struct DATA_SET__ALPHA_FFT{
	double	mov_a;
	bool	b_mov_Effect_On;
	double	mov_a_0_12;
	bool	b_mov0_Effect_On;
	double	mov_a_1_2;
	double	a_indicator;
	double	a_particle;
	bool	b_GeneratedImage_on;
	bool	b_text_on;
	double	a_Strobe;
	
	DATA_SET__ALPHA_FFT()
	{
		set_allParam_zero();
	}
	
	void set_allParam_zero()
	{
		mov_a = 0;
		b_mov_Effect_On = 0;
		mov_a_0_12 = 0;
		b_mov0_Effect_On = 0;
		mov_a_1_2 = 0;
		a_indicator = 0;
		a_particle = 0;
		b_GeneratedImage_on = 0;
		b_text_on = 0;
		a_Strobe = 0;
	}
};

struct TIME_N_DATASET__ALPHA_FFT{
	int time_ms;
	DATA_SET__ALPHA_FFT DataSet;
	
	void set_allParam_zero()
	{
		DataSet.set_allParam_zero();
	}
};


/**************************************************
Singleton
	https://ja.wikipedia.org/wiki/Singleton_%E3%83%91%E3%82%BF%E3%83%BC%E3%83%B3
**************************************************/
class THREAD__VJ_ALPHA_FFT : public THREAD_BASE
{
private:
	/****************************************
	enum
	****************************************/
	enum{
		NUM_SAMPLES_PER_BUFFER = 3000, // 99 sec(たまたま。切りのいい値)
	};
	enum{
		UDP_SEND_PORT = 12350,
	};

	/****************************************
	****************************************/
	TIME_N_DATASET__ALPHA_FFT *TimeTable[NUM_BUFFERS];
	TIME_N_DATASET__ALPHA_FFT data_to_output;
	
	int id_from, id_to;
	bool b_1stUpdate;
	
	ofxUDPManager udpConnection;
	
	/****************************************
	function
	****************************************/
	/********************
	singleton
	********************/
	THREAD__VJ_ALPHA_FFT();
	~THREAD__VJ_ALPHA_FFT();
	THREAD__VJ_ALPHA_FFT(const THREAD__VJ_ALPHA_FFT&); // Copy constructor. no define.
	THREAD__VJ_ALPHA_FFT& operator=(const THREAD__VJ_ALPHA_FFT&); // コピー代入演算子. no define.
	
	/********************
	********************/
	void set_LogFile_id();
	void charge(int BufferId_toCharge);
	
	/********************
	********************/
	void SetData_DataToCharge(FILE* fp, TIME_N_DATASET__ALPHA_FFT &DatasetToCharge);
	
public:
	/****************************************
	****************************************/
	/********************
	********************/
	static THREAD__VJ_ALPHA_FFT* getInstance(){
		static THREAD__VJ_ALPHA_FFT inst;
		return &inst;
	}
	
	/********************
	********************/
	void Reset();
	
	/********************
	********************/
	void exit();
	void setup();
	void update(int now_ms);
	void draw(float* spectrum = NULL, int N_SPECTRUM = 0);
	void draw_black(float* spectrum = NULL, int N_SPECTRUM = 0);
};

