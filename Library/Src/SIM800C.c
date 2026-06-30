/*
 * SIM800C.c
 *
 *  Created on: Jun 9, 2026
 *      Author: win
 */


#include "SIM800C.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern UART_HandleTypeDef huart1;

uint8_t gsm_rx_byte;

volatile uint16_t gsm_rx_index = 0;

char gsm_rx_buffer[GSM_RX_BUFFER_SIZE];

static bool GSM_HTTP_Open(void);
static void GSM_HTTP_Close(void);
static bool GSM_HTTP_SetURL(const char *url);
static bool GSM_HTTP_ReadResponse(char *response,
                                  uint16_t responseSize);

GSM_InitResult_t GSM_Init(void)
{
    int rssi;

    printf("\r\n");
    printf("========== GSM INIT ==========\r\n");

    /* Test modem */

    if(GSM_SendCommandEx(
            "AT\r\n",
            2000,
            "OK",
            "ERROR") != GSM_RES_OK)
    {
        printf("ERROR : MODEM\r\n");
        return GSM_ERR_AT;
    }

    printf("MODEM OK\r\n");

    /* Disable echo */

    GSM_SendCommandEx(
            "ATE0\r\n",
            1000,
            "OK",
            "ERROR");

    /* Enable verbose error */

    GSM_SendCommandEx(
            "AT+CMEE=2\r\n",
            1000,
            "OK",
            "ERROR");

    /* SIM ready */

    if(!GSM_IsSimReady())
    {
        printf("ERROR : SIM\r\n");
        return GSM_ERR_SIM;
    }

    printf("SIM READY\r\n");

    /* Wait network registration */

    printf("WAIT NETWORK...\r\n");

    uint32_t startTime = HAL_GetTick();

    while((HAL_GetTick() - startTime) < 60000)
    {
        if(GSM_IsRegistered())
        {
            break;
        }

        HAL_Delay(1000);
        printf(".");
    }

    printf("\r\n");

    if(!GSM_IsRegistered())
    {
        printf("ERROR : NETWORK\r\n");
        return GSM_ERR_NETWORK;
    }

    printf("NETWORK REGISTERED\r\n");

    /* Signal Quality */

    rssi = GSM_GetSignalQuality();

    if(rssi < 0)
    {
        printf("ERROR : SIGNAL\r\n");
        return GSM_ERR_SIGNAL;
    }

    printf("RSSI = %d\r\n", rssi);

    if(rssi < 10)
    {
        printf("WARNING : WEAK SIGNAL\r\n");
    }

    printf("GSM READY\r\n");

    return GSM_INIT_OK;
}

bool GSM_IsSimReady(void)
{
    return (GSM_SendCommandEx(
                "AT+CPIN?\r\n",
                3000,
                "+CPIN: READY",
                "ERROR") == GSM_RES_OK);
}

bool GSM_IsRegistered(void)
{
    GSM_ClearBuffer();

    if(GSM_SendCommandEx(
            "AT+CREG?\r\n",
            3000,
            "OK",
            "ERROR") != GSM_RES_OK)
    {
        return false;
    }

    if(strstr(gsm_rx_buffer, "+CREG: 0,1"))
        return true;

    if(strstr(gsm_rx_buffer, "+CREG: 0,5"))
        return true;

    return false;
}

bool GSM_IsGPRSRegistered(void)
{
    GSM_ClearBuffer();

    if(GSM_SendCommandEx(
            "AT+CGREG?\r\n",
            3000,
            "OK",
            "ERROR") != GSM_RES_OK)
    {
        return false;
    }

    if(strstr(gsm_rx_buffer, "+CGREG: 0,1"))
        return true;

    if(strstr(gsm_rx_buffer, "+CGREG: 0,5"))
        return true;

    return false;
}

void GSM_ClearBuffer(void)
{
    memset(gsm_rx_buffer, 0, sizeof(gsm_rx_buffer));
    gsm_rx_index = 0;
}


GSM_Result_t GSM_SendCommandEx(
        const char *cmd,
        uint32_t timeout,
        const char *okStr,
        const char *errStr)
{
    GSM_ClearBuffer();

    HAL_UART_Transmit(
            &huart1,
            (uint8_t*)cmd,
            strlen(cmd),
            1000);

    uint32_t tickStart = HAL_GetTick();

    while((HAL_GetTick() - tickStart) < timeout)
    {
        if(okStr != NULL)
        {
            if(strstr(gsm_rx_buffer, okStr) != NULL)
            {
            	//HAL_Delay(300);
            	//printf("RX:\r\n%s\r\n", gsm_rx_buffer);
                return GSM_RES_OK;
            }
        }

        if(errStr != NULL)
        {
            if(strstr(gsm_rx_buffer, errStr) != NULL)
            {
                return GSM_RES_ERROR;
            }
        }

        HAL_Delay(50);
    }

    return GSM_RES_TIMEOUT;
}

