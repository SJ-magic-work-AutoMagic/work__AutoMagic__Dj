/************************************************************
************************************************************/
#include "ofApp.h"

/************************************************************
param
************************************************************/
enum{
	OSC_DMX_LIGHT,
	OSC_VJ,
	
	NUM_OSC_TARGET,
};

static OSC_TARGET OscTarget[] = {
	OSC_TARGET("10.0.0.2", 12346, 12345), // OSC_DMX_LIGHT,
	// OSC_TARGET("127.0.0.1", 12346, 12345), // OSC_DMX_LIGHT,
	
	OSC_TARGET("10.0.0.3", 12349, 12348), // OSC_VJ,
};

static bool b_OscReady[NUM_OSC_TARGET];
static float t_timerFrom[NUM_OSC_TARGET];

/************************************************************
************************************************************/
/******************************
******************************/
ofApp::ofApp()
: t_SeekTo_ms(0)
, VOL_INIT(1.0)
, VOL_STEP(0.05)
, timer_StartStaying_inThis_State(0)
, thread_DmxTimeTable(THREAD__DMX_KEY_TIMETABLE::getInstance())
{
}

/******************************
******************************/
void ofApp::exit()
{
	/********************
	********************/
	thread_DmxTimeTable.exit();
	try{
		/********************
		stop済みのthreadをさらにstopすると、Errorが出るようだ。
		********************/
		thread_DmxTimeTable.stopThread();
		while(thread_DmxTimeTable.isThreadRunning()){
			thread_DmxTimeTable.waitForThread(true);
		}
	}catch(...){
		printf("Thread exiting Error\n");
	}
	
	/********************
	********************/
	fclose_LogFile();

	/********************
	********************/
	ofxOscMessage m;
	m.setAddress("/Quit");
	m.addIntArg(1);
	for(int i = 0; i < NUM_OSC_TARGET; i++){
		OscTarget[i].OscSend.sendMessage(m);
	}
	
	/********************
	********************/
	printMessage("Good-bye");
	
	std::exit(1);
}


//--------------------------------------------------------------
void ofApp::setup(){
	/********************
	********************/
	font.loadFont("RictyDiminished-Regular.ttf", 15);
	
	/********************
	各種基本設定
	********************/
	ofSetWindowTitle("Dj");
	ofSetFrameRate(60);
	ofSetWindowShape(WIDTH, HEIGHT);
	ofSetEscapeQuitsApp(false);
	
	fopen_LogFile();
	
	/********************
	********************/
	gui.setup( "Parameters", "settings.xml" );
	
	gui.add( SmoothFilterThreshTime.setup( "SmoothTime", 0.3, 0, 1.0 ) );	// 小さくする程Responceが良くなる.
	gui.add( NonLinearFilter_ThreshLev.setup( "NonLinLev", 0, 0, 0.3 ) );	// 0.02
	gui.add( b_DownLimit.setup( "Down limit", false ) );
	gui.add( Down_per_Frame_60fps.setup( "Down60fps", 0.015, 0.001, 0.2 ) );
	
	b_showGui = false;
	
	for(int i = 0; i < N; i++){
		spectrum[i] = 0;
	}
		
	/********************
	********************/
	thread_DmxTimeTable.setup();
	
	/********************
	********************/
	sound.loadSound("music.wav");
	if(!sound.isLoaded())	{ ERROR_MSG(); ofExit(); }
	
	sound.setLoop(true);
	sound.setMultiPlay( true );
	// sound.setSpeed( 1.0 );
	sound.setVolume(VOL_INIT);
	
	/********************
	********************/
	Reset();
	
	/********************
	********************/
	thread_DmxTimeTable.startThread(true, false); // blocking, non verboss
}

/******************************
******************************/
void ofApp::Reset()
{
	/********************
	********************/
	Transition(STATE__WAIT_MYSELF_READY);
	
	/********************
	********************/
	for(int i = 0; i < NUM_OSC_TARGET; i++){
		b_OscReady[i] = false;
	}
}

