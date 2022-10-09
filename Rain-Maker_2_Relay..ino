/**********************************************************************************
 
  Preferences--> Aditional boards Manager URLs : 
   http://arduino.esp8266.com/stable/package_esp8266com_index.json,https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  
   Download Board ESP32 (2.0.3) : 
 
 **********************************************************************************/

#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"

const char *service_name = "PROV_technolab";
const char *pop = "0000000";

// define the Device Names
char deviceName_1[] = "light one";
char deviceName_2[] = "light two";


// define the GPIO connected with Relays and switches
static uint8_t RelayPin1 = 23;  //D23
static uint8_t RelayPin2 = 22;  //D22


static uint8_t SwitchPin1 = 34;  //D13
static uint8_t SwitchPin2 = 35;  //D12


static uint8_t wifiLed    = 12;
static uint8_t PowerLed    = 13;//D2
static uint8_t gpio_reset = 0;

/* Variable for reading pin status*/
// Relay State
bool toggleState_1 = LOW; //Define integer to remember the toggle state for relay 1
bool toggleState_2 = LOW; //Define integer to remember the toggle state for relay 2


// Switch State
bool SwitchState_1 = LOW;
bool SwitchState_2 = LOW;


//The framework provides some standard device types like switch, lightbulb, fan, temperature sensor.
static Switch my_switch1(deviceName_1, &RelayPin1);
static Switch my_switch2(deviceName_2, &RelayPin2);


void sysProvEvent(arduino_event_t *sys_event)
{
    switch (sys_event->event_id) {      
        case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32
        Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
        printQR(service_name, pop, "ble");
#else
        Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", service_name, pop);
        printQR(service_name, pop, "softap");
#endif        
        break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.printf("\nConnected to Wi-Fi!\n");
        digitalWrite(wifiLed, true);
        break;
    }
}

void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx)
{
    const char *device_name = device->getDeviceName();
    const char *param_name = param->getParamName();

    if(strcmp(device_name, deviceName_1) == 0) {
      
      Serial.printf("Lightbulb = %s\n", val.val.b? "true" : "false");
      
      if(strcmp(param_name, "Power") == 0) {
          Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
        toggleState_1 = val.val.b;
        (toggleState_1 == false) ? digitalWrite(RelayPin1, LOW) : digitalWrite(RelayPin1, HIGH);
        param->updateAndReport(val);
      }
      
    } else if(strcmp(device_name, deviceName_2) == 0) {
      
      Serial.printf("Switch value = %s\n", val.val.b? "true" : "false");

      if(strcmp(param_name, "Power") == 0) {
        Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
        toggleState_2 = val.val.b;
        (toggleState_2 == false) ? digitalWrite(RelayPin2, LOW) : digitalWrite(RelayPin2, HIGH);
        param->updateAndReport(val);
      }
  
    } 
  
     
    }  


void manual_control()
{
  if (digitalRead(SwitchPin1) == LOW && SwitchState_1 == LOW) {
    digitalWrite(RelayPin1, LOW);
    toggleState_1 = 1;
    SwitchState_1 = HIGH;
    my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
    Serial.println("Switch-1 on");
  }
  if (digitalRead(SwitchPin1) == HIGH && SwitchState_1 == HIGH) {
    digitalWrite(RelayPin1, HIGH);
    toggleState_1 = 0;
    SwitchState_1 = LOW;
    my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_1);
    Serial.println("Switch-1 off");
  }
  if (digitalRead(SwitchPin2) == LOW && SwitchState_2 == LOW) {
    digitalWrite(RelayPin2, LOW);
    toggleState_2 = 1;
    SwitchState_2 = HIGH;
    my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
    Serial.println("Switch-2 on");
  }
  if (digitalRead(SwitchPin2) == HIGH && SwitchState_2 == HIGH) {
    digitalWrite(RelayPin2, HIGH);
    toggleState_2 = 0;
    SwitchState_2 = LOW;
    my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, toggleState_2);
    Serial.println("Switch-2 off");
  } 
}  

void setup()
{
    uint32_t chipId = 0;
    
    Serial.begin(115200);
    
    // Set the Relays GPIOs as output mode
    pinMode(RelayPin1, OUTPUT);
    pinMode(RelayPin2, OUTPUT);
   
    pinMode(wifiLed, OUTPUT);
    pinMode(PowerLed, OUTPUT);
    
    // Configure the input GPIOs
    pinMode(SwitchPin1, INPUT);
    pinMode(SwitchPin2, INPUT);
    pinMode(gpio_reset, INPUT);
    
    // Write to the GPIOs the default state on booting
    digitalWrite(RelayPin1, !toggleState_1);
    digitalWrite(RelayPin2, !toggleState_2);
    digitalWrite(wifiLed, LOW);
    digitalWrite(PowerLed, HIGH);

    Node my_node;    
    my_node = RMaker.initNode("ESP32_Relay_2");

    //Standard switch device
    my_switch1.addCb(write_callback);
    my_switch2.addCb(write_callback);
 

    //Add switch device to the node   
    my_node.addDevice(my_switch1);
    my_node.addDevice(my_switch2);
  

    //This is optional 
    RMaker.enableOTA(OTA_USING_PARAMS);
    //If you want to enable scheduling, set time zone for your region using setTimeZone(). 
    //The list of available values are provided here https://rainmaker.espressif.com/docs/time-service.html
    // RMaker.setTimeZone("Asia/Shanghai");
    // Alternatively, enable the Timezone service and let the phone apps set the appropriate timezone
    RMaker.enableTZService();
    RMaker.enableSchedule();

    //Service Name
    for(int i=0; i<17; i=i+8) {
      chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    Serial.printf("\nChip ID:  %d Service Name: %s\n", chipId, service_name);

    Serial.printf("\nStarting ESP-RainMaker\n");
    RMaker.start();

    WiFi.onEvent(sysProvEvent);
#if CONFIG_IDF_TARGET_ESP32
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif

    my_switch1.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);
    my_switch2.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, false);
 
}

void loop()
{
    // Read GPIO0 (external button to reset device
    if(digitalRead(gpio_reset) == LOW) { //Push button pressed
        Serial.printf("Reset Button Pressed!\n");
        // Key debounce handling
        delay(100);
        int startTime = millis();
        while(digitalRead(gpio_reset) == LOW) delay(50);
        int endTime = millis();

        if ((endTime - startTime) > 10000) {
          // If key pressed for more than 10secs, reset all
          Serial.printf("Reset to factory.\n");
          RMakerFactoryReset(2);
        } else if ((endTime - startTime) > 3000) {
          Serial.printf("Reset Wi-Fi.\n");
          // If key pressed for more than 3secs, but less than 10, reset Wi-Fi
          RMakerWiFiReset(2);
        }
    }
    delay(100);

    if (WiFi.status() != WL_CONNECTED)
    {
      //Serial.println("WiFi Not Connected");
      digitalWrite(wifiLed, false);
    }
    else
    {
      //Serial.println("WiFi Connected");
      digitalWrite(wifiLed, true);
    }

    manual_control();
}
