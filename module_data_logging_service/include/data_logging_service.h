/**
 * @file data_logging_service.h
 * @brief
 * @author Synapticon GmbH <support@synapticon.com>
 */

#ifndef DATA_LOGGING_SERVICE_H_
#define DATA_LOGGING_SERVICE_H_

#include <spiffs_service.h>
#include <motion_control_service.h>
#include <motor_control_structures.h>

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

#endif
