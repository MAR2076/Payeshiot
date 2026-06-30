/*
 * GPS.h
 *
 *  Created on: Jun 30, 2026
 *      Author: win
 */

#ifndef INC_GPS_H_
#define INC_GPS_H_

#include "stdint.h"
#include "stdbool.h"

/*#include "stdint.h"


#define GPS_RX_BUFFER_SIZE      256

#define GPS_PACKET_SIZE         128

#define GPS_ENABLE_GGA          1

#define GPS_ENABLE_RMC          1

#define GPS_ENABLE_GSV          1

#define GPS_ENABLE_GSA          1*/



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

/*

void GPS_Init(void);

void GPS_Update(void);

uint8_t GPS_IsFix(void);

GPS_Data_t* GPS_GetData(void);



uint16_t GPS_DMA_GetWritePos(void);
void gps_rx_byte(char c);
uint8_t NMEA_Checksum(char *str);
uint8_t NMEA_HexToByte(char *hex);
uint8_t NMEA_Validate(char *msg);
char* NMEA_GetField(char *msg, uint8_t index);
void GPS_ParseRMC(char *msg);
void GPS_ParseGGA(char *msg);
double GPS_Convert(char *val);
void GPS_LineReady(char *line);
void GPS_UART_IdleCallback(void);*/



/* ================= CONFIG ================= */

#define GPS_RING_SIZE 256   // power of 2 توصیه می‌شود

/* ================= API ================= */

void     GPS_Ring_Init(void);

/* Called from main loop */
void     GPS_Ring_Process(void);

/* Get next byte (optional use) */
bool     GPS_Ring_ReadByte(uint8_t *byte);

/* Check available bytes */
uint16_t GPS_Ring_Available(void);

/* DMA update hook */
void     GPS_Ring_DMA_Update(uint16_t dma_pos);


//---------Tokenize--------//
#define GPS_MAX_FIELDS   20

typedef struct
{
    const char *ptr;
    uint8_t len;
}GPS_Field_t;

typedef struct
{
    GPS_Field_t field[GPS_MAX_FIELDS];
    uint8_t count;
}GPS_Token_t;

/* API */
bool GPS_Tokenize(char *sentence, GPS_Token_t *out);

//-------------GPS_Parser--------------//

typedef struct
{
    GPS_Data_t working;
    uint8_t rmc_ready;
    uint8_t gga_ready;
    uint8_t gsv_ready;

}GPS_ParserContext_t;

void GPS_Parser_Init(GPS_ParserContext_t *ctx);

void GPS_ParseSentence(GPS_ParserContext_t *ctx, GPS_Token_t *t);

void GPS_Parser_CommitIfReady(GPS_ParserContext_t *ctx, GPS_Data_t *final);

//-------------GPS_Utils--------------//

uint8_t  GPS_ParseUint8(const GPS_Field_t *f);
uint32_t GPS_ParseUint32(const GPS_Field_t *f);

float    GPS_ParseFloat(const GPS_Field_t *f);

/* ddmm.mmmm → degree */
double   GPS_ParseCoordinate(const GPS_Field_t *f);


//-------------Final API--------------//

void GPS_Init(void);

/* Main periodic task */
void GPS_Process(void);

/* State */
bool GPS_IsFix(void);
bool GPS_IsDataValid(void);

uint32_t GPS_GetLastUpdateTime(void);

/* Data access */
const GPS_Data_t* GPS_GetData(void);

/* Optional */
void GPS_Reset(void);

#endif
