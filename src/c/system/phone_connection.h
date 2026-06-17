#pragma once

#include <stdbool.h>

/**
 * register a phone connection event listener for the application
 * there can be only one single callback
 */
void register_phone_connection_listener();

/**
 * unregister the phone connection event listener
 */
void unregister_phone_connection_listener();

/**
 * @return the current phone connection state
 */
bool phone_connection_is_connected();

/**
 * manually fire a phone connection event with its current state
 */
void force_phone_connection_update();
