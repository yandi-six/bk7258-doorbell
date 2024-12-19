/*!
*****************************************************************************

*****************************************************************************
*/

#if 1
#include "cfg_record_state.h"

SDK_RECORD_STATE_CFG runRecordstateCfg;

SDK_CFG_MAP RecstateMap[] = {
    {"userRecord", &runRecordstateCfg.state, SDK_CFG_DATA_TYPE_S32, "", "rw", 0, 1, NULL}, //++++
    {
        NULL,
    }

};

void RecstateCfgPrint()
{
    os_printf("*************** Recstate **************\n");

    os_printf("Record state:\n");
    CfgPrintMap(RecstateMap);

    os_printf("*************** Recstate **************\n\n");
}

int RecstateCfgDelete()
{
    int ret = CfgWriteDelete(RECSTATE_CFG_FILE);
    if (ret != 0)
    {
        os_printf("CfgWriteDelete %s error.\n", RECSTATE_CFG_FILE);
        return ret;
    }
    return 0;
}

int RecstateCfgSave()
{
    printf("RecstateCfgSave--------------------------------------");
    cJSON *root;

    char *out;

    root = cJSON_CreateObject(); //\B4\B4\BD\A8\CF\EEĿ

    CfgAddCjson(root, "userRecord", RecstateMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(RECSTATE_CFG_FILE, out);
    if (ret != 0)
    {
        os_printf("CfgWriteToFile %s error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!.\n", RECSTATE_CFG_FILE);
        return -1;
    }

    os_free(out);
    cJSON_Delete(root);

    return 0;
}

int RecstateCfgLoadDefValue()
{
    CfgLoadDefValue(RecstateMap);
    return 0;
}

// 加载配置文件
int RecstateCfgLoad()
{
    //printf("RecstateCfgLoad--------------------------------------");
    char *data = NULL;
    data = CfgReadFromFile(RECSTATE_CFG_FILE);
    if (data == NULL)
    {
        //\B4\D3\C5\E4\D6\C3\CEļ\FE\B6\C1ȡʧ\B0ܣ\AC\D4\F2ʹ\D3\C3Ĭ\C8ϲ\CE\CA\FD
        os_printf("load %s error, so to load default cfg param.\n", RECSTATE_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json)
    {
        //\B4\D3\C5\E4\D6\C3\CEļ\FE\BD\E2\CE\F6cjsonʧ\B0ܣ\AC\D4\F2ʹ\D3\C3Ĭ\C8ϲ\CE\CA\FD
        os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        os_free(data);
        goto err;
    }

    CfgParseCjson(json, "userRecord", RecstateMap);

    os_free(data);
    return 0;

err:
    RecstateCfgLoadDefValue();
    RecstateCfgSave();
    return 0;
}

#endif
cJSON *RecstateCfgLoadJson()
{

    char *data = NULL;
    data = CfgReadFromFile(RECSTATE_CFG_FILE);
    if (data == NULL)
    {
        //\B4\D3\C5\E4\D6\C3\CEļ\FE\B6\C1ȡʧ\B0ܣ\AC\D4\F2ʹ\D3\C3Ĭ\C8ϲ\CE\CA\FD
        // webcam_debug("load %s error, so to load default cfg param.\n", RECSTATE_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json)
    {
        //\B4\D3\C5\E4\D6\C3\CEļ\FE\BD\E2\CE\F6cjsonʧ\B0ܣ\AC\D4\F2ʹ\D3\C3Ĭ\C8ϲ\CE\CA\FD
        // webcam_error("Error before: [%s]\n", cJSON_GetErrorPtr());
        os_free(data);
        goto err;
    }

    os_free(data);
    return json;
err:
    if (data != NULL)
    {
        os_free(data);
    }
    return NULL;
}
