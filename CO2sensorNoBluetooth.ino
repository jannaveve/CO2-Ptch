#include <Wire.h>
#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp;

/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN 17 // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 10

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ400);

#define BUTTON_PIN1 4 //good
#define BUTTON_PIN2 19 //bad

bool oldState1 = HIGH;
bool oldState2 = HIGH;
bool beingBad = false;

void setup() {
  Serial.begin(9600);
  
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);  

  Serial.println("SGP30 test");
  strip.begin();
  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);

  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
}

int counter = 0;
void loop() {
ButtonPress();  
SensorReading();
SensorPixelColor();

}


void SensorReading()
{
    // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
  //float temperature = 22.1; // [°C]
  //float humidity = 45.2; // [%RH]
  //sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");
  
  if (! sgp.IAQmeasureRaw()) {
    Serial.println("Raw Measurement failed");
    return;
  }
  //Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
  //Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");
 
  delay(1000);

  counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }
    Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
  }
  }

void SensorPixelColor()
{
      if (sgp.eCO2 < 500 && beingBad == false)
    {
      Serial.println("green");
      colorWipe(strip.Color(40, 70, 0), 50); //light
      }

    if (sgp.eCO2 > 1000 && beingBad== false)
    {
      Serial.println("GREEN");
      colorWipe(strip.Color(0, 255 ,20), 50); // dark
      }

    if (sgp.eCO2 < 500 && beingBad == true)
    {
      Serial.println("red");
      colorWipe(strip.Color(200, 40, 0), 50); // light
      }

      if (sgp.eCO2 > 1000 && beingBad == true)
    {
      Serial.println("RED");
      colorWipe(strip.Color(255, 0, 0), 50); // Dark
      }
      
    if (sgp.TVOC < 100) {
      strip.setBrightness(10);
      }

    if (sgp.TVOC >= 300) {
      strip.setBrightness(25);
      }

    if (sgp.TVOC >= 400){
      strip.setBrightness(50);
      }
    if  (sgp.TVOC >= 700){
    strip.setBrightness(100);
    }
  }

void colorWipe(uint32_t c, uint8_t wait) {
for(uint16_t i=0; i<strip.numPixels(); i++) {  

strip.setPixelColor(i, c);
strip.show();

delay(wait);
}
}

void ButtonPress()
{
  // Get current button state.
bool newState1 = digitalRead(BUTTON_PIN1);
strip.clear(); // Initialize all pixels to ‘off’
// Check if state changed from high to low (button press).
if (newState1 == LOW && oldState1 == HIGH) {
// Short delay to debounce button.
delay(20);
// Check if button is still low after debounce.
newState1 = digitalRead(BUTTON_PIN1);
if (newState1 == LOW) {
beingBad = false;
}

}

// Set the last button state to the old state.
oldState1 = newState1;

// Get current button state.
bool newState2 = digitalRead(BUTTON_PIN2);
strip.clear(); // Initialize all pixels to ‘off’
// Check if state changed from high to low (button press).
if (newState2 == LOW && oldState2 == HIGH) {
// Short delay to debounce button.
delay(20);
// Check if button is still low after debounce.
newState2 = digitalRead(BUTTON_PIN2);
if (newState2 == LOW) {
beingBad = true;
}

}

// Set the last button state to the old state.
oldState2 = newState2;

//Serial.println(beingBad);

}

  
