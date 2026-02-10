// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <platform.h>
#include <xs1.h>

#define TIMEOUT_MS XS1_TIMER_HZ // 1 second

int main(void)
{ 
    timer delay;
    unsigned time_now;
    unsigned timeout;
    delay :> time_now;
    timeout = time_now + TIMEOUT_MS;

    while (1) {
        select {
            case delay when timerafter(timeout) :> unsigned current:
                timeout += TIMEOUT_MS;
                printf("Hello, World!\n");
                break;
        }
    }
    return 0;
}
