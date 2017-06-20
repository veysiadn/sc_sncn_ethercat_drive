
#include <data_logging_service.h>
#include <xs1.h>
#include <print.h>
#include <string.h>
#include <safestring.h>

unsigned short file_descriptor;


void data_logging_init(client SPIFFSInterface ?i_spiffs)
{
    file_descriptor = i_spiffs.open_file(LOG_FILE_NAME, strlen(LOG_FILE_NAME), (SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR));
    if ((cfd < 0)||(cfd > SPIFFS_MAX_FILE_DESCRIPTOR))
    {
        printstrln("Error opening file");
    }
    else
    {
        printstr("File created: ");
        printintln(cfd);
    }
}

void data_logging_save(client SPIFFSInterface ?i_spiffs, client interface shared_memory_interface i_shared_memory)
{



}


void data_logging_service(
        interface DataLoggingInterface server ?i_logif[n_logif],
        client SPIFFSInterface ?i_spiffs,
        client interface shared_memory_interface i_shared_memory,
        unsigned n_logif)
{
    timer t;
    unsigned time = 0;
    int whait_for_reply = 0;

    /* wait some time until ethercat handler is ready */
    t :> time;

    if (isnull(i_spiffs)) {
            // error spiffs
            return;
        }

    if (isnull(i_shared_memory)) {
              // error shared_memory
              return;
          }

    select {
        case i_spiffs.service_ready():
        break;
    }

    printstrln(">>   DATA LOGGING SERVICE STARTING...\n");

    data_logging_init(i_spiffs);

    while (1) {

        select {

            case !isnull(i_logif) => i_logif[int i].log_user_command(char msg[], unsigned int timestamp) -> unsigned short res:

                break;

            case !isnull(i_logif) => i_logif[int i].log_error(char msg[], unsigned int timestamp) -> unsigned short res:

                break;

            case t when timerafter(time + LOG_DATA_INTERVAL) :> void :
                if (wait_for_reply) {

                    data_logging_save(i_spiffs, i_shared_memory);

                 } else {
                     t :> time;
                 }
                 break;

            default:
                break;


            }


    }
}

