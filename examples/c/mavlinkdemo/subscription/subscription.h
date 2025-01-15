#ifndef _SUBSCRIPTION_H_
#define _SUBSCRIPTION_H_

#include <stdbool.h>
#include <stdint.h>

int subscription_init();

int publish(uint8_t *buf, int len);

int subscriber_read(int index, uint8_t *buf, int len);

void subscriber_reset(int index);

int subscriber_set_active(int index, bool active);

bool subscriber_available(int index);

#endif // _SUBSCRIPTION_H_