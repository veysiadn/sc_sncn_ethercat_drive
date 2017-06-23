
#include <data_logging_service.h>
#include <xs1.h>
#include <print.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <safestring.h>

unsigned short file_descriptor;
unsigned char curr_log_file_no = 0;
unsigned char log_timer_active = 0;
unsigned int timer_interval;
unsigned int max_log_file_size = 15000;

char logging_file_name[2][SPIFFS_MAX_FILENAME_SIZE] = {LOG_FILE_NAME1, LOG_FILE_NAME2};


int get_config_value(char title[], char end_marker[], char buffer[])
{
    char * begin_pos;
    char * end_pos;
    char value_buffer[SPIFFS_MAX_DATA_BUFFER_SIZE];

    //just to read opened file size
    int res = 0;

    begin_pos = strstr(buffer, title);
    end_pos =  strstr(buffer,  end_marker);

    if ((!begin_pos)||(!end_pos))
    {
        printstrln("Config: value not found");
        return -1;
    }

    memset(value_buffer, '\0', SPIFFS_MAX_DATA_BUFFER_SIZE);
    memcpy(value_buffer, begin_pos + strlen(title), end_pos - (begin_pos  + strlen(end_marker)));
    res = atoi(value_buffer);

    memset(begin_pos, ' ', (end_pos - begin_pos) + 1);

    return res;
}



int read_log_config(client SPIFFSInterface ?i_spiffs)
{
    char config_buffer[SPIFFS_MAX_DATA_BUFFER_SIZE];
    int file_size;
    int res;

    file_descriptor = i_spiffs.open_file(LOG_CONFIG_FILE, strlen(LOG_CONFIG_FILE), SPIFFS_RDONLY);
    if (file_descriptor < 0)
    {
        printstrln("Error opening log configuration file");
        return -1;

    }
    else
    if (file_descriptor > SPIFFS_MAX_FILE_DESCRIPTOR)
    {
        printstrln("Log configuration file file not found ");
        return -1;

     }
     else
     {
         printstr("File opened: ");
         printintln(file_descriptor);
     }

    file_size = i_spiffs.get_file_size(file_descriptor);

    memset(config_buffer, '\0', SPIFFS_MAX_DATA_BUFFER_SIZE);
    res = i_spiffs.read(file_descriptor, config_buffer, file_size);
    i_spiffs.close_file(file_descriptor);
    config_buffer[file_size] = CONFIG_END_OF_STRING_MARKER[0];

    timer_interval = get_config_value(CONFIG_INTERVAL_TITLE, CONFIG_END_OF_STRING_MARKER, config_buffer);

    if (timer_interval < MIN_LOG_INTERVAL)
    {
        printstrln("Config: Incorrect timer interval");
        return -1;
    }
    timer_interval *= LOG_INTERVAL_MULT;

    printintln(get_config_value(CONFIG_LAST_LOG_TITLE, CONFIG_END_OF_STRING_MARKER, config_buffer));


    return 0;

}


int open_log_file(client SPIFFSInterface ?i_spiffs, char reset_existing)
{
    int res;
    char log_buf[768];
    unsigned short flags = 0;

    if (reset_existing)
        //clear existing file
        flags = (SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR);
    else
        //just continue to recording
        flags = SPIFFS_RDWR;

    //close previous file, if opened
    if (file_descriptor > 0) i_spiffs.close_file(file_descriptor);

    //Trying to open existing LOG file
    file_descriptor = i_spiffs.open_file(logging_file_name[curr_log_file_no], strlen(logging_file_name[curr_log_file_no]), flags);
    if ((file_descriptor < 0))
    {
        printstrln("Error opening file");
    }
    else
    //File not found, creating of new file
    if (file_descriptor > SPIFFS_MAX_FILE_DESCRIPTOR)
    {
            printstr("LOG file not found, creating of new file: ");
            printstrln(logging_file_name[curr_log_file_no]);
            file_descriptor = i_spiffs.open_file(logging_file_name[curr_log_file_no], strlen(logging_file_name[curr_log_file_no]), (SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR));
            if ((file_descriptor < 0)||(file_descriptor > SPIFFS_MAX_FILE_DESCRIPTOR))
            {
                  printstrln("Error opening file");
                  return -1;
            }
            else
            {
                printstr("File created: ");
                printintln(file_descriptor);
            }

            safememset(log_buf, 0, sizeof(log_buf));
            printf(log_buf, "MOTOR CONTROL DATA LOG\n\nCompTorq TorqSet V_dc  I_b  I_c  Angle Hall QEI_idx AngleVel Pos Singlet Velocity SensTimeStmp SecPos SecSinglet SecVel SecSensTimeStmp Temp AI_a1 AI_a2 AI_a3 AI_a4\n\n");

            res = i_spiffs.write(file_descriptor, log_buf, strlen(log_buf));
            i_spiffs.flush(file_descriptor);

            printintln(res);
    }
    else
    {
        printstrln("LOG file opened");
        i_spiffs.seek(file_descriptor, 0, SPIFFS_SEEK_END);
    }

    return 0;
}

