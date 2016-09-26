#include <aJSON.h>
#include "SPI.h"
#include "WiFi.h"

#include "M2XStreamClient.h"

char ssid[] = "Home"; //  your network SSID (name)
char pass[] = "The Jewish Family 157 Palm";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

char deviceId[] = "45528f2cf30a017db28efeef048b4d49"; // Device you want to push to
char m2xKey[] = "e0802e355722294a442a5a0ef523e1de"; // Your M2X access key

char streamName1[] = "Voltage"; // Stream you want to push to
char streamName2[] = "Current"; // Stream you want to push to
char streamName3[] = "Frequency"; // Stream you want to push to
char streamName4[] = "Active"; // Stream you want to push to
char streamName5[] = "Reactive"; // Stream you want to push to
char streamName6[] = "Apparent"; // Stream you want to push to
char streamName7[] = "PF"; // Stream you want to push to
