/*
 * SIM800C.h
 *
 *  Created on: Jun 9, 2026
 *      Author: win
 */

#ifndef INC_SIM800C_H_
#define INC_SIM800C_H_


#include "main.h"
#include <stdbool.h>

#define GSM_RX_BUFFER_SIZE 1024

extern uint8_t gsm_rx_byte;
extern volatile uint16_t gsm_rx_index;
extern char gsm_rx_buffer[GSM_RX_BUFFER_SIZE];

typedef enum
{
    GSM_RES_TIMEOUT = 0,
    GSM_RES_OK,
    GSM_RES_ERROR

} GSM_Result_t;

typedef enum
{
    GSM_INIT_OK = 0,

    GSM_ERR_AT,
    GSM_ERR_SIM,
    GSM_ERR_NETWORK,
    GSM_ERR_SIGNAL

} GSM_InitResult_t;

typedef struct
{
    uint16_t status;
    uint16_t length;
} GSM_HTTP_Result_t;

GSM_InitResult_t GSM_Init(void);
bool GSM_IsSimReady(void);
bool GSM_IsRegistered(void);

void GSM_ClearBuffer(void);

GSM_Result_t GSM_SendCommandEx(
        const char *cmd,
        uint32_t timeout,
        const char *okStr,
        const char *errStr);

int GSM_GetSignalQuality(void);

bool GSM_GetOperator(char *name, uint16_t size);

bool GSM_GPRS_Open(
        const char *apn,
        const char *user,
        const char *pass);

bool GSM_GPRS_GetIP(char *ip, uint16_t size);

bool GSM_GPRS_Close(void);

//static int GSM_HTTP_GetLength(void);
bool GSM_HTTP_GET(
        const char *url,
        char *response,
        uint16_t responseSize);

bool GSM_HTTP_POST(
        const char *url,
        const char *json,
        char *response,
        uint16_t responseSize);




typedef struct
{
    uint32_t measure_at;        // Unix Timestamp (UTC)

    double latitude;            // degrees
    double longitude;           // degrees

    float distance;             // km (مجموع مسافت)

    float speed;                // km/h

    float fuel_usage;           // L/h یا L/100km (باید مشخص کنیم)

    uint16_t rpm;

    uint8_t cell_signal;        // 0..31

    uint8_t operator_id;        // enum

    int16_t temperature;        // x10

    uint8_t retry_count;

} SensorRecord_t;

#endif /* INC_SIM800C_H_ */
