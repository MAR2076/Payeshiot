/*
 * GPS.c
 *
 *  Created on: Jun 30, 2026
 *      Author: win
 */

#include "GPS.h"
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart2;

#define GPS_DMA_SIZE 128

uint8_t gps_dma_buffer[GPS_DMA_SIZE];

volatile uint16_t gps_write_pos = 0;
volatile uint16_t gps_read_pos  = 0;


void GPS_Init(void)
{
    HAL_UART_Receive_DMA(&huart2,
                         gps_dma_buffer,
                         GPS_DMA_SIZE);

    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
}

uint16_t GPS_DMA_GetWritePos(void)
{
    return (GPS_DMA_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx));
}

void GPS_Update(void)
{
    uint16_t pos = GPS_DMA_GetWritePos();

    while(gps_read_pos != pos)
    {
        char c = gps_dma_buffer[gps_read_pos];

        gps_rx_byte(c);

        gps_read_pos++;

        if(gps_read_pos >= GPS_DMA_SIZE)
            gps_read_pos = 0;
    }
}

#define GPS_LINE_SIZE 128

static char line[GPS_LINE_SIZE];
static uint16_t idx = 0;

void gps_rx_byte(char c)
{
    if(c == '$')
    {
        idx = 0; // sync on start
    }

    if(idx < GPS_LINE_SIZE - 1)
    {
        line[idx++] = c;
    }

    if(c == '\n')
    {
        line[idx] = 0;
        idx = 0;

        GPS_LineReady(line);
    }
}

void GPS_LineReady(char *line)
{
    if(strncmp(line, "$GPRMC", 6) == 0 ||
       strncmp(line, "$GNRMC", 6) == 0)
    {
        GPS_ParseRMC(line);
    }

    if(strncmp(line, "$GPGGA", 6) == 0 ||
       strncmp(line, "$GNGGA", 6) == 0)
    {
        GPS_ParseGGA(line);
    }
}

void GPS_UART_IdleCallback(void)
{
    uint16_t len = GPS_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);

    // TODO: process frame

    GPS_ProcessDMA(gps_dma_buffer, len);

    // restart DMA (important)
    HAL_UART_Receive_DMA(&huart2,
                         gps_dma_buffer,
                         GPS_DMA_BUFFER_SIZE);
}




