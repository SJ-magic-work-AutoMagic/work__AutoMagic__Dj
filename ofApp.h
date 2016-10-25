/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxGui.h"

#include "sj_common.h"
#include "th_DMX.h"


/************************************************************
************************************************************/

/**************************************************
**************************************************/
class ofApp : public ofBaseApp{
private:
	/****************************************
	****************************************/
	enum{
		WIDTH = 300,
		HEIGHT = 400,
	};
	
	enum STATE{
		STATE__WAIT_MYSELF_READY,
		STATE__WAIT_CHILDREN_READY,
		STATE__RUNNING,
	};
	
	enum{
		TIMEOUT__CHILDREN_NORESPONSE_SEC = 2,
	};
	
	enum{
		THREAD_TIMETABLE__DMX,
		/*
		THREAD_TIMETABLE__VJ_CONTENTS_CHANGE_TIMING,
		*/
		
		NUM_THREAD_TIMETABLE,
	};
	
	/****************************************
	****************************************/
	ofTrueTypeFont font;
	
	const double VOL_INIT;
	const double VOL_STEP;
	ofSoundPlayer sound;
	
	int now_ms;
	int t_SeekTo_ms;
	
	double timer_StartStaying_inThis_State;
	
	enum STATE State;
	
	THREAD_BASE* thread_TimeTable[NUM_THREAD_TIMETABLE];
	
	/********************
	********************/
	enum{
		N = 256,
	};
	
	float spectrum[ N ];
	
    ofxPanel gui;
    ofxFloatSlider SmoothFilterThreshTime;
    ofxFloatSlider NonLinearFilter_ThreshLev;
	ofxToggle b_DownLimit;
    ofxFloatSlider Down_per_Frame_60fps;
    
    bool b_showGui;
	
	/****************************************
	****************************************/
	void Transition(enum STATE NewState);
	void Reset();
	void draw_time(double FrameRate);
	void print_timer();
	void print_total_musicTime();
	void music_try_Seek(int temp_t_SeekTo_ms);
	void getSpectrum();
	
	
public:
	/****************************************
	****************************************/
	ofApp();
	void exit();
	
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
};


