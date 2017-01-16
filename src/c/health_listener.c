#include <pebble.h>
#define EXTERN
#include "health_listener.h"
#include "stepcount.h"

static bool registered = false;

static void health_handler(HealthEventType event, void *context) {
  // Which type of event occurred?
  switch(event) {
    case HealthEventSignificantUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventSignificantUpdate event");
      s_heartrate_bpm = health_service_peek_current_value(HealthMetricHeartRateBPM);
      s_step_count = (int) health_service_sum_today(HealthMetricStepCount);
    
      APP_LOG(APP_LOG_LEVEL_DEBUG, "stepcount: %d", s_step_count);
    
      update_stepcount();
      break;
    case HealthEventMovementUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventMovementUpdate event");

      s_step_count = (int) health_service_sum_today(HealthMetricStepCount);
    
      APP_LOG(APP_LOG_LEVEL_DEBUG, "stepcount: %d", s_step_count);
    
      update_stepcount();
      break;
    case HealthEventSleepUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventSleepUpdate event");
      break;
    case HealthEventHeartRateUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventHeartRateUpdate event");
      break;
    case HealthEventMetricAlert:
      APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventMetricAlert event");
      s_step_count = (int) health_service_sum_today(HealthMetricStepCount);
    
      APP_LOG(APP_LOG_LEVEL_DEBUG, "stepcount: %d", s_step_count);
    
      update_stepcount();
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_INFO, "Ignoring unhandled event");
      
  }
}

void register_health_event_listener() {
  if (registered) {
    return;
  }
  
  #if defined(PBL_HEALTH)
  // Attempt to subscribe 
  if(!health_service_events_subscribe(health_handler, NULL)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "registering health listener");
  s_heartrate_bpm = health_service_peek_current_value(HealthMetricHeartRateBPM);
  s_step_count = health_service_peek_current_value(HealthMetricStepCount);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "stepcount: %d", s_step_count);
  
  registered = true;
  #else
  APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  #endif
}

void unregister_health_event_listener() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "unregistering health listener");
  
  health_service_events_unsubscribe();
  registered = false;
}