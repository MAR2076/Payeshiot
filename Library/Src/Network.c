


#include "Network.h"
#include "SIM800C.h"
#include <stdio.h>

static NET_State_t netState;

static uint32_t retryTick;

static bool networkOnline = false;

#define NETWORK_RETRY_TIME_MS    30000UL
#define APN_NAME   "mcinet"


void Network_Init(void)
{
    netState = NET_STATE_CHECK_NETWORK;

    retryTick = 0;

    networkOnline = false;
}


bool Network_IsOnline(void)
{
    return networkOnline;
}


void Network_Task(void)
{
    switch(netState)
    {
    case NET_STATE_IDLE:

        break;

    case NET_STATE_CHECK_NETWORK:

        printf("CHECK NETWORK\r\n");

        if(GSM_IsRegistered())
        {
            printf("NETWORK OK\r\n");

            netState = NET_STATE_OPEN_GPRS;
        }
        else
        {
            printf("NETWORK FAIL\r\n");

            retryTick = HAL_GetTick();

            netState = NET_STATE_WAIT_RETRY;
        }

        break;

    case NET_STATE_OPEN_GPRS:

        printf("OPEN GPRS\r\n");

        if(GSM_GPRS_Open(APN_NAME, "", ""))
        {
            printf("GPRS OK\r\n");

            networkOnline = true;

            netState = NET_STATE_UPLOAD;
        }
        else
        {
            printf("GPRS FAIL\r\n");

            networkOnline = false;

            retryTick = HAL_GetTick();

            netState = NET_STATE_WAIT_RETRY;
        }

        break;

    case NET_STATE_UPLOAD:

    	networkOnline = true;

        break;

    case NET_STATE_WAIT_RETRY:

        networkOnline = false;

        if((HAL_GetTick() - retryTick) >
                NETWORK_RETRY_TIME_MS)
        {
            netState = NET_STATE_CHECK_NETWORK;
        }

        break;

    default:

        netState = NET_STATE_CHECK_NETWORK;

        break;
    }
}

