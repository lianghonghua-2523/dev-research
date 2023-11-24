#ifndef __Wifi_H
#define __Wifi_H
#include "esp_err.h"
char ipaddr[32];

int GetWiFiStatus(void);
void SetWiFiStatus(int status);
esp_err_t WifiInit(void);
esp_err_t WifiConnect(const char *ssid,const char *password);
esp_err_t WifiDisconnect(void);






#endif
