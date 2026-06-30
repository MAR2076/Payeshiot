/*
 * GPS.c
 *
 *  Created on: Jun 30, 2026
 *      Author: win
 */

#include "GPS.h"
#include "stm32f1xx_hal.h"
#include <string.h>
/*#include "stm32f1xx_hal.h"

#include <stdio.h>
#include <stdlib.h>

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
    return (GPS_DMA_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx));
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

uint8_t NMEA_Checksum(char *str)
{
    if(*str != '$')
        return 0;

    uint8_t cs = 0;
    str++; // skip $

    while(*str && *str != '*')
    {
        cs ^= (uint8_t)(*str);
        str++;
    }

    return cs;
}


uint8_t NMEA_HexToByte(char *hex)
{
    uint8_t high, low;

    high = (hex[0] > '9') ? (hex[0] - 'A' + 10) : (hex[0] - '0');
    low  = (hex[1] > '9') ? (hex[1] - 'A' + 10) : (hex[1] - '0');

    return (high << 4) | low;
}

uint8_t NMEA_Validate(char *msg)
{
    char *star = strchr(msg, '*');

    if(!star)
        return 0;

    uint8_t received = NMEA_HexToByte(star + 1);
    uint8_t calculated = NMEA_Checksum(msg);

    return (received == calculated);
}

char* NMEA_GetField(char *msg, uint8_t index)
{
    uint8_t comma = 0;

    while(*msg)
    {
        if(*msg == ',')
        {
            comma++;
            msg++;

            if(comma == index)
                return msg;
        }
        else
        {
            msg++;
        }
    }

    return NULL;
}

void GPS_ParseRMC(char *msg)
{
    if(!NMEA_Validate(msg))
        return; // discard corrupted packet

    char *p;
    GPS_Data_t GPS;

    // Time
    p = NMEA_GetField(msg,1);
    if(p)
    {
        GPS.Time.Hour   = (p[0]-'0')*10 + (p[1]-'0');
        GPS.Time.Minute = (p[2]-'0')*10 + (p[3]-'0');
        GPS.Time.Second = (p[4]-'0')*10 + (p[5]-'0');
    }

    // Status
    p = NMEA_GetField(msg,2);
    if(p)
        GPS.Status.Fix = (*p == 'A');

    // Latitude
    p = NMEA_GetField(msg,3);
    if(p)
        GPS.Position.Latitude = GPS_Convert(p);

    p = NMEA_GetField(msg,4);
    if(p && *p == 'S')
        GPS.Position.Latitude *= -1;

    // Longitude
    p = NMEA_GetField(msg,5);
    if(p)
        GPS.Position.Longitude = GPS_Convert(p);

    p = NMEA_GetField(msg,6);
    if(p && *p == 'W')
        GPS.Position.Longitude *= -1;

    // Speed (knots)
    p = NMEA_GetField(msg,7);
    if(p)
        GPS.Motion.SpeedKnots = atof(p);

    // Date
    p = NMEA_GetField(msg,9);
    if(p)
    {
        GPS.Date.Day   = (p[0]-'0')*10 + (p[1]-'0');
        GPS.Date.Month = (p[2]-'0')*10 + (p[3]-'0');
        GPS.Date.Year  = (p[4]-'0')*10 + (p[5]-'0');
    }

    // conversions
    GPS.Motion.SpeedKm = GPS.Motion.SpeedKnots * 1.852f;
    GPS.Motion.SpeedMS = GPS.Motion.SpeedKm / 3.6f;
}

void GPS_ParseGGA(char *msg)
{
    if(!NMEA_Validate(msg))
        return;

    char *p;
    GPS_Data_t GPS;

    // Fix quality
    p = NMEA_GetField(msg,6);
    if(p)
        GPS.Status.Satellites = atoi(p);

    // Satellites
    p = NMEA_GetField(msg,7);
    if(p)
        GPS.Status.Satellites = atoi(p);

    // Altitude
    p = NMEA_GetField(msg,9);
    if(p)
        GPS.Motion.Altitude = atof(p);
}

double GPS_Convert(char *val)
{
    double raw = atof(val);

    int deg = (int)(raw / 100);
    double min = raw - (deg * 100);

    return deg + (min / 60.0);
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
    uint16_t len = GPS_DMA_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);

    // TODO: process frame

    GPS_ProcessDMA(gps_dma_buffer, len);

    // restart DMA (important)
    HAL_UART_Receive_DMA(&huart2,
                         gps_dma_buffer,
                         GPS_DMA_BUFFER_SIZE);
}*/



/* External UART handle (GPS UART) */
extern UART_HandleTypeDef huart2;

/* ================= INTERNAL BUFFER ================= */

static uint8_t ring_buffer[GPS_RING_SIZE];

/* Software pointers */
static volatile uint16_t head = 0; // write
static volatile uint16_t tail = 0; // read

/* ================= DMA POSITION ================= */

static uint16_t last_dma_pos = 0;

