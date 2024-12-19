/*!
*****************************************************************************

*****************************************************************************
*/

#include "cfg_common.h"
#include "cfg_all.h"
#if CONFIG_VFS
#include "bk_vfs.h"
#include "bk_filesystem.h"
#else
#include "vfs_file_minor.h"
#endif
beken_mutex_t g_cfg_write_mutex = NULL;

int CfgWriteDelete(const char *filename){
    char path[100] = {0};
    os_memset(path, 0, sizeof(path));
    sprintf(path, "%s%s", CFG_DIR, filename);
     int ret = -1;
#if CONFIG_VFS
    ret = bk_vfs_unlink(path);
#else
    ret = vfs_file_unlink(path);
#endif
    return ret;

}

//负责将字符串数据写入到指定的配置文件中
int CfgWriteToFile(const char *filename, const char *data)
{
    
    rtos_lock_mutex(&g_cfg_write_mutex);
    char path[100] = {0};

    os_memset(path, 0, sizeof(path));
    sprintf(path, "%s%s", CFG_DIR, filename);
    os_printf("to save %s\n", path);

    int fd = -1;
    int ret = -1;

#if CONFIG_VFS
    fd = bk_vfs_open(path, O_RDWR | O_CREAT );
#else
    fd = vfs_file_open(path, O_RDWR | O_CREAT);
#endif
    if (fd < 0) {
		os_printf("can't open %s\n", path);
		rtos_unlock_mutex(&g_cfg_write_mutex);
		return -1;
    }
    //os_printf("to write %s\n", path);
#if CONFIG_VFS
    ret = bk_vfs_write(fd, data, strlen(data));
#else
    ret = vfs_file_write(fd, data, strlen(data));
#endif
    if(ret<0){
    	os_printf("write to %s, ret=%d\n", filename, ret);
    }else{
	ret = 0;
    }
    
#if CONFIG_VFS
    bk_vfs_close(fd);
#else
    vfs_file_close(fd);
#endif
    rtos_unlock_mutex(&g_cfg_write_mutex);
    return ret;	

}


//更适合写入二进制数据或固定长度的非字符串数据
int CfgWriteToFile2(const char *filename, const char *data,int length)
{
    os_printf("CfgWriteToFile2 %s\n", filename);
    rtos_lock_mutex(&g_cfg_write_mutex);
    char path[100] = {0};

    os_memset(path, 0, sizeof(path));
    sprintf(path, "%s%s", CFG_DIR, filename);
    //os_printf("to save %s\n", path);

    int fd;
    int ret;
#if CONFIG_VFS
    fd = bk_vfs_open(path, O_RDWR | O_CREAT );
#else
     fd = vfs_file_open(path, O_RDWR | O_CREAT );
#endif
    if (fd < 0) {
		os_printf("can't open %s\n", path);
		rtos_unlock_mutex(&g_cfg_write_mutex);
		return -1;
    }
#if CONFIG_VFS
    ret = bk_vfs_write(fd, data, length);
#else
    ret = vfs_file_write(fd, data, length);
#endif
    if(ret<0){
    	os_printf("write to %s, ret=%d\n", filename, ret);
    }else{
	ret = 0;
    }
#if CONFIG_VFS
    bk_vfs_close(fd);
#else
    vfs_file_close(fd);
#endif
    rtos_unlock_mutex(&g_cfg_write_mutex);
    return ret;	

}

