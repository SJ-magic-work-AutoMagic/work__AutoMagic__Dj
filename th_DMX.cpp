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
: fp(NULL)
, t_ofs_ms(0)
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
	
	if(fp)	fclose(fp);
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::exit()
{
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::threadedFunction()
{
	while(isThreadRunning()) {
		bool b_Empty_copy[NUM_BUFFERS];
		
		lock();
		for(int i = 0; i < NUM_BUFFERS; i++){
			b_Empty_copy[i] = b_Empty[i];
		}
		unlock();
		
		for(int i = 0; i < NUM_BUFFERS; i++){
			if(!b_EOF && b_Empty_copy[i]){
				charge(i);
			}
		}
	}
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::setOffset(int ofs)
{
	this->lock();
	t_ofs_ms = ofs;
	this->unlock();
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::charge(int BufferId_toCharge)
{
	/********************
	********************/
	float ElapsedTime_f = ofGetElapsedTimef();
	
	/********************
	********************/
	if(b_EOF)	return;
	
	/********************
	********************/
	char buf_Log[BUF_SIZE];
	sprintf(buf_Log, "%.3f,,[%d] Charge Start\n", ElapsedTime_f, BufferId_toCharge);
	fprint_debug_Log(buf_Log);
	
	
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
			unlock();
			
			b_EOF = true;
			
			/* */
			sprintf(buf_Log, "%.3f,,[%d] Last Charge Finish\n", ElapsedTime_f, BufferId_toCharge);
			fprint_debug_Log(buf_Log);
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
					fprint_debug_Log(buf_Log);
					return;
				}
			}
		}
	}
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::setup()
{
	/********************
	********************/
	udpConnection.Create();
	udpConnection.Connect("10.0.0.2", UDP_SEND_PORT);
	// udpConnection.Connect("127.0.0.1", UDP_SEND_PORT);
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
	this->lock();
	
	/********************
	********************/
	if(fp)	{ fclose(fp); fp = NULL; }
	
	fp = fopen("../../../data/StoryBoard.txt", "r");
	if(fp == NULL)	{ ERROR_MSG(); ofExit(1); }
	
	b_End = false;
	b_EOF = false;
	
	for(int i = 0; i < NUM_BUFFERS; i++){
		b_Empty[i] = true;
	}
	
	BufferId = 0;
	id_from = 0;
	id_to = 0;
	
	b_1stUpdate = true;
	
	t_ofs_ms = 0;
	
	/********************
	********************/
	this->unlock();
}

/******************************
******************************/
bool THREAD__DMX_KEY_TIMETABLE::IsReady()
{
	/********************
	fileの最後まで読み込みが完了している.
	全てのBufferを使わずに最後まで格納できてしまうこともあるので.
	********************/
	if(b_EOF) return true;


	/********************
	********************/
	bool b_Empty_copy[NUM_BUFFERS];
	
	this->lock();
	for(int i = 0; i < NUM_BUFFERS; i++){
		b_Empty_copy[i] = b_Empty[i];
	}
	this->unlock();
	
	for(int i = 0; i < NUM_BUFFERS; i++){
		if(b_Empty_copy[i] == true)	return false;
	}
	
	return true;
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::update(int now_ms)
{
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
			fprint_debug_Log(buf_Log);
			
			Wait_NextBufferFilled(10);
			
			this->lock();
			b_Empty[BufferId] = true;
			this->unlock();
			
			BufferId = get_NextBufferId();
			id_to = 0;
			
			/* */
			sprintf(buf_Log, "%.3f,%d,Buffer Change Finish(BufferId to = %d)\n", ElapsedTime_f, now_ms, BufferId);
			fprint_debug_Log(buf_Log);
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
int THREAD__DMX_KEY_TIMETABLE::get_NextBufferId()
{
	int NextBufferId = BufferId + 1;
	if(NUM_BUFFERS <= NextBufferId) NextBufferId = 0;
	
	return NextBufferId;
}

/******************************
param
	timeout
		timeout in second.

return
	0	:OK. Completed within timeout.
	1	:NG. 実際は、0secで抜ける想定なので、ERROR exitとしてある.
******************************/
bool THREAD__DMX_KEY_TIMETABLE::Wait_NextBufferFilled(double timeout)
{
	/********************
	********************/
	float ElapsedTime_f = ofGetElapsedTimef();
	
	/********************
	********************/
	bool b_Log_printed = false; // 一度でも待たされた場合はOne time Logを残す.
	
	double time_StepIn_sec = ElapsedTime_f;
	
	/********************
	********************/
	int NextBufferId = get_NextBufferId();
	
	while( ElapsedTime_f - time_StepIn_sec < timeout ){
		this->lock();
		bool b_Empty_copy = b_Empty[NextBufferId];
		this->unlock();
		
		if(!b_Empty_copy){
			return 0;
			
		}else if(!b_Log_printed){
			b_Log_printed = true;
			
			/* */
			char buf_Log[BUF_SIZE];
			sprintf(buf_Log, ",,Wait NextBuffer Filled occured\n");
			fprint_debug_Log(buf_Log);
		}
		// Sleep(1); // ms
		usleep(1000);
	}
	
	ERROR_MSG();
	ofExit();
	return 1;
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::draw()
{
	string message = "";
	
	for(int i = 0; i < NUM_LEDS; i++){
		message +=	ofToString(int(Leds_out.Led[i].R))		+ "," + 
					ofToString(int(Leds_out.Led[i].G))		+ "," +
					ofToString(int(Leds_out.Led[i].B))		+ "," +
					ofToString(int(Leds_out.Led[i].W))		+ "," +
					ofToString(int(Leds_out.Led[i].Pan))	+ "," +
					ofToString(int(Leds_out.Led[i].Tilt))	+ "|";
	}
	
	udpConnection.Send(message.c_str(),message.length());
}

/******************************
******************************/
void THREAD__DMX_KEY_TIMETABLE::draw_black()
{
	string message = "";
	
	for(int i = 0; i < NUM_LEDS; i++){
		message +=	ofToString(0)	+ "," + 
					ofToString(0)	+ "," +
					ofToString(0)	+ "," +
					ofToString(0)	+ "," +
					ofToString(0)	+ "," +
					ofToString(0)	+ "|";
	}
	
	udpConnection.Send(message.c_str(),message.length());
}



