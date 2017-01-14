#pragma once

#ifndef EXTERN
#define EXTERN extern
#endif

// current heartrate
EXTERN int s_heartrate_bpm;
EXTERN int s_step_count;

// register a single event listener for the application
// there can only be one single callback
void register_health_event_listener();

// unregister the event listener
void unregister_health_event_listener();