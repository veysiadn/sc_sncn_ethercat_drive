/**
 * @file file_service.h
 * @brief Simple flash file service to store configuration parameter
 * @author Synapticon GmbH <support@synapticon.com>
 */

#pragma once

#include <spiffs_service.h>
#include <motion_control_service.h>

#define LOG_FILE_NAME "logging1"

#define LOG_DATA_INTERVAL 500000000


typedef interface DataLoggingInterface DataLoggingInterface;

interface DataLoggingInterface {


    [[guarded]] unsigned short log_user_command(void);

    [[guarded]] unsigned short log_error(char msg[], unsigned int timestamp);

};

enum eLogMsgType {
    LOG_MSG_COMMAND = 0
    ,LOG_MSG_ERROR
    ,LOG_MSG_DATA
};

void data_logging_service(
        interface DataLoggingInterface server ?i_logif[n_logif],
        client SPIFFSInterface ?i_spiffs,
        client interface MotionControlInterface i_motion_control,
        unsigned n_logif);
