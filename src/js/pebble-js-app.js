Pebble.addEventListener("ready",
    function(e) {
        console.log("JS starting...");
	navigator.geolocation.getCurrentPosition(coords);
    }
);

function coords(position) {
    //    can manually set coordinates here for testing...
    //    position.coords.latitude = 40.67;
    //    position.coords.longitude = -73.94;

    console.log("Got latitude: " + position.coords.latitude);
    console.log("Got longitude: " + position.coords.longitude);

    Pebble.sendAppMessage( { "latitude": position.coords.latitude.toString(),
			     "longitude": position.coords.longitude.toString() },
		       function(e) { console.log("Successfully delivered message with transactionId="
						 + e.data.transactionId); },
		       function(e) { console.log("Unsuccessfully delivered message with transactionId="
						 + e.data.transactionId); });
}
