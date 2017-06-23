/**
 * @file file_service.h
 * @brief Simple flash file service to store configuration parameter
 * @author Synapticon GmbH <support@synapticon.com>
 */

#pragma once

#include <spiffs_service.h>
#include <motion_control_service.h>

#define LOG_FILE_NAME1 "logging1"
#define LOG_FILE_NAME2 "logging2"

#define LOG_CONFIG_FILE "log_config"

#define CONFIG_INTERVAL_TITLE "INTERVAL = "
#define CONFIG_LAST_LOG_TITLE "LAST_LOG_FILE = "
#define CONFIG_END_OF_STRING_MARKER "\n"

#define MIN_LOG_INTERVAL 100
#define LOG_INTERVAL_MULT 100000


typedef interface DataLoggingInterface DataLoggingInterface;

interface DataLoggingInterface {


    [[guarded]] unsigned short log_user_command(void);

    [[guarded]] unsigned short log_error(unsigned error_code) ;

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
