sunset-watch
============

Pebble Sunset Watch Face

This watchface grabs the user's location from the connected phone on init and calculates sunrise and sunset times for that location.  The appropriate portion of the 24-hour face will display black.  Additionally, an approximation of the current phase of the moon is drawn.

The watch face is configurable via a (hosted) html/js interface accessible from withing the Pebble phone application.  Configurable options are:
- Whether or not there is a second hand.
- Whether or not the phase of the moon is displayed.
- Whether or not hour-marking numbers are displayed.
- Whether or not the digital time is displayed.
- Whether or not the remaining battery percentage is displayed.
- Whether or not to account for DST when calculating sunrise/sunset times.
- To calculate sunrise/sunset times based on a manually-configured timezone.

- Michael Ehrmann (Boldo) for the original SunClock source
- Chad Harp for the Almanac source
- Dersie for beta testing the revised code