int check_log_file_size(client SPIFFSInterface ?i_spiffs)
{
    int res;

    res = i_spiffs.get_file_size(file_descriptor);

    if (res < 0)
    {
        printstrln("Error getting file size");
        return -1;
    }

    if (res > max_log_file_size)
    {
        if (curr_log_file_no == 0)
            curr_log_file_no = 1;
        else
          if (curr_log_file_no == 1)
              curr_log_file_no = 0;

        printstr("Switching LOG file to: ");
        printstrln(logging_file_name[curr_log_file_no]);

        if (open_log_file(i_spiffs, 1) != 0)
        {
            //error opening file
            return -1;
        }
    }

    return 0;
}

int data_logging_init(client SPIFFSInterface ?i_spiffs)
{

    if (read_log_config(i_spiffs) != 0)
    {
        //error in config file
        return -1;
    }

    if (open_log_file(i_spiffs, 0) != 0)
    {
        //error opening file
        return -1;
    }

    if (check_log_file_size(i_spiffs) != 0)
    {
        //error checking of file
        return -1;
    }


    return 0;

}

void data_logging_save(client SPIFFSInterface ?i_spiffs, client interface MotionControlInterface i_motion_control)
{
    int res;
    char log_buf[768];
    UpstreamControlData ucd;
    DownstreamControlData dcd;

    {ucd, dcd} = i_motion_control.read_control_data();

    safememset(log_buf, 0, sizeof(log_buf));
    sprintf(log_buf, " %d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
    ucd.computed_torque,
    ucd.torque_set,

    ucd.V_dc,
    ucd.I_b,
    ucd.I_c,

    ucd.angle,
    ucd. hall_state,
    ucd.qei_index_found,

    ucd.angle_velocity,

    ucd.position,
    ucd.singleturn,
    ucd.velocity,

    ucd.sensor_timestamp,

    ucd.secondary_position,
    ucd.secondary_singleturn,
    ucd.secondary_velocity,

    ucd.secondary_sensor_timestamp,

    ucd.temperature,

    ucd.analogue_input_a_1,
    ucd.analogue_input_a_2,
    ucd.analogue_input_b_1,
    ucd.analogue_input_b_2);


    res = i_spiffs.write(file_descriptor, log_buf, strlen(log_buf));
    i_spiffs.flush(file_descriptor);

    printint(res);
    printstr(" ");
    printintln(i_spiffs.get_file_size(file_descriptor));

}


void data_logging_service(
        interface DataLoggingInterface server ?i_logif[n_logif],
        client SPIFFSInterface ?i_spiffs,
        client interface MotionControlInterface i_motion_control,
        unsigned n_logif)
{
    timer t;
    unsigned time = 0;
    int whait_for_reply = 0;



    if (isnull(i_spiffs)) {
            // error spiffs
            return;
        }

    if (isnull(i_motion_control)) {
              // error shared_memory
              return;
          }

    select {
        case i_spiffs.service_ready():
        break;
    }

    printstrln(">>   DATA LOGGING SERVICE STARTING...");

    if (data_logging_init(i_spiffs) != 0)
    {
       //service init
       return;
    }
    else
        log_timer_active = 1;

    while (1) {

        select {

            case !isnull(i_logif) => i_logif[int i].log_user_command(void) -> unsigned short res:
                    char command_buf[128];
                    UpstreamControlData ucd;
                    DownstreamControlData dcd;
                    {ucd, dcd} = i_motion_control.read_control_data();

                    safememset(command_buf, 0, sizeof(command_buf));
                    sprintf(command_buf, "User command: Position: %d, Velocity %d, Torque: %d, Offset Torque: %d\n", dcd.position_cmd, dcd.velocity_cmd, dcd.torque_cmd, dcd.offset_torque);
                    res = i_spiffs.write(file_descriptor, command_buf, strlen(command_buf));
                    i_spiffs.flush(file_descriptor);

                    printintln(res);

                break;

            case !isnull(i_logif) => i_logif[int i].log_error(unsigned error_code) -> unsigned short res:

                break;

            case t when timerafter(time + timer_interval) :> void :
                    if (log_timer_active)
                    {
                        data_logging_save(i_spiffs, i_motion_control);

                        if (check_log_file_size(i_spiffs) != 0)
                            log_timer_active = 0;

                    }
                    t :> time;

                 break;

            default:
                break;


            }


    }
}

