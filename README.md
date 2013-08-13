sunset-watch
============

Pebble Sunrise/Sunset and Moon Phase Watch Face

This is a rewrite of the watchface at https://github.com/KarbonPebbler/KP_Sun_Moon_Vibe_Clock, which was badly broken by SDK changes.  This version uses no bitmaps, and since the GCompOps aren't working for vector stuff yet, it does some truly silly stuff to simulate transparency.

To make it work for your location, modify config.h for your Latitude and Longitude and Timezone (although you may have to fiddle with the timezone... I seem to to have needed to go +5 instead of +7...).  I understand very little of the math in the code that I took from the KP watchface.

The original author is KarbonPebbler, and he adds the additional credits to his project:
- Michael Ehrmann (Boldo) for the original SunClock source
- Chad Harp for the Almanac source
- Dersie for beta testing the revised code
