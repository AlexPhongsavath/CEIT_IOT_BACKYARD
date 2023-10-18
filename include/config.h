#ifndef __CONFIG__
#define __CONFIG__
#define __DEVELOPMENT__
// #define __PRODUCTION__
// WiFi config
const char *ssid = "CEIT-IoT-Smart-Farm"; // Enter your WiFi name
const char *password = "IoT@Farm2024"; // Enter WiFi password


// client Info
// const char *mqtt_server = "45.32.111.51";
// const char *mqtt_server = "390acf6596d1409b8efba47ae295ef9b.s2.eu.hivemq.cloud";
//const char *mqtt_server = "202.137.130.47";
const char *mqtt_server = "broker.hivemq.com";


const int mqtt_port = 1883;
// const char *mqttUser = "mrcadmin";
// const char *mqttPass = "mrcAdmin@2023";

// publish the machine status topic
#define PUB_Topic "CEIT/IOT/FARM/DATA"

#endif
