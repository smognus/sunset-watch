var config_url = "http://192.168.0.82/sunset-watch-config.html"

Pebble.addEventListener("ready",
    function(e) {
        console.log("JS starting...");
	navigator.geolocation.getCurrentPosition(coords_received,coords_failed);
    }
);

function coords_failed(err) {
    console.log("Didn't get coordinates with error: " + err.code);
}

function coords_received(position) {
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

Pebble.addEventListener("appmessage", function(e) {
    console.log("Received from phone: " + JSON.stringify(e.payload));
});
			
Pebble.addEventListener("showConfiguration", function(e) {
    Pebble.openURL(config_url);
});

Pebble.addEventListener("webviewclosed", function(e) {
    console.log("Configuration closed");
    console.log(e.response);
    if (e.response) {
	var options = JSON.parse(decodeURIComponent(e.response));
	console.log("Options = " + JSON.stringify(options));
	Pebble.sendAppMessage( options );
    }
    else {
	console.log("User clicked cancel.");
    }
});
