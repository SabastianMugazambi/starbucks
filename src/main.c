#include <pebble.h>
#include "main.h"


static Window *window;
static TextLayer *text_1_layer;
static TextLayer *text_2_layer;
static TextLayer *label_layer;
static TextLayer *time_layer;

static AppSync sync;
static uint8_t sync_buffer[64];

enum {
  OUR_LOCATION = 0x0,
  OUR_THEFT = 0
};

void sync_tuple_changed_callback(const uint32_t key,
        const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  
  switch (key) {
    case OUR_LOCATION:
    
      
      text_layer_set_text(text_1_layer, new_tuple->value->cstring);
      break;
  }  
}

// http://stackoverflow.com/questions/21150193/logging-enums-on-the-pebble-watch/21172222#21172222
char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "... Sync Error: %s", translate_error(app_message_error));
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00";

  strftime(time_text, sizeof(time_text), "%I:%M", tick_time);
  text_layer_set_text(time_layer, time_text);
}


static void init_clock(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  time_layer = text_layer_create(GRect(0, 20, bounds.size.w, bounds.size.h-100));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);
  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
}

static void init_location_search(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  label_layer = text_layer_create((GRect) { .origin = { 0, 90 }, .size = { bounds.size.w, 100 } });
  text_layer_set_text(label_layer, "Zues Safety Report");
  text_layer_set_text_color(label_layer, GColorWhite);
  text_layer_set_text_alignment(label_layer, GTextAlignmentCenter);
  text_layer_set_background_color(label_layer, GColorClear);
  text_layer_set_font(label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(label_layer));

  text_1_layer = text_layer_create((GRect) { .origin = { 0, 115 }, .size = { bounds.size.w, bounds.size.h } });
  text_layer_set_text(text_1_layer, "Loading...");
  text_layer_set_text_color(text_1_layer, GColorWhite);
  text_layer_set_text_alignment(text_1_layer, GTextAlignmentCenter);
  text_layer_set_background_color(text_1_layer, GColorClear);
  text_layer_set_overflow_mode(text_1_layer, GTextOverflowModeFill);
  text_layer_set_font(text_1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_1_layer));
  
  
  Tuplet initial_values[] = {
     TupletCString(OUR_LOCATION, "Loading...")
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
}

static void window_load(Window *window) {
  init_location_search(window);
  init_clock(window);
}

static void window_unload(Window *window) {
  text_layer_destroy(text_1_layer);
  text_layer_destroy(text_2_layer);
  text_layer_destroy(label_layer);
  text_layer_destroy(time_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  app_message_open(64, 64);
  
  const bool animated = true;
  window_stack_push(window, animated);
  window_set_background_color(window, GColorBlack);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}

//the butons???????????????????????????????????????????
void send_int(uint8_t key, uint8_t cmd)
{
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
 
    Tuplet value = TupletInteger(key, cmd);
    dict_write_tuplet(iter, &value);
 
    app_message_outbox_send();
}


/*
Call the send function with an int value for every button.
1-up
2-long up
3-select
4-long select
5-down
6-long down
*/

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_int(0, 3);  
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_int(0, 1);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_int(0, 5);
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_int(0, 4);
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_int(0, 2);
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  send_int(0, 6);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 700, down_long_click_handler, NULL);
}


