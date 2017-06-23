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

#define CONFIG_DATA_INTERVAL_TITLE       "LOG_DATA_INTERVAL_MS="
#define CONFIG_ERROR_LOG_INTERVAL_TITLE  "LOG_ERRORS_INTERVAL_US="
#define CONFIG_LOG_FILE_MAX_TITLE        "MAX_LOG_FILE_SIZE_BYTES="
#define CONFIG_ERR_CODES_COUNT           "ERR_CODES_COUNT="
#define CONFIG_LOG_FILE_NAME1_TITLE      "LOG1_FILE_NAME="
#define CONFIG_LOG_FILE_NAME2_TITLE      "LOG2_FILE_NAME="
#define CONFIG_LOG_ERR_TITLE "ERR_%d="

#define CONFIG_END_OF_STRING_MARKER "\n"

#define MIN_LOG_INTERVAL 100
#define LOG_INTERVAL_MULT 100000

#define CONFIG_MAX_STRING_SIZE 128
#define CONFIG_MAX_ERROR_TITLES 32


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


typedef struct {

    int data_timer_interval;
    int error_timer_interval;
    int max_log_file_size;
    char err_codes_count;
    char log_file_name[2][SPIFFS_MAX_FILENAME_SIZE];
    char errors_titles[CONFIG_MAX_ERROR_TITLES][CONFIG_MAX_STRING_SIZE];

} DataLoggingConfig;


void data_logging_service(
        interface DataLoggingInterface server ?i_logif[n_logif],
        client SPIFFSInterface ?i_spiffs,
        client interface MotionControlInterface i_motion_control,
        unsigned n_logif);
