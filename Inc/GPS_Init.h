/*!
 * \file GPS_Init.h
 * \brief
 *
 *  Created on: Nov 6, 2024
 *      Author: Kris Jaxa
 *            @ Jaxasoft, Freeware
 *              v.1.0.0
 */

#ifndef INC_GPS_INIT_H_
#define INC_GPS_INIT_H_

namespace gps_init
{
extern bool gps_test;
int connect_gpsBoard();
void My_UART4_Init(int baud);
void stimulus();

const int SZ {20};
extern char buf[SZ];
}

#endif /* INC_GPS_INIT_H_ */