/* ================= INIT ================= */

void GPS_Ring_Init(void)
{
    head = 0;
    tail = 0;
    last_dma_pos = 0;

    HAL_UART_Receive_DMA(&huart2,
                         ring_buffer,
                         GPS_RING_SIZE);
}

/* ================= DMA UPDATE ================= */

void GPS_Ring_DMA_Update(uint16_t dma_pos)
{
    /* DMA writes directly into ring_buffer */

    last_dma_pos = dma_pos;

    head = dma_pos;
}

/* ================= AVAILABLE DATA ================= */

uint16_t GPS_Ring_Available(void)
{
    if(head >= tail)
        return head - tail;

    return (GPS_RING_SIZE - tail) + head;
}

/* ================= READ BYTE ================= */

bool GPS_Ring_ReadByte(uint8_t *byte)
{
    if(head == tail)
        return false;

    *byte = ring_buffer[tail];

    tail++;

    if(tail >= GPS_RING_SIZE)
        tail = 0;

    return true;
}

/* ================= PROCESS ================= */

void GPS_Ring_Process(void)
{
    /* In this design, head is updated externally */

    /* Nothing heavy here → parser layer consumes data */
}


//----------Tokenize-------------//


/* internal helper */
static void add_field(GPS_Token_t *t, const char *start, uint8_t len)
{
    if(t->count < GPS_MAX_FIELDS)
    {
        t->field[t->count].ptr = start;
        t->field[t->count].len = len;
        t->count++;
    }
}

bool GPS_Tokenize(char *sentence, GPS_Token_t *out)
{
    char *p = sentence;
    char *field_start = sentence;

    uint8_t len = 0;

    out->count = 0;

    /* safety */
    if(!sentence || !out)
        return false;

    while(*p)
    {
        if(*p == ',' || *p == '*')
        {
            len = (uint8_t)(p - field_start);

            add_field(out, field_start, len);

            field_start = p + 1;
        }

        p++;
    }

    /* last field (checksum or end) */
    if(field_start < p)
    {
        len = (uint8_t)(p - field_start);
        add_field(out, field_start, len);
    }

    return (out->count > 0);
}

static inline bool field_eq(GPS_Field_t *f, const char *str)
{
    uint8_t i = 0;

    while(i < f->len && str[i])
    {
        if(f->ptr[i] != str[i])
            return false;

        i++;
    }

    return (i == f->len && str[i] == '\0');
}

static uint32_t field_to_uint(GPS_Field_t *f)
{
    uint32_t val = 0;
    uint8_t i = 0;

    while(i < f->len)
    {
        val = val * 10 + (f->ptr[i] - '0');
        i++;
    }

    return val;
}

static double field_to_float(GPS_Field_t *f)
{
    float val = 0;
    float div = 1;
    uint8_t i = 0;
    uint8_t dot = 0;

    while(i < f->len)
    {
        if(f->ptr[i] == '.')
        {
            dot = 1;
        }
        else
        {
            val = val * 10 + (f->ptr[i] - '0');

            if(dot)
                div *= 10;
        }

        i++;
    }

    return val / div;
}

//-------------GPS_Utils--------------//

uint8_t GPS_ParseUint8(const GPS_Field_t *f)
{
    uint8_t val = 0;

    for(uint8_t i = 0; i < f->len; i++)
    {
        val = (val * 10) + (f->ptr[i] - '0');
    }

    return val;
}

uint32_t GPS_ParseUint32(const GPS_Field_t *f)
{
    uint32_t val = 0;

    for(uint8_t i = 0; i < f->len; i++)
    {
        val = (val * 10) + (f->ptr[i] - '0');
    }

    return val;
}

float GPS_ParseFloat(const GPS_Field_t *f)
{
    float value = 0.0f;
    float div = 1.0f;
    uint8_t i = 0;
    uint8_t dot = 0;

    for(i = 0; i < f->len; i++)
    {
        if(f->ptr[i] == '.')
        {
            dot = 1;
            continue;
        }

        value = value * 10.0f + (f->ptr[i] - '0');

        if(dot)
            div *= 10.0f;
    }

    return value / div;
}

double GPS_ParseCoordinate(const GPS_Field_t *f)
{
    double raw = GPS_ParseFloat(f);

    int deg = (int)(raw / 100);
    double min = raw - (deg * 100);

    return deg + (min / 60.0);
}

//-------------GPS_Parser--------------//

void GPS_Parser_Init(GPS_ParserContext_t *ctx)
{
    memset(ctx, 0, sizeof(GPS_ParserContext_t));
}


