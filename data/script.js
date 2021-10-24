// Get current sensor readings when the page loads
window.addEventListener('load', getReadings);
window.addEventListener('load', getValues);
window.addEventListener('load', getState);

// Function to get current readings on the web page when it loads for the first time
function getReadings() {
  let xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState === 4 && this.status === 200) {
      let myObj = JSON.parse(this.responseText);
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
  let xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState === 4 && this.status === 200) {
      let myObj = JSON.parse(this.responseText);
      console.log(myObj);
      document.getElementById("temp_setting_val").innerHTML = myObj.temp_setting;
    }
  };
  xhr.open("GET", "/values", true);
  xhr.send();
}

// Function to get and update Power state on the web page
function getState(){
  let xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState === 4 && this.status === 200) {
      let myObj = JSON.parse(this.responseText);
      console.log(myObj);
      let state = myObj.state;
      console.log("State" + state);
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
xhr.open("GET", "/state", true);
xhr.send();
}

// Send Requests to Control POWER
function toggleCheckbox (element) {
  let xhr = new XMLHttpRequest();
  if (element.checked){
    xhr.open("GET", "/power?state=1", true);
    document.getElementById("power_switch_state").innerHTML = "ON";
  }
  else {
    xhr.open("GET", "/power?state=0", true);
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
    document.getElementById("temp").innerHTML = obj.temperature;
    document.getElementById("hum").innerHTML = obj.humidity;
    document.getElementById("pres").innerHTML = obj.pressure;
  }, false);
}


