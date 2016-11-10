/************************************************************
************************************************************/
#include "thread_Base.h"


/************************************************************
************************************************************/

/******************************
******************************/
THREAD_BASE::THREAD_BASE()
: fp(NULL)
, t_ofs_ms(0)
, b_valid(false)
, LogFile_id(THREAD_TIMETABLE__DMX)
{
}

/******************************
******************************/
THREAD_BASE::~THREAD_BASE()
{
	if(fp)	fclose(fp);
}

/******************************
******************************/
void THREAD_BASE::threadedFunction()
{
	while(isThreadRunning()) {
		bool b_EOF_Copy;
		bool b_Empty_copy[NUM_BUFFERS];
		
		lock();
		for(int i = 0; i < NUM_BUFFERS; i++){
			b_Empty_copy[i] = b_Empty[i];
		}
		b_EOF_Copy = b_EOF;
		unlock();
		
		for(int i = 0; i < NUM_BUFFERS; i++){
			if(!b_EOF_Copy && b_Empty_copy[i]){
				charge(i);
			}
		}
		
		sleep(1);
	}
}

/******************************
******************************/
int THREAD_BASE::get_NextBufferId()
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
bool THREAD_BASE::Wait_NextBufferFilled(double timeout)
{
	/********************
	********************/
	bool b_Log_printed = false; // 一度でも待たされた場合はOne time Logを残す.
	
	double time_StepIn_sec = ofGetElapsedTimef();
	
	/********************
	********************/
	int NextBufferId = get_NextBufferId();
	
	while( ofGetElapsedTimef() - time_StepIn_sec < timeout ){
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
			fprint_debug_Log(buf_Log, LogFile_id);
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
void THREAD_BASE::Reset()
{
	this->lock();
	{
		if(fp)	{ fclose(fp); fp = NULL; }
		
		b_End = false;
		b_EOF = false;
		
		for(int i = 0; i < NUM_BUFFERS; i++){
			b_Empty[i] = true;
		}
		
		BufferId = 0;
		t_ofs_ms = 0;
	}	
	this->unlock();
}

/******************************
******************************/
void THREAD_BASE::setOffset(int ofs)
{
	this->lock();
	t_ofs_ms = ofs;
	this->unlock();
}

/******************************
******************************/
bool THREAD_BASE::IsReady()
{
	/********************
	fileの最後まで読み込みが完了している.
	全てのBufferを使わずに最後まで格納できてしまうこともあるので.
	********************/
	lock();
	bool b_EOF_Copy = b_EOF;
	unlock();
	if(b_EOF_Copy) return true;


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


/************************************************************
************************************************************/

/******************************
******************************/
THREAD_BASE_STEPOVER::THREAD_BASE_STEPOVER()
: LastINT_sec(-1)
{
}

/******************************
******************************/
THREAD_BASE_STEPOVER::~THREAD_BASE_STEPOVER()
{
}

/******************************
******************************/
void THREAD_BASE_STEPOVER::exit()
{
}

/******************************
******************************/
void THREAD_BASE_STEPOVER::charge(int BufferId_toCharge)
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
		if(fscanf(fp, "%s", buf) == EOF){
			SetTime_DataToCharge(-1);
			chargeTimeTable_byCopying(BufferId_toCharge, Charge_id);
			
			lock();
			b_Empty[BufferId_toCharge] = false;
			b_EOF = true;
			unlock();
			
			/* */
			sprintf(buf_Log, "%.3f,,[%d] Last Charge Finish\n", ElapsedTime_f, BufferId_toCharge);
			fprint_debug_Log(buf_Log, LogFile_id);
			return;
			
		}else{
			if(strcmp(buf, "<time_ms>") == 0){
				/********************
				********************/
				fscanf(fp, "%s", buf);
				SetTime_DataToCharge(atoi(buf));
				
				/********************
				Read and set.
				********************/
				SetData_DataToCharge(fp);
				
				/********************
				********************/
				lock();
				int t_ofs_ms_temp = t_ofs_ms;
				unlock();
				
				if(t_ofs_ms_temp == 0){
					chargeTimeTable_byCopying(BufferId_toCharge, Charge_id);
					Charge_id++;
				}else{
					if(t_ofs_ms_temp <= get_TimeData_from_DataToCharge()){
						chargeTimeTable_byCopying(BufferId_toCharge, Charge_id);
						Charge_id++;
						
						/* */
						lock();
						t_ofs_ms = 0;
						unlock();
						
					}else{
						// discard.
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
void THREAD_BASE_STEPOVER::setup()
{
	set_LogFile_id();
	
	Reset();
}

/******************************
******************************/
void THREAD_BASE_STEPOVER::Reset()
{
	/********************
	********************/
	this->THREAD_BASE::Reset();
	
	
	/********************
	********************/
	this->lock();
	
	/********************
	********************/
	fp = open_ScenarioFile();
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
	
	id = 0;
	b_send = false;
	
	/********************
	********************/
	this->unlock();
}

/******************************
******************************/
void THREAD_BASE_STEPOVER::update(int now_ms)
{
	/********************
	********************/
	if(!b_valid) return;
	
	/********************
	********************/
	float ElapsedTime_f = ofGetElapsedTimef();
	
	/********************
	********************/
	if(b_End)	return;
	
	/********************
	********************/
	char buf_Log[BUF_SIZE];
	
	while( get_TimeData_from_TimeTable(BufferId, id) <= now_ms ){
		if( (LastINT_sec == -1) || (ElapsedTime_f - LastINT_sec < 0.2) ){
			b_send = true;
			set_outputData(BufferId, id);
		}
		id++;
		if(NUM_SAMPLES_PER_BUFFER <= id){
			/* */
			sprintf(buf_Log, "%.3f,%d,Buffer Change Start(BufferId from = %d)\n", ElapsedTime_f, now_ms, BufferId);
			fprint_debug_Log(buf_Log, LogFile_id);
			
			Wait_NextBufferFilled(1);
			
			this->lock();
			b_Empty[BufferId] = true;
			this->unlock();
			
			BufferId = get_NextBufferId();
			id = 0;
			
			/* */
			sprintf(buf_Log, "%.3f,%d,Buffer Change Finish(BufferId to = %d)\n", ElapsedTime_f, now_ms, BufferId);
			fprint_debug_Log(buf_Log, LogFile_id);
		}
		
		if(get_TimeData_from_TimeTable(BufferId, id) == -1){
			b_End = true;
			return;
		}
	}
	
	LastINT_sec = ElapsedTime_f;
}

/******************************
******************************/
void THREAD_BASE_STEPOVER::draw(float* spectrum, int N_SPECTRUM)
{
	/********************
	********************/
	if(!b_valid) return;
	
	/********************
	********************/
	if(b_send){
		b_send = false;
		
		ofxOscMessage m;
		setOscMessage_for_draw(m);
		OscTarget[OSC_VJ].OscSend.sendMessage(m);
	}
}

/******************************
******************************/
void THREAD_BASE_STEPOVER::draw_black(float* spectrum, int N_SPECTRUM)
{
	// nothing.
}