static void GPS_ParseRMC(GPS_ParserContext_t *ctx, GPS_Token_t *t)
{
    /* Time */
    if(t->count > 1)
    {
        uint32_t time = 0;

        for(int i = 0; i < t->field[1].len; i++)
            time = time * 10 + (t->field[1].ptr[i] - '0');

        ctx->working.Time.Hour   = time / 10000;
        ctx->working.Time.Minute = (time / 100) % 100;
        ctx->working.Time.Second = time % 100;
    }

    /* Status */
    if(t->count > 2)
    {
        ctx->working.Status.Fix = (t->field[2].ptr[0] == 'A');
    }

    /* Latitude */
    if(t->count > 3)
    {
        ctx->working.Position.Latitude =
        	GPS_ParseCoordinate(&t->field[3]);
    }

    if(t->count > 4 && t->field[4].ptr[0] == 'S')
        ctx->working.Position.Latitude *= -1;

    /* Longitude */
    if(t->count > 5)
    {
        ctx->working.Position.Longitude =
        	GPS_ParseCoordinate(&t->field[5]);
    }

    if(t->count > 6 && t->field[6].ptr[0] == 'W')
        ctx->working.Position.Longitude *= -1;

    /* Speed (knots) */
    if(t->count > 7)
    {
        ctx->working.Motion.SpeedKnots =
            GPS_ParseFloat(&t->field[7]);
    }

    /* Date */
    if(t->count > 9)
    {
        uint32_t date = 0;

        for(int i = 0; i < t->field[9].len; i++)
            date = date * 10 + (t->field[9].ptr[i] - '0');

        ctx->working.Date.Day   = date / 10000;
        ctx->working.Date.Month = (date / 100) % 100;
        ctx->working.Date.Year  = date % 100;
    }
}

static void GPS_ParseGGA(GPS_ParserContext_t *ctx, GPS_Token_t *t)
{
    /* Fix quality */
    if(t->count > 6)
        ctx->working.Status.Quality = t->field[6].ptr[0] - '0';

    /* Satellites */
    if(t->count > 7)
        ctx->working.Status.Satellites =
            GPS_ParseUint8(&t->field[7]);

    /* HDOP */
    if(t->count > 8)
        ctx->working.Status.HDOP =
            GPS_ParseFloat(&t->field[8]);

    /* Altitude */
    if(t->count > 9)
        ctx->working.Motion.Altitude =
            GPS_ParseFloat(&t->field[9]);
}

void GPS_ParseSentence(GPS_ParserContext_t *ctx, GPS_Token_t *t)
{
    if(t->count < 1)
        return;

    /* RMC */
    if(t->field[0].len >= 6 &&
       t->field[0].ptr[3] == 'R' &&
       t->field[0].ptr[4] == 'M' &&
       t->field[0].ptr[5] == 'C')
    {
        GPS_ParseRMC(ctx, t);
        ctx->rmc_ready = 1;
    }

    /* GGA */
    else if(t->field[0].len >= 6 &&
            t->field[0].ptr[3] == 'G' &&
            t->field[0].ptr[4] == 'G' &&
            t->field[0].ptr[5] == 'A')
    {
        GPS_ParseGGA(ctx, t);
        ctx->gga_ready = 1;
    }
}

void GPS_Parser_CommitIfReady(GPS_ParserContext_t *ctx, GPS_Data_t *final)
{
    if(ctx->rmc_ready && ctx->gga_ready)
    {
        *final = ctx->working;

        ctx->rmc_ready = 0;
        ctx->gga_ready = 0;
    }
}

//-------------Final API--------------//

static GPS_ParserContext_t parser;

static GPS_Data_t gps_final;

static uint32_t last_update_tick = 0;

static uint8_t valid = 0;

void GPS_Init(void)
{
    GPS_Ring_Init();
    GPS_Parser_Init(&parser);

    memset(&gps_final, 0, sizeof(gps_final));

    valid = 0;
    last_update_tick = 0;
}

void GPS_Process(void)
{
    uint8_t c;
    static char sentence[128];
    static uint8_t idx = 0;

    GPS_Token_t tokens;
    while(GPS_Ring_ReadByte(&c))
        {
            if(c == '$')
            {
                idx = 0;
            }

            if(idx < sizeof(sentence) - 1)
            {
                sentence[idx++] = c;
            }

            if(c == '\n')
            {
                sentence[idx] = 0;
                if(GPS_Tokenize(sentence, &tokens))
                 {
                  GPS_ParseSentence(&parser, &tokens);

                  GPS_Parser_CommitIfReady(&parser, &gps_final);

                  valid = 1;
                 last_update_tick = HAL_GetTick();
                }

               idx = 0;
            }
       }
}

bool GPS_IsFix(void)
{
    return (gps_final.Status.Fix == 1);
}

bool GPS_IsDataValid(void)
{
    uint32_t now = HAL_GetTick();

    /* 2 seconds timeout */
    if((now - last_update_tick) > 2000)
    {
        valid = 0;
    }

    return valid;
}

const GPS_Data_t* GPS_GetData(void)
{
    return &gps_final;
}

void GPS_Reset(void)
{
    memset(&gps_final, 0, sizeof(gps_final));
    GPS_Parser_Init(&parser);
    valid = 0;
}


