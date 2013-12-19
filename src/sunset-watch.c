#include <pebble.h>
#include "my_math.h"
#include "suncalc.h"
#include "config.h"

static Window *window;
static Layer *face_layer;
static Layer *hand_layer;
static Layer *sunlight_layer;
static Layer *sunrise_sunset_text_layer;
static Layer *moon_layer;
int current_sunrise_sunset_day = -1;
int current_moon_day = -1;
float sunriseTime;
float sunsetTime;
int phase;

void adjustTimezone(float* time) 
{
  int corrected_time = 12 + TIMEZONE;
  *time += corrected_time;
    if (*time > 24) *time -= 24;
    if (*time < 0) *time += 24;
}

// results are pretty awful for `outline_pixel' values larger than 2...
static void draw_outlined_text(GContext* ctx, char* text, GFont font, GRect rect, GTextOverflowMode mode, GTextAlignment alignment, int outline_pixels, bool inverted) {
  (inverted) ? graphics_context_set_text_color(ctx, GColorBlack) :
    graphics_context_set_text_color(ctx, GColorWhite);

  GRect volatile_rect = rect;
  for (int i=0; i<outline_pixels; i++) {
    volatile_rect.origin.x+=1;
    graphics_draw_text(ctx,
		       text,
		       font,
		       volatile_rect,
		       mode,
		       alignment,
		       NULL);
  }
  volatile_rect = rect;
  for (int i=0; i<outline_pixels; i++) {
    volatile_rect.origin.y+=1;
    graphics_draw_text(ctx,
		       text,
		       font,
		       volatile_rect,
		       mode,
		       alignment,
		       NULL);
  }
  volatile_rect = rect;
  for (int i=0; i<outline_pixels; i++) {
    volatile_rect.origin.x-=1;
    graphics_draw_text(ctx,
		       text,
		       font,
		       volatile_rect,
		       mode,
		       alignment,
		       NULL);
  }
  volatile_rect = rect;
  for (int i=0; i<outline_pixels; i++) {
    volatile_rect.origin.y-=1;
    graphics_draw_text(ctx,
		       text,
		       font,
		       volatile_rect,
		       mode,
		       alignment,
		       NULL);
  }
  (inverted) ? graphics_context_set_text_color(ctx, GColorWhite) :
    graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx,
		     text,
		     font,
		     rect,
		     mode,
		     alignment,
		     NULL);
}

static void draw_dot(GContext* ctx, GPoint center, int radius) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx,GColorBlack);
  graphics_fill_circle(ctx, center, radius);
  graphics_draw_circle(ctx, center, radius);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_circle(ctx, center, radius+1);
}

static void face_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);


  /*******************************
    DRAW PRIMITIVES FOR THIS LAYER
  *********************************/
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_circle(ctx, center, 65);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_circle(ctx, center, 66);
  // yes, I'm really doing this...
  for (int i=67;i<72;i++) {
      graphics_draw_circle(ctx, center, i);
  }
  center.y+=1;
  for (int i=67;i<72;i++) {
      graphics_draw_circle(ctx, center, i);
  }
  center.y-=1;
  graphics_context_set_stroke_color(ctx, GColorBlack);
  for (int i=72;i<120;i++) {
      graphics_draw_circle(ctx, center, i);
  }
  center.y+=1;
  for (int i=72;i<120;i++) {
      graphics_draw_circle(ctx, center, i);
  }

  float rad_factor = M_PI / 180;
  float deg_step;

  // draw semi major hour marks
  deg_step = 45;
  for (int i=0;i<8;i++) {
    float current_rads = rad_factor * (deg_step * i);
    float x = 72 + 60 * (my_cos(current_rads));
    float y = 84 + 60 * (my_sin(current_rads));
    GPoint current_point;
    current_point.x = (int16_t)x;
    current_point.y = (int16_t)y;
    draw_dot(ctx, current_point, 3);
  }
  // draw each hour mark
  deg_step = 15;
  for (int i=0;i<24;i++) {
    float current_rads = rad_factor * (deg_step * i);
    float x = 72 + 60 * (my_cos(current_rads));
    float y = 84 + 60 * (my_sin(current_rads));
    GPoint current_point;
    current_point.x = (int16_t)x;
    current_point.y = (int16_t)y;
    draw_dot(ctx, current_point, 1);
  }
  // draw major hour marks
  deg_step = 90;
  for (int i=0;i<4;i++) {
    float current_rads = rad_factor * (deg_step * i);
    float x = 72 + 60 * (my_cos(current_rads));
    float y = 84 + 60 * (my_sin(current_rads));
    GPoint current_point;
    current_point.x = (int16_t)x;
    current_point.y = (int16_t)y;
    draw_dot(ctx, current_point, 4);
  }

  /*************************
    DRAW TEXT FOR THIS LAYER
  **************************/

  // draw hour text
  struct tm fake_time;
  char *time_format = "%l";  
  char hour_text[] = "12";

  fake_time.tm_hour = 6;
  strftime(hour_text, sizeof(hour_text), time_format, &fake_time);

  // draw semi major hour text
  fake_time.tm_hour = 6;
  strftime(hour_text, sizeof(hour_text), time_format, &fake_time);

  deg_step = 45;
  for (int i=0;i<8;i++) {
    float current_rads = rad_factor * (deg_step * i);
    float x = 72 + 48 * (my_cos(current_rads));
    float y = 84 + 48 * (my_sin(current_rads));
    GPoint current_point;
    current_point.x = (int16_t)x;
    current_point.y = (int16_t)y;

    draw_outlined_text(ctx,
  		       hour_text,
  		       fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
  		       GRect(current_point.x-10, current_point.y-12, 20, 20),
  		       GTextOverflowModeWordWrap,
  		       GTextAlignmentCenter,
  		       1,
  		       false);
    (fake_time.tm_hour == 12) ? fake_time.tm_hour = 0 : true;
    fake_time.tm_hour += 3;
    strftime(hour_text, sizeof(hour_text), time_format, &fake_time);
  }
}