//从文件系统中读取指定配置文件的内容
char *CfgReadFromFile(const char *filename)
{
    os_printf("CfgReadFromFile %s\n", filename);
    char path[100] = {0};

    //Ŀ¼�����ڣ��򴴽�
/*
    if(access(CFG_DIR, F_OK) != 0 ) {
        os_printf("to create cfg dir: %s\n", CFG_DIR);
        if((mkdir(CFG_DIR, 0777)) < 0)
        {
            os_printf("mkdir %s failed\n", CFG_DIR);
            return NULL;
        }
    }
*/
    char *data = NULL;
    os_memset(path, 0, sizeof(path));
    sprintf(path, "%s%s", CFG_DIR, filename);

    int fd;
    int ret;
    struct stat st;
#if CONFIG_VFS
    ret = bk_vfs_stat(path, &st);
#else
   ret = vfs_file_stat(path, &st);
#endif
    if (ret != 0) {
        os_printf("stat error %s  ret %d\n",path,ret);
        return NULL;
    }
    int fileSize = st.st_size;
    if(fileSize<=0){
        os_printf("fileSize error %d\n",fileSize);
        return NULL;
    }

    data = os_malloc(fileSize);
    if(!data) {
        return data;
    }
#if CONFIG_VFS
    fd = bk_vfs_open(path, O_RDONLY);
#else
    fd = vfs_file_open(path, O_RDONLY);
#endif
    if (fd < 0) {
		os_printf("can't open %s\n", path);
		if(data!= NULL){
			os_free(data);
			data = NULL;
		}
		return data;
    }
#if CONFIG_VFS
    int len = bk_vfs_read(fd, data, fileSize);
#else
    int len = vfs_file_read(fd, data, fileSize);
#endif
    if(len<0){
		os_printf("read failed %s\n", path);
		if(data!= NULL){
			os_free(data);
			data = NULL;
		}
		return data;
    }
     //os_printf("read from %s, ret=%d\n", path, ret);
#if CONFIG_VFS
    bk_vfs_close(fd);
#else
    vfs_file_close(fd);
#endif
    return data;

    //os_printf("to load %s\n", path);

}

//根据配置映射表中的数据类型将默认值加载到相应的内存位置
static int CfgLoadDefValueItem(SDK_CFG_MAP *map)
{
    int tmp;
    double temp;
    switch (map->dataType) {
        case SDK_CFG_DATA_TYPE_U32:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned int *)(map->dataAddress)) = (unsigned int)tmp;
            break;
        case SDK_CFG_DATA_TYPE_U16:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned short *)(map->dataAddress)) = (unsigned short)tmp;
            break;
        case SDK_CFG_DATA_TYPE_U8:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned char *)(map->dataAddress)) = (unsigned char)tmp;
            break;
        case SDK_CFG_DATA_TYPE_S32:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((int *)(map->dataAddress)) = tmp;
            break;
        //  case SDK_CFG_DATA_TYPE_S64://++++
        //     tmp = strtol(map->defaultValue, NULL, 0);
        //     *((int64_t *)(map->dataAddress)) = (int64_t)tmp;
        //     break;
        //  case SDK_CFG_DATA_TYPE_TIMET://++++
        //     tmp = strtoul(map->defaultValue, NULL, 0);
        //     *((uint32_t *)(map->dataAddress)) = (uint32_t)tmp;
        //     break;
        case SDK_CFG_DATA_TYPE_S16:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((signed short *)(map->dataAddress)) = (signed short)tmp;
            break;
        case SDK_CFG_DATA_TYPE_S8:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((signed char *)(map->dataAddress)) = (signed char)tmp;
            break;
		case SDK_CFG_DATA_TYPE_FLOAT:
			temp = strtod(map->defaultValue, NULL);
            *(( float *)(map->dataAddress)) = (float)temp;
            break;

        case SDK_CFG_DATA_TYPE_STRING:
	  if (map->dataAddress && (map->max > 1)) {
		tmp = (int)map->max - 1;
                strncpy((char *)map->dataAddress, map->defaultValue, tmp);
                ((char *)map->dataAddress)[tmp] = '\0';
            } else {
                os_printf("if type is string, addr can't be null and the upper limit must greater than 1.\n");
            }
            break;
        case SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING:
	  if (map->dataAddress && (map->max > 1)) {
		tmp = (int)map->max - 1;
                strncpy((char *)map->dataAddress, map->defaultValue, tmp);
                ((char *)map->dataAddress)[tmp] = '\0';
            } else {
                os_printf("if type is string, addr can't be null and the upper limit must greater than 1.\n");
            }
            break;
        case SDK_CFG_DATA_TYPE_JSON_ARRAY_STRING:
	  if (map->dataAddress && (map->max > 1)) {
		tmp = (int)map->max - 1;
                strncpy((char *)map->dataAddress, map->defaultValue, tmp);
                ((char *)map->dataAddress)[tmp] = '\0';
            } else {
                os_printf("if type is string, addr can't be null and the upper limit must greater than 1.\n");
            }
            break;
        case SDK_CFG_DATA_TYPE_ACTION:
	    {
		SDK_ACTION *action = (SDK_ACTION *)map->dataAddress;
		if(strcmp(map->stringName,"alarmIn.actions") == 0){
			//AlarmInLoadActionsDefValue(action);
		}else if(strcmp(map->stringName,"md.actions") == 0){
			//MdLoadActionsDefValue(action);
		}else if(strcmp(map->stringName,"schedule.actions") == 0){
			//ScheduleLoadActionsDefValue(action);
		}else if(strcmp(map->stringName,"pir.actions") == 0){
			//ScheduleLoadActionsDefValue(action);
		}else if(strcmp(map->stringName,"actions") == 0){
			//ScenemodeLoadActionsDefValue(action);
		}
	    }
            break;
        case SDK_CFG_DATA_TYPE_STIME:
            {
                
                SDK_SCHEDTIME *stime = NULL;
                SDK_SCHEDTIME *stimeweek = (SDK_SCHEDTIME *)map->dataAddress;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    stime = stimeweek;
                    for (j = 0; j < 12; j ++) {
			//webcam_warning("CfgLoadDefValueItem %d  %d",i,j);
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
            }
            break;
        case SDK_CFG_DATA_TYPE_SLICE:
            {
                unsigned int *slice = (unsigned int *)map->dataAddress;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 3; j ++) {
                        *slice = strtoul("4294967295", NULL, 0);
                        slice ++;
                    }
                }
            }
            break;
        default:
            os_printf("unknown data type in map definition!\n");
            break;
    }

    return 0;
}

