
#include <data_logging_service.h>
#include <xs1.h>
#include <print.h>
#include <string.h>
#include <stdio.h>
#include <safestring.h>

unsigned short file_descriptor;


void data_logging_init(client SPIFFSInterface ?i_spiffs)
{
    int res;
    char log_buf[768];


    //Trying to open existing LOG file
    file_descriptor = i_spiffs.open_file(LOG_FILE_NAME, strlen(LOG_FILE_NAME), (SPIFFS_RDWR));
    if ((file_descriptor < 0))
    {
        printstrln("Error opening file");
    }
    else
        //File not found, creating of new file
    if (file_descriptor > SPIFFS_MAX_FILE_DESCRIPTOR)
    {
            printstrln("LOG file not found, creating of new file");
            file_descriptor = i_spiffs.open_file(LOG_FILE_NAME, strlen(LOG_FILE_NAME), (SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR));
            if ((file_descriptor < 0)||(file_descriptor > SPIFFS_MAX_FILE_DESCRIPTOR))
            {
                  printstrln("Error opening file");
            }
            else
            {
                printstr("File created: ");
                printintln(file_descriptor);
            }

            safememset(log_buf, 0, sizeof(log_buf));
            sprintf(log_buf, "MOTOR CONTROL DATA LOG\n\nV_dc  I_b  I_c  Angle Hall QEI_idx AngleVel Pos Singlet Velocity SensTimeStmp SecPos SecSinglet SecVel SecSensTimeStmp Temp AI_a1 AI_a2 AI_a3 AI_a4\n\n");

            res = i_spiffs.write(file_descriptor, log_buf, strlen(log_buf));
            i_spiffs.flush(file_descriptor);

            printintln(res);
    }
    else
    {
        printstrln("LOG file opened");
        i_spiffs.seek(file_descriptor, 0, SPIFFS_SEEK_END);
    }

}

void data_logging_save(client SPIFFSInterface ?i_spiffs, client interface MotionControlInterface i_motion_control)
{
    int res;
    char log_buf[768];
    UpstreamControlData ucd;
    DownstreamControlData dcd;

    {ucd, dcd} = i_motion_control.read_control_data();

    safememset(log_buf, 0, sizeof(log_buf));
    sprintf(log_buf, " %d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
    ucd.computed_torque,
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

    printintln(res);

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

    data_logging_init(i_spiffs);

    while (1) {

        select {

            case !isnull(i_logif) => i_logif[int i].log_user_command(char msg[], unsigned int timestamp) -> unsigned short res:

                break;

            case !isnull(i_logif) => i_logif[int i].log_error(char msg[], unsigned int timestamp) -> unsigned short res:

                break;

            case t when timerafter(time + LOG_DATA_INTERVAL) :> void :

                    data_logging_save(i_spiffs, i_motion_control);

                     t :> time;

                 break;

            default:
                break;


            }


    }
}

