var locationOptions = {timeout: 15000, maximumAge: 60000}; 

function fetch_location_error(err) {
  console.log(err);
  Pebble.sendAppMessage({"0": "ERROR", "1": "ERROR"});
}

function fetch_location_data(pos) {
  console.log(pos.coords.latitude);
  console.log(pos.coords.longitude);
  latitude = pos.coords.latitude;
  longitude = pos.coords.longitude;
  console.log(latitude);
  Pebble.sendAppMessage({"0": "location", "1": longitude.toFixed(4), "2": latitude.toFixed(4)});
}

function readyHandler(e) {
    console.log("JS: Ready.");
    //Pebble.sendAppMessage( {"0": "ready"} );
    window.navigator.geolocation.getCurrentPosition(fetch_location_data, fetch_location_error, locationOptions);
}


function receivedHandler(message) {
    if(message.payload.status == "retrieve") {
        console.log("JS: Recieved status \"retrieve\", getting location...");
        window.navigator.geolocation.getCurrentPosition(fetch_location_data, fetch_location_error, locationOptions);
    }
}

Pebble.addEventListener("ready", readyHandler);
Pebble.addEventListener("appmessage", receivedHandler);

