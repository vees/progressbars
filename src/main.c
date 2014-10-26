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
  //snprintf(time_buffer, 7, latitude);
  //text_layer_set_text(text_layer, time_buffer);
}

static Layer *background_layer;
static BitmapLayer *b_clockface_layer, *w_clockface_layer;
static GBitmap *b_clockface_image, *w_clockface_image;

static void save_data() {
  /* Save data to persistent storage if we have it.*/
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create background clock including the daylight path.
  background_layer = layer_create(bounds);
  layer_add_child(window_layer, background_layer);

  w_clockface_image = gbitmap_create_with_resource(RESOURCE_ID_CLOCKFACE);
  w_clockface_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(w_clockface_layer, w_clockface_image);
  bitmap_layer_set_background_color(w_clockface_layer, GColorClear);
  bitmap_layer_set_compositing_mode(w_clockface_layer, GCompOpAssign);
  layer_add_child(background_layer, bitmap_layer_get_layer(w_clockface_layer));
}

static void window_unload(Window *window) {
  // Save data to persistent storage
  save_data();

  // Destroy TextLayers.

  // Destroy GBitmaps.
  gbitmap_destroy(w_clockface_image);

  // Destroy BitmapLayrs.
  bitmap_layer_destroy(w_clockface_layer);

  // Destroy Layers.
}


static void init() {
  // Initialize window.
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
    
  //window = window_create();
  window_stack_push(window, true /* Animated */);
  
  // Message registration
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_open(256, 256);
    
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