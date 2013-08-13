// All of the code that handles the math to figure out sunrise, sunset,
// moon phase, and associated paths is taken from the authors of the "24H Sun Moon Phase" watchface
// written by KarbonPebbler:
// http://www.mypebblefaces.com/view?fID=2270&aName=KarbonPebbler&pageTitle=24H+Sun+Moon+Phase&auID=1528
// 
// Additional credits cited in that project:
// - Michael Ehrmann (Boldo) for the original SunClock source
// - Chad Harp for the Almanac source
// - Dersie for beta testing the revised code

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "my_math.h"
#include "suncalc.h"
#include "config.h"

#define MY_UUID { 0x89, 0xCF, 0x77, 0x51, 0x1B, 0xE0, 0x48, 0x11, 0x97, 0x3E, 0xA2, 0x4F, 0x99, 0x61, 0xB7, 0xC5 }
PBL_APP_INFO(MY_UUID,
             "Sunset Watch", "Smognus",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
Layer clockface_layer;
Layer hand_layer;
Layer sunlight_layer;
Layer sunrise_sunset_text_layer;
Layer moon_layer;
int current_moon_day = -1;
int phase;
int current_sunrise_sunset_day = -1;
float sunriseTime;
float sunsetTime;

int moon_phase(PblTm *time) {
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

static void moon_layer_update_proc(Layer* layer, GContext* ctx) {
  PblTm time;
  get_time(&time);

  // don't do any calculations if they've already been done for today.
  if (current_moon_day != time.tm_mday) {
    phase = moon_phase(&time);
    // we don't have to calculate this again until tomorrow...
    current_moon_day = time.tm_mday;
  }

  int moon_y = 108;  // y-axis position of the moon's center
  int moon_r = 15;   // radius of the moon

  // draw the moon...
  if (phase != 27) {
    graphics_context_set_fill_color(ctx,GColorWhite);
    graphics_fill_circle(ctx, GPoint(72,moon_y), moon_r);
  }
  if (phase == 27) {
    graphics_context_set_stroke_color(ctx,GColorWhite);
    graphics_draw_circle(ctx, GPoint(72,moon_y), moon_r);
  }

  if (phase != 15 && phase != 27 ) { 
    // draw the waxing occlusion...
    graphics_context_set_fill_color(ctx,GColorBlack);
    graphics_fill_circle(ctx, GPoint(72-((phase * 2) + 3),moon_y), moon_r);
    // draw the waning occlusion...
    graphics_context_set_fill_color(ctx,GColorBlack);
    graphics_fill_circle(ctx, GPoint(132-((phase * 2) + 3),moon_y), moon_r);
  }

  // mask off the "daylight" portion of the watchface, otherwise, we
  // see the occlusion circles where the "night" portion does not cover.
  // This is probably the messiest bit of the watch app, since it assumes
  // that a lot of things are happening in the right order to work...
  struct GRect fr = layer_get_frame(&window.layer);
  GPoint center = grect_center_point(&fr);
  GPath sun_path_moon_mask;
  gpath_init(&sun_path_moon_mask, &sun_path_moon_mask_info);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_move_to(&sun_path_moon_mask, center);
  gpath_draw_outline(ctx, &sun_path_moon_mask);
  gpath_draw_filled(ctx, &sun_path_moon_mask);
}

void adjustTimezone(float* time) 
{
  *time += TIMEZONE;
  if (*time > 24) *time -= 24;
  if (*time < 0) *time += 24;
}

static void draw_dot(GContext* ctx, GPoint center, int radius) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx,GColorBlack);
  graphics_fill_circle(ctx, center, radius);
  graphics_draw_circle(ctx, center, radius);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_circle(ctx, center, radius+1);
}

