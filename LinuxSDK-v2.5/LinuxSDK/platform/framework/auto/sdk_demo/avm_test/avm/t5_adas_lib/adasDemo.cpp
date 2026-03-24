#include "sf_xcb_bsd.h"
#include "sf_xcb_ldw.h"
#include "sf_xcb_mod.h"
#include "sf_xcb_fcw.h"

/**************************************************************************************/
/**************************************************************************************/
void *LDW_process(void *arg) 
{
    sf_xcb_ldw_output_t *output = NULL;
	sf_xcb_ldw_params_t params;
	params.fStSpeed = 52;
	params.fActSpeed = 60;
	params.nCarWidth = 180;
	params.nWheelbase = 260;
	float fSpeed = 0;

	long long timeStampMs;
	bool cameraON[4] = {1,1,1,1 };
	const char *auth_file = SAVE_LICENSE_PATH;
	char mapPath[500], mapPath_default[500];
	char calibOutPath[500], calibOutPath_default[500];

	sprintf(mapPath, "%s/Ldw2DMappingModel", g_pCreateModelDir);
	sprintf(mapPath_default, "%s/Ldw2DMappingModel", g_pGivenModelDir);
	sprintf(calibOutPath, "%s/calibOut", g_pCreateModelDir);
	sprintf(calibOutPath_default, "%s/calibOut", g_pGivenModelDir);

	//printf("[655B_sfavm360][%s][%d]  mapPath=%s,mapPath_default=%s,calibOutPath=%s,calibOutPath_default=%s  \n", __func__,__LINE__,mapPath,mapPath_default,calibOutPath,calibOutPath_default);

	pthread_mutex_lock(&mutexin);
	void *pLDW = sf_xcb_ldw_init(&params, mapPath, mapPath_default,auth_file);
	pthread_mutex_unlock(&mutexin);
    
    while(1)
    {uint8_t u8Speed = msg.tState.u8Speed;
		uint8_t u8Switch1 = msg.adas_switch;
		xcb_avm_car_state_gear_t &emGear = msg.tState.emGear;//获取挡位信息
		
        fSpeed = (float)u8Speed;//获取速度信息

		{
			#if ADASLOG
				printf("[(fSpeed > 60) && (fSpeed < 120)]speed=%f LDW run\n", fSpeed);
			#endif

      		
            timeStampMs = tm.getCurrentTime();
			pthread_mutex_lock(&get_data2);
			memcpy(ADASIMG[0],NV21IMG[0], NV21SIZE);
			pthread_mutex_unlock(&get_data2);
			
			pthread_mutex_lock(&get_data3);
			memcpy(ADASIMG[1],NV21IMG[1], NV21SIZE);
			pthread_mutex_unlock(&get_data3);
			
			pthread_mutex_lock(&get_data1);
			memcpy(ADASIMG[2],NV21IMG[2], NV21SIZE);
			pthread_mutex_unlock(&get_data1);

			pthread_mutex_lock(&get_data4);
			memcpy(ADASIMG[3],NV21IMG[3], NV21SIZE);
			pthread_mutex_unlock(&get_data4);

      		sf_xcb_ldw_process(pLDW, ADASIMG[0], ADASIMG[1], ADASIMG[2], ADASIMG[3],fSpeed, timeStampMs, &output);

			if (output && (output->rightLine.isDetected != 0 && output->leftLine.isDetected != 0))
			{
					ldw_lline[0] = (output->leftcolorline.line.x0 - 235) * 20; // 1pix=20mm
					ldw_lline[1] = (300 - output->leftcolorline.line.y0) * 20;
					ldw_lline[2] = (output->leftcolorline.line.x1 - 235) * 20;
					ldw_lline[3] = (300 - output->leftcolorline.line.y1) * 20;
					ldw_rline[0] = (output->rightcolorline.line.x0 - 235) * 20;
					ldw_rline[1] = (300 - output->rightcolorline.line.y0) * 20;
					ldw_rline[2] = (output->rightcolorline.line.x1 - 235) * 20;
					ldw_rline[3] = (300 - output->rightcolorline.line.y1) * 20;
					adas_lline_color[0] = output->leftcolorline.R;
					adas_lline_color[1] = output->leftcolorline.G;
					adas_lline_color[2] = output->leftcolorline.B;
					adas_lline_color[3] = 1;

					adas_rline_color[0] = output->rightcolorline.R;
					adas_rline_color[1] = output->rightcolorline.G;
					adas_rline_color[2] = output->rightcolorline.B;
					adas_rline_color[3] = 1;
					ldw_detected = true; //意思是检测到线就为true 就可以画线 

			}
			else
			{ 
				for(int i=0;i<4;i++)
				{
					ldw_rline[i] = 0;
					ldw_lline[i] = 0;
				}
				ldw_detected = false; 
				
			}
 			
		}
    }
    sf_xcb_ldw_release(&pLDW);
    pthread_exit(NULL);
}
/**************************************************************************************/
/**************************************************************************************/
void *Mod_process(void *arg) 
{
	
	bool cameraON[4] = {1,1,1,1 };
	const char *auth_file = SAVE_LICENSE_PATH;
	char calibOutPath[500], calibOutPath_default[500];

	
	sprintf(mapPath_default, "%s/Ldw2DMappingModel", g_pGivenModelDir);
	sprintf(calibOutPath, "%s/calibOut", g_pCreateModelDir);
	sprintf(calibOutPath_default, "%s/calibOut", g_pGivenModelDir);

	constexpr int rows  = 720;
	constexpr int cols  = 1280;
	int dist_horizontal = 300;
	int dist_vertical   = 300;
	float fAngle        = 0;
	int nOutputNum      = 0;
	sf_xcb_mod_car_gear_state_t emGearStatus = Drive_gear;

	pthread_mutex_lock(&mutexin);
	void *pMOD = sf_xcb_mod_init(dist_horizontal, dist_vertical, cols, rows,calibOutPath, calibOutPath_default,auth_file);
	pthread_mutex_unlock(&mutexin);

	sf_xcb_mod_output_t *mod_output = 0;
	xcb_avm_msg_t7_t &msg = g_port.m_tMsg_t7;
	
	while (1)
	{
		
		uint8_t u8Speed = msg.tState.u8Speed;
		uint8_t u8Switch1 = msg.adas_switch;
		xcb_avm_car_state_gear_t &emGear = msg.tState.emGear;//获取挡位信息
		fSpeed = (float)u8Speed;//获取速度信息
		
		{
			
			pthread_mutex_lock(&get_data2);
			memcpy(ADASIMG[0],NV21IMG[0], NV21SIZE);
			pthread_mutex_unlock(&get_data2);
				
			pthread_mutex_lock(&get_data3);
			memcpy(ADASIMG[1],NV21IMG[1], NV21SIZE);
			pthread_mutex_unlock(&get_data3);
			
			pthread_mutex_lock(&get_data1);
			memcpy(ADASIMG[2],NV21IMG[2], NV21SIZE);
			pthread_mutex_unlock(&get_data1);
			
			pthread_mutex_lock(&get_data4);
			memcpy(ADASIMG[3],NV21IMG[3], NV21SIZE);
			pthread_mutex_unlock(&get_data4);

			emGearStatus = Parking_gear;
			
			#if ADASLOG
				printf("[(speed<5 )]====================speed=%f MOD run\n", fSpeed);
			#endif

	        sf_xcb_mod_process(pMOD, ADASIMG[0], ADASIMG[1], ADASIMG[2], ADASIMG[3],fSpeed,Parking_gear,fAngle, &mod_output);
			if (mod_output)
			{
				if(mod_output[0].nObjNum > 0)
				{
					//printf("MOD CAR Back: %d objects are detected!\n", mod_output[0].nObjNum);
					// //MOD 后方报警
					mod_back_warning = true;
				}
				else
				{
					mod_back_warning = false;
				}

				if (mod_output[1].nObjNum > 0)
				{
					//printf("MOD CAR L: %d objects are detected!\n", mod_output[1].nObjNum);
					mod_left_warning = true;
				}
				else
				{
					mod_left_warning = false;
				}

				if (mod_output[2].nObjNum > 0)
				{
					//printf("MOD CAR F: %d objects are detected!\n", mod_output[2].nObjNum);
					mod_front_warning = true;
				}
				else
				{
					mod_front_warning = false;
				}

				if (mod_output[3].nObjNum > 0)
				{
					mod_right_warning = true;
					//printf("MOD CAR RIGHT: %d objects are detected!\n", mod_output[3].nObjNum);
					// //MOD 右边报警
				}
				else
				{
					mod_right_warning = false;
				}

				
			}
			
		}
		
	}

	
	sf_xcb_mod_release(&pMOD);

	pthread_exit(NULL);
}

