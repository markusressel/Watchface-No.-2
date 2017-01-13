#pragma once

#ifndef EXTERN
#define EXTERN extern
#endif

// current battery level
EXTERN int s_battery_level;
EXTERN bool s_battery_charging, s_battery_cable_connected;