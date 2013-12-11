#include <pebble.h>
#define MAX_PAGE (3)

static Window *window;


//for clock
static Layer *time_layer;
static TextLayer *text_date_layer;
static TextLayer *text_day_layer;
static TextLayer *text_time_layer;

static TextLayer *stock_text_layer[2][4];
static TextLayer *last_sync_text_layer;
static AppSync sync;
static uint8_t sync_buffer[124];
static int last_sync = 0;
static bool bluetooth_connected;

typedef enum {
    Start,
    Stop,
    PrevPage,
    NextPage,
    Change,
    Refresh
} State;

enum Stock {
    STOCK1_SYMBOL = 0x0,         // TUPLE_CSTRING
    STOCK1_CHANGE = 0x1,         // TUPLE_CSTRING
    STOCK2_SYMBOL = 0x2,         // TUPLE_CSTRING
    STOCK2_CHANGE = 0x3,         // TUPLE_CSTRING
    STOCK3_SYMBOL = 0x4,         // TUPLE_CSTRING
    STOCK3_CHANGE = 0x5,         // TUPLE_CSTRING
    STOCK4_SYMBOL = 0x6,         // TUPLE_CSTRING
    STOCK4_CHANGE = 0x7          // TUPLE_CSTRING
    
};



void line_layer_update_callback(Layer *layer, GContext* ctx) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
    
    last_sync++;
    if (bluetooth_connected) {
        static char sync_text[25];
        snprintf(sync_text, sizeof(sync_text), "Last Sync: %d min(s)", last_sync);
        text_layer_set_text(last_sync_text_layer, sync_text);
    }
    
    // Need to be static because they're used by the system later.
    static char time_text[] = "00:00";
    static char date_text[] = "00";
    static char day_text[] = "XXX";
    
    char *time_format;
    
    
    // TODO: Only update the date when it's changed.
    strftime(day_text, sizeof(day_text), "%a", tick_time);
    text_layer_set_text(text_day_layer, day_text);
    
    strftime(date_text, sizeof(date_text), "%e", tick_time);
    text_layer_set_text(text_date_layer, date_text);
    
    
    if (clock_is_24h_style()) {
        time_format = "%R";
    } else {
        time_format = "%I:%M";
    }
    
    strftime(time_text, sizeof(time_text), time_format, tick_time);
    
    // Kludge to handle lack of non-padded hour format string
    // for twelve hour clock.
    if (!clock_is_24h_style() && (time_text[0] == '0')) {
        memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }
    
    text_layer_set_text(text_time_layer, time_text);
}


static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "SYNC TUPLE");
    last_sync = 0;
    static char sync_text[25];
    snprintf(sync_text, sizeof(sync_text), "Last Sync: %d min(s)", last_sync);
    text_layer_set_text(last_sync_text_layer, sync_text);
    if (!bluetooth_connected) {
        snprintf(sync_text, sizeof(sync_text), "Lost Connection");
        text_layer_set_text(last_sync_text_layer, sync_text);
    }
    
    switch (key) {
        case STOCK1_SYMBOL:
            text_layer_set_text(stock_text_layer[0][0], new_tuple->value->cstring);
            break;
        case STOCK1_CHANGE:
            text_layer_set_text(stock_text_layer[1][0], new_tuple->value->cstring);
            break;
        case STOCK2_SYMBOL:
            text_layer_set_text(stock_text_layer[0][1], new_tuple->value->cstring);
            break;
        case STOCK2_CHANGE:
            text_layer_set_text(stock_text_layer[1][1], new_tuple->value->cstring);
            break;
        case STOCK3_SYMBOL:
            text_layer_set_text(stock_text_layer[0][2], new_tuple->value->cstring);
            break;
        case STOCK3_CHANGE:
            text_layer_set_text(stock_text_layer[1][2], new_tuple->value->cstring);
            break;
        case STOCK4_SYMBOL:
            text_layer_set_text(stock_text_layer[0][3], new_tuple->value->cstring);
            break;
        case STOCK4_CHANGE:
            text_layer_set_text(stock_text_layer[1][3], new_tuple->value->cstring);
            break;
    }
    

}

