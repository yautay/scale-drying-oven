// Get current sensor readings when the page loads
window.addEventListener('load', getReadings);
window.addEventListener('load', getValues);
window.addEventListener('load', getStates);

// Function to get current readings on the web page when it loads for the first time
function getReadings() {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      document.getElementById("temp").innerHTML = myObj.temperature;
      document.getElementById("hum").innerHTML = myObj.humidity;
      document.getElementById("pres").innerHTML = myObj.pressure;
    } 
  };
  xhr.open("GET", "/readings", true);
  xhr.send();
}
// Function to get current values on the web page when it loads for the first time
function getValues(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      document.getElementById("temp_setting_val").innerHTML = myObj.temp_setting_val;
    }
  };
  xhr.open("GET", "/values", true);
  xhr.send();
}
// Function to get and update GPIO states on the web page
function getStates(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      var output = myObj.power_switch.output;
      var state = myObj.power_switch.state;
      console.log(output);
      console.log(state);
      if (state == "1") {
        document.getElementById(output).checked = true;
        document.getElementById("power_switch_state").innerHTML = "ON";
      }
      else {
        document.getElementById(output).checked = false;
        document.getElementById("power_switch_state").innerHTML = "OFF";
      }
    }
};
xhr.open("GET", "/states", true);
xhr.send();
}
// Send Requests to Control POWER
function toggleCheckbox () {
  var xhr = new XMLHttpRequest();
  if (element.checked){
    xhr.open("GET", "/update?output=power_switch&state=1", true);
    document.getElementById("power_switch_state").innerHTML = "ON";
  }
  else {
    xhr.open("GET", "/update?output=power_switch&state=0", true);
    document.getElementById("power_switch_state").innerHTML = "OFF";
  }
  xhr.send();
}
// Create an Event Source to listen for events
if (!!window.EventSource) {
  var source = new EventSource('/events');
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);
  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("temp").innerHTML = obj.temperature;
    document.getElementById("hum").innerHTML = obj.humidity;
    document.getElementById("pres").innerHTML = obj.pressure;
  }, false);
}


