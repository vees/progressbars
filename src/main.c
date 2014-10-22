#include <pebble.h>

static Window *window;

static TextLayer *text_layer;

static char time_buffer[16], log_buffer[256];

static char *longitude, *latitude;
//int latitude, longitude;

static void in_received_handler(DictionaryIterator *message, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "PEBBLE: call to in_received_handler");
  char *status = (char*)dict_find(message, 0)->value;
  if(strcmp(status, "location") == 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "PEBBLE: Got location");
    longitude = (char*)dict_find(message, 1)->value;
    latitude = (char*)dict_find(message, 2)->value;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "PEBBLE: got %s", latitude);
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  snprintf(log_buffer, 64, "PEBBLE: Failed to Send: reason %d", (int) reason);
  APP_LOG(APP_LOG_LEVEL_DEBUG, log_buffer);
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "PEBBLE: Message sent successfully.");
}


static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "PEBBLE: App Message Dropped!");
}

static void minute_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "PEBBLE: Tick");
  // time_t now = time(NULL);
  //strftime(time_buffer, sizeof("00:00"), "%H:%M", tick_time);
  snprintf(time_buffer, 7, latitude);
  text_layer_set_text(text_layer, time_buffer);
}

static void init() {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  
  // Message registration
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_open(256, 256);
  
  latitude = "0"; 
  longitude = "0";

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  text_layer = text_layer_create((GRect){ .origin = { 0, 30 }, .size = bounds.size });
  text_layer_set_text(text_layer, clock_is_24h_style() ? "Mode:\n24" : "Mode:\n12");
  // Font reference at https://developer.getpebble.com/blog/2013/07/24/Using-Pebble-System-Fonts/
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) minute_tick_handler);
  //tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) second_tick_handler);
}

static void deinit() {
  text_layer_destroy(text_layer);
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}