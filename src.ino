
// Blynk IOT Smart Agriculture Monitoring System

#include <SPI.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>

char auth[] = "N1jlINldA-44BEcay6DW5Xdt7Mp_4Ixn";       //Authentication code sent by Blynk
/*char ssid[] = "KRESPO";                        //WiFi SSID
char pass[] = "abcd12345";                //WiFi Password*/

char ssid[] = "R-3-11 2.4Ghz";                        //WiFi SSID
char pass[] = "TIME@311";                //WiFi Password

#define BLYNK_TEMPLATE_ID "TMPLYbwc9A7U"
#define BLYNK_DEVICE_NAME "IDP"
#define BLYNK_AUTH_TOKEN "N1jlINldA-44BEcay6DW5Xdt7Mp_4Ixn"
#define BLYNK_PRINT Serial
#define ONE_WIRE_BUS D6
#define FertilizerPin D4 // Fertilizer Pin D4
#define buzzer D5 //Buzzer Pin D5
#define rainPin D7 // rain pin D7
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);// Soil temperature

int rainState = 0;
int lastRainState = 0;
const int AirValue = 1024;   //you need to replace this value with Value_1
const int WaterValue = 557;  //you need to replace this value with Value_2
const int SensorPin = A0;   // soil pin A0
int soilMoistureValue = 0;
int soilmoisturepercent = 0;
int relayFertilizer = D0;   // relay pin D0
int relayWater = D1;   // relay pin D1
#define pirPin D3
int pirValue;
int pinValue;
int fertVirValue;
const unsigned long period = 5000; // duration of pump running
unsigned long startMillis = millis();// store initial time

//Read value from blynk
BLYNK_WRITE(V0)
{
  pinValue = param.asInt(); // virtual_pin_value of IR switch
}

BLYNK_WRITE(V5)
{
  fertVirValue = param.asInt(); // virtual_pin_value of Virtual Fert Pump switch
}

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V0);  // will cause BLYNK_WRITE(V0) to be executed
  Blynk.syncVirtual(V5);  // will cause BLYNK_WRITE(V5) to be executed
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  pinMode(relayFertilizer, OUTPUT);
  pinMode(relayWater, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(FertilizerPin, INPUT_PULLUP);
  sensors.begin(); // Dallas temperature
}

void getPirValue(void)        //Get PIR Data
{
  pirValue = digitalRead(pirPin);
  if (not pirValue)
  {
    Serial.println("Motion detected!!!");
    Blynk.notify("Motion detected in your farm") ;
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);
    delay(1000);
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);
    delay(1000);
  }
  else {
    Serial.println("No Motion.");
    Blynk.notify("No Motion detected in your farm") ;
  }
}

void loop() {
  Blynk.run();
  Serial.println("Time: ");
  Serial.println(millis());

  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  Serial.println("Soil Temperature: ");
  Serial.print(temp);
  Serial.print(" ");
  Serial.println("C");
  Blynk.virtualWrite(V2, temp); //Dallas Temperature

  soilMoistureValue = analogRead(SensorPin);  //put Sensor insert into soil
  Serial.println("Soil Moisture Value: ");
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);// map value to %
  Blynk.virtualWrite(V1, soilmoisturepercent); //Soil Moisture sensor

  if (soilmoisturepercent > 100)
  {
    // display Soil Moisture%
    Serial.println("Soil Moisture% = 100 %");
  }
  
  else if (soilmoisturepercent < 0)
  {
    // display Soil Moisture%
    Serial.println("Soil Moisture% = 0 %");
  }
  
  else if (soilmoisturepercent >= 0 && soilmoisturepercent <= 100)
  {
    // display Soil Moisture%
    Serial.println("Soil Moisture% = ");
    Serial.print(soilmoisturepercent);
    Serial.println(" %");
  }
  
  if (soilmoisturepercent >= 0 && soilmoisturepercent <= 45)  // set the moisture% limit to trigger pump
  {
    Serial.println("Needs water, send notification !!!");
    //send notification
    Blynk.notify("Plants need water... Pump is activated") ;
    digitalWrite(relayWater, LOW);
    Serial.println("Pump is ON");
  }
  
  else if (soilmoisturepercent > 45 && soilmoisturepercent <= 100)
  {
    Serial.println("Soil Moisture level looks good...");
    digitalWrite(relayWater, HIGH);
    Serial.println("Pump is OFF");
  }

  else{
    Serial.println("Soil Moisture% larger than 100%");
  }

  rainState = digitalRead(rainPin);
  Serial.println("Rainstate: ");
  Serial.println(rainState);

  //rain sensor initiation
  if (rainState == 0 && lastRainState == 0) {
    Serial.println("It's Raining outside!");
    Blynk.notify("It's Raining outside!") ;
    WidgetLED led1(V3);
    led1.on();
    lastRainState = 1;
  }

  //the rain keeps on
  else if (rainState == 0 && lastRainState == 1) {
    delay(1000);
    Serial.println("Still Raining!");
  }
  
  else {
    Serial.println("No Rains...");
    WidgetLED led1(V3);
    led1.off();
    lastRainState = 0;
  }

  //code for fertilizer level sensor
  int fertilizerState = digitalRead(FertilizerPin);
  Serial.println("Fertilizer State: ");
  Serial.println(fertilizerState);

  if (fertilizerState == 0) {
    Serial.println("Fertilizer is enough.");
    Blynk.notify("Fertilizer is enough.") ;
    WidgetLED led2(V4);
    led2.on();
  }

  else if(fertilizerState == 1){
    Serial.println("Fertilizer is depleted, please replenish!!!");
    Blynk.notify("Fertilizer is depleted, please replenish!!!") ;
    WidgetLED led2(V4);
    led2.off();
  }

  // code for blynk app to switch on OR off the PIR sensor, so the buzzer can be shut off when people going in

  if (pinValue == HIGH)
  {
    getPirValue();
  }

  // code for turning on the weekly automation for FERTILIZER PUMP
  Serial.println("Fert Pump Virtual Pin: ");
  Serial.println(fertVirValue);
  

   if (fertVirValue == HIGH)
  { 
    unsigned long currentMillis = millis();// store the real time
    digitalWrite(relayFertilizer, LOW);
    delay(period);
    digitalWrite(relayFertilizer,HIGH);
    delay(55000);
  }

   else{
    digitalWrite(relayFertilizer,HIGH);
  }

  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  delay(5000);
}
