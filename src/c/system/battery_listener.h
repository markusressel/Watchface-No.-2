#pragma once

// register a battery event listener for the application
// there can be only one single callback
void register_battery_listener();

// unregister the battery event listener
void unregister_battery_listener();

// manually fire a battery event with its current state
void force_battery_update();