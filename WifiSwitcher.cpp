#include "WifiSwitcher.h"

// Constructor
WifiSwitcher::WifiSwitcher(){
  this->state = STATION;
  this->server = new ESP8266WebServer(80);
}

WifiSwitcher::WifiSwitcher (const WifiSwitcher& src){
    this->server = src.server;
}

// Destructor
WifiSwitcher::~WifiSwitcher(){
  WiFi.disconnect();
  this->server->stop();
  delete this->server;  
}

// Public Methods
/// HotSpot Methods
boolean WifiSwitcher::launchHotSpot(char* ssid, char* password){
  Serial.begin(115200);
  Serial.println();

  Serial.print("Setting soft-AP ... ");


  WiFi.mode(WIFI_AP);// Need by MDNS WIFI_AP
  
  this->initHTTPRoutes();

  this->ssid = String(ssid);
  this->password = String(password);

  IPAddress Ip(192, 168, 1, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);

  boolean rtn = WiFi.softAP(ssid, password);
  if (!MDNS.begin("esp12emeric", WiFi.softAPIP())){
    this->state = DOWN;
  } else {
    this->state = STATION;
  }
  return rtn;
}

int WifiSwitcher::howManyDevices(){
  return WiFi.softAPgetStationNum();
}

/// Web Service Methods
boolean WifiSwitcher::waitingForInternetWifiConnection(){
  this->server->handleClient();
}

// Private Methods
/// HTTP Page Methods
void WifiSwitcher::initHTTPRoutes(){
  //Setting route availables
//  this->server->onNotFound(std::bind(&WifiSwitcher::getHTTPNotFound, this));
  this->server->on("/", std::bind(&WifiSwitcher::getHTTPIndexPage, this));
  this->server->on("/connectInternet", std::bind(&WifiSwitcher::postHTTPInternetWifi, this));
  this->server->on("/ping", [](){
    //this->server->send(200, "text/plain", "It's Work !");
  });

  //Here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  
  //Ask server to track these headers
  server->collectHeaders(headerkeys, headerkeyssize);

  //Starting Server
  this->server->begin();
}

void WifiSwitcher::getHTTPIndexPage(){
  // Read File Index.html
  String content = "<html><body><form action='/connectInternet' method='POST'>To log in wifi: <br>";
  content += "SSID:<input type='text' name='SSID' placeholder='ssid'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>"+ this->errorMessage +"<br>";
  content += "You also can go <a href='/ping'>here</a></body></html>";
  this->server->send(200, "text/html", content);
}

void WifiSwitcher::postHTTPInternetWifi(){
  this->server->sendHeader("Cache-Control","no-cache");
  this->server->sendHeader("Set-Cookie","ESPSESSIONID=1");
  
  if (this->server->hasArg("SSID") && this->server->hasArg("PASSWORD")){
    if (ssid && password){
      WiFi.disconnect();
      WiFi.mode(WIFI_AP_STA);
      WiFi.begin(this->server->arg("SSID").c_str(), this->server->arg("PASSWORD").c_str()); 
      boolean hasError = false;
      while (WiFi.status() != WL_CONNECTED) { 
          switch(WiFi.status()) {
            case WL_NO_SSID_AVAIL:
                this->errorMessage = "NO SSID AVAILABLE";
                hasError = true;
                break;
            case WL_CONNECT_FAILED:
                this->errorMessage = "CONNECTION FAILED";
                hasError = true;
                break;
            case WL_IDLE_STATUS:
                this->errorMessage = "IDLE STATUS";
                hasError = true;
                break;
          }

          if (hasError){
            break;
          }
          
          digitalWrite(LED_BUILTIN, LOW);
          delay(1000);
          digitalWrite(LED_BUILTIN, HIGH);
          delay(1000);
      }
      
      if (WiFi.status() != WL_CONNECTED){
        WiFi.softAP(ssid.c_str(), password.c_str());
        this->server->sendHeader("Location","/");
        digitalWrite(LED_BUILTIN, LOW);
      } else {
        this->state = LINKED;
        this->server->sendHeader("Location","/ping");
      }
      
      
    }
  } else {
    this->errorMessage = "Please enter ssid and password";
  }

  this->server->send(301);
}

void WifiSwitcher::getHTTPNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += this->server->uri();
  message += "\nMethod: ";
  message += (this->server->method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += this->server->args();
  message += "\n";
  for (uint8_t i=0; i<this->server->args(); i++){
    message += " " + this->server->argName(i) + ": " + this->server->arg(i) + "\n";
  }
  this->server->send(404, "text/plain", message);
}

