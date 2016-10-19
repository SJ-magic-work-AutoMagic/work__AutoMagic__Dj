/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxGui.h"

#include "sj_common.h"
#include "th_DMX.h"


/************************************************************
************************************************************/

/**************************************************
**************************************************/
class OSC_SEND{
private:
	char IP[BUF_SIZE];
	int Port;

	ofxOscSender sender;
	
public:
	OSC_SEND(const char* _IP, int _Port)
	{
		if(_Port != -1){
			sprintf(IP, "%s", _IP);
			Port = _Port;
			
			sender.setup(IP, Port);
		}
	}
	
	void sendMessage(ofxOscMessage &message)
	{
		if(Port != -1){
			sender.sendMessage(message);
		}
	}
};

class OSC_RECEIVE{
private:
	int Port;
	ofxOscReceiver	receiver;
	
public:
	OSC_RECEIVE(int _Port)
	{
		if(_Port != -1){
			Port = _Port;
			receiver.setup(Port);
		}
	}
	
	bool hasWaitingMessages()
	{
		if(Port == -1){
			return false;
		}else{
			return receiver.hasWaitingMessages();
		}
	}
	
	bool getNextMessage(ofxOscMessage *msg)
	{
		if(Port == -1){
			return false;
		}else{
			return receiver.getNextMessage(msg);
		}
	}
};

class OSC_TARGET{
private:
public:
	OSC_SEND	OscSend;
	OSC_RECEIVE	OscReceive;
	
	OSC_TARGET(const char* _SendTo_IP, int _SendTo_Port, int _Receive_Port)
	: OscSend(_SendTo_IP, _SendTo_Port), OscReceive(_Receive_Port)
	{
	}
};


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
	
	THREAD__DMX_KEY_TIMETABLE& thread_DmxTimeTable;
	
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


