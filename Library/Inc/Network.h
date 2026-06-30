
#ifndef NETWORK_H_
#define NETWORK_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    NET_STATE_IDLE = 0,

    NET_STATE_CHECK_NETWORK,

    NET_STATE_OPEN_GPRS,

    NET_STATE_UPLOAD,

    NET_STATE_WAIT_RETRY

} NET_State_t;

void Network_Init(void);

void Network_Task(void);

bool Network_IsOnline(void);

#endif
