#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*
  Optical Heart Rate Detection (PBA Algorithm) using the MAX30105 Breakout
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  This is a demo to show the reading of heart rate or beats per minute (BPM) using
  a Penpheral Beat Amplitude (PBA) algorithm.

  It is best to attach the sensor to your finger using a rubber band or other tightening
  device. Humans are generally bad at applying constant pressure to a thing. When you
  press your finger against the sensor it varies enough to cause the blood in your
  finger to flow differently which causes the sensor readings to go wonky.

  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected

  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.
*/

#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 2; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates // store heart rate values.
byte rateSpot = 0;  // This variable keeps track of the current index in the rates array where the next heart rate value should be stored.
long lastBeat = 0; //Time at which the last beat occurred  // variable will store the timestamp of the last detected heartbeat.

float beatsPerMinute;      // storing the current heart rate in beats per minute (BPM)
int beatAvg;               // storing an average value of heart rate readings

#define SCREEN_WIDTH 128         // OLED display width, in pixels
#define SCREEN_HEIGHT 64         // OLED display height, in pixels
#define OLED_RESET    -1         // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Declaring the display name (display)

void setup()
{
  Wire.setClock(400000);                         // Set I2C speed to 400kHz
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);     // Start the OLED display
  display.setTextColor(WHITE);

  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0xC8); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 20);
  display.print("BPM:");
  display.print(0);
  display.display();

}

void loop()
{
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);



  if (irValue < 50000)
  {
    Serial.print(" No finger?");

    display.clearDisplay();
    display.setCursor(5,10);
    display.setTextSize(2);
    display.print("Place your");
    display.setCursor(30,30);
    display.setTextSize(2);
    display.print("Finger");  
    display.display();
  }else {

  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 20);
  display.print("BPM:");
  display.print(beatAvg);
  display.display();
  }

  Serial.println();
}


