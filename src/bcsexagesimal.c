/*
 
 BCSexagesimal
 
 https://github.com/keelanc/sexagesimal
 
 Based on the Pebble team's Just A Bit
 and https://github.com/keelanc/a_bit_different
 
 */

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x9C, 0x49, 0xD4, 0x91, 0x2B, 0xB5, 0x42, 0x64, 0xB5, 0x23, 0xF1, 0xD1, 0x27, 0x08, 0x43, 0x33 }
PBL_APP_INFO(MY_UUID,
             "BCSexagesimal", "keelanchufor.com",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
Layer display_layer;
TextLayer text_day_layer;
TextLayer text_date_layer;

static char day_text[] = "XXX";
static char date_text[] = "XXX\n00";


#define CIRCLE_RADIUS 12
#define CIRCLE_LINE_THICKNESS 2

void draw_cell(GContext* ctx, GPoint center, bool filled) {
	// Each "cell" represents a binary digit or 0 or 1.
	
	graphics_context_set_fill_color(ctx, GColorWhite);
	
	graphics_fill_circle(ctx, center, CIRCLE_RADIUS);
	
	if (!filled) {
		// This is our ghetto way of drawing circles with a line thickness
		// of more than a single pixel.
		graphics_context_set_fill_color(ctx, GColorBlack);
		
		graphics_fill_circle(ctx, center, CIRCLE_RADIUS - CIRCLE_LINE_THICKNESS);
	}
	
}

#define CELLS_PER_ROW 3
#define CELLS_PER_COLUMN 6

#define CIRCLE_PADDING 14 - CIRCLE_RADIUS // Number of padding pixels on each side
#define CELL_SIZE (2 * (CIRCLE_RADIUS + CIRCLE_PADDING)) // One "cell" is the square that contains the circle.
#define TOP_PADDING (168 - (CELLS_PER_COLUMN * CELL_SIZE))
#define LEFT_PADDING (144 - (CELLS_PER_ROW * CELL_SIZE))


GPoint get_center_point_from_cell_location(unsigned short x, unsigned short y) {
	// Cell location (0,0) is upper left, location (5,3) is lower right.
	return GPoint(LEFT_PADDING + (CELL_SIZE/2) + (CELL_SIZE * x),
				  TOP_PADDING + (CELL_SIZE/2) + (CELL_SIZE * y));
}


void draw_cell_column_for_sexa(GContext* ctx, unsigned short digit, unsigned short cell_column, unsigned short default_max_rows) {
	// Converts the supplied decimal digit into Binary Coded Decimal form and
	// then draws a row of cells on screen--'1' binary values are filled, '0' binary values are not filled.
	// `default_max_rows` helps start drawing from the very bottom
	for (int cell_row_index = default_max_rows; cell_row_index >= 0; cell_row_index--) {
		draw_cell(ctx, get_center_point_from_cell_location(cell_column, cell_row_index), (digit >> (default_max_rows - cell_row_index)) & 0x1);
	}
}


// The cell column offsets for each digit
#define HOURS_COL 0
#define MINUTES_COL 1
#define SECONDS_COL 2

#define DEFAULT_MAX_ROWS (CELLS_PER_COLUMN - 1)


unsigned short get_display_hour(unsigned short hour) {
	
	if (clock_is_24h_style()) {
		return hour;
	}
	
	// convert 24hr to 12hr
	unsigned short display_hour = hour % 12;
	// Converts "0" to "12"
	return display_hour ? display_hour : 12;
	
}


void display_layer_update_callback(Layer *me, GContext* ctx) {
	(void)me;
	
	PblTm t;
	get_time(&t);
	
	unsigned short display_hour = get_display_hour(t.tm_hour);
	
	draw_cell_column_for_sexa(ctx, display_hour, HOURS_COL, DEFAULT_MAX_ROWS);
	
	draw_cell_column_for_sexa(ctx, t.tm_min, MINUTES_COL, DEFAULT_MAX_ROWS);
	
	draw_cell_column_for_sexa(ctx, t.tm_sec, SECONDS_COL, DEFAULT_MAX_ROWS);
	
}


void update_watchface(PblTm* t) {
	
	string_format_time(day_text, sizeof(day_text), "%a", t);
	string_format_time(date_text, sizeof(date_text), "%b\n%e", t);
	text_layer_set_text(&text_day_layer, day_text);
	text_layer_set_text(&text_date_layer, date_text);
	
}


void handle_init(AppContextRef ctx) {
	// initializing app
	
	(void)ctx;
	
	window_init(&window, "BCSexagesimal watch");
	window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, GColorBlack);
	
	// init the bit layer
	layer_init(&display_layer, window.layer.frame);
	display_layer.update_proc = &display_layer_update_callback; // REF: .update_proc points to a function that draws the layer
	layer_add_child(&window.layer, &display_layer);
	
	
	resource_init_current_app(&APP_RESOURCES);
	
	// init the day text layer
	text_layer_init(&text_day_layer, GRect(0, 0, LEFT_PADDING, 80));
	text_layer_set_font(&text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GOTHAM_BOLD_24)));
	text_layer_set_text_color(&text_day_layer, GColorWhite);
	text_layer_set_background_color(&text_day_layer, GColorClear);
	layer_add_child(&window.layer, &text_day_layer.layer);
	
	// init the date text layer
	text_layer_init(&text_date_layer, GRect(0, 20, LEFT_PADDING, 80));
	text_layer_set_font(&text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GOTHAM_LIGHT_24)));
	text_layer_set_text_color(&text_date_layer, GColorWhite);
	text_layer_set_background_color(&text_date_layer, GColorClear);
	layer_add_child(&window.layer, &text_date_layer.layer);
	
	// load watchface immediately
	PblTm t;
	get_time(&t);
	update_watchface(&t);
	
}


void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
	// doing something on the second
	
	(void)ctx;
	
	update_watchface(t->tick_time);
	
}


void pbl_main(void *params) {
	PebbleAppHandlers handlers = {
		.init_handler = &handle_init,
		.tick_info = {
			.tick_handler = &handle_tick,
			.tick_units = SECOND_UNIT
		}
	};
	app_event_loop(params, &handlers);
}
