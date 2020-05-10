// #ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
// #else
//   #include <ESP8266WiFi.h>
//   #include <ESPAsyncTCP.h>
// #endif
 #include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char* ssid = "AFFOZ_Office";
const char* password = "&L2I@PxQ";

const char* PARAM_INPUT_1 = "relay";
const char* PARAM_INPUT_2 = "state";

#define RELAY_NO    true

// Set number of relays
#define NUM_RELAYS  3

String state_value = "00";

// Assign each GPIO to a relay
int relayGPIOs[NUM_RELAYS] = {19, 21, 33};
const int output = 19;
const int output2 = 21;
const int output3 = 33;

const int buttonPin = 23;
const int buttonPin2 = 5;
const int buttonPin3 = 15;
// Variables will change:
int ledState = LOW;
int ledState2 = LOW;  
int ledState3 = LOW;            // the current state of the output pin


int buttonState;


int buttonState2;
int buttonState3;             // the current reading from the input pin
int lastButtonState = LOW;
int lastButtonState2 = LOW;
int lastButtonState3 = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay2 = 50;    // the debounce time; increase if the output flickers
unsigned long lastDebounceTime3 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay3 = 50;    // the debounce time; increase if the output flickers

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?state=1", true); }
  else { xhr.open("GET", "/update?state=0", true); }
      if(element.checked){ xhr.open("GET", "/update?relay="+element.dataset.id+"&state=1", true); }
       else { xhr.open("GET", "/update?relay="+element.dataset.id+"&state=0", true); }

  xhr.send();
}

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      console.log("==== start =====");
      console.log(this.responseText);
      console.log(this.responseText[0]);
      console.log(this.responseText[1]);
      console.log("==== end =====");
      for (i = 1; i <= 3; i++) {
        document.getElementById('button-' + i).checked =  this.responseText[i - 1] == '1';
        document.getElementById('state-' + i).innerHTML = this.responseText[i - 1] == '1' ? 'On' : 'Off';
      }

    }
  };
  xhttp.open("GET", "/state", true);
  xhttp.send();
}, 500 ) ;

</script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    for(int i=1; i<=NUM_RELAYS; i++){
      String relayStateValue = relayState(i);
      String outputStateValue = outputState();
      buttons+= "<h4>Relay #" + String(i) + " - GPIO " + relayGPIOs[i-1] + "<span data-id=\"" + String(i) + "\" id=\"state-" + String(i) + "\"><span></h4><label class=\"switch\"><input data-id=\"" + String(i) + "\" type=\"checkbox\" onchange=\"toggleCheckbox(this); return false;\" id=\"button-" + String(i) + "\" "+ relayStateValue + "><span class=\"slider\"></span></label>";
      // buttons+= "<h4>Relay #" + String(i) + " - GPIO " + relayGPIOs[i-1] + "<span id=\"state-" + String(i) + "\"><span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + relayStateValue + "><span class=\"slider\"></span></label>";
      // buttons+= "<h4>Relay #" + String(i) + " - GPIO " + relayGPIOs[i-1] + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" "+ relayStateValue +"><span class=\"slider\"></span></label>";
    }
    // String outputStateValue = outputState();
    // buttons+= "<h4>Output - GPIO 2 - State <span id=\"outputState\"><span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";
   
    return buttons;
  }
  return String();
}




String outputState(){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}
String outputState2(){
  if(digitalRead(output2)){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}
String outputState3(){
  if(digitalRead(output3)){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}
String relayState(int numRelay){

Serial.println(numRelay);

  if(RELAY_NO){

Serial.println("IF 1");


    if(digitalRead(relayGPIOs[numRelay-1])){
Serial.println("IF 2");

      return "";
    }
    else {
      return "checked";
    }
  }
  else {

Serial.println("ELSE 1");

    if(digitalRead(relayGPIOs[numRelay-1])){
Serial.println("ELSE 2");

      return "checked";
    }
    else {
      return "";
    }
  }
  return "";
}
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);
  pinMode(buttonPin, INPUT_PULLDOWN);
  pinMode(output2, OUTPUT);
  digitalWrite(output2, LOW);
  pinMode(buttonPin2, INPUT_PULLDOWN);
  pinMode(output3, OUTPUT);
  digitalWrite(output3, LOW);
  pinMode(buttonPin3, INPUT_PULLDOWN);

  for(int i=1; i<=NUM_RELAYS; i++){
    pinMode(relayGPIOs[i-1], OUTPUT);
    if(RELAY_NO){
      digitalWrite(relayGPIOs[i-1], HIGH);
    }
    else{
      digitalWrite(relayGPIOs[i-1], LOW);
    }
  }
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2))
    {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
      
      Serial.println(relayGPIOs[inputMessage.toInt()-1]);
      
      Serial.println(!inputMessage2.toInt());
      digitalWrite(relayGPIOs[inputMessage.toInt()-1], inputMessage2.toInt());
    }
    else
    {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/state
  // server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
  //   request->send(200, "text/plain", String(digitalRead(output)).c_str());
  // });
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String data = "";
    data += digitalRead(output) == HIGH ? '1' : '0';
    data += digitalRead(output2) == HIGH ? '1' : '0';
    data += digitalRead(output3) == HIGH ? '1' : '0';
    
    request->send(200, "text/plain", data);
  });

  // Start server
  server.begin();
}
  
void loop() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);
  int reading2 = digitalRead(buttonPin2);
  int reading3 = digitalRead(buttonPin3);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if (reading2 != lastButtonState2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis();
  }
  if (reading3 != lastButtonState3) {
    // reset the debouncing timer
    lastDebounceTime3 = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
        digitalWrite(output, ledState);
        Serial.println("nut nhan da duoc nhan 1");
        state_value = "11";
      } else {
        state_value = "10";
      }
    }
    if (reading2 != buttonState2)
    {
      buttonState2 = reading2;

      // only toggle the LED if the new button state is HIGH
      if (buttonState2 == HIGH)
      {
        ledState2 = !ledState2;
        digitalWrite(output2, ledState2);
        Serial.println("nut nhan da duoc nhan 2");
        //21
                        state_value = "21";

      } else {
        // 20
            state_value = "20";
      }
    }
    if (reading3 != buttonState3)
    {
      buttonState3 = reading3;

      // only toggle the LED if the new button state is HIGH
      if (buttonState3 == HIGH)
      {
        ledState3 = !ledState3;
        digitalWrite(output3, ledState3);
        Serial.println("nut nhan da duoc nhan 3");
        // 31
                    state_value = "31";

      } else {
        // 30
                            state_value = "30";

      }
    }
  }

  // // set the LED:
  // digitalWrite(output, ledState);
  // digitalWrite(output2, ledState2);
  // digitalWrite(output3, ledState3);

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
  lastButtonState2 = reading2;
  lastButtonState3 = reading3;
  
}