#include "NetUtils.h"

void EasyNet::configureWiFi(String ssid, String password, String hostname) {
  this->ssid = ssid;
  this->password = password;
  this->hostname = hostname;
  WiFi.hostname(hostname);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
}
