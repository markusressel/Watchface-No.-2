#pragma once

// register a single tick listener for the application
// there can only be one single callback
void register_tick_listener();

// unregister the tick listener
void unregister_tick_listener();