/******************************
******************************/
void ofApp::Transition(enum STATE NewState)
{
	float ElapsedTime_f = ofGetElapsedTimef();
	char buf_Log[BUF_SIZE];
	
	State = NewState;
	
	switch(State){
		case STATE__WAIT_MYSELF_READY:
			printMessage("STATE__WAIT_MYSELF_READY");
			sprintf(buf_Log, "%.3f,,> STATE__WAIT_MYSELF_READY\n", ElapsedTime_f);
			break;
			
		case STATE__WAIT_CHILDREN_READY:
			printMessage("STATE__WAIT_CHILDREN_READY");
			sprintf(buf_Log, "%.3f,,> STATE__WAIT_CHILDREN_READY\n", ElapsedTime_f);
			
			for(int i = 0; i < NUM_OSC_TARGET; i++){
				b_OscReady[i] = false;
				t_timerFrom[i] = ElapsedTime_f;
			}

			break;
			
		case STATE__RUNNING:
			printMessage("STATE__RUNNING");
			sprintf(buf_Log, "%.3f,,> STATE__RUNNING\n", ElapsedTime_f);
			
			strcat(buf_Log, ",,");
			for(int i = 0; i < NUM_OSC_TARGET; i++){
				char buf_temp[BUF_SIZE];
				sprintf(buf_temp, "%d+", b_OscReady[i]);
				
				strcat(buf_Log, buf_temp);
			}
			strcat(buf_Log, "\n");
			break;
	}
	fprint_debug_Log(buf_Log);
	timer_StartStaying_inThis_State = ElapsedTime_f;
}

/******************************
******************************/
void ofApp::print_timer()
{
	printf("timer:%8.3f\r", ofGetElapsedTimef() - timer_StartStaying_inThis_State);
}

//--------------------------------------------------------------
void ofApp::update(){
	/********************
	********************/
	float ElapsedTime_f = ofGetElapsedTimef();

	/********************
	Loop速すぎると、時間表示など、おかしい部分があった.
	musicはmusicで速く回りすぎないよう、"Last_INT_music_ms"を使って制御.
	********************/
	const double LOOP_SPEED_TOO_FAST_SEC = 0.005;
	static double Last_INT_system_ms = ElapsedTime_f;
	if(ElapsedTime_f - Last_INT_system_ms < LOOP_SPEED_TOO_FAST_SEC){
		return;
	}
	Last_INT_system_ms = ElapsedTime_f;
	
	/********************
	********************/
	for(int i = 0; i < NUM_OSC_TARGET; i++){
		while(OscTarget[i].OscReceive.hasWaitingMessages()){
			ofxOscMessage m;
			OscTarget[i].OscReceive.getNextMessage(&m);
			
			/********************
			********************/
			switch(State){
				case STATE__WAIT_MYSELF_READY:
				case STATE__RUNNING:
					// none. 読み捨て.
					break;
					
				case STATE__WAIT_CHILDREN_READY:
					if(m.getAddress() == "/Ready"){
						if(m.getArgAsInt32(0))	b_OscReady[i] = true;
						else					b_OscReady[i] = false;

						t_timerFrom[i] = ElapsedTime_f;
					}
					break;
			}
		}
	}
	
	/********************
	********************/
	static int Last_INT_music_ms = 0;
	static bool b_1stINT_toPlay = false;
	
	switch(State){
		case STATE__WAIT_MYSELF_READY:
			if( thread_DmxTimeTable.IsReady() ){
				Transition(STATE__WAIT_CHILDREN_READY);
			}
			break;
			
		case STATE__WAIT_CHILDREN_READY:
		{
			int counter = 0;
			
			for(int i = 0; i < NUM_OSC_TARGET; i++){
				if( b_OscReady[i] || (TIMEOUT__CHILDREN_NORESPONSE_SEC < ElapsedTime_f - t_timerFrom[i]) ){
					counter++;
				}
			}
			if(NUM_OSC_TARGET <= counter){
				Transition(STATE__RUNNING);
				
				sound.play();
				sound.setPositionMS(t_SeekTo_ms);
				t_SeekTo_ms = 0;
				Last_INT_music_ms = -1;
				
				b_1stINT_toPlay = true;
				
			}else{
				ofxOscMessage m;
				m.setAddress("/AreYouReady");
				m.addIntArg(1);
				for(int i = 0; i < NUM_OSC_TARGET; i++){
					OscTarget[i].OscSend.sendMessage(m);
				}
			}
		}
			break;
			
		case STATE__RUNNING:
			break;
	}
	
	/********************
	********************/
	if(State == STATE__RUNNING){
		char buf_Log[BUF_SIZE];
		
		if(t_SeekTo_ms != 0){
			sprintf(buf_Log, "%.3f,%d,music Seek\n", ElapsedTime_f, now_ms);
			fprint_debug_Log(buf_Log);
				
			sound.stop();
			thread_DmxTimeTable.stopThread();
			thread_DmxTimeTable.waitForThread(true);
			thread_DmxTimeTable.Reset();
			thread_DmxTimeTable.setOffset(t_SeekTo_ms);
			thread_DmxTimeTable.startThread(true, false);
			
			Reset();
			
		}else{
			ofSoundUpdate();
			getSpectrum();
			now_ms = sound.getPositionMS();
			
			if(b_1stINT_toPlay){
				b_1stINT_toPlay = false;
				
				sprintf(buf_Log, "%.3f,%d,1st INT to Play\n", ElapsedTime_f, now_ms);
				fprint_debug_Log(buf_Log);
			}
			
			if(now_ms < Last_INT_music_ms){ // Loop
				printMessage("music Loop");
				sprintf(buf_Log, "%.3f,%d,music Loop\n", ElapsedTime_f, now_ms);
				fprint_debug_Log(buf_Log);
				
				sound.stop();
				thread_DmxTimeTable.stopThread();
				thread_DmxTimeTable.waitForThread(true);
				thread_DmxTimeTable.Reset();
				thread_DmxTimeTable.startThread(true, false);
				
				Reset();
				
				
			}else if(Last_INT_music_ms < now_ms){
				/* */
				thread_DmxTimeTable.update(now_ms);
				
				/* */
				Last_INT_music_ms = now_ms;
				
			}else{
				/* Loop速すぎて同時刻の場合はSkip */
			}
		}
	}

}

