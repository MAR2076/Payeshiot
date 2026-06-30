/*
 * GPS.h
 *
 *  Created on: Jun 30, 2026
 *      Author: win
 */

#ifndef INC_GPS_H_
#define INC_GPS_H_



#include "stdint.h"


#define GPS_RX_BUFFER_SIZE      256

#define GPS_PACKET_SIZE         128

#define GPS_ENABLE_GGA          1

#define GPS_ENABLE_RMC          1

#define GPS_ENABLE_GSV          1

#define GPS_ENABLE_GSA          1



typedef struct
{
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;

}GPS_Time_t;


typedef struct
{
    uint8_t Day;
    uint8_t Month;
    uint8_t Year;

}GPS_Date_t;


typedef struct
{
    double Latitude;
    double Longitude;

}GPS_Position_t;


typedef struct
{
    float Altitude;

    float SpeedKnots;

    float SpeedKm;

    float SpeedMS;

}GPS_Motion_t;


typedef struct
{
    uint8_t Fix;

    uint8_t Quality;

    uint8_t Satellites;

    float HDOP;

}GPS_Status_t;


typedef struct
{
    GPS_Time_t Time;

    GPS_Date_t Date;

    GPS_Position_t Position;

    GPS_Motion_t Motion;

    GPS_Status_t Status;

}GPS_Data_t;



void GPS_Init(void);

void GPS_Update(void);

uint8_t GPS_IsFix(void);

GPS_Data_t* GPS_GetData(void);


#endif /* INC_GPS_H_ */
