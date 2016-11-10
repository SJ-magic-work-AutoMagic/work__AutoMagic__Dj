/************************************************************
************************************************************/
#include <unistd.h>
#include "th_VJ_AlphaFFT.h"
#include "th_DMX.h"

/************************************************************
function
************************************************************/

/******************************
******************************/
THREAD__VJ_ALPHA_FFT::THREAD__VJ_ALPHA_FFT()
{
	for(int i = 0; i < NUM_BUFFERS; i++){
		TimeTable[i] = new TIME_N_DATASET__ALPHA_FFT[NUM_SAMPLES_PER_BUFFER];
	}
}

/******************************
******************************/
THREAD__VJ_ALPHA_FFT::~THREAD__VJ_ALPHA_FFT()
{
	for(int i = 0; i < NUM_BUFFERS; i++){
		delete[] TimeTable[i];
	}
}

/******************************
******************************/
void THREAD__VJ_ALPHA_FFT::exit()
{
}

/******************************
******************************/
void THREAD__VJ_ALPHA_FFT::SetData_DataToCharge(FILE* fp, TIME_N_DATASET__ALPHA_FFT &DatasetToCharge)
{
	char buf[BUF_SIZE];
	
	fscanf(fp, "%s", buf);
	DatasetToCharge.DataSet.mov_a = atof(buf);
	
	fscanf(fp, "%s", buf);
	DatasetToCharge.DataSet.b_mov_Effect_On = atoi(buf);

	fscanf(fp, "%s", buf);
	DatasetToCharge.DataSet.mov_a_0_12 = atof(buf);
	
	fscanf(fp, "%s", buf);
	DatasetToCharge.DataSet.b_mov0_Effect_On = atoi(buf);
	
	fscanf(fp, "%s", buf);
	DatasetToCharge.DataSet.mov_a_1_2 = atof(buf);

	fscanf(fp, "%s", buf);
	DatasetToCharge.DataSet.a_indicator = atof(buf);
	
	fscanf(fp, "%s", buf);
	DatasetToCharge.DataSet.a_particle = atof(buf);
	
	fscanf(fp, "%s", buf);
	DatasetToCharge.DataSet.b_GeneratedImage_on = atoi(buf);
	
	fscanf(fp, "%s", buf);
	DatasetToCharge.DataSet.b_text_on = atoi(buf);
	
	fscanf(fp, "%s", buf);
	DatasetToCharge.DataSet.a_Strobe = atof(buf);
}