static void send_stat(State state){
    
    if (bluetooth_connected) {
        
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Send State: %d", state);
        
        static char sync_text[25];
        snprintf(sync_text, sizeof(sync_text), "Status: Syncing");
        text_layer_set_text(last_sync_text_layer, sync_text);
        
        DictionaryIterator *iter;
        if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
            return;
        }
        if (dict_write_uint8(iter, 8, state) != DICT_OK) {
            return;
        }
        dict_write_end(iter);
        app_message_outbox_send();
    }
}






static void accel_tap_handler (AccelAxisType axis, int32_t direction) {
    
    //if (direction == 1) {
    
    //}else{
    //send_stat(PrevPage);
    //}
    
    //static char text[10];
    
    switch (axis){
        case ACCEL_AXIS_X:
            //snprintf(text, sizeof(text), "X %d", direction);
            //text_layer_set_text(text_layer3, text);
            send_stat(Change);
            break;
        case ACCEL_AXIS_Y:
            //snprintf(text, sizeof(text), "Y %d", direction);
            //text_layer_set_text(text_layer3, text);
            send_stat(Change);
            break;
        case ACCEL_AXIS_Z:
            //snprintf(text, sizeof(text), "Z %d", direction);
            //text_layer_set_text(text_layer3, text);
            send_stat(Change);
            break;
        default:
            break;
    }
}


static void bluetooth_connection_handler (bool connected){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Bluetooth connection");
    bluetooth_connected = connected;
    vibes_double_pulse();
    if (!connected) {
        static char sync_text[25];
        snprintf(sync_text, sizeof(sync_text), "Lost Connection");
        text_layer_set_text(last_sync_text_layer, sync_text);
    }else{
        //javascript won't be ready
        //send_stat(Start);
    }
}


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    //text_layer_set_text(text_layer, "Select");
    send_stat(Refresh);
    
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
    //text_layer_set_text(text_layer, "Select");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "LONG CLICK");

    
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    //text_layer_set_text(text_layer, "Up");
    //change_page(PrevPage);
    send_stat(Change);

}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    //text_layer_set_text(text_layer, "Down");
    //change_page(NextPage);
    send_stat(Change);
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
    window_long_click_subscribe(BUTTON_ID_SELECT,0,NULL,select_long_click_handler);
}

static void time_layer_draw(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, bounds, 5, GCornersAll);
    
}

