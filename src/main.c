#include <pebble.h>

static Window *window;

static TextLayer *text_layer;

static char time_buffer[16], log_buffer[256];

static char *longitude, *latitude;

static Layer *background_layer;
static BitmapLayer *b_clockface_layer, *w_clockface_layer;
static GBitmap *b_clockface_image, *w_clockface_image;

static Layer *bar_1, *bar_2, *bar_3;

/*
bar1 2,2 141,13
bar2 2,21 141,32
bar3 2,40 141,51
bar4 2,59 141,70
bar5 2,78 141,89
bar6 2,97 141,108
bar7 2,116 141,127
bar8 2,135 141,146
*/

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

static void second_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(bar_1);
}

static void minute_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(bar_2);
}

static void hour_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(bar_3);
}

static void save_data() {
  /* Save data to persistent storage if we have it.*/
}

static void bar_1_draw(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    time_t now = time(NULL);
    int16_t width_by_time = (bounds.size.w * (now % 60)) / 60; 
  
    GRect timebound = GRect(bounds.origin.x,bounds.origin.y, width_by_time, bounds.size.h);  
  
    // Draw a black filled rectangle with sharp corners
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, timebound, 0, GCornerNone);
}

static void bar_2_draw(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    time_t now = time(NULL);
    int16_t width_by_time = (bounds.size.w * ((now/60) % 60)) / 60; 
  
    GRect timebound = GRect(bounds.origin.x,bounds.origin.y, width_by_time, bounds.size.h);  
  
    // Draw a black filled rectangle with sharp corners
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, timebound, 0, GCornerNone);
}

static void bar_3_draw(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    time_t now = time(NULL);
    int16_t width_by_time = (bounds.size.w * ((now/3600) % 60)) / 60; 
  
    GRect timebound = GRect(bounds.origin.x,bounds.origin.y, width_by_time, bounds.size.h);  
  
    // Draw a black filled rectangle with sharp corners
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, timebound, 0, GCornerNone);
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
  
  bar_1 = layer_create(GRect(2,2,140,12));
  layer_add_child(background_layer, bar_1);
  layer_set_update_proc(bar_1, bar_1_draw);    

  bar_2 = layer_create(GRect(2,21,140,12));
  layer_add_child(background_layer, bar_2);
  layer_set_update_proc(bar_2, bar_2_draw);   
  
  bar_3 = layer_create(GRect(2,41,140,12));
  layer_add_child(background_layer, bar_3);
  layer_set_update_proc(bar_3, bar_3_draw);   
  
  time_t now = time(NULL);
  struct tm *startup_time = gmtime(&now);
  second_tick_handler(startup_time, SECOND_UNIT);
  minute_tick_handler(startup_time, MINUTE_UNIT);
  hour_tick_handler(startup_time, HOUR_UNIT);
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

  tick_timer_service_subscribe(HOUR_UNIT, (TickHandler) hour_tick_handler);
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) minute_tick_handler);
  tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) second_tick_handler);

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