//加载配置项的默认值
int CfgLoadDefValue(SDK_CFG_MAP *mapArray)
{
    //os_printf("CfgLoadDefValue\n");
    SDK_CFG_MAP *map = NULL;
    int i =0;
    while (mapArray[i].stringName != NULL) {
	//os_printf("CfgLoadDefValue  %s\n",mapArray[i].stringName);
        map = &mapArray[i];
        CfgLoadDefValueItem(map);
        i++;
    }
    //os_printf("CfgLoadDefValue end\n");
    return 0;
}

//将配置数据转换成 JSON 格式
static int CfgDataToCjsonByMapItem(SDK_CFG_MAP *map, cJSON *root)
{
    int tmp;
	double temp;

    switch (map->dataType) {
        case SDK_CFG_DATA_TYPE_U32:
            tmp = *((unsigned int *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
        case SDK_CFG_DATA_TYPE_U16:
            tmp = *((unsigned short *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
        case SDK_CFG_DATA_TYPE_U8:
            tmp = *((unsigned char *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
        case SDK_CFG_DATA_TYPE_S32:
            tmp = *((signed int *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
        case SDK_CFG_DATA_TYPE_S16:
            tmp = *((signed short *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
        case SDK_CFG_DATA_TYPE_S8:
            tmp = *((signed char *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
		case SDK_CFG_DATA_TYPE_FLOAT:
            temp = *(( float *)(map->dataAddress));
            if (temp > map->max || temp < map->min)
                temp = strtod(map->defaultValue, NULL);
            cJSON_AddNumberToObject(root, map->stringName, temp);
            break;

        case SDK_CFG_DATA_TYPE_STRING:
            cJSON_AddStringToObject(root, map->stringName, (char *)map->dataAddress);//
            break;
        case SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING:
	    {
		    cJSON *item =  cJSON_CreateObject();
		    item = cJSON_Parse((char *)map->dataAddress);
		    if(item!=NULL){
			 cJSON_AddItemToObject(root, map->stringName, item);
		    }
	    }
	    break;
        case SDK_CFG_DATA_TYPE_JSON_ARRAY_STRING:
	    {
		    cJSON *item =  cJSON_CreateObject();
		    item = cJSON_Parse((char *)map->dataAddress);
		    if(item!=NULL){
			 cJSON_AddItemToObject(root, map->stringName, item);
		    }
	    }
	    break;
        case SDK_CFG_DATA_TYPE_ACTION:
            {
		SDK_ACTION *action = (SDK_ACTION *)map->dataAddress;
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
		  cJSON_AddItemToObject(root, map->stringName, array);
                }
	    }
	    break;
        case SDK_CFG_DATA_TYPE_STIME:
            {
                SDK_SCHEDTIME *stime = NULL;
                SDK_SCHEDTIME *stimeweek = (SDK_SCHEDTIME *)map->dataAddress;

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
							//printf("********** SDK_SCHEDTIME *********  %d  %d\n",i,j);
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
		        cJSON_AddItemToObject(root, map->stringName, array);
		}
            }
            break;

        case SDK_CFG_DATA_TYPE_SLICE:
            {
                unsigned int *slice = (unsigned int *)map->dataAddress;

                int i, j;
                char tmp[20] = {0};
                cJSON *array = cJSON_CreateArray();
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 3; j ++) {
                        sprintf(tmp, "%u", *slice);
                        cJSON_AddItemToArray(array, cJSON_CreateString(tmp));
                        slice ++;
                    }
                }
                cJSON_AddItemToObject(root, map->stringName, array);
            }
            break;
        default:
            os_printf("unknown data type in map definition!\n");
            break;
    }

    return 0;
}


//将配置数据转换为 JSON 格式
cJSON *CfgDataToCjsonByMap(SDK_CFG_MAP *mapArray)
{
    cJSON *root;

    root = cJSON_CreateObject();//������Ŀ
    int i =0;

    while(mapArray[i].stringName != NULL) {
        CfgDataToCjsonByMapItem(&mapArray[i], root);
        i ++;
    }

    return root;
}

//将 JSON 数据反序列化回应用程序的内存结构中
static int CfgCjsonToDataByMapItem(SDK_CFG_MAP *map, cJSON *json)
{
    
    int tmp;
    double temp;

    switch (map->dataType) {
        case SDK_CFG_DATA_TYPE_U32:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned int *)(map->dataAddress)) = (unsigned int)tmp;
            break;
        case SDK_CFG_DATA_TYPE_U16:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned short *)(map->dataAddress)) = (unsigned short)tmp;
            break;
        case SDK_CFG_DATA_TYPE_U8:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned char *)(map->dataAddress)) = (unsigned char)tmp;
            break;
        case SDK_CFG_DATA_TYPE_S32:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            *((int *)(map->dataAddress)) = tmp;
            break;
        case SDK_CFG_DATA_TYPE_S16:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            *((signed short *)(map->dataAddress)) = (signed short)tmp;
            break;
        case SDK_CFG_DATA_TYPE_S8:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            *((signed char *)(map->dataAddress)) = (signed char)tmp;
            break;
		case SDK_CFG_DATA_TYPE_FLOAT:
            temp = json->valuedouble;
            if (temp > map->max || temp < map->min)
                temp = strtod(map->defaultValue, NULL);
            *(( float *)(map->dataAddress)) = ( float)temp;
            break;

        case SDK_CFG_DATA_TYPE_STRING:
	   if (map->dataAddress && (map->max > 1)) {
				tmp = (int)map->max - 1;
                strncpy((char *)map->dataAddress, json->valuestring, tmp);
                ((char *)map->dataAddress)[tmp] = '\0';
            } else {
                os_printf("if type is string, addr can't be null and the upper limit must greater than 1.\n");
            }
            break;
        case SDK_CFG_DATA_TYPE_JSON_OBJTCT_STRING:
	   {
		   if (map->dataAddress && (map->max > 1)) {
			tmp = (int)map->max - 1;
			char * buf = cJSON_Print(json);
			if(buf){
				strncpy((char *)map->dataAddress, buf, tmp);
				((char *)map->dataAddress)[tmp] = '\0';
				free(buf);
				buf = NULL;
			}
		    } else {
		        os_printf("if type is string, addr can't be null and the upper limit must greater than 1.\n");
		    }
	    }
            break;
        case SDK_CFG_DATA_TYPE_ACTION:
	    {
		SDK_ACTION *action = (SDK_ACTION *)map->dataAddress;
		os_memset(action,0,MAX_ACTIONS*sizeof(SDK_ACTION));
		int i;
		cJSON *actionitem = NULL;
		for (i = 0; i < MAX_ACTIONS; i ++) {
			actionitem = cJSON_GetArrayItem(json, i);
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
	    break;
        case SDK_CFG_DATA_TYPE_STIME:
            {
                SDK_SCHEDTIME *stime = NULL;
                SDK_SCHEDTIME *stimeweek = (SDK_SCHEDTIME *)map->dataAddress;
                cJSON *scheditem = NULL;
                cJSON *weektitem = NULL;
                int i, j, res;
                int startHour, startMin, stopHour, stopMin;
                for (i = 0; i < 7; i ++) {
                        stime = stimeweek;
                        weektitem = cJSON_GetArrayItem(json, i);
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
            }
            break;

        case SDK_CFG_DATA_TYPE_SLICE:
            {
                unsigned int *slice = (unsigned int *)map->dataAddress;
                cJSON *tmp = NULL;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 3; j ++) {
                        tmp = cJSON_GetArrayItem(json, i * 3 + j);
                        if (tmp) {
                            *slice = strtoul(tmp->valuestring, NULL, 0);
                            slice ++;
                        }
                    }
                }
            }
            break;
        default:
            os_printf("unknown data type in map definition!\n");
            break;
    }

    return 0;
}

//从一个 cJSON 对象中读取配置数据，并将这些数据写回到应用程序的内部数据结构中
int CfgCjsonToDataByMap(SDK_CFG_MAP *mapArray, cJSON *json)
{
    int i =0;
    cJSON *tmp = NULL;

    while (mapArray[i].stringName != NULL) {
        tmp = cJSON_GetObjectItem(json, mapArray[i].stringName);
        if (tmp) {
	   // webcam_warning("CfgCjsonToDataByMap [%s] cjson get", mapArray[i].stringName);
            CfgCjsonToDataByMapItem(&mapArray[i], tmp);
        } else {
            //�������ļ�����cjsonʧ�ܣ���ʹ��Ĭ�ϲ���
            //webcam_warning("[%s] cjson get failed", mapArray[i].stringName);
            CfgLoadDefValueItem(&mapArray[i]);
        }
        i++;
    }

    return 0;
}


//将配置数据以格式化的形式输出
static void CfgPrintItem(SDK_CFG_MAP *map)
{
    switch (map->dataType) {
        case SDK_CFG_DATA_TYPE_U32:
            os_printf("%s : %u \n", map->stringName, *((unsigned int *)(map->dataAddress)));
            break;
        case SDK_CFG_DATA_TYPE_U16:
            os_printf("%s : %u \n", map->stringName, *((unsigned short *)(map->dataAddress)));
            break;
        case SDK_CFG_DATA_TYPE_U8:
            os_printf("%s : %u \n", map->stringName, *((unsigned char *)(map->dataAddress)));
            break;
        case SDK_CFG_DATA_TYPE_S32:
            os_printf("%s : %d \n", map->stringName, *((signed int *)(map->dataAddress)));
            break;
        case SDK_CFG_DATA_TYPE_S16:
            os_printf("%s : %d \n", map->stringName, *((signed short *)(map->dataAddress)));
            break;
        case SDK_CFG_DATA_TYPE_S8:
            os_printf("%s : %d \n", map->stringName, *((signed char *)(map->dataAddress)));
            break;
		case SDK_CFG_DATA_TYPE_FLOAT:
			os_printf("%s : %f \n", map->stringName, *(( float *)(map->dataAddress)));
            break;

        case SDK_CFG_DATA_TYPE_STRING:
            os_printf("%s : %s \n", map->stringName, (char *)(map->dataAddress));
            break;

        case SDK_CFG_DATA_TYPE_STIME:
            {
                os_printf("%s : [\n", map->stringName);
                SDK_SCHEDTIME *stime = (SDK_SCHEDTIME *)map->dataAddress;

                int i, j;
                for (i = 0; i < 7; i ++) {
                   // printf("    date %d: ", i);
		    os_printf("[\n");
                    for (j = 0; j < 12; j ++) {
                        os_printf("%02d:%02d - %02d:%02d, ", stime->startHour, \
                                                  stime->startMin, \
                                                  stime->stopHour, \
                                                  stime->stopMin);
                        stime ++;
                    }
                    os_printf("\n]\n");
                }
                os_printf("\n]\n");
            }
            break;

        case SDK_CFG_DATA_TYPE_SLICE:
            {
                os_printf("%s : [\n", map->stringName);
                unsigned int *slice = (unsigned int *)map->dataAddress;

                int i, j;
                for (i = 0; i < 7; i ++) {
                    os_printf("    date %d: ", i);
                    for (j = 0; j < 3; j ++) {
                        os_printf("%u, ", (*slice));
                        slice ++;
                    }
                    os_printf("\n");
                }
                os_printf("]\n");
            }
            break;
        default:
            os_printf("unknown data type in map definition!\n");
            break;
    }
}

//遍历打印配置数据
int CfgPrintMap(SDK_CFG_MAP *mapArray)
{
    int i =0;
    SDK_CFG_MAP *map = NULL;
    while (mapArray[i].stringName != NULL) {
        map = &mapArray[i];
        CfgPrintItem(map);
        i++;
    }

    return 0;
}

//从一个 cJSON 对象中解析特定的子对象
cJSON* CfgParseCjson(cJSON *root, const char *str, SDK_CFG_MAP *mapArray)
{
    cJSON *item = NULL;
    item = cJSON_GetObjectItem(root, str);
    if (!item) {
        //os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        return NULL;
    }
    CfgCjsonToDataByMap(mapArray, item);
    return item;
}

//从文件中读取配置数据，并将这些数据解析为 JSON 格式，然后反序列化到应用程序的内部数据结构
int CfgLoad(const char *filename, const char *str, SDK_CFG_MAP *mapArray)
{
    char *data = NULL;
    data = CfgReadFromFile(filename);
    if (data == NULL) {
        //�������ļ���ȡʧ�ܣ���ʹ��Ĭ�ϲ���
        os_printf("load %s error, so to load default cfg param\n", filename);
        goto err2;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json) {
        //�������ļ�����cjsonʧ�ܣ���ʹ��Ĭ�ϲ���
        os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err2;
    }

    CfgParseCjson(json, str, mapArray);

    cJSON_Delete(json);
    free(data);
    return 0;

err2:
    //webcam_warning("CfgLoadDefValue  %s",filename);
    CfgLoadDefValue(mapArray);
    CfgSave(filename,str, mapArray);
    return 0;
}


//将配置数据转换为 JSON 格式，并将其作为子对象添加到一个已存在的 cJSON 对象中
cJSON* CfgAddCjson(cJSON *root, const char *str, SDK_CFG_MAP *mapArray)
{
    cJSON *item = NULL;
    item = CfgDataToCjsonByMap(mapArray);
    cJSON_AddItemToObject(root, str, item);

    return item;
}

//保存配置信息到一个文件中
int CfgSave(const char *filename, const char *str, SDK_CFG_MAP *mapArray)
{
    int ret = 0;
    char *out;
    cJSON *root;

    root = cJSON_CreateObject();//������Ŀ

    CfgAddCjson(root, str, mapArray);

    out = cJSON_Print(root);
    ret = CfgWriteToFile(filename, out);
    if (ret != 0) {
        os_printf("CfgSave %s error.\n", filename);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

//从配置映射（SDK_CFG_MAP 类型的数组）中查找指定名称的配置项，并将其默认值复制到由 value 参数指定的内存位置
int CfgGetDefByName(SDK_CFG_MAP *mapArray, const char *item_name, void *value)
{
    if ((value == NULL) || (item_name == NULL) || (mapArray == NULL)) {
        os_printf("param error!\n");
        return -1;
    }

    SDK_CFG_MAP *map = NULL;
    int i =0;
    while (mapArray[i].stringName != NULL) {
        map = &mapArray[i];
        if (strcmp(map->stringName, item_name) != 0)
            break;
        i++;
    }
    if (mapArray[i].stringName == NULL)
        return -1;

    int tmp;
	double temp;
    switch (map->dataType) {
        case SDK_CFG_DATA_TYPE_U32:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned int *)value) = (unsigned int)tmp;
            break;
        case SDK_CFG_DATA_TYPE_U16:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned short *)value) = (unsigned short)tmp;
            break;
        case SDK_CFG_DATA_TYPE_U8:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned char *)value) = (unsigned char)tmp;
            break;
        case SDK_CFG_DATA_TYPE_S32:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((int *)value) = tmp;
            break;
        case SDK_CFG_DATA_TYPE_S16:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((signed short *)value) = (signed short)tmp;
            break;
        case SDK_CFG_DATA_TYPE_S8:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((signed char *)value) = (signed char)tmp;
            break;

		case SDK_CFG_DATA_TYPE_FLOAT:
            temp = strtod(map->defaultValue, NULL);
            *(( float *)value) = ( float)temp;
            break;

        case SDK_CFG_DATA_TYPE_STRING:
			if (map->max > 1) {
				tmp = (int)map->max - 1;
                strncpy((char *)value, map->defaultValue, tmp);
                ((char *)value)[tmp] = '\0';
            } else {
                os_printf("if type is string, addr can't be null and the upper limit must greater than 1.\n");
            }
            break;
	case SDK_CFG_DATA_TYPE_ACTION:
	   {
		SDK_ACTION *action = (SDK_ACTION *)value;
		if(strcmp(item_name,"alarmIn.actions") == 0){
			//AlarmInLoadActionsDefValue(action);
		}else if(strcmp(map->stringName,"md.actions") == 0){
			//MdLoadActionsDefValue(action);
		}else if(strcmp(map->stringName,"schedule.actions") == 0){
			//ScheduleLoadActionsDefValue(action);
		}else if(strcmp(map->stringName,"pir.actions") == 0){
			//ScheduleLoadActionsDefValue(action);
		}else if(strcmp(map->stringName,"actions") == 0){
			//ScenemodeLoadActionsDefValue(action);
		}
	   }
	   break;
        case SDK_CFG_DATA_TYPE_STIME:
            {
                SDK_SCHEDTIME *stime = (SDK_SCHEDTIME *)value;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 12; j ++) {
			if(j == 0){
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
                }
            }
            break;
        case SDK_CFG_DATA_TYPE_SLICE:
            {
                unsigned int *slice = (unsigned int *)value;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 3; j ++) {
                        *slice = strtoul("4294967295", NULL, 0);
                        slice ++;
                    }
                }
            }
            break;
        default:
            os_printf("unknown data type in map definition!\n");
            break;
    }

    return 0;
}