static void window_load(Window *window) {
    //Layer *window_layer = window_get_root_layer(window);
    //GRect bounds = layer_get_bounds(window_layer);
    
    //start
    //send_stat(Start);
    
    time_layer = layer_create(GRect(0, 125, 144, 168-125));
    layer_set_update_proc(time_layer, time_layer_draw);
    layer_add_child(window_get_root_layer(window), time_layer);
    
    text_time_layer = text_layer_create(GRect(10, 125, 100, 168-125));
    text_layer_set_text_color(text_time_layer, GColorBlack);
    text_layer_set_background_color(text_time_layer, GColorWhite);
    //text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49)));
    text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    //text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_time_layer));
    //layer_add_child(time_layer, text_layer_get_layer(text_time_layer));
    
    text_day_layer = text_layer_create(GRect(100, 125, 144-100, 168-125));
    //text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);
    text_layer_set_text_color(text_day_layer, GColorBlack);
    text_layer_set_background_color(text_day_layer, GColorClear);
    text_layer_set_font(text_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(text_day_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_day_layer));
    //layer_add_child(time_layer, text_layer_get_layer(text_time_layer));
    
    text_date_layer = text_layer_create(GRect(100, 144, 144-100, 168-144));
    //text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);
    text_layer_set_text_color(text_date_layer, GColorBlack);
    text_layer_set_background_color(text_date_layer, GColorClear);
    text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_date_layer));
    //layer_add_child(time_layer, text_layer_get_layer(text_time_layer));
    
    tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
    
    
    //stock
    
    for (int i=0; i<4; i++) {
        stock_text_layer[0][i] = text_layer_create(GRect(5 , 8+i*28, 65, 28));
        stock_text_layer[1][i] = text_layer_create(GRect(68, 8+i*28, 144-68-5, 28));
        text_layer_set_font(stock_text_layer[0][i], fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
        text_layer_set_font(stock_text_layer[1][i], fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
        text_layer_set_background_color(stock_text_layer[0][i], GColorBlack);
        text_layer_set_background_color(stock_text_layer[1][i], GColorBlack);
        text_layer_set_text_color(stock_text_layer[0][i], GColorWhite);
        text_layer_set_text_color(stock_text_layer[1][i], GColorWhite);
        text_layer_set_text_alignment(stock_text_layer[1][i], GTextAlignmentRight);
    }
    
    for (int i=0; i<4; i++) {
        layer_add_child(window_get_root_layer(window), text_layer_get_layer(stock_text_layer[0][i]));
        layer_add_child(window_get_root_layer(window), text_layer_get_layer(stock_text_layer[1][i]));
    }
    
    last_sync_text_layer = text_layer_create(GRect(5, 0, 144-5, 20));
    text_layer_set_text(last_sync_text_layer, "Status: Syncing");
    text_layer_set_text_color(last_sync_text_layer, GColorWhite);
    text_layer_set_background_color(last_sync_text_layer, GColorClear);
    text_layer_set_font(last_sync_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(last_sync_text_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(last_sync_text_layer));
    
    //have to put it in the end
    send_stat(Start);
}

static void window_unload(Window *window) {
    //start
     send_stat(Stop);
    //layer_destroy(line_layer);
    for (int i=0; i<4; i++) {
        text_layer_destroy(stock_text_layer[0][i]);
        text_layer_destroy(stock_text_layer[1][i]);
    }
    text_layer_destroy(last_sync_text_layer);
    text_layer_destroy(text_date_layer);
    text_layer_destroy(text_day_layer);
    text_layer_destroy(text_time_layer);
    layer_destroy(time_layer);
    
    
}

static void init(void) {
    window = window_create();
    window_set_background_color(window, GColorBlack);
    window_set_fullscreen(window, true);
    window_set_click_config_provider(window, click_config_provider);
    

       
    accel_tap_service_subscribe(&accel_tap_handler);
    bluetooth_connected = bluetooth_connection_service_peek();
    bluetooth_connection_service_subscribe(bluetooth_connection_handler);
    
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    
    
    const int inbound_size = 124;
    const int outbound_size = 256;
    app_message_open(inbound_size, outbound_size);
    
    Tuplet initial_values[] = {
        //TupletInteger(STOCK1_PRICE, (uint8_t) 30),
        TupletCString(STOCK1_SYMBOL, "XXXX"),
        TupletCString(STOCK1_CHANGE, "88.88"),
        TupletCString(STOCK2_SYMBOL, "XXXX"),
        TupletCString(STOCK2_CHANGE, "88.88"),
        TupletCString(STOCK3_SYMBOL, "XXXX"),
        TupletCString(STOCK3_CHANGE, "88.88"),
        TupletCString(STOCK4_SYMBOL, "XXXX"),
        TupletCString(STOCK4_CHANGE, "88.88"),
    };
    
    app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
                  sync_tuple_changed_callback, sync_error_callback, NULL);
    
    const bool animated = true;
    window_stack_push(window, animated);


}

static void deinit(void) {

    tick_timer_service_unsubscribe();
    accel_tap_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();
    app_sync_deinit(&sync);
    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
