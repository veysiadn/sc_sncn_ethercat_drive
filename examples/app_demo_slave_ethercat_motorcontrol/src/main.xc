/* INCLUDE BOARD SUPPORT FILES FROM module_board-support */
#include <COM_ECAT-rev-a.inc>
#include <CORE_C22-rev-a.inc>
#include <IFM_DC100-rev-b.inc>

/**
 * @file test_ethercat-mode.xc
 * @brief Test illustrates usage of Motor Control with Ethercat
 * @author Synapticon GmbH (www.synapticon.com)
 */

#include <qei_service.h>
#include <hall_service.h>
#include <pwm_service.h>
#include <adc_service.h>
#include <motorcontrol_service.h>
#include <gpio_service.h>

#include <velocity_ctrl_service.h>
#include <position_ctrl_service.h>
#include <torque_ctrl_service.h>
#include <profile_control.h>

#include <ethercat_drive_service.h>

#include <ethercat_service.h>
#include <fw_update_service.h>

 //Configure your default motor parameters in config/bldc_motor_config.h
#include <qei_config.h>
#include <hall_config.h>
#include <motorcontrol_config.h>
#include <control_config.h>
#include <ethercat_modes_config.h>

EthercatPorts ethercat_ports = SOMANET_COM_ETHERCAT_PORTS;
PwmPorts pwm_ports = SOMANET_IFM_PWM_PORTS;
WatchdogPorts wd_ports = SOMANET_IFM_WATCHDOG_PORTS;
ADCPorts adc_ports = SOMANET_IFM_ADC_PORTS;
FetDriverPorts fet_driver_ports = SOMANET_IFM_FET_DRIVER_PORTS;
HallPorts hall_ports = SOMANET_IFM_HALL_PORTS;
QEIPorts qei_ports = SOMANET_IFM_QEI_PORTS;
port gpio_ports[4] = {  SOMANET_IFM_GPIO_D0,
                        SOMANET_IFM_GPIO_D1,
                        SOMANET_IFM_GPIO_D2,
                        SOMANET_IFM_GPIO_D3 };

int main(void)
{
    /* Motor control channels */
    chan c_adctrig, c_pwm_ctrl;

    interface GPIOInterface i_gpio[2];
    interface WatchdogInterface i_watchdog[3];
    interface ADCInterface i_adc[3];
    interface HallInterface i_hall[5];
    interface QEIInterface i_qei[5];
    interface MotorcontrolInterface i_motorcontrol[5];

    interface TorqueControlInterface i_torque_control[3];
    interface PositionControlInterface i_position_control[3];
    interface VelocityControlInterface i_velocity_control[3];

    /* EtherCat Communication channels */
    chan coe_in;          // CAN from module_ethercat to consumer
    chan coe_out;         // CAN from consumer to module_ethercat
    chan eoe_in;          // Ethernet from module_ethercat to consumer
    chan eoe_out;         // Ethernet from consumer to module_ethercat
    chan eoe_sig;
    chan foe_in;          // File from module_ethercat to consumer
    chan foe_out;         // File from consumer to module_ethercat
    chan pdo_in;
    chan pdo_out;
    chan c_nodes[1], c_flash_data; // Firmware channels

    par
    {
        /************************************************************
         *                          COM_TILE
         ************************************************************/

        /* Ethercat Communication Handler Loop */
        on tile[COM_TILE] :
        {
            ethercat_service(coe_out, coe_in, eoe_out, eoe_in, eoe_sig,
                                foe_out, foe_in, pdo_out, pdo_in, ethercat_ports);
        }

        /* Firmware Update Loop over Ethercat */
        on tile[COM_TILE] :
        {
            fw_update_service(p_spi_flash, foe_out, foe_in, c_flash_data, c_nodes, null);
        }

        /* Ethercat Motor Drive Loop */
        on tile[APP_TILE_1] :
        {
            CyclicSyncTorqueConfig cyclic_sync_torque_config;
            init_cst_config(cyclic_sync_torque_config);

            CyclicSyncVelocityConfig cyclic_sync_velocity_config;
            init_csv_config(cyclic_sync_velocity_config);

            CyclicSyncPositionConfig cyclic_sync_position_config;
            init_csp_config(cyclic_sync_position_config);

            ProfilePositionConfig profile_position_config;
            init_pp_config(profile_position_config);

            ProfileVelocityConfig profile_velocity_config;
            init_pv_config(profile_velocity_config);

            ProfileTorqueConfig profile_torque_config;
            init_pt_config(profile_torque_config);

            ethercat_drive_service(cyclic_sync_position_config, cyclic_sync_velocity_config, cyclic_sync_torque_config,
                                    profile_position_config, profile_velocity_config, profile_torque_config,
                                    pdo_out, pdo_in, coe_out,
                                    i_motorcontrol[3], i_hall[4], i_qei[4], i_gpio[0],
                                    i_torque_control[0], i_velocity_control[0], i_position_control[0]);
        }

        on tile[APP_TILE_2]:
        {
            par
            {
                /* Position Control Loop */
                {
                     ControlConfig position_ctrl_config;
                     init_position_control_config(position_ctrl_config); // Initialize PID parameters for Position Control

                     /* Control Loop */
                     position_control_service(position_ctrl_config, i_hall[1], i_qei[1], i_motorcontrol[0],
                                                 i_position_control);
                }

                /* Velocity Control Loop */
                {
                    ControlConfig velocity_ctrl_config;
                    init_velocity_control_config(velocity_ctrl_config); // Initialize PID parameters for Velocity Control

                    /* Control Loop */
                    velocity_control_service(velocity_ctrl_config, i_hall[2], i_qei[2], i_motorcontrol[1],
                                                i_velocity_control);
                }

                /* Torque Control Loop */
                {
                    /* Torque Control Loop */
                    ControlConfig torque_ctrl_config;
                    init_torque_control_config(torque_ctrl_config);  // Initialize PID parameters for Torque Control

                    /* Control Loop */
                    torque_control_service(torque_ctrl_config, i_adc[0], i_motorcontrol[2], i_hall[3], i_qei[3],
                                                i_torque_control);
                }
            }
        }

        /************************************************************
         *                          IFM_TILE
         ************************************************************/
        on tile[IFM_TILE]:
        {
            par
            {
                /* ADC Loop */
                adc_service(adc_ports, c_adctrig, i_adc);

                /* PWM Loop */
                pwm_triggered_service(pwm_ports, c_adctrig, c_pwm_ctrl);

                /* Watchdog Server */
                watchdog_service(wd_ports, i_watchdog);

                /* Hall Server */
                {
                    HallConfig hall_config;
                    init_hall_config(hall_config);

                    hall_service(hall_ports, hall_config, i_hall);
                }

                /* QEI Server */
                {
                    QEIConfig qei_config;
                    init_qei_config(qei_config);

                    qei_service(qei_ports, qei_config, i_qei);
                }

                /* Motor Commutation loop */
                {
                    MotorcontrolConfig motorcontrol_config;
                    init_motorcontrol_config(motorcontrol_config);

                    motorcontrol_service(fet_driver_ports, motorcontrol_config,
                                            c_pwm_ctrl, i_hall[0], i_qei[0], i_watchdog[0], i_motorcontrol);
                }

                /* GPIO Digital Server */
                gpio_service(gpio_ports, i_gpio);
            }
        }
    }

    return 0;
}