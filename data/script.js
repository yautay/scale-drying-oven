// Get current sensor readings when the page loads
window.addEventListener('load', getReadings);
window.addEventListener('load', getValues);

// Function to get current readings on the web page when it loads for the first time
function getReadings() {
  let xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState === 4 && this.status === 200) {
      let myObj = JSON.parse(this.responseText);
      console.log(myObj);
      document.getElementById("display_temperature_chamber").innerHTML = myObj.temperature;
      document.getElementById("display_humidity").innerHTML = myObj.humidity;
      document.getElementById("display_pressure").innerHTML = myObj.pressure;
      document.getElementById("display_bed_params").innerHTML = myObj.bed_adc;
    } 
  };
  xhr.open("GET", "/readings", true);
  xhr.send();
}

// Function to get current values on the web page when it loads for the first time
function getValues(){
  let xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState === 4 && this.status === 200) {
      let myObj = JSON.parse(this.responseText);
      console.log(myObj);
      document.getElementById("setting_temperature_chamber").innerHTML = myObj.chamber;
      document.getElementById("setting_bed_adc").innerHTML = myObj.bed;
      let state = myObj.power;
      if (state === "1") {
        document.getElementById("power_switch").checked = true;
        document.getElementById("power_switch_state").innerHTML = "ON";
      }
      else {
        document.getElementById("power_switch").checked = false;
        document.getElementById("power_switch_state").innerHTML = "OFF";
      }
    }
  };
  xhr.open("GET", "/settings", true);
  xhr.send();
}

// Send Requests to Control POWER
function toggleCheckbox (element) {
  let xhr = new XMLHttpRequest();
  if (element.checked){
    xhr.open("POST", "/settings?power=1", true);
    document.getElementById("power_switch_state").innerHTML = "ON";
  }
  else {
    xhr.open("POST", "/settings?power=0", true);
    document.getElementById("power_switch_state").innerHTML = "OFF";
  }
  xhr.send();
}

// Create an Event Source to listen for events
if (!!window.EventSource) {
  let source = new EventSource('/events');
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);
  source.addEventListener('error', function(e) {
    if (e.target.readyState !== EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("display_temperature_chamber").innerHTML = obj.tempetature;
    document.getElementById("display_humidity").innerHTML = obj.humidity;
    document.getElementById("display_pressure").innerHTML = obj.pressure;
    document.getElementById("display_bed_params").innerHTML = obj.bed_adc;
  }, false);
}