int GSM_GetSignalQuality(void)
{
    char *ptr;

    GSM_Result_t result;

    result = GSM_SendCommandEx(
                "AT+CSQ\r\n",
                2000,
                "+CSQ:",
                "ERROR");

    if(result != GSM_RES_OK)
    {
        return -1;
    }

    ptr = strstr(gsm_rx_buffer, "+CSQ:");

    if(ptr == NULL)
    {
        return -1;
    }

    int rssi;

    if(sscanf(ptr, "+CSQ: %d", &rssi) == 1)
    {
        return rssi;
    }

    return -1;
}

bool GSM_GetOperator(char *name, uint16_t size)
{
    char *start;
    char *end;

    GSM_ClearBuffer();

    if(GSM_SendCommandEx(
            "AT+COPS?\r\n",
            5000,
            "OK",
            "ERROR") != GSM_RES_OK)
    {
        return false;
    }

    start = strchr(gsm_rx_buffer, '\"');

    if(start == NULL)
    {
        return false;
    }

    start++;

    end = strchr(start, '\"');

    if(end == NULL)
    {
        return false;
    }

    uint16_t len = end - start;

    if(len >= size)
    {
        len = size - 1;
    }

    memcpy(name, start, len);

    name[len] = 0;

    return true;
}


bool GSM_GPRS_Open(
        const char *apn,
        const char *user,
        const char *pass)
{
    char cmd[128];
    GSM_Result_t res;

    printf("OPEN GPRS\r\n");

    GSM_SendCommandEx(
                    "AT+SAPBR=0,1\r\n",
                    10000,
                    "OK",
                    "ERROR");

    HAL_Delay(3000);
    printf("STEP 1\r\n");

    res = GSM_SendCommandEx(
            "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n",
            5000,
            "OK",
            "ERROR");

    printf("Contype Result = %d\r\n", res);
    printf("%s\r\n", gsm_rx_buffer);

    if(res != GSM_RES_OK)
    {
        return false;
    }

//----------------------------------------------------//
    sprintf(cmd,
            "AT+SAPBR=3,1,\"APN\",\"%s\"\r\n",
            apn);

    res = GSM_SendCommandEx(
            cmd,
            5000,
            "OK",
            "ERROR");

    printf("Contype Result = %d\r\n", res);
    printf("%s\r\n", gsm_rx_buffer);

    if(res != GSM_RES_OK)
    {
        return false;
    }


    if((user != NULL) && (strlen(user) > 0))
    {
        sprintf(cmd,
                "AT+SAPBR=3,1,\"USER\",\"%s\"\r\n",
                user);

        if(GSM_SendCommandEx(
                cmd,
                5000,
                "OK",
                "ERROR") != GSM_RES_OK)
        {
            return false;
        }
    }

    if((pass != NULL) && (strlen(pass) > 0))
    {
        sprintf(cmd,
                "AT+SAPBR=3,1,\"PWD\",\"%s\"\r\n",
                pass);

        if(GSM_SendCommandEx(
                cmd,
                5000,
                "OK",
                "ERROR") != GSM_RES_OK)
        {
            return false;
        }
    }

    res = GSM_SendCommandEx(
    		"AT+SAPBR=1,1\r\n",                     // Open Data Connection
            30000,
            "OK",
            "ERROR");

    printf("Contype Result = %d\r\n", res);
    printf("%s\r\n", gsm_rx_buffer);

    if(res != GSM_RES_OK)
    {
        return false;
    }


    return true;
}


bool GSM_GPRS_GetIP(char *ip, uint16_t size)
{
    char *start;
    char *end;

    GSM_ClearBuffer();

    if(GSM_SendCommandEx(
            "AT+SAPBR=2,1\r\n",
            5000,
            "OK",
            "ERROR") != GSM_RES_OK)
    {
        return false;
    }

    start = strchr(gsm_rx_buffer, '\"');

    if(start == NULL)
    {
        return false;
    }

    start++;

    end = strchr(start, '\"');

    if(end == NULL)
    {
        return false;
    }

    uint16_t len = end - start;

    if(len >= size)
    {
        len = size - 1;
    }

    memcpy(ip, start, len);

    ip[len] = 0;

    return true;
}


bool GSM_GPRS_Close(void)
{
    return (GSM_SendCommandEx(
                "AT+SAPBR=0,1\r\n",
                10000,
                "OK",
                "ERROR") == GSM_RES_OK);
}




static bool GSM_HTTP_GetResult(
        GSM_HTTP_Result_t *result)
{
    char *ptr;
    int method;

    ptr = strstr(gsm_rx_buffer,
                 "+HTTPACTION:");

    if(ptr == NULL)
    {
        return false;
    }

    if(sscanf(ptr,
              "+HTTPACTION: %d,%hu,%hu",
              &method,
              &result->status,
              &result->length) != 3)
    {
        return false;
    }

    return true;
}

static bool GSM_HTTP_Open(void)
{
    GSM_SendCommandEx(
            "AT+HTTPTERM\r\n",
            2000,
            "OK",
            "ERROR");

    if(GSM_SendCommandEx(
            "AT+HTTPINIT\r\n",
            5000,
            "OK",
            "ERROR") != GSM_RES_OK)
    {
        return false;
    }

    if(GSM_SendCommandEx(
            "AT+HTTPPARA=\"CID\",1\r\n",
            3000,
            "OK",
            "ERROR") != GSM_RES_OK)
    {
        return false;
    }

    if(GSM_SendCommandEx(
                "AT+HTTPSSL=1\r\n",
                3000,
                "OK",
                "ERROR") != GSM_RES_OK)
        {
            return false;
        }

    return true;
}