/******************************
******************************/
void ofApp::getSpectrum()
{
	/********************
	********************/
	//Get current spectrum with N bands
	float *val = ofSoundGetSpectrum( N );
	//We should not release memory of val,
	//because it is managed by sound engine
	
	/********************
	********************/
	float val_ave[N];
	
	/* */
	static float LastINT_sec = 0;
	float now = ofGetElapsedTimef();
	float dt = ofClamp(now - LastINT_sec, 0, 0.1);
	LastINT_sec = now;
	
	/* average */
	double SmoothFilterAlpha;
	if(0 < SmoothFilterThreshTime){
		double tangent = 1 / SmoothFilterThreshTime;
		
		if(dt < SmoothFilterThreshTime)	SmoothFilterAlpha = tangent * dt;
		else							SmoothFilterAlpha = 1;
	}else{

		SmoothFilterAlpha = 1;
	}
	
	/* Non Linear */
	double NonLinearFilter_k;
	if(0 < NonLinearFilter_ThreshLev){
		NonLinearFilter_k = 1/NonLinearFilter_ThreshLev;
	}else{
		NonLinearFilter_k = 0;
	}
	
	/* Down Speed */
	const float Down_per_ms = Down_per_Frame_60fps / 16.6;
	float DownRatio = Down_per_ms * dt * 1000;
	
	/* */
	for ( int i=0; i<N; i++ ) {
		/* */
		val_ave[i] = SmoothFilterAlpha * val[i] + (1 - SmoothFilterAlpha) * spectrum[i];
		
		/* */
		double diff = val_ave[i] - spectrum[i];
		if(0 < NonLinearFilter_ThreshLev){
			if( (0 <= diff) && (diff < NonLinearFilter_ThreshLev) ){
				diff = NonLinearFilter_k * pow(diff, 2);
			}else if( (-NonLinearFilter_ThreshLev < diff) && (diff < 0) ){
				diff = -NonLinearFilter_k * pow(diff, 2);
			}else{
				diff = diff;
			}
		}
		float val_NonLinearFilter_out = spectrum[i] + diff;
		
		/* */
		if(b_DownLimit){
			spectrum[i] *= (1 - DownRatio);
			spectrum[i] = max( spectrum[i], val_NonLinearFilter_out );
		}else{
			spectrum[i] = val_NonLinearFilter_out;
		}
	}	
}

