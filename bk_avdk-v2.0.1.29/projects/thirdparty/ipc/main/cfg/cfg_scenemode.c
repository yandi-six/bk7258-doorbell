
#include <stdio.h>
#include <ctype.h>
#include "cfg_scenemode.h"
#include "sdk_log.h"
SDK_NET_SCENEMODSECFG runScenemodeCfg;



SDK_CFG_MAP scenemodeMap[] = {
    {"selected",            &(runScenemodeCfg.selected),        SDK_CFG_DATA_TYPE_S32, "0", "rw", 0, 1, NULL},
    {"actions", &(runScenemodeCfg.actions[0]), SDK_CFG_DATA_TYPE_ACTION, "0", "rw", 0, 84, NULL},
    {NULL,},

};
int ScenemodeClearSchedTimes(SDK_SCHEDTIME *schedtime){
            //printf("********** ScenemodeSchedTimeDefValue *********\n");
                SDK_SCHEDTIME *stime = NULL;
                SDK_SCHEDTIME *stimeweek = schedtime;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    stime = stimeweek;
                    for (j = 0; j < 12; j ++) {
			
		            stime->startHour = 0;
		            stime->startMin = 0;
		            stime->stopHour = 0;
		            stime->stopMin = 0;
			
                        stime ++;
                    }
		    stimeweek+=12;
                }

    return 0;
}
int ScenemodeClearActions(SDK_ACTION *action){
        //printf("********** ScenemodeLoadActionsDefValue *********\n");
      	SDK_ACTION *temp = action;
	int i = 0;
	for(i =0;i< MAX_ACTIONS;i++){
	   memset(temp,0,sizeof(SDK_ACTION));
	   temp++;
        }
        return 1;
}

int ScenemodeSchedTimeDefValue(SDK_SCHEDTIME *schedtime){
            //printf("********** ScenemodeSchedTimeDefValue *********\n");
                SDK_SCHEDTIME *stime = NULL;
                SDK_SCHEDTIME *stimeweek = schedtime;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    stime = stimeweek;
                    for (j = 0; j < 12; j ++) {
			if(j ==0){
				
		                stime->startHour = 0;
		                stime->startMin = 0;
		                stime->stopHour = 24;
		                stime->stopMin = 0;
			}else{
		                stime->startHour = 0;
		                stime->startMin = 0;
		                stime->stopHour = 0;
		                stime->stopMin = 0;
			}
                        stime ++;
                    }
		    stimeweek+=12;
                }

    return 0;
}
int ScenemodeDefValue(SDK_NET_SCENEMODE *scenemode){
      // printf("********** ScenemodeDefValue *********  %s\n",scenemode->scenemodeName);
       int index = 0;
      int i = 0;
      for(i = 0;i<MAX_EVENTS;i++){
          scenemode->events[i].enable = 0;
          scenemode->events[i].index = 0;
      }
#ifdef MODULE_SUPPORT_SCHEDULE
       scenemode->events[index].enable = 1;
       scenemode->events[index].index = index;
       snprintf(scenemode->events[index].eventName,MAX_STR_LEN_32,"%s","schedule");
       ScenemodeLoadActionsDefValue(&(scenemode->events[index].actions[0]));
       ScenemodeSchedTimeDefValue(&(scenemode->events[index].scheduleTime[0][0]));
       index++;
#endif
#ifdef MODULE_SUPPORT_MD
       scenemode->events[index].enable = 1;
       scenemode->events[index].index = index;
       snprintf(scenemode->events[index].eventName,MAX_STR_LEN_32,"%s","Motion Detect");
       ScenemodeLoadActionsDefValue(&(scenemode->events[index].actions[0]));
       ScenemodeSchedTimeDefValue(&(scenemode->events[index].scheduleTime[0][0]));
       index++;
#endif
#ifdef MODULE_SUPPORT_ALARM_IN
       scenemode->events[index].enable = 1;
       scenemode->events[index].index = index;
       snprintf(scenemode->events[index].eventName,MAX_STR_LEN_32,"%s","Alarm In");
       ScenemodeLoadActionsDefValue(&(scenemode->events[index].actions[0]));
       ScenemodeSchedTimeDefValue(&(scenemode->events[index].scheduleTime[0][0]));
       index++;
#endif
#ifdef MODULE_SUPPORT_PIR
       scenemode->events[index].enable = 1;
       scenemode->events[index].index = index;
       snprintf(scenemode->events[index].eventName,MAX_STR_LEN_32,"%s","Pir");
       ScenemodeLoadActionsDefValue(&(scenemode->events[index].actions[0]));
       ScenemodeSchedTimeDefValue(&(scenemode->events[index].scheduleTime[0][0]));
       index++;
#endif

    return 1;
}
int ScenemodeCfgLoadDefValue()
{
    //printf("********** ScenemodeCfgLoadDefValue *********\n");
    CfgLoadDefValue(scenemodeMap);
    //printf("********** CfgLoadDefValue *********\n");
    int i = 0;
    for(i = 0;i<MAX_SCENEMODES;i++){
		snprintf(runScenemodeCfg.scenemodes[i].scenemodeName,MAX_STR_LEN_32,"Scenemode %d",i);
		ScenemodeDefValue(&(runScenemodeCfg.scenemodes[i]));

    }
    


    return 0;
}

int ScenemodeLoadActionsDefValue(SDK_ACTION *action){
        //printf("********** ScenemodeLoadActionsDefValue *********\n");
      	SDK_ACTION *temp = action;
	int i = 0;
	for(i =0;i< MAX_ACTIONS;i++){
	   memset(temp,0,sizeof(SDK_ACTION));
	   temp++;
        }
	temp = action;
        int index = 0;
#ifdef MODULE_SUPPORT_RECORD
	temp->selected = 1;
        temp->index = index;
	snprintf(temp->lable,MAX_STR_LEN_32,"%s","record");
        index++;
        temp++;
#endif

#ifdef MODULE_SUPPORT_SNAP
	temp->selected = 1;
        temp->index = index;
	snprintf(temp->lable,MAX_STR_LEN_32,"%s","scan");
        index++;
        temp++;
#endif

#ifdef MODULE_SUPPORT_ALARM_OUT
	temp->selected = 1;
        temp->index = index;
	snprintf(temp->lable,MAX_STR_LEN_32,"%s","alarmout");
        index++;
        temp++;
#endif


        return 1;
}
int ScenemodeValueActiontoJson(cJSON *item,SDK_ACTION *actionin){
		// printf("********** ScenemodeValueActiontoJson *********\n");
                SDK_ACTION *action = actionin;
		int i;
                cJSON *array = cJSON_CreateArray();
		if(array){
			 for (i = 0; i < MAX_ACTIONS; i ++) {

				if(action->selected == 1){
					cJSON *item =  cJSON_CreateObject();
					if(item!= NULL){

					
					cJSON *indexitem = cJSON_CreateNumber(action->index);
					cJSON_AddItemToObject(item,"index",indexitem);

					cJSON *lableitem = cJSON_CreateString(action->lable);
					cJSON_AddItemToObject(item,"lable",lableitem);

					cJSON_AddItemToArray(array, item);
					}

				}
				action++;
			    
			 }
		  cJSON_AddItemToObject(item, "actions", array);
                }
    return 1;
}
int ScenemodeValuescheduletoJson(cJSON *item,SDK_SCHEDTIME *schedule){
       // printf("********** ScenemodeValuescheduletoJson *********\n");
        SDK_SCHEDTIME *stime = NULL;
        SDK_SCHEDTIME *stimeweek = schedule;

                int i, j;
                char tmp[16] = {0};
                cJSON *array = cJSON_CreateArray();
		if(array){
		        for (i = 0; i < 7; i ++) {
			    cJSON *weekarray = cJSON_CreateArray();
			    stime = stimeweek;
			    if(weekarray){
				    for (j = 0; j < 12; j ++) {
					if(stime->startHour>0 ||stime->startMin >0 ||  stime->stopHour>0 || stime->stopMin>0){
						cJSON *item =  cJSON_CreateObject();
						if(item!= NULL){
							
							snprintf(tmp, sizeof(tmp),"%02d:%02d", stime->startHour, stime->startMin);
							cJSON *startitem = cJSON_CreateString(tmp);
							cJSON_AddItemToObject(item,"start",startitem);

							snprintf(tmp,sizeof(tmp), "%02d:%02d",  stime->stopHour,stime->stopMin);
							cJSON *enditem = cJSON_CreateString(tmp);
							cJSON_AddItemToObject(item,"end",enditem);

							cJSON_AddItemToArray(weekarray, item);
						}
					}
				        stime ++;
				    }
				    cJSON_AddItemToArray(array, weekarray);
			    }
			    stimeweek+=12;
		        }
		        cJSON_AddItemToObject(item, "scheduleTime", array);
		}

     return 1;
}
int ScenemodeValueEventtoJson(cJSON *jscenemode,SDK_EVENT *event){
	       //printf("********** ScenemodeValueEventtoJson *********\n");
              int i;
              cJSON *array = cJSON_CreateArray();
		if(array){
			 for (i = 0; i < MAX_EVENTS; i ++) {

				if(event->enable == 1){
					cJSON *item =  cJSON_CreateObject();
					if(item!= NULL){
					
						cJSON *indexitem = cJSON_CreateNumber(event->index);
						cJSON_AddItemToObject(item,"index",indexitem);

						cJSON *lableitem = cJSON_CreateString(event->eventName);
						cJSON_AddItemToObject(item,"eventName",lableitem);
						ScenemodeValueActiontoJson(item,&(event->actions[0]));
						ScenemodeValuescheduletoJson(item,&(event->scheduleTime[0][0]));

						cJSON_AddItemToArray(array, item);
					}

				}
				event++;
			    
			 }
		  cJSON_AddItemToObject(jscenemode, "evens", array);
                }
   return 1;
}
int ScenemodeValuetoJson(cJSON *jscenemodes,SDK_NET_SCENEMODE *scenemode){
	//printf("********** ScenemodeValuetoJson *********\n");
        cJSON *jscenemode = cJSON_CreateObject();
        cJSON_AddItemToObject(jscenemode,"scenemodeName",cJSON_CreateString(scenemode->scenemodeName));
        ScenemodeValueEventtoJson(jscenemode,&scenemode->events[0]);
        
	cJSON_AddItemToArray(jscenemodes,jscenemode);
	return 1;
}
int ScenemodeCfgSave(){
    // printf("********** ScenemodeCfgSave *********\n");
    cJSON *root;
    cJSON *scenemodetop;
    char *out;
    int i = 0;
    root = cJSON_CreateObject();//创建项目

    scenemodetop = CfgAddCjson(root, "scenemode", scenemodeMap);
    if(scenemodetop!= NULL){
	    cJSON *scenemodes;
	    scenemodes = cJSON_CreateArray();
	    if(scenemodes){
		    for(i = 0;i<MAX_SCENEMODES;i++){
			ScenemodeValuetoJson(scenemodes,&(runScenemodeCfg.scenemodes[i]));
		    }
		    
		    cJSON_AddItemToObject(scenemodetop,"scenemodes",scenemodes);
	    }
    }


    out = cJSON_Print(root);

    int ret = CfgWriteToFile(SCENEMODE_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", SCENEMODE_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}
int ScenemodeCfgReadScheduleFromJson(cJSON * jscheduleTime,SDK_SCHEDTIME *scheduleTime){
  
		SDK_SCHEDTIME *stime = NULL;
                SDK_SCHEDTIME *stimeweek = scheduleTime;
                cJSON *scheditem = NULL;
                cJSON *weektitem = NULL;
                int i, j, res;
                int startHour, startMin, stopHour, stopMin;
                for (i = 0; i < 7; i ++) {
                        stime = stimeweek;
                        weektitem = cJSON_GetArrayItem(jscheduleTime, i);
                        if (weektitem) {
			     for (j = 0; j < 12; j ++) {
                           
				    scheditem = cJSON_GetArrayItem(weektitem, j);
				    if(scheditem){
					  cJSON *startitem =  cJSON_GetObjectItem(scheditem,"start");
					  cJSON *enditem =  cJSON_GetObjectItem(scheditem,"end");
					  if(startitem != NULL && enditem!= NULL){
							res = sscanf(startitem->valuestring, "%02d:%02d", &startHour, &startMin);
							if(res == 2){
							    stime->startHour = startHour;
							    stime->startMin = startMin;
							}
							res = sscanf(enditem->valuestring, "%02d:%02d", &stopHour, &stopMin);
							if(res == 2){
							    stime->stopHour = stopHour;
							    stime->stopMin = stopMin;						
							}
							
					  }
				    }
				    stime++;
			    }
			    
                            
                        }
			stimeweek +=12;
                    
                }

	return 1;
}
int ScenemodeCfgReadActionsFromJson(cJSON * jactions,SDK_ACTION *action){
	int i;
	if(cJSON_IsArray(jactions)){
		int size = cJSON_GetArraySize(jactions);
		cJSON *actionitem = NULL;
		for (i = 0; i<size && i < MAX_ACTIONS; i ++) {
			actionitem = cJSON_GetArrayItem(jactions, i);
			if(actionitem!= NULL){
				cJSON *indexitem =  cJSON_GetObjectItem(actionitem,"index");
			        cJSON *lableitem =  cJSON_GetObjectItem(actionitem,"lable");
				if(indexitem!= NULL && lableitem!= NULL){
					action->selected = 1;
					action->index  = cJSON_GetNumberValue(indexitem);
					snprintf(action->lable,MAX_STR_LEN_32,"%s",cJSON_GetStringValue(lableitem));

				        action++;

				}

			}
		}
	 }

	return 1;
}
int ScenemodeCfgReadEventFromJson(cJSON * jevent,SDK_EVENT *event){
        event->enable = 1;
	cJSON *jseventName = cJSON_GetObjectItem(jevent,"eventName");
	if(jseventName!= NULL){
	    snprintf(event->eventName,MAX_STR_LEN_32,"%s",cJSON_GetStringValue(jseventName));
	}
        cJSON *jsevenindex = cJSON_GetObjectItem(jevent,"index");
	if(jsevenindex!= NULL){
		event->index = cJSON_GetNumberValue(jsevenindex);
	}
	cJSON *jactions = cJSON_GetObjectItem(jevent,"actions");
	if(jactions!= NULL){
		SDK_ACTION *action = &(event->actions[0]); 
		ScenemodeCfgReadActionsFromJson(jactions,action);

	}
	cJSON *jscheduleTime = cJSON_GetObjectItem(jevent,"scheduleTime");
	if(jscheduleTime!= NULL){
		SDK_SCHEDTIME *scheduleTime = &(event->scheduleTime[0][0]); 
		ScenemodeCfgReadScheduleFromJson(jscheduleTime,scheduleTime);

	}
	return 1;
}
int ScenemodeCfgReadScenemodeFromJson(cJSON * jscenemode,SDK_NET_SCENEMODE *scenemode){
	int i = 0;
	cJSON *jscenemodeName = cJSON_GetObjectItem(jscenemode,"scenemodeName");
	if(jscenemodeName!= NULL){
	    snprintf(scenemode->scenemodeName,MAX_STR_LEN_32,"%s",cJSON_GetStringValue(jscenemodeName));
	}
	cJSON * jevens = cJSON_GetObjectItem(jscenemode,"evens");
	if(jevens!= NULL){
		if(cJSON_IsArray(jevens)){
			for(i = 0;i<MAX_EVENTS;i++){
				scenemode->events[i].enable = 0;
                                scenemode->events[i].index = 0;
                                snprintf(scenemode->events[i].eventName,MAX_STR_LEN_32,"%s","");
				ScenemodeClearSchedTimes(&(scenemode->events[i].scheduleTime[0][0]));
				ScenemodeClearActions(&(scenemode->events[i].actions[0]));
			}
			int size = cJSON_GetArraySize(jevens);
			for(i = 0;i<size && i<MAX_EVENTS;i++){
				cJSON *jevent = cJSON_GetArrayItem(jevens,i);
				SDK_EVENT *event = &(scenemode->events[i]);
				ScenemodeCfgReadEventFromJson(jevent,event);
			}
		}

	}
	
	return 1;
}
int ScenemodeCfgLoadScenemodesFromJson(cJSON * jscenemodes){
	int i = 0;
	if(cJSON_IsArray(jscenemodes)){
		int size = cJSON_GetArraySize(jscenemodes);
		for(i = 0;i<size && i<MAX_SCENEMODES;i++){
			cJSON *jscenemode = cJSON_GetArrayItem(jscenemodes,i);
			if(jscenemode){
				SDK_NET_SCENEMODE *scenemode = &(runScenemodeCfg.scenemodes[i]);
				ScenemodeCfgReadScenemodeFromJson(jscenemode,scenemode);

			}

		}
	}

	return 1;
}
int ScenemodeCfgLoad(){

    char *data = NULL;
    data = CfgReadFromFile(SCENEMODE_FILE);
    if (data == NULL) {
        webcam_error("load %s error, so to load default cfg param.\n", SCENEMODE_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //从配置文件解析cjson失败，则使用默认参数
        webcam_error("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }

    cJSON * jscenemode  = CfgParseCjson(json, "scenemode", scenemodeMap);
    if(jscenemode!= NULL){
        cJSON * jscenemodes = cJSON_GetObjectItem(jscenemode,"scenemodes");
	if(jscenemodes!= NULL){
		ScenemodeCfgLoadScenemodesFromJson(jscenemodes);
	}
    }
    cJSON_Delete(json);
    free(data);
    return 0;

err:
    ScenemodeCfgLoadDefValue();
    ScenemodeCfgSave();
    return 0;


}

cJSON * ScenemodeCfgLoadJson()
{
    char *data = NULL;
    data = CfgReadFromFile(SCENEMODE_FILE);
    if (data == NULL) {
        webcam_error("load %s error, so to load default cfg param.\n", SCENEMODE_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        webcam_error("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }
    cJSON * jscenemode  = CfgParseCjson(json, "scenemode", scenemodeMap);
    if(jscenemode!= NULL){
        cJSON * jscenemodes = cJSON_GetObjectItem(jscenemode,"scenemodes");
	if(jscenemodes!= NULL){
		ScenemodeCfgLoadScenemodesFromJson(jscenemodes);
	}
    }  
    free(data);
    return json;

err:
    if(data){
        free(data);
    }
    return NULL;
}
int ScenemodeJsonSaveCfg(cJSON *json)
{
    char *out;
    out = cJSON_Print(json);

    int ret = CfgWriteToFile(SCENEMODE_FILE, out);
    if (ret != 0) {
        webcam_error("CfgWriteToFile %s error.", SCENEMODE_FILE);
        return -1;
    }

    free(out);
    ScenemodeCfgLoad();

    return 0;
}