/**************************************************************************************/
/**************************************************************************************/
void *Bsd_process(void *arg)
{
	constexpr int rows 	= 720;
	constexpr int cols 	= 1280;
	int dist_horizontal 	= 300;
	int dist_vertical 	= 300;
	float fActSpeed 	= 30.0f;
	const char *auth_file 	= SAVE_LICENSE_PATH;
	int idx = 0;

	char calibOutPath[500], calibOutPath_default[500];

	sprintf(calibOutPath, "%s/calibOut", g_pCreateModelDir);
	sprintf(calibOutPath_default, "%s/calibOut", g_pGivenModelDir);

	bool bbsdl  = false;
	bool bbsdr  = false;
	bool bbsdlr = false;

	pthread_mutex_lock(&mutexin);
	void *pBSD = sf_xcb_bsd_init(fActSpeed, dist_horizontal, dist_vertical, cols,rows, calibOutPath, calibOutPath_default,auth_file);
	pthread_mutex_unlock(&mutexin);

	sf_xcb_bsd_event_t *event = 0;
	xcb_avm_msg_t7_t &msg = g_port.m_tMsg_t7;

	while (1)
	{
		
		uint8_t u8SpeedVal  = msg.tState.u8Speed;
		uint8_t u8Switch = msg.adas_switch;
		float fCurSpeedBsd  = (float)u8SpeedVal;


		if((fCurSpeedBsd > 30) && (fCurSpeedBsd < 120))//if((fCurSpeed > 30) && (fCurSpeed < 120))
		{
			#if ADASLOG
				printf("[(fCurSpeedBsd > 30) && (fCurSpeedBsd < 120)]BSD run at speed%f ================\n", fCurSpeedBsd);
			#endif
			
			pthread_mutex_lock(&get_data2);
			memcpy(ADASIMG[0],NV21IMG[0], NV21SIZE);
			pthread_mutex_unlock(&get_data2);			

      			sf_xcb_bsd_process(pBSD, ADASIMG[0], fCurSpeedBsd, &event);
			if (event)
			{
				if (*event == sf_xcb_bsd_event_t::SF_XCB_BSD_Left_Warning)
				{
					//debug("|******* Warning from Left of car!**********|\n");
					
				}
				else if (*event == sf_xcb_bsd_event_t::SF_XCB_BSD_Right_Warning)
				{
					//debug("|******* Warning from Right of car!**********|\n");
					
				}
				else if (*event == sf_xcb_bsd_event_t::SF_XCB_BSD_Left_Right_Warning)
				{
					//debug("|******* Warning from both Sides of car!**********|\n");
					
				}
				else 
				{
				    //printf("[BSD]-------2340-------------u8Speed=%f \n", fCurSpeed);
					
				}
			}
		}
		
	}
	sf_xcb_bsd_release(&pBSD);

	pthread_exit(NULL);
}