/******************************
******************************/
void ofApp::draw_time(double FrameRate)
{
	/********************
	********************/
	char buf[BUF_SIZE];
	
	int min	= now_ms / 1000 / 60;
	int sec	= now_ms / 1000 - min * 60;
	int ms	= now_ms % 1000;
	
	sprintf(buf, "%6d:%6d:%6d\n%7.2f", min, sec, ms, FrameRate);
	
	
	/********************
	********************/
	ofSetColor(255, 255, 255);
	
	font.drawString(buf, 10, 50);
	
	/*
	float w = font.stringWidth(buf);
	float h = font.stringHeight(buf);
	float x = ofGetWidth() / 2 - w / 2;
	float y = ofGetHeight() / 2 + h / 2;
	
	font.drawString(buf, x, y);
	*/
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofBackground(30);
	
	if(State == STATE__RUNNING){
		/********************
		********************/
		draw_time( ofGetFrameRate() );
		thread_DmxTimeTable.draw();
		
		if(b_showGui) gui.draw();
		
	}else{
		thread_DmxTimeTable.draw_black();
		print_timer();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	/********************
	********************/
	enum STATE_INPUT{
		STATE_INPUT_NONE,
		STATE_INPUT_SEEK_TARGET,
	};
	enum{
		BUF_SIZE = 100,
	};
	
	static enum STATE_INPUT StateInput = STATE_INPUT_NONE;
	static int input[BUF_SIZE];
	static int index = 0;
	
	/********************
	********************/
	switch(key){
		case OF_KEY_UP:
		{
			double vol = sound.getVolume();
			vol += VOL_STEP;
			if(1.0 < vol) vol = 1.0;
			sound.setVolume(vol);
			printMessage("set volume");
			printf("%.2f\n", vol);
		}
			break;
			
		case OF_KEY_DOWN:
		{
			double vol = sound.getVolume();
			vol -= VOL_STEP;
			if(vol < 0) vol = 0;
			sound.setVolume(vol);
			printMessage("set volume");
			printf("%.2f\n", vol);
		}
			break;
			
		case OF_KEY_RIGHT:
			break;
			
		case OF_KEY_LEFT:
			break;
		
		case OF_KEY_ESC:
			printMessage("ESC disabled");
			break;
			
		case OF_KEY_RETURN:
			if(StateInput == STATE_INPUT_SEEK_TARGET){
				index--;
				
				if(index < 0){
					// nothing.
				}else{
					int input_val = 0;
					int i = 0;
					
					while(0 <= index){
						input_val += input[i] * pow(10, index);
						
						i++;
						index--;
					}
					
					music_try_Seek(input_val * 1000);
				}
			}
			
			StateInput = STATE_INPUT_NONE;
			
			break;
			
		case 'h':
			b_showGui = !b_showGui;
			break;
			
		case 'k':
			if(State == STATE__RUNNING){
				switch(StateInput){
					case STATE_INPUT_NONE:
						StateInput = STATE_INPUT_SEEK_TARGET;
						index = 0;
						
						printMessage("input SeekTarget[sec]:");
						break;
						
					case STATE_INPUT_SEEK_TARGET:
						StateInput = STATE_INPUT_NONE;
						
						printMessage("Cancel to input SeekTarget");
						break;
				}
			}
			break;
			
		case 't':
			print_total_musicTime();
			break;
			
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if(StateInput == STATE_INPUT_SEEK_TARGET){
				if(index < BUF_SIZE){
					input[index] = key - '0';
					index++;
				}
				printf("%d", key - '0');
				fflush(stdout);
			}
			break;
		
		default:
			break;
	}
}

/******************************
******************************/
void ofApp::print_total_musicTime()
{
	double pos = sound.getPosition();
	int pos_ms = sound.getPositionMS();
	
	printMessage("Total Music Time");
	
	if(pos <= 0){
		printf("please try again\n");
		
	}else{
		const int TotalLength_ms = int(pos_ms / pos);
		int min	= TotalLength_ms / 1000 / 60;
		int sec	= TotalLength_ms / 1000 - min * 60;
		int ms	= TotalLength_ms % 1000;
		
		printf("%5d:%5d:%5d = %d[sec]\n", min, sec, ms, TotalLength_ms / 1000);
	}
	printf("\n");
}

/******************************
******************************/
void ofApp::music_try_Seek(int temp_t_SeekTo_ms)
{
	double pos = sound.getPosition();
	int pos_ms = sound.getPositionMS();
	
	if(pos <= 0){
		printMessage("Try seek failed : please try again");
		return;
		
	}else{
		const int TotalLength = int(pos_ms / pos);
		const int MARGIN_MS = 5000;
		
		if( (temp_t_SeekTo_ms < 0) || (TotalLength < temp_t_SeekTo_ms + MARGIN_MS) ){
			printMessage("Try seek : out of range");
			
		}else{
			t_SeekTo_ms = temp_t_SeekTo_ms;
			printMessage("Try seek OK");
		}
		
		return;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