// results are pretty awful for `outline_pixel' values larger than 2...
static void draw_outlined_text(GContext* ctx, char* text, GFont font, GRect rect, GTextOverflowMode mode, GTextAlignment alignment, int outline_pixels, bool inverted) {
  (inverted) ? graphics_context_set_text_color(ctx, GColorBlack) :
    graphics_context_set_text_color(ctx, GColorWhite);

  GRect volatile_rect = rect;
  for (int i=0; i<outline_pixels; i++) {
    volatile_rect.origin.x+=1;
    graphics_text_draw(ctx,
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
    graphics_text_draw(ctx,
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
    graphics_text_draw(ctx,
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
    graphics_text_draw(ctx,
		       text,
		       font,
		       volatile_rect,
		       mode,
		       alignment,
		       NULL);
  }
  (inverted) ? graphics_context_set_text_color(ctx, GColorWhite) :
    graphics_context_set_text_color(ctx, GColorBlack);
  graphics_text_draw(ctx,
		     text,
		     font,
		     rect,
		     mode,
		     alignment,
		     NULL);
}

static void clockface_layer_update_proc(Layer* layer, GContext* ctx) {
  // draw face outlines
  struct GRect fr = layer_get_frame(layer);
  GPoint center = grect_center_point(&fr);
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
  // draw hour text
  PblTm fake_time;
  char *time_format = "%l";  
  char hour_text[] = "12";

  fake_time.tm_hour = 6;
  string_format_time(hour_text, sizeof(hour_text), time_format, &fake_time);

  // draw semi major hour text
  fake_time.tm_hour = 6;
  string_format_time(hour_text, sizeof(hour_text), time_format, &fake_time);

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
    string_format_time(hour_text, sizeof(hour_text), time_format, &fake_time);
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
}

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

  static const GPathInfo p_sunrise_line_info = {
    .num_points = 4,
    .points = (GPoint []) {{0,0},{-0,0},{-0,-100},{0,-100}}
  };
  static const GPathInfo p_sunset_line_info = {
    .num_points = 4,
    .points = (GPoint []) {{0,0},{-0,0},{-0,-100},{0,-100}}
  };

static void sunrise_sunset_text_layer_update_proc(Layer* layer, GContext* ctx) {
  PblTm time;
  PblTm sunrise_time;
  PblTm sunset_time;
  static char sunrise_text[] = "     ";
  static char sunset_text[] = "     ";
  static char time_text[] = "     ";
  static char month_text[] = "   ";
  static char day_text[] = "  ";
  char *time_format = "%l:%M";
  char *month_format = "%b";
  char *day_format = "%e";
  // probably don't really need to do this, but it populates everything nicely...
  get_time(&sunrise_time);
  get_time(&sunset_time);

  get_time(&time);

  // don't calculate these if they've already been done for the day.
  if (current_sunrise_sunset_day != time.tm_mday) {
    sunriseTime = calcSunRise(time.tm_year, time.tm_mon+1, time.tm_mday, LATITUDE, LONGITUDE, 91.0f);
    sunsetTime = calcSunSet(time.tm_year, time.tm_mon+1, time.tm_mday, LATITUDE, LONGITUDE, 91.0f);
    adjustTimezone(&sunriseTime);
    adjustTimezone(&sunsetTime);
    // don't calculate them again until tomorrow
    current_sunrise_sunset_day = time.tm_mday;
  }

  // print sunrise/sunset times
  sunrise_time.tm_min = (int)(60*(sunriseTime-((int)(sunriseTime))));
  sunrise_time.tm_hour = (int)sunriseTime;
  string_format_time(sunrise_text, sizeof(sunrise_text), time_format, &sunrise_time);
  sunset_time.tm_min = (int)(60*(sunsetTime-((int)(sunsetTime))));
  sunset_time.tm_hour = (int)sunsetTime;
  string_format_time(sunset_text, sizeof(sunset_text), time_format, &sunset_time);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_text_draw(ctx,
		     sunrise_text,
		     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
		     GRect(3, 145, 144-3, 168-145),
		     GTextOverflowModeWordWrap,
		     GTextAlignmentLeft,
		     NULL);
  graphics_text_draw(ctx,
		     sunset_text,
		     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
		     GRect(3, 145, 144-3, 168-145),
		     GTextOverflowModeWordWrap,
		     GTextAlignmentRight,
		     NULL);

  // draw current time
  string_format_time(time_text, sizeof(time_text), time_format, &time);
  draw_outlined_text(ctx,
		     time_text,
		     fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
		     GRect(42, 47, 64, 32),
  		     GTextOverflowModeWordWrap,
  		     GTextAlignmentCenter,
		     1,
		     false);

  //draw current date month
  string_format_time(month_text, sizeof(month_text), month_format, &time);
  draw_outlined_text(ctx,
		     month_text,
		     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
		     GRect(3, 0, 144-3, 32),
  		     GTextOverflowModeWordWrap,
  		     GTextAlignmentLeft,
		     0,
		     true);
  //draw current date day
  string_format_time(day_text, sizeof(day_text), day_format, &time);
  draw_outlined_text(ctx,
		     day_text,
		     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
		     GRect(3, 0, 144-13, 32),
  		     GTextOverflowModeWordWrap,
  		     GTextAlignmentRight,
		     0,
		     true);
}

static void sunlight_layer_update_proc(Layer* layer, GContext* ctx) {
  PblTm time;
  PblTm sunrise_time;
  PblTm sunset_time;

  // probably don't really need to do this...
  get_time(&sunrise_time);
  get_time(&sunset_time);

  get_time(&time);
  float sunriseTime = calcSunRise(time.tm_year, time.tm_mon+1, time.tm_mday, LATITUDE, LONGITUDE, 91.0f);
  float sunsetTime = calcSunSet(time.tm_year, time.tm_mon+1, time.tm_mday, LATITUDE, LONGITUDE, 91.0f);

  adjustTimezone(&sunriseTime);
  adjustTimezone(&sunsetTime);

  // get the center point of the current window, which will
  // be the origin of the sunrise/sunset lines
  struct GRect fr = layer_get_frame(&window.layer);
  GPoint center = grect_center_point(&fr);

  sun_path_info.points[1].x = (int16_t)(my_sin(sunriseTime/24 * M_PI * 2) * 120);
  sun_path_info.points[1].y = -(int16_t)(my_cos(sunriseTime/24 * M_PI * 2) * 120);

  sun_path_info.points[4].x = (int16_t)(my_sin(sunsetTime/24 * M_PI * 2) * 120);
  sun_path_info.points[4].y = -(int16_t)(my_cos(sunsetTime/24 * M_PI * 2) * 120);

  sun_path_moon_mask_info.points[1].x = (int16_t)(my_sin(sunriseTime/24 * M_PI * 2) * 120);
  sun_path_moon_mask_info.points[1].y = -(int16_t)(my_cos(sunriseTime/24 * M_PI * 2) * 120);

  sun_path_moon_mask_info.points[4].x = (int16_t)(my_sin(sunsetTime/24 * M_PI * 2) * 120);
  sun_path_moon_mask_info.points[4].y = -(int16_t)(my_cos(sunsetTime/24 * M_PI * 2) * 120);

  GPath sun_path;
  gpath_init(&sun_path, &sun_path_info);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  gpath_move_to(&sun_path, center);
  gpath_draw_outline(ctx, &sun_path);
  gpath_draw_filled(ctx, &sun_path);
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
  struct GRect fr = layer_get_frame(layer);
  GPoint center = grect_center_point(&fr);

  GPath p_hour_hand;
  GPath p_second_hand;

  PblTm time;
  get_time(&time);

  // NEXT LINE FOR TESTING ONLY
  //  time.tm_hour = 12;

  int32_t second_angle = time.tm_sec * 6;

  // 24 hour hand
  int32_t hour_angle = (((time.tm_hour * 60) + time.tm_min) / 4) + 180;

  // draw the hour hand
  graphics_context_set_stroke_color(ctx, GColorWhite);
  gpath_init(&p_hour_hand, &p_hour_hand_info);
  graphics_context_set_fill_color(ctx, GColorBlack);
  gpath_move_to(&p_hour_hand, center);
  gpath_rotate_to(&p_hour_hand, TRIG_MAX_ANGLE / 360 * hour_angle);
  gpath_draw_filled(ctx, &p_hour_hand);
  gpath_draw_outline(ctx, &p_hour_hand);

  // draw the second hand
  graphics_context_set_stroke_color(ctx, GColorBlack);
  gpath_init(&p_second_hand, &p_second_hand_info);
  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_move_to(&p_second_hand, center);
  gpath_rotate_to(&p_second_hand, TRIG_MAX_ANGLE / 360 * second_angle);
  gpath_draw_filled(ctx, &p_second_hand);
  gpath_draw_outline(ctx, &p_second_hand);
}

void handle_init(AppContextRef ctx) {
  window_init(&window, "Window Name");
  window_stack_push(&window, true /* Animated */);
  layer_init(&clockface_layer, window.layer.frame);
  layer_init(&hand_layer, window.layer.frame);
  layer_init(&sunlight_layer, window.layer.frame);
  layer_init(&sunrise_sunset_text_layer, window.layer.frame);
  layer_init(&moon_layer, window.layer.frame);

  clockface_layer.update_proc = &clockface_layer_update_proc;
  hand_layer.update_proc = &hand_layer_update_proc;
  sunlight_layer.update_proc = &sunlight_layer_update_proc;
  sunrise_sunset_text_layer.update_proc = &sunrise_sunset_text_layer_update_proc;
  moon_layer.update_proc = &moon_layer_update_proc;

  layer_add_child(&window.layer, &sunlight_layer);
  layer_add_child(&window.layer, &moon_layer);
  layer_add_child(&window.layer, &clockface_layer);
  layer_add_child(&window.layer, &hand_layer);
  layer_add_child(&window.layer, &sunrise_sunset_text_layer);
}

void handle_deinit(AppContextRef ctx) {
}

void handle_tick(AppContextRef ctx, PebbleTickEvent* tick) {
  (void) tick;
  layer_mark_dirty(&window.layer);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = SECOND_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