/**************************************************************************************/
/**************************************************************************************/
void * fcw_process(void *arg)
{
	constexpr int rows = 720;
	constexpr int cols = 1280;
	int dist_horizontal = 300;
	int dist_vertical = 300;

	const char *auth_file = SAVE_LICENSE_PATH;
	sf_xcb_fcw_output_t *fcwoutput;
	char pCalibOutPath[500], pDefaultCalibOutPath[500];   

	int light_init_flag = 0;
	void *pLightavg = NULL;
	float fratio[4] = {1.0, 1.0, 1.0, 1.0};
	char param_path[200];
	char default_param_path[200];

	sprintf(pCalibOutPath, "%s/calibOut", g_pCreateModelDir);
	sprintf(pDefaultCalibOutPath, "%s/calibOut", g_pGivenModelDir);

	pthread_mutex_lock(&mutexin);
	void *pfcw = sf_xcb_fcw_init(dist_horizontal, dist_vertical,cols, rows, pCalibOutPath, pDefaultCalibOutPath,auth_file); //fcw init 
	pthread_mutex_unlock(&mutexin);

	xcb_avm_msg_t7_t &msg = g_port.m_tMsg_t7;

	while (1)
	{
		
		uint8_t u8SpeedVal  = msg.tState.u8Speed;
		uint8_t u8Switch = msg.adas_switch;
		float fCurSpeedFcw  = (float)u8SpeedVal;
		#if  ADASLOG
			printf("[fCurSpeed>30]===========FCW fCurSpeed=%f ===========\n", fCurSpeedFcw);
		#endif

		if(fCurSpeedFcw > 30)
		{	
			pthread_mutex_lock(&get_data1);
			memcpy(ADASIMG[2],NV21IMG[2], NV21SIZE);
			pthread_mutex_unlock(&get_data1);
			sf_xcb_fcw_process(pfcw, ADASIMG[2], fCurSpeedFcw); //传入前摄像头数据（unsigned char* 1280*720*3/2）和当前车速 m/s
			sf_xcb_fcw_get_detect_obj(pfcw, &fcwoutput);
			if (fcwoutput)
			{
				if(fcwoutput->nObjNum > 0)
				{
					for (int i = 0; i < fcwoutput->nObjNum; i++)
					{
						if (fcwoutput->obj[i].distance < 40 )//前方车辆运动速度低于本车速度且距离小于15m 
						{
							
							//printf("FCW warning is ============true\n");
							fcw_detected = true;
						}
						else
						{
							//printf("FCW warning is ============false\n");
							fcw_detected = false;
						}
					}
				}
				else
				{
					fcw_detected = false;
				}
			}
			else
			{
			   fcw_detected = false;
			}
		}

		
	}

	sf_xcb_fcw_release(&pfcw);

	pthread_exit(NULL);
}