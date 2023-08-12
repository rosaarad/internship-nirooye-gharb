#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <SPIFFS.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <Hash.h>
  #include <FS.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

File myFile ;

const int n=10;
String ssidArray[n];

const char* wifi_ssid = "";
const char* wifi_password = "";

const char* esp_ssid = "default_Esp32";
const char* esp_password = "123456";

const char* PARAM_esp_ssid = "input1";
const char* PARAM_esp_pass = "input2";
const char* PARAM_wifi_ssid = "input3";
const char* PARAM_wifi_pass = "input4";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="refresh" content="5">
  <script>
    function submitMessage() {
      alert("Saved value to ESP SPIFFS");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
  </script></head><body>
  <h1>Access point Setting</h1>
  <form action="/get" target="hidden-form">
    <input type="text" name="input1" placeholder="set your esp32 ssid">
    <input type="text" name="input2" placeholder="set your esp32 password">
    <input type="submit" value="Save" onclick="submitMessage()">
  </form><br>
  <h1>Station Setting</h1>
  <form action="/get" target="hidden-form">
    <select name="input3" id="dynamicSelect">
        <option value="default">set your wifi ssid</option>
    </select>
    <input type="text" name="input4" placeholder="set your wifi password">
    <input type="submit" value="Save" onclick="submitMessage()">
  </form><br>
  <iframe style="display:none" name="hidden-form"></iframe>
  <script>            
    var select = document.getElementById("dynamicSelect");
    listNetworks();
    var arr2 = ["h","b"];

    
    var arr = ssidArray;


    for(var i = 0; i < arr.length; i++){
        var option = document.createElement("OPTION");
        var txt = document.createTextNode(arr[i]);
        option.appendChild(txt);
        option.setAttribute("value",arr[i]);
        select.insertBefore(option,select.lastChild);
    }             
  </script>  
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  file.close();
  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

// Replaces placeholder with stored values
String processor(const String &var){
  //Serial.println(var);
  if(var == "input1"){
    return readFile(SPIFFS, "/input1.txt");
  }
  else if(var == "input2"){
    return readFile(SPIFFS, "/input2.txt");
  }
  else if(var == "input3"){
    return readFile(SPIFFS, "/input3.txt");
  }
  else if(var == "input4"){
    return readFile(SPIFFS, "/input4.txt");    
  }
  return String();
}

void listNetworks() {
  Serial.println("** Scan Networks **");
  // WiFi.scanNetworks will return the number of networks found
  int numSsid = WiFi.scanNetworks();
  if (numSsid == 0) {
      Serial.println("no networks found");
  } else {
    Serial.print(numSsid);
    Serial.println(" networks found");
    for (int i = 0; i < numSsid; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      ssidArray[i] = WiFi.SSID(i);
//      myFile = SPIFFS.open("/SSID.txt", "w");
//      myFile.print(SSID_File);
//      myFile.close();
//      ESP.restart();
    }

}
}

void setup() {
  Serial.begin(115200);

  delay(100);
  Serial.println();
  
  // Initialize SPIFFS
  #ifdef ESP32
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #else
    if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #endif

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;

    if (request->hasParam(PARAM_esp_ssid)||request->hasParam(PARAM_esp_pass)) {
      inputMessage1 = request->getParam(PARAM_esp_ssid)->value();
      writeFile(SPIFFS, "/input1.txt", inputMessage1.c_str());
      inputMessage2 = request->getParam(PARAM_esp_pass)->value();
      writeFile(SPIFFS, "/input2.txt", inputMessage2.c_str());
      ESP. restart();
    }

    else if (request->hasParam(PARAM_wifi_ssid)||request->hasParam(PARAM_wifi_pass)) {
      inputMessage1 = request->getParam(PARAM_wifi_ssid)->value();
      writeFile(SPIFFS, "/input3.txt", inputMessage1.c_str());
      inputMessage2 = request->getParam(PARAM_wifi_pass)->value();
      writeFile(SPIFFS, "/input4.txt", inputMessage2.c_str());
      ESP. restart();
    }
    else {
      inputMessage1 = "No message sent";
    }
    Serial.println(inputMessage1);
    request->send(200, "text/text", inputMessage1);
    Serial.println(inputMessage2);
    request->send(200, "text/text", inputMessage2);
  });

    server.on("/hello", HTTP_GET, [](AsyncWebServerRequest * request) {
 
    if (ON_STA_FILTER(request)) {
      request->send(200, "text/plain", "Hello from STA");
      return;
 
    } else if (ON_AP_FILTER(request)) {
      request->send(200, "text/plain", "Hello from AP");
      return;
    }
 
    request->send(200, "text/plain", "Hello from undefined");
  });

  String yourInput_esp_ssid = readFile(SPIFFS, "/input1.txt");
  esp_ssid = const_cast<const char*>(yourInput_esp_ssid.c_str());
  //esp_ssid = yourInput_esp_ssid.c_str();

  String yourInput_esp_pass = readFile(SPIFFS, "/input2.txt");
  esp_password = const_cast<const char*>(yourInput_esp_pass.c_str());
  //esp_password = yourInput_esp_pass.c_str();

  String yourInput_wifi_ssid = readFile(SPIFFS, "/input3.txt");
  wifi_ssid = const_cast<const char*>(yourInput_wifi_ssid.c_str());

  String yourInput_wifi_pass = readFile(SPIFFS, "/input4.txt");
  wifi_password = const_cast<const char*>(yourInput_wifi_pass.c_str());

  Serial.print("***  yourInput_esp_ssid: ");
  Serial.println(esp_ssid);
  Serial.print("*** yourInput_esp_pass: ");  
  Serial.println(esp_password);
  Serial.print("*** yourInput_wifi_ssid: ");  
  Serial.println(wifi_ssid);
  Serial.print("*** yourInput_wifi_pass: ");  
  Serial.println(wifi_password);
   
  WiFi.mode(WIFI_MODE_APSTA);
 
  WiFi.softAP(esp_ssid, esp_password);
  WiFi.begin(wifi_ssid, wifi_password);
 
  if (WiFi.status() != WL_CONNECTED) {
    for(int i=1;i<=5;i++){
      delay(500);
      Serial.println("Connecting to WiFi..");
    }
  }

  Serial.print("ESP32 IP as soft AP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("ESP32 IP on the WiFi network: ");
  Serial.println(WiFi.localIP());
  listNetworks();
  for (int i = 0; i < n; i++) {
      Serial.println(ssidArray[i]);
      writeFile(SPIFFS, "/ssidArray.txt", ssidArray[i]);
  }

  server.onNotFound(notFound);
  server.begin();
}

void loop() {

}