int is_in_schedule_slice(SDK_U32 *slice)
{
/*
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = localtime_r(&ts, &tt);

    unsigned int tmp;
    if (pTm->tm_hour < 8) {
        //tmp = runMdCfg.scheduleSlice[pTm->tm_wday][0];
        tmp = *(slice + pTm->tm_wday * 3 + 0);
        pTm->tm_hour -= 0;
    } else if (pTm->tm_hour > 15) {
        //tmp = runMdCfg.scheduleSlice[pTm->tm_wday][2];
        tmp = *(slice + pTm->tm_wday * 3 + 2);
        pTm->tm_hour -= 16;
    } else {
        //tmp = runMdCfg.scheduleSlice[pTm->tm_wday][1];
        tmp = *(slice + pTm->tm_wday * 3 + 1);
        pTm->tm_hour -= 8;
    }
    //os_printf("slice:%u\n", tmp);

    int mask1 = -1;
    if (pTm->tm_min < 15)
        mask1 = 0;
    if ((pTm->tm_min >= 15) && (pTm->tm_min < 30))
        mask1 = 1;
    if ((pTm->tm_min >= 30) && (pTm->tm_min < 45))
        mask1 = 2;
    if ((pTm->tm_min >= 45) && (pTm->tm_min < 60))
        mask1 = 3;

    int mask2 = pTm->tm_hour * 4 + mask1;
    //os_printf("mask:%d\n", mask2);

    int ret = (((1 << mask2) & tmp) == 0) ? 0 : 1;
    //os_printf("ret:%d\n", ret);

    return ret;
*/
    return 0;
}