static void GSM_HTTP_Close(void)
{
    GSM_SendCommandEx(
            "AT+HTTPTERM\r\n",
            2000,
            "OK",
            "ERROR");
}

static bool GSM_HTTP_SetURL(
        const char *url)
{
    char cmd[256];

    snprintf(cmd,
             sizeof(cmd),
             "AT+HTTPPARA=\"URL\",\"%s\"\r\n",
             url);

    return (GSM_SendCommandEx(
                cmd,
                5000,
                "OK",
                "ERROR") == GSM_RES_OK);
}

static bool GSM_HTTP_ReadResponse(
        char *response,
        uint16_t responseSize)
{
    GSM_ClearBuffer();

    if(GSM_SendCommandEx(
            "AT+HTTPREAD\r\n",
            10000,
            "OK",
            "ERROR") != GSM_RES_OK)
    {
        return false;
    }

    strncpy(response,
            gsm_rx_buffer,
            responseSize - 1);

    response[responseSize - 1] = '\0';

    return true;
}

bool GSM_HTTP_GET(
        const char *url,
        char *response,
        uint16_t responseSize)
{
    GSM_HTTP_Result_t httpResult;

    response[0] = '\0';

    if(!GSM_HTTP_Open())
        return false;

    if(!GSM_HTTP_SetURL(url))
    {
        GSM_HTTP_Close();
        return false;
    }

    if(GSM_SendCommandEx(
            "AT+HTTPACTION=0\r\n",
            15000,
            "+HTTPACTION:",
            "ERROR") != GSM_RES_OK)
    {
        GSM_HTTP_Close();
        return false;
    }

    if(!GSM_HTTP_GetResult(&httpResult))
    {
        GSM_HTTP_Close();
        return false;
    }

    printf("HTTP Status = %u\r\n",
           httpResult.status);

    printf("HTTP Length = %u\r\n",
           httpResult.length);

    if(httpResult.status != 200)
    {
        GSM_HTTP_Close();
        return false;
    }

    if(!GSM_HTTP_ReadResponse(
            response,
            responseSize))
    {
        GSM_HTTP_Close();
        return false;
    }

    GSM_HTTP_Close();

    return true;
}

static bool GSM_HTTP_SendData(
        const char *data,
        uint32_t timeout)
{
    char cmd[32];

    sprintf(cmd,
            "AT+HTTPDATA=%u,%lu\r\n",
            (unsigned int)strlen(data),
            timeout);

    GSM_ClearBuffer();

    HAL_UART_Transmit(
            &huart1,
            (uint8_t*)cmd,
            strlen(cmd),
            1000);

    uint32_t tickstart = HAL_GetTick();

    while((HAL_GetTick() - tickstart) < 5000)
    {
        if(strstr(gsm_rx_buffer, "DOWNLOAD"))
        {
            break;
        }
    }

    if(strstr(gsm_rx_buffer, "DOWNLOAD") == NULL)
    {
        return false;
    }

    HAL_UART_Transmit(
            &huart1,
            (uint8_t*)data,
            strlen(data),
            1000);

    tickstart = HAL_GetTick();

    while((HAL_GetTick() - tickstart) < timeout)
    {
        if(strstr(gsm_rx_buffer, "OK"))
        {
            return true;
        }
    }

    return false;
}


bool GSM_HTTP_POST(
        const char *url,
        const char *json,
        char *response,
        uint16_t responseSize)
{
    GSM_HTTP_Result_t httpResult;

    if(!GSM_HTTP_Open())
        return false;

    if(!GSM_HTTP_SetURL(url))
    {
        GSM_HTTP_Close();
        return false;
    }

    if(GSM_SendCommandEx(
            "AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n",
            3000,
            "OK",
            "ERROR") != GSM_RES_OK)
    {
        GSM_HTTP_Close();
        return false;
    }

    if(!GSM_HTTP_SendData(
            json,
            10000))
    {
        GSM_HTTP_Close();
        return false;
    }

    if(GSM_SendCommandEx(
            "AT+HTTPACTION=1\r\n",
            15000,
            "+HTTPACTION:",
            "ERROR") != GSM_RES_OK)
    {
        GSM_HTTP_Close();
        return false;
    }

    if(!GSM_HTTP_GetResult(&httpResult))
    {
        GSM_HTTP_Close();
        return false;
    }

    printf("HTTP Status = %u\r\n",
           httpResult.status);

    printf("HTTP Length = %u\r\n",
           httpResult.length);

    if(httpResult.status != 200)
    {
        GSM_HTTP_Close();
        return false;
    }

    if(!GSM_HTTP_ReadResponse(
            response,
            responseSize))
    {
        GSM_HTTP_Close();
        return false;
    }

    GSM_HTTP_Close();

    return true;
}

