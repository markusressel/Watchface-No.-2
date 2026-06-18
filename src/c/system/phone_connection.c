#include <pebble.h>
#include "phone_connection.h"
#include "../ui/layer/status.h"
#include "../ui/layer/weather.h"

static bool s_registered = false;
static bool s_is_connected = false;

static void connection_handler(bool connected) {
    if (s_is_connected != connected) {
        s_is_connected = connected;
        status_layer_update();

        if (connected) {
            weather_request_update();
        }
    }
}

void register_phone_connection_listener() {
    if (s_registered) {
        return;
    }

    s_is_connected = connection_service_peek_pebble_app_connection();
    connection_service_subscribe((ConnectionHandlers){
        .pebble_app_connection_handler = connection_handler
    });
    s_registered = true;
}

void unregister_phone_connection_listener() {
    if (!s_registered) {
        return;
    }

    connection_service_unsubscribe();
    s_registered = false;
}

bool phone_connection_is_connected() {
    return s_is_connected;
}

void force_phone_connection_update() {
    s_is_connected = connection_service_peek_pebble_app_connection();
    status_layer_update();
}
