#include <pebble.h>
#define EXTERN
#include "health_listener.h"
#include "../ui/layer/stepcount.h"
#include "../ui/layer/heartrate.h"
#include "../settings/persist_keys.h"
#include "../ui/layer/widget.h"
#include "../settings/clay_settings.h"

static bool registered = false;

#if defined(PBL_HEALTH)
static void health_handler(HealthEventType event, void *context) {
    // Which type of event occurred?
    switch (event) {
        case HealthEventSignificantUpdate:
            APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventSignificantUpdate event");
            int hr_sig = health_service_peek_current_value(HealthMetricHeartRateBPM);
            if (hr_sig > 0) {
                s_heartrate_bpm = hr_sig;
            }
            s_step_count = health_service_sum_today(HealthMetricStepCount);

            APP_LOG(APP_LOG_LEVEL_DEBUG, "stepcount: %d", s_step_count);

            update_stepcount();
            update_heartrate();
            break;
        case HealthEventMovementUpdate:
            APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventMovementUpdate event");

            s_step_count = health_service_sum_today(HealthMetricStepCount);

            APP_LOG(APP_LOG_LEVEL_DEBUG, "stepcount: %d", s_step_count);

            update_stepcount();
            break;
        case HealthEventSleepUpdate:
            APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventSleepUpdate event");
            break;
        case HealthEventHeartRateUpdate:
            APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventHeartRateUpdate event");
            int hr_val = health_service_peek_current_value(HealthMetricHeartRateBPM);
            if (hr_val > 0) {
                s_heartrate_bpm = hr_val;

                APP_LOG(APP_LOG_LEVEL_DEBUG, "heartrate: %d", s_heartrate_bpm);

                update_heartrate();
            }
            break;
        case HealthEventMetricAlert:
            APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventMetricAlert event");
            s_step_count = health_service_sum_today(HealthMetricStepCount);

            APP_LOG(APP_LOG_LEVEL_DEBUG, "stepcount: %d", s_step_count);

            update_stepcount();
            break;
        default:
            APP_LOG(APP_LOG_LEVEL_INFO, "Ignoring unhandled event");
    }
}
#endif

static bool is_health_widget_active() {
    ClaySettings *settings = clay_get_settings();
    int row_count = settings->LayoutRowCount;
    
    for (int i = 0; i < row_count; i++) {
        int widget = 0;
        switch (i) {
            case 0: widget = settings->Row0Widget; break;
            case 1: widget = settings->Row1Widget; break;
            case 2: widget = settings->Row2Widget; break;
            case 3: widget = settings->Row3Widget; break;
            case 4: widget = settings->Row4Widget; break;
            case 5: widget = settings->Row5Widget; break;
            case 6: widget = settings->Row6Widget; break;
            default: break;
        }
        if (widget == WIDGET_STEPCOUNT || widget == WIDGET_HEARTRATE) {
            return true;
        }
    }
    return false;
}

void register_health_event_listener() {
    if (registered) {
        return;
    }

#if defined(PBL_HEALTH)
    if (!is_health_widget_active()) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Health listener not registered: no health widget active");
        return;
    }

    // Attempt to subscribe 
    if (!health_service_events_subscribe(health_handler, NULL)) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "registering health listener");

    if (persist_exists(PERSIST_KEY_LAST_HEARTRATE)) {
        s_heartrate_bpm = persist_read_int(PERSIST_KEY_LAST_HEARTRATE);
    } else {
        s_heartrate_bpm = 0;
    }

    int current_hr = health_service_peek_current_value(HealthMetricHeartRateBPM);
    if (current_hr > 0) {
        s_heartrate_bpm = current_hr;
    }

    s_step_count = health_service_peek_current_value(HealthMetricStepCount);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "stepcount: %d", s_step_count);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "heartrate: %d", s_heartrate_bpm);

    registered = true;
#else
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
#endif
}

void unregister_health_event_listener() {
    if (!registered) {
        return;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "unregistering health listener");

#if defined(PBL_HEALTH)
    health_service_events_unsubscribe();
    if (s_heartrate_bpm > 0) {
        persist_write_int(PERSIST_KEY_LAST_HEARTRATE, s_heartrate_bpm);
    }
#endif
    registered = false;
}