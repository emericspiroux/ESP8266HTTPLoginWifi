#include "WifiSwitcher.h"

WifiSwitcher wifiSwitcher;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  if (wifiSwitcher.launchHotSpot("ESP12Emeric", "0671755362")){
      digitalWrite(LED_BUILTIN, LOW);
  } else {
       blinkError();
  }
}

void loop()
{
  switch (wifiSwitcher.state){
    case STATION:
      wifiSwitcher.waitingForInternetWifiConnection();
      break;
    case LINKED:
      wifiSwitcher.waitingForInternetWifiConnection();
      break;
    case DOWN:
      blinkError();
      break;
  }
}

void blinkError(){
  while (1){
          digitalWrite(LED_BUILTIN, LOW);
          delay(1000);
          digitalWrite(LED_BUILTIN, HIGH);
  }
}

