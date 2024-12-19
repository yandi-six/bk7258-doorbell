#ifndef _VFS_FILE_H
#define _VFS_FILE_H
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <components/system.h>
#include <os/os.h>
#include <os/mem.h>
#include <os/str.h>
#include <sys/time.h>
#include <driver/mailbox_channel.h>
#include "project_defs.h"
#include "rtc_bk.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_vfs_files(project_config_t *config);
void uninit_vfs_files();
void vfs_file_handle_req_rx(webrtc_cmd_t *pcmd);
void vfs_file_handle_resp_rx(webrtc_cmd_t *pcmd);
#ifdef __cplusplus
}
#endif


#endif /* _VFS_FILE_H */
