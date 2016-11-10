/************************************************************
************************************************************/
#include <unistd.h>
#include "th_DMX.h"

/************************************************************
function
************************************************************/

/******************************
******************************/
THREAD__DMX_KEY_TIMETABLE::THREAD__DMX_KEY_TIMETABLE()
{
	for(int i = 0; i < NUM_BUFFERS; i++){
		TimeTable[i] = new LED_KEYS[NUM_SAMPLES_PER_BUFFER];
	}
}

/******************************
******************************/
THREAD__DMX_KEY_TIMETABLE::~THREAD__DMX_KEY_TIMETABLE()
{
	for(int i = 0; i < NUM_BUFFERS; i++){
		delete[] TimeTable[i];
	}
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::exit()
{
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::charge(int BufferId_toCharge)
{
	/********************
	********************/
	if(!b_valid) return;
	
	/********************
	********************/
	float ElapsedTime_f = ofGetElapsedTimef();
	
	/********************
	********************/
	lock();
	bool b_EOF_Copy = b_EOF;
	unlock();
	if(b_EOF_Copy)	return;
	
	/********************
	********************/
	char buf_Log[BUF_SIZE];
	sprintf(buf_Log, "%.3f,,[%d] Charge Start\n", ElapsedTime_f, BufferId_toCharge);
	fprint_debug_Log(buf_Log, LogFile_id);
	
	
	char buf[BUF_SIZE];
	int Charge_id = 0;
	
	while(1){
		LED_KEYS LedKeyToCharge;
		static LED_KEYS LedKeyToCharge_pre;
		
		if(fscanf(fp, "%s", buf) == EOF){
			/********************
			********************/
			LedKeyToCharge.time_ms = -1;
			TimeTable[BufferId_toCharge][Charge_id] = LedKeyToCharge;
			
			lock();
			b_Empty[BufferId_toCharge] = false;
			b_EOF = true;
			unlock();
			
			/* */
			sprintf(buf_Log, "%.3f,,[%d] Last Charge Finish\n", ElapsedTime_f, BufferId_toCharge);
			fprint_debug_Log(buf_Log, LogFile_id);
			return;
			
		}else{
			if(strcmp(buf, "<NumLeds>") == 0){
				fscanf(fp, "%s", buf);
				int Scenario_NUM_LEDS = atoi(buf);
				if(Scenario_NUM_LEDS != NUM_LEDS){
					ERROR_MSG(); ofExit(1);
				}
				
			}else if(strcmp(buf, "<time_ms>") == 0){
				/********************
				********************/
				fscanf(fp, "%s", buf);
				LedKeyToCharge.time_ms = atoi(buf);
				
				/********************
				********************/
				for(int i = 0; i < NUM_LEDS; i++){
					/* */
					fscanf(fp, "%s", buf);
					int LedId = atoi(buf);
					
					/* */
					fscanf(fp, "%s", buf);
					LedKeyToCharge.Led[LedId].R = atoi(buf);
					
					fscanf(fp, "%s", buf);
					LedKeyToCharge.Led[LedId].G = atoi(buf);
					
					fscanf(fp, "%s", buf);
					LedKeyToCharge.Led[LedId].B = atoi(buf);
					
					fscanf(fp, "%s", buf);
					LedKeyToCharge.Led[LedId].W = atoi(buf);
					
					fscanf(fp, "%s", buf);
					LedKeyToCharge.Led[LedId].Pan = atoi(buf);

					fscanf(fp, "%s", buf);
					LedKeyToCharge.Led[LedId].Tilt = atoi(buf);
				}
				
				/********************
				********************/
				lock();
				int t_ofs_ms_temp = t_ofs_ms;
				unlock();
				
				if(t_ofs_ms_temp == 0){
					TimeTable[BufferId_toCharge][Charge_id] = LedKeyToCharge;
					Charge_id++;
				}else{
					if(t_ofs_ms_temp <= LedKeyToCharge.time_ms){
						TimeTable[BufferId_toCharge][Charge_id] = LedKeyToCharge_pre;
						Charge_id++;
						
						TimeTable[BufferId_toCharge][Charge_id] = LedKeyToCharge;
						Charge_id++;
						
						
						/* */
						lock();
						t_ofs_ms = 0;
						unlock();
						
					}else{
						LedKeyToCharge_pre = LedKeyToCharge;
					}
				}
				
				/********************
				********************/
				if(NUM_SAMPLES_PER_BUFFER <= Charge_id){
					lock();
					b_Empty[BufferId_toCharge] = false;
					unlock();
					
					/* */
					sprintf(buf_Log, "%.3f,,[%d] Charge Finish\n", ElapsedTime_f, BufferId_toCharge);
					fprint_debug_Log(buf_Log, LogFile_id);
					return;
				}
			}
		}
	}
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::set_LogFile_id()
{
	LogFile_id = THREAD_TIMETABLE__DMX;
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::setup()
{
	/********************
	********************/
	set_LogFile_id();
	
	/********************
	********************/
	udpConnection.Create();
	// udpConnection.Connect("10.0.0.2", UDP_SEND_PORT);
	udpConnection.Connect("127.0.0.1", UDP_SEND_PORT);
	udpConnection.SetNonBlocking(true);
	
	/********************
	********************/
	Reset();
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::Reset()
{
	/********************
	********************/
	this->THREAD_BASE::Reset();
	
	
	/********************
	********************/
	this->lock();
	
	/********************
	********************/
	fp = fopen("../../../data/StoryBoard.txt", "r");
	if(fp == NULL){
		ERROR_MSG();
		b_valid = false;
		
		stopThread();
		while(isThreadRunning()){
			waitForThread(true);
		}

	}else{
		b_valid = true;
	}
	
	
	id_from = 0;
	id_to = 0;
	
	b_1stUpdate = true;
	
	/********************
	********************/
	this->unlock();
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::update(int now_ms)
{
	/********************
	********************/
	if(!b_valid) return;
	
	/********************
	********************/
	float ElapsedTime_f = ofGetElapsedTimef();
	
	/********************
	********************/
	if(b_End){
		Leds_out.set_Param_of_AllLed_zero();
		return;
	}
	
	/********************
	********************/
	static LED_KEYS *Leds_From;
	static LED_KEYS *Leds_To;
	static LED_KEYS Dataset_Exchange;
	
	if(b_1stUpdate){
		b_1stUpdate = false;
		
		Leds_From	= &TimeTable[BufferId][id_from];
		Leds_To		= &TimeTable[BufferId][id_to];
	}
	
	char buf_Log[BUF_SIZE];
	while( !((Leds_From->time_ms <= now_ms) && (now_ms < Leds_To->time_ms)) ){
		id_from = id_to;
		Leds_From = &TimeTable[BufferId][id_from];
		
		id_to++;
		if(NUM_SAMPLES_PER_BUFFER <= id_to){
			/* */
			sprintf(buf_Log, "%.3f,%d,Buffer Change Start(BufferId from = %d)\n", ElapsedTime_f, now_ms, BufferId);
			fprint_debug_Log(buf_Log, LogFile_id);
			
			Wait_NextBufferFilled(1);
			
			/* */
			Dataset_Exchange = *Leds_From;
			Leds_From = &Dataset_Exchange;
			
			/* */
			this->lock();
			b_Empty[BufferId] = true;
			this->unlock();
			
			BufferId = get_NextBufferId();
			id_to = 0;
			
			/* */
			sprintf(buf_Log, "%.3f,%d,Buffer Change Finish(BufferId to = %d)\n", ElapsedTime_f, now_ms, BufferId);
			fprint_debug_Log(buf_Log, LogFile_id);
		}
		
		Leds_To = &TimeTable[BufferId][id_to];
		
		if(Leds_To->time_ms == -1){
			b_End = true;
			Leds_out.set_Param_of_AllLed_zero();
			return;
		}
	}
	
	/********************
	********************/
	for(int i = 0; i < NUM_LEDS; i++){
		/* */
		Leds_out.Led[i].R = (unsigned char)(double(Leds_From->Led[i].R) + double(now_ms - Leds_From->time_ms) / (Leds_To->time_ms - Leds_From->time_ms) * (Leds_To->Led[i].R - Leds_From->Led[i].R));
		Leds_out.Led[i].G = (unsigned char)(double(Leds_From->Led[i].G) + double(now_ms - Leds_From->time_ms) / (Leds_To->time_ms - Leds_From->time_ms) * (Leds_To->Led[i].G - Leds_From->Led[i].G));
		Leds_out.Led[i].B = (unsigned char)(double(Leds_From->Led[i].B) + double(now_ms - Leds_From->time_ms) / (Leds_To->time_ms - Leds_From->time_ms) * (Leds_To->Led[i].B - Leds_From->Led[i].B));
		Leds_out.Led[i].W = (unsigned char)(double(Leds_From->Led[i].W) + double(now_ms - Leds_From->time_ms) / (Leds_To->time_ms - Leds_From->time_ms) * (Leds_To->Led[i].W - Leds_From->Led[i].W));
		
		/* */
		Leds_out.Led[i].Pan		= int(double(Leds_From->Led[i].Pan) + double(now_ms - Leds_From->time_ms) / (Leds_To->time_ms - Leds_From->time_ms) * (Leds_To->Led[i].Pan - Leds_From->Led[i].Pan));
		Leds_out.Led[i].Tilt	= int(double(Leds_From->Led[i].Tilt) + double(now_ms - Leds_From->time_ms) / (Leds_To->time_ms - Leds_From->time_ms) * (Leds_To->Led[i].Tilt - Leds_From->Led[i].Tilt));
		
		
		/* */
		Leds_out.Led[i].LimitCheck();
	}
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::draw(float* spectrum, int N_SPECTRUM)
{
	/********************
	********************/
	if(!b_valid) return;
	
	/********************
	********************/
	string message = "";
	
	for(int i = 0; i < NUM_LEDS; i++){
		message +=	ofToString(int(Leds_out.Led[i].R))		+ "," + 
					ofToString(int(Leds_out.Led[i].G))		+ "," +
					ofToString(int(Leds_out.Led[i].B))		+ "," +
					ofToString(int(Leds_out.Led[i].W))		+ "," +
					ofToString(int(Leds_out.Led[i].Pan))	+ "," +
					ofToString(int(Leds_out.Led[i].Tilt))	+ "|";
	}
	
	/********************
	送り先のIPが不在だと、Errorとなり、関数の向こう側でError message表示し続けるので。
	********************/
	if(udpConnection.Send(message.c_str(),message.length()) == -1){
		ERROR_MSG();
		b_valid = false;
	}
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::draw_black(float* spectrum, int N_SPECTRUM)
{
	/********************
	********************/
	if(!b_valid) return;
	
	/********************
	********************/
	string message = "";
	
	for(int i = 0; i < NUM_LEDS; i++){
		message +=	ofToString(0)	+ "," + 
					ofToString(0)	+ "," +
					ofToString(0)	+ "," +
					ofToString(0)	+ "," +
					ofToString(0)	+ "," +
					ofToString(0)	+ "|";
	}
	
	/********************
	送り先のIPが不在だと、Errorとなり、関数の向こう側でError message表示し続けるので。
	********************/
	if(udpConnection.Send(message.c_str(),message.length()) == -1){
		ERROR_MSG();
		b_valid = false;
	}
}



