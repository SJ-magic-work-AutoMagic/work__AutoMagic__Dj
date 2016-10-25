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


