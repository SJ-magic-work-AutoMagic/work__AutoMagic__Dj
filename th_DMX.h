/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxNetwork.h"

#include "sj_common.h"


/************************************************************
class
************************************************************/

/**************************************************
**************************************************/
struct LED_KEY{
	/****************************************
	param
	****************************************/
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char W;
	
	// 16 bit(13ch mode)なので、intで受ける
	int Pan;
	int Tilt;
	
	/****************************************
	function
	****************************************/
	LED_KEY()
	{
		set_allParam_zero();
	}
	
	void set_allParam_zero()
	{
		R		= 0;
		G		= 0;
		B		= 0;
		W		= 0;
		Pan		= 0;
		Tilt	= 0;
	}
	
	void LimitCheck()
	{
		enum{
			MAX_8BIT = 255,
			MAX_16BIT = 65535,
		};
		
		/* */
		if(R < 0)					R = 0;
		else if(MAX_8BIT < R)		R = MAX_8BIT;

		if(G < 0)					G = 0;
		else if(MAX_8BIT < G)		G = MAX_8BIT;

		if(B < 0)					B = 0;
		else if(MAX_8BIT < B)		B = MAX_8BIT;

		if(W < 0)					W = 0;
		else if(MAX_8BIT < W)		W = MAX_8BIT;

		/* */
		if(Pan < 0)					Pan = 0;
		else if(MAX_16BIT < Pan)	Pan = MAX_16BIT;

		if(Tilt < 0)				Tilt = 0;
		else if(MAX_16BIT < Tilt)	Tilt = MAX_16BIT;
	}
};

/**************************************************
**************************************************/
struct LED_KEYS{
	/****************************************
	param
	****************************************/
	int time_ms;
	LED_KEY Led[NUM_LEDS];
	
	/****************************************
	function
	****************************************/
	void set_Param_of_AllLed_zero()
	{
		for(int i = 0; i < NUM_LEDS; i++){
			Led[i].set_allParam_zero();
		}
	}
};

/**************************************************
Singleton
	https://ja.wikipedia.org/wiki/Singleton_%E3%83%91%E3%82%BF%E3%83%BC%E3%83%B3
**************************************************/
class THREAD__DMX_KEY_TIMETABLE : public ofThread
{
private:
	/****************************************
	enum
	****************************************/
	enum{
		NUM_SAMPLES_PER_BUFFER = 3000, // 99 sec(たまたま。切りのいい値)
		NUM_BUFFERS = 2,
	};
	enum{
		UDP_SEND_PORT = 12347,
	};

	/****************************************
	****************************************/
	LED_KEYS *TimeTable[NUM_BUFFERS];
	bool b_Empty[NUM_BUFFERS];
	
	LED_KEYS Leds_out;
	
	int id_from, id_to;
	int BufferId;
	
	FILE* fp;
	bool b_End;
	bool b_EOF;
	bool b_1stUpdate;
	
	int t_ofs_ms;
	
	ofxUDPManager udpConnection;
	
	/****************************************
	function
	****************************************/
	/********************
	singleton
	********************/
	THREAD__DMX_KEY_TIMETABLE();
	~THREAD__DMX_KEY_TIMETABLE();
	THREAD__DMX_KEY_TIMETABLE(const THREAD__DMX_KEY_TIMETABLE&); // Copy constructor. no define.
	THREAD__DMX_KEY_TIMETABLE& operator=(const THREAD__DMX_KEY_TIMETABLE&); // コピー代入演算子. no define.
	
	/********************
	********************/
	void charge(int BufferId_toCharge);
	bool Wait_NextBufferFilled(double timeout);
	int get_NextBufferId();
	void threadedFunction();
	
	
public:
	/****************************************
	****************************************/
	static THREAD__DMX_KEY_TIMETABLE& getInstance(){
		static THREAD__DMX_KEY_TIMETABLE inst;
		return inst;
	}
	
	void exit();
	
	void setOffset(int ofs);
	
	void setup();
	void Reset();
	bool IsReady();
	void update(int now_ms);
	
	void draw();
	void draw_black();
};