/******************************
******************************/
void THREAD__VJ_ALPHA_FFT::charge(int BufferId_toCharge)
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
		TIME_N_DATASET__ALPHA_FFT DatasetToCharge;
		static TIME_N_DATASET__ALPHA_FFT DatasetToCharge_pre;
		
		if(fscanf(fp, "%s", buf) == EOF){
			/********************
			********************/
			DatasetToCharge.time_ms = -1;
			TimeTable[BufferId_toCharge][Charge_id] = DatasetToCharge;
			
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
				DatasetToCharge.time_ms = atoi(buf);
				
				/********************
				********************/
				SetData_DataToCharge(fp, DatasetToCharge);
				
				/********************
				********************/
				lock();
				int t_ofs_ms_temp = t_ofs_ms;
				unlock();
				
				if(t_ofs_ms_temp == 0){
					TimeTable[BufferId_toCharge][Charge_id] = DatasetToCharge;
					Charge_id++;
				}else{
					if(t_ofs_ms_temp <= DatasetToCharge.time_ms){
						TimeTable[BufferId_toCharge][Charge_id] = DatasetToCharge_pre;
						Charge_id++;
						
						TimeTable[BufferId_toCharge][Charge_id] = DatasetToCharge;
						Charge_id++;
						
						
						/* */
						lock();
						t_ofs_ms = 0;
						unlock();
						
					}else{
						DatasetToCharge_pre = DatasetToCharge;
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
void THREAD__VJ_ALPHA_FFT::set_LogFile_id()
{
	LogFile_id = THREAD_TIMETABLE__VJ_ALPHA_FFT;
}

/******************************
******************************/
void THREAD__VJ_ALPHA_FFT::setup()
{
	/********************
	********************/
	set_LogFile_id();
	
	/********************
	********************/
	udpConnection.Create();
	// udpConnection.Connect("10.0.0.3", UDP_SEND_PORT);
	udpConnection.Connect("127.0.0.1", UDP_SEND_PORT);
	udpConnection.SetNonBlocking(true);
	
	/********************
	********************/
	Reset();
}

/******************************
******************************/
void THREAD__VJ_ALPHA_FFT::Reset()
{
	/********************
	********************/
	this->THREAD_BASE::Reset();
	
	
	/********************
	********************/
	this->lock();
	
	/********************
	********************/
	fp = fopen("../../../data/VJSB_Alpha_fft.txt", "r");
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
void THREAD__VJ_ALPHA_FFT::update(int now_ms)
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
		data_to_output.set_allParam_zero();
		return;
	}
	
	/********************
	********************/
	static TIME_N_DATASET__ALPHA_FFT *Dataset_From;
	static TIME_N_DATASET__ALPHA_FFT *Dataset_To;
	static TIME_N_DATASET__ALPHA_FFT Dataset_Exchange;
	if(b_1stUpdate){
		b_1stUpdate = false;
		
		Dataset_From	= &TimeTable[BufferId][id_from];
		Dataset_To		= &TimeTable[BufferId][id_to];
	}
	
	char buf_Log[BUF_SIZE];
	while( !((Dataset_From->time_ms <= now_ms) && (now_ms < Dataset_To->time_ms)) ){
		id_from = id_to;
		Dataset_From = &TimeTable[BufferId][id_from];
		
		id_to++;
		if(NUM_SAMPLES_PER_BUFFER <= id_to){
			/* */
			sprintf(buf_Log, "%.3f,%d,Buffer Change Start(BufferId from = %d)\n", ElapsedTime_f, now_ms, BufferId);
			fprint_debug_Log(buf_Log, LogFile_id);
			
			Wait_NextBufferFilled(1);
			
			/* */
			Dataset_Exchange = *Dataset_From;
			Dataset_From = &Dataset_Exchange;
			
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
		
		Dataset_To = &TimeTable[BufferId][id_to];
		
		if(Dataset_To->time_ms == -1){
			b_End = true;
			data_to_output.set_allParam_zero();
			return;
		}
	}
	
	/********************
	********************/
	data_to_output.DataSet.mov_a = Dataset_From->DataSet.mov_a + double(now_ms - Dataset_From->time_ms) / (Dataset_To->time_ms - Dataset_From->time_ms) * (Dataset_To->DataSet.mov_a - Dataset_From->DataSet.mov_a);
	data_to_output.DataSet.mov_a_0_12 = Dataset_From->DataSet.mov_a_0_12 + double(now_ms - Dataset_From->time_ms) / (Dataset_To->time_ms - Dataset_From->time_ms) * (Dataset_To->DataSet.mov_a_0_12 - Dataset_From->DataSet.mov_a_0_12);
	data_to_output.DataSet.mov_a_1_2 = Dataset_From->DataSet.mov_a_1_2 + double(now_ms - Dataset_From->time_ms) / (Dataset_To->time_ms - Dataset_From->time_ms) * (Dataset_To->DataSet.mov_a_1_2 - Dataset_From->DataSet.mov_a_1_2);
	data_to_output.DataSet.a_indicator = Dataset_From->DataSet.a_indicator + double(now_ms - Dataset_From->time_ms) / (Dataset_To->time_ms - Dataset_From->time_ms) * (Dataset_To->DataSet.a_indicator - Dataset_From->DataSet.a_indicator);
	data_to_output.DataSet.a_particle = Dataset_From->DataSet.a_particle + double(now_ms - Dataset_From->time_ms) / (Dataset_To->time_ms - Dataset_From->time_ms) * (Dataset_To->DataSet.a_particle - Dataset_From->DataSet.a_particle);
	data_to_output.DataSet.a_Strobe = Dataset_From->DataSet.a_Strobe + double(now_ms - Dataset_From->time_ms) / (Dataset_To->time_ms - Dataset_From->time_ms) * (Dataset_To->DataSet.a_Strobe - Dataset_From->DataSet.a_Strobe);

	data_to_output.DataSet.b_mov_Effect_On = Dataset_From->DataSet.b_mov_Effect_On;
	data_to_output.DataSet.b_mov0_Effect_On = Dataset_From->DataSet.b_mov0_Effect_On;
	data_to_output.DataSet.b_GeneratedImage_on = Dataset_From->DataSet.b_GeneratedImage_on;
	data_to_output.DataSet.b_text_on = Dataset_From->DataSet.b_text_on;
}

/******************************
******************************/
void THREAD__VJ_ALPHA_FFT::draw(float* spectrum, int N_SPECTRUM)
{
	/********************
	********************/
	if(!b_valid) return;
	
	/********************
	********************/
	string message = "";
	
	message +=	ofToString(data_to_output.DataSet.mov_a)				+ "," + 
				ofToString(data_to_output.DataSet.b_mov_Effect_On)		+ "," + 
				ofToString(data_to_output.DataSet.mov_a_0_12)			+ "," + 
				ofToString(data_to_output.DataSet.b_mov0_Effect_On)		+ "," + 
				ofToString(data_to_output.DataSet.mov_a_1_2)			+ "," + 
				ofToString(data_to_output.DataSet.a_indicator)			+ "," + 
				ofToString(data_to_output.DataSet.a_particle)			+ "," + 
				ofToString(data_to_output.DataSet.b_GeneratedImage_on)	+ "," + 
				ofToString(data_to_output.DataSet.b_text_on)			+ "," + 
				ofToString(data_to_output.DataSet.a_Strobe)			+ "|";
				
	if(spectrum != NULL){
		for(int i = 0; i < N_SPECTRUM; i++){
			message += ofToString(spectrum[i]) + ",";
		}
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
void THREAD__VJ_ALPHA_FFT::draw_black(float* spectrum, int N_SPECTRUM)
{
	/********************
	********************/
	if(!b_valid) return;
	
	/********************
	********************/
	string message = "";
	
	message +=	ofToString(0)	+ "," + 
				ofToString(0)	+ "," + 
				ofToString(0)	+ "," + 
				ofToString(0)	+ "," + 
				ofToString(0)	+ "," + 
				ofToString(0)	+ "," + 
				ofToString(0)	+ "," + 
				ofToString(0)	+ "," + 
				ofToString(0)	+ "," + 
				ofToString(0)	+ "|";
	
	if(spectrum != NULL){
		for(int i = 0; i < N_SPECTRUM; i++){
			message += ofToString(spectrum[i]) + ",";
		}
	}
	
	/********************
	送り先のIPが不在だと、Errorとなり、関数の向こう側でError message表示し続けるので。
	********************/
	if(udpConnection.Send(message.c_str(),message.length()) == -1){
		ERROR_MSG();
		b_valid = false;
	}
}



