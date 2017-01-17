/*
 * canopen_service.h
 *
 *  Created on: 14.12.2016
 *      Author: hstroetgen
 */


#ifndef CANOPEN_SERVICE_H_
#define CANOPEN_SERVICE_H_

#include "co_interface.h"
#include "canod_constants.h"
#include "canod_datatypes.h"
#include "pdo_handler.h"

/**
 * @brief Provides a service, which managed the object dictionary communication and stores all necessary values.
 * @param i_od  Interface for setting and getting OD values.
 */
void canopen_service(server interface ODCommunicationInterface i_od[3]);

#endif /* CANOPEN_SERVICE_H_ */
