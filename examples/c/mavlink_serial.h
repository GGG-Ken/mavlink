/**
 * @file mavlink_main.h
 * @author LIN
 * @brief My personal mavlink message wrapper for this project.
 * 
 * 
 * 
 * @version 0.1
 * @date 2021-08-31
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _MAVLINK_MAIN_H_
#define _MAVLINK_MAIN_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct __mavlink_message mavlink_message_t;

#define MAVLINK_SEND(msg) do { \
    char buf[256]; \
    int len = mavlink_msg_to_send_buffer(buf, msg); \
    mavlink_publish(buf, len); \
} while(0)

#define MAVLINK_SYS_ID 1
extern struct Publisher *mavlink_publisher;

int mavlink_init();

int mavlink_send(mavlink_message_t *msg);

int mavlink_publish(uint8_t *buf, int len);

void mavlink_communication(const int fd, const bool exit_on_error, const bool exit_on_idle, const bool wait_for_heartbeat);

int mavlink_get_active_connections();

void mavlink_on_connection_active();

void mavlink_on_connection_inactive();

//----- Utility

int mavlink_ocupy_usable_channel();

void mavlink_release_channel(int chan);

bool mavlink_is_channel_occupied(int chan);

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>

#define _COLOR_RED "\033[0;31m"
#define _COLOR_BLUE "\033[0;34m"
#define _COLOR_NONE "\033[1;0m"

#define LOG(format, ...) printf("[%s]: " format, __func__, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) fprintf(stderr, _COLOR_RED "[%s] Error: " format _COLOR_NONE, __func__, ##__VA_ARGS__)

#endif // _LOGGER_H_

#endif // _MAVLINK_MAIN_H_