//将 SDK_SCHEDTIME 结构体中的时间信息转换为两个 64 位无符号整数
int timepoint_to_u64(SDK_SCHEDTIME *pSchedTime, unsigned long long *start_64, unsigned long long *stop_64)
{
    char str[30];
    os_memset(str, 0, sizeof(str));
    sprintf(str, "%02d%02d", pSchedTime->startHour, pSchedTime->startMin);
    *start_64 = atoll(str);

    os_memset(str, 0, sizeof(str));
    sprintf(str, "%02d%02d", pSchedTime->stopHour, pSchedTime->stopMin);
    *stop_64 = atoll(str);

    return 0;
}


int is_in_schedule_timepoint(SDK_SCHEDTIME *time_point)
{
    int ret = 0;
/*
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = localtime_r(&ts, &tt);

    char str[30];
    os_memset(str, 0, sizeof(str));
    sprintf(str, "%02d%02d", pTm->tm_hour, pTm->tm_min);
    unsigned long long now_64 = atoll(str);

    int i;
 
    unsigned long long start_64 = 0;
    unsigned long long stop_64 = 0;
    SDK_SCHEDTIME *tPoint = NULL;
    for (i = 0; i < 4; i ++) {
        tPoint = (time_point + pTm->tm_wday * 4 + i);
        timepoint_to_u64(tPoint, &start_64, &stop_64);
        if ((now_64 >= start_64) && (now_64 <= stop_64)) {
            ret = 1;
            break;
        }
    }
*/
    return ret;
}