static const GPathInfo p_hour_hand_info = {
  .num_points = 6,
  .points = (GPoint []) {{4,0},{0,8},{-4,0},{-3,-60},{0,-65},{3,-60}}
};

  static const GPathInfo p_second_hand_info = {
    .num_points = 6,
    .points = (GPoint []) {{4,0},{0,8},{-4,0},{-3,-60},{0,-65},{3,-60}}
  };

static void hand_layer_update_proc(Layer* layer, GContext* ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  static GPath *p_hour_hand = NULL;
  static GPath *p_second_hand = NULL;

  time_t now_epoch = time(NULL);
  struct tm *now = localtime(&now_epoch);

  // NEXT LINE FOR TESTING ONLY
  //  time.tm_hour = 12;

  int32_t second_angle = now->tm_sec * 6;

  // 24 hour hand
  int32_t hour_angle = (((now->tm_hour * 60) + now->tm_min) / 4) + 180;

  // draw the hour hand
  graphics_context_set_stroke_color(ctx, GColorWhite);
  p_hour_hand = gpath_create(&p_hour_hand_info);
  graphics_context_set_fill_color(ctx, GColorBlack);
  gpath_move_to(p_hour_hand, center);
  gpath_rotate_to(p_hour_hand, TRIG_MAX_ANGLE / 360 * hour_angle);
  gpath_draw_filled(ctx, p_hour_hand);
  gpath_draw_outline(ctx, p_hour_hand);

  // draw the second hand
  graphics_context_set_stroke_color(ctx, GColorBlack);
  p_second_hand = gpath_create(&p_second_hand_info);
  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_move_to(p_second_hand, center);
  gpath_rotate_to(p_second_hand, TRIG_MAX_ANGLE / 360 * second_angle);
  gpath_draw_filled(ctx, p_second_hand);
  gpath_draw_outline(ctx, p_second_hand);
}

GPathInfo sun_path_moon_mask_info = {
  5,
  (GPoint []) {
    {0, 0},
    {-73, +84}, //replaced by sunrise angle
    {-73, -84}, //top left
    {+73, -84}, //top right
    {+73, +84}, //replaced by sunset angle
  }
};

GPathInfo sun_path_info = {
  5,
  (GPoint []) {
    {0, 0},
    {-73, +84}, //replaced by sunrise angle
    {-73, +84}, //bottom left
    {+73, +84}, //bottom right
    {+73, +84}, //replaced by sunset angle
  }
};

