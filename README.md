sunset-watch
============

Pebble Sunrise/Sunset and Moon Phase Watch Face

It is a 24hour face with an hour hand and a second hand. Sunrise/sunset times are calculated, and the face is overlaid with with day/night masks. The phase of the moon is calculated and drawn in the night mask area. Sunrise and sunset times are in the bottom left and bottom right corners, respectiviely.

This is a rewrite of the watchface at https://github.com/KarbonPebbler/KP_Sun_Moon_Vibe_Clock, which was badly broken by SDK changes. This version uses no bitmaps, and since the GCompOps aren't working for vector stuff yet, it does some truly silly stuff to simulate transparency.

To make it work for your location, modify config.h for your Latitude and Longitude and Timezone.

The original author is KarbonPebbler, and he adds the additional credits to his project:

- Michael Ehrmann (Boldo) for the original SunClock source
- Chad Harp for the Almanac source
- Dersie for beta testing the revised code
