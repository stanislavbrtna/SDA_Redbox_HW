/*
 * power_management.h
 *
 *  Created on: 24. 4. 2021
 *      Author: stanislaw
 */

#ifndef DRIVERS_POWER_MANAGEMENT_H_
#define DRIVERS_POWER_MANAGEMENT_H_

#include "../sda_platform.h"

void sda_sleep();
void lowBattCheckAndHalt();


#endif /* DRIVERS_POWER_MANAGEMENT_H_ */