static void sunlight_layer_update_proc(Layer* layer, GContext* ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  time_t now_epoch = time(NULL);
  struct tm *time = localtime(&now_epoch);
  /* struct tm *sunrise_time = localtime(&now_epoch); */
  /* struct tm *sunset_time = localtime(&now_epoch); */

  float sunriseTime = calcSunRise(time->tm_year, time->tm_mon+1, time->tm_mday, LATITUDE, LONGITUDE, 91.0f);
  float sunsetTime = calcSunSet(time->tm_year, time->tm_mon+1, time->tm_mday, LATITUDE, LONGITUDE, 91.0f);

  adjustTimezone(&sunriseTime);
  adjustTimezone(&sunsetTime);

  sun_path_info.points[1].x = (int16_t)(my_sin(sunriseTime/24 * M_PI * 2) * 120);
  sun_path_info.points[1].y = -(int16_t)(my_cos(sunriseTime/24 * M_PI * 2) * 120);

  sun_path_info.points[4].x = (int16_t)(my_sin(sunsetTime/24 * M_PI * 2) * 120);
  sun_path_info.points[4].y = -(int16_t)(my_cos(sunsetTime/24 * M_PI * 2) * 120);

  sun_path_moon_mask_info.points[1].x = (int16_t)(my_sin(sunriseTime/24 * M_PI * 2) * 120);
  sun_path_moon_mask_info.points[1].y = -(int16_t)(my_cos(sunriseTime/24 * M_PI * 2) * 120);

  sun_path_moon_mask_info.points[4].x = (int16_t)(my_sin(sunsetTime/24 * M_PI * 2) * 120);
  sun_path_moon_mask_info.points[4].y = -(int16_t)(my_cos(sunsetTime/24 * M_PI * 2) * 120);

  struct GPath *sun_path;
  sun_path = gpath_create(&sun_path_info);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  gpath_move_to(sun_path, center);
  gpath_draw_outline(ctx, sun_path);
  gpath_draw_filled(ctx, sun_path);
}

static void sunrise_sunset_text_layer_update_proc(Layer* layer, GContext* ctx) {
  time_t now_epoch = time(NULL);
  struct tm *now = localtime(&now_epoch);
  struct tm *sunrise_time = localtime(&now_epoch);
  struct tm *sunset_time = localtime(&now_epoch);
  static char sunrise_text[] = "     ";
  static char sunset_text[] = "     ";
  static char time_text[] = "     ";
  static char month_text[] = "   ";
  static char day_text[] = "  ";
  char *time_format = "%l:%M";
  char *month_format = "%b";
  char *day_format = "%e";

  // don't calculate these if they've already been done for the day.
  if (current_sunrise_sunset_day != now->tm_mday) {
    sunriseTime = calcSunRise(now->tm_year, now->tm_mon+1, now->tm_mday, LATITUDE, LONGITUDE, 91.0f);
    sunsetTime = calcSunSet(now->tm_year, now->tm_mon+1, now->tm_mday, LATITUDE, LONGITUDE, 91.0f);
    adjustTimezone(&sunriseTime);
    adjustTimezone(&sunsetTime);
    // don't calculate them again until tomorrow
    current_sunrise_sunset_day = now->tm_mday;
  }

  // draw current time (modify for 12-hour display)
  (now->tm_hour > 12) ? now->tm_hour-=12 : true;
  strftime(time_text, sizeof(time_text), time_format, now);
  draw_outlined_text(ctx,
		     time_text,
		     fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
		     GRect(42, 47, 64, 32),
  		     GTextOverflowModeWordWrap,
  		     GTextAlignmentCenter,
		     1,
		     false);

  // print sunrise/sunset times (modify for 12-hour display)
  (sunrise_time->tm_hour > 12) ? sunrise_time->tm_hour-=12 : true;
  (sunset_time->tm_hour > 12) ? sunset_time->tm_hour-=12 : true;
  sunrise_time->tm_min = (int)(60*(sunriseTime-((int)(sunriseTime))));
  sunrise_time->tm_hour = (int)sunriseTime - 12;
  strftime(sunrise_text, sizeof(sunrise_text), time_format, sunrise_time);
  sunset_time->tm_min = (int)(60*(sunsetTime-((int)(sunsetTime))));
  sunset_time->tm_hour = (int)sunsetTime + 12;
  strftime(sunset_text, sizeof(sunset_text), time_format, sunset_time);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx,
		     sunrise_text,
		     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
		     GRect(3, 145, 144-3, 168-145),
		     GTextOverflowModeWordWrap,
		     GTextAlignmentLeft,
		     NULL);
  graphics_draw_text(ctx,
		     sunset_text,
		     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
		     GRect(3, 145, 144-3, 168-145),
		     GTextOverflowModeWordWrap,
		     GTextAlignmentRight,
		     NULL);

  //draw current date month
  strftime(month_text, sizeof(month_text), month_format, now);
  draw_outlined_text(ctx,
		     month_text,
		     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
		     GRect(3, 0, 144-3, 32),
  		     GTextOverflowModeWordWrap,
  		     GTextAlignmentLeft,
		     0,
		     true);
  //draw current date day
  strftime(day_text, sizeof(day_text), day_format, now);
  draw_outlined_text(ctx,
		     day_text,
		     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
		     GRect(3, 0, 144-13, 32),
  		     GTextOverflowModeWordWrap,
  		     GTextAlignmentRight,
		     0,
		     true);
}

int moon_phase(struct tm *time) {
  int y,m;
  double jd;
  int jdn;
  y = time->tm_year + 1900;
  m = time->tm_mon + 1;
  jdn = time->tm_mday-32075+1461*(y+4800+(m-14)/12)/4+367*(m-2-(m-14)/12*12)/12-3*((y+4900+(m-14)/12)/100)/4;
  jd = jdn-2451550.1;
  jd /= 29.530588853;
  jd -= (int)jd;
  return (int)(jd*27 + 0.5); /* scale fraction from 0-27 and round by adding 0.5 */
}

static void moon_layer_update_proc(Layer* layer, GContext* ctx) {
  time_t now_epoch = time(NULL);
  struct tm *now = localtime(&now_epoch);

  // don't do any calculations if they've already been done for today.
  if (current_moon_day != now->tm_mday) {
    phase = moon_phase(now);
    // we don't have to calculate this again until tomorrow...
    current_moon_day = now->tm_mday;
  }

  int moon_y = 108;  // y-axis position of the moon's center
  int moon_r = 15;   // radius of the moon

  // draw the moon...
  if (phase != 27) {
    graphics_context_set_fill_color(ctx,GColorWhite);
    graphics_fill_circle(ctx, GPoint(72,moon_y), moon_r);
  }
  if (phase == 27 || phase == 0) {
    graphics_context_set_stroke_color(ctx,GColorWhite);
    graphics_draw_circle(ctx, GPoint(72,moon_y), moon_r);
  }

  if (phase != 15 && phase != 27 ) { 
    if (phase < 15) {
      // draw the waxing occlusion...
      graphics_context_set_fill_color(ctx,GColorBlack);
      graphics_fill_circle(ctx, GPoint(72 - (phase * 6), moon_y), moon_r + (phase * 4));
    }

    if (phase > 15) {
      // draw the waning occlusion...
      int phase_factor = abs(phase-30);
      graphics_context_set_fill_color(ctx,GColorBlack);
      graphics_fill_circle(ctx, GPoint(((72-3) + (phase_factor * 6)), moon_y), moon_r + (phase_factor * 4));
    }
  }

  // mask off the "daylight" portion of the watchface, otherwise, we
  // see the occlusion circles where the "night" portion does not cover.
  // This is probably the messiest bit of the watch app, since it assumes
  // that a lot of things are happening in the right order to work...
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  struct GPath *sun_path_moon_mask;
  sun_path_moon_mask = gpath_create(&sun_path_moon_mask_info);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_move_to(sun_path_moon_mask, center);
  gpath_draw_outline(ctx, sun_path_moon_mask);
  gpath_draw_filled(ctx, sun_path_moon_mask);
}

static void window_unload(Window *window) {
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // sunlight_layer
  sunlight_layer = layer_create(bounds);
  layer_set_update_proc(sunlight_layer, &sunlight_layer_update_proc);
  layer_add_child(window_layer, sunlight_layer);

  // moon_layer
  moon_layer = layer_create(bounds);
  layer_set_update_proc(moon_layer, &moon_layer_update_proc);
  layer_add_child(window_layer, moon_layer);

  // clockface_layer
  face_layer = layer_create(bounds);
  layer_set_update_proc(face_layer, &face_layer_update_proc);
  layer_add_child(window_layer, face_layer);

  // hand_layer
  hand_layer = layer_create(bounds);
  layer_set_update_proc(hand_layer, &hand_layer_update_proc);
  layer_add_child(window_layer, hand_layer);

  // sunrise_sunset_text_layer
  sunrise_sunset_text_layer = layer_create(bounds);
  layer_set_update_proc(sunrise_sunset_text_layer, &sunrise_sunset_text_layer_update_proc);
  layer_add_child(window_layer, sunrise_sunset_text_layer);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(face_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

static void deinit(void) {
  layer_remove_from_parent(moon_layer);
  layer_remove_from_parent(hand_layer);
  //  delay_and_redraw(500);
  layer_remove_from_parent(sunlight_layer);
  //  delay_and_redraw(500);
  layer_remove_from_parent(sunrise_sunset_text_layer);
  //  delay_and_redraw(500);
  layer_remove_from_parent(face_layer);
  //  delay_and_redraw(500);

  layer_destroy(face_layer);
  layer_destroy(hand_layer);
  layer_destroy(sunlight_layer);
  layer_destroy(sunrise_sunset_text_layer);
  layer_destroy(moon_layer);

  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
