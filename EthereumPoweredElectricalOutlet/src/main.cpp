#include <Arduino.h>
#include "Elkrem.h"
#include <qrcode.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

#define OLED_CLK 18
#define OLED_MOSI 23

// These are needed for both hardware & softare SPI
#define OLED_CS 15
#define OLED_RESET 4
#define OLED_DC 21
//0x049ddf47e51dcc2bc9b7387b500141da15b455e516356d2d7c9d4b3a0e9e4d3e377562c6e9badac0b210b6922e59806ddd834c41e54fa7f570b1d124c7bbed2176
#define MACHINE_ACCOUNT "0xade6696F50ceC29063bF5D03a27c6F9DD1eDaC33"
#define MACHINE_PRIVATE_KEY "0x8E92C58A7CAB236FE9AF0FBB4C6F7C969F8B7806F79270734B93AB4576245DCA"
#define RelayPin 26
#define SensorValue 34
#define LedPin 16
#define Button1 13
#define Button2 14
#define Button3 27

unsigned long lastMillis = 0;
unsigned long ScreenMillis = 0;
double KwH_Purchased = 1; //Amount of electrical capacity the payee purchased
float Percentage = 0.0;
double AmpsRMS = 0; //RMS Amperage measured at any instance
double KwH = 0;     //Variable for KWH tracking per session
double Kw = 0;
bool startFlag = false;
bool testFlag = false;
bool screenFlag = false;
float KWH_Price = 0.1;

// this is software SPI, slower but any pins
Adafruit_SSD1325 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
QRcode qrcode(&display);

char welcomeMessage[] = "Hi there, I'm an Ethereum machine built using https://elkrem.io hardware. \n\n Commands list:\n \
- On\n \
- Off\n \
- Blink\n \
- Help\n\n \
You can also check out #elkremmachine on Status \
";

static const unsigned char PROGMEM logo16_glcd_bmp[] =
    {0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf2,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x73, 0x80, 0x00,
     0x00, 0x00, 0x00, 0x1c, 0x71, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x38, 0x70, 0xe0, 0x00, 0x00, 0x00,
     0x00, 0x70, 0x70, 0x70, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x70, 0x38, 0x00, 0x00, 0x00, 0x01, 0xc0,
     0xe0, 0x1c, 0x00, 0x00, 0x00, 0x03, 0x81, 0xc4, 0x0e, 0x00, 0x00, 0x00, 0x07, 0x03, 0x8e, 0x07,
     0x00, 0x00, 0x00, 0x0e, 0x07, 0x07, 0x03, 0x80, 0x00, 0x00, 0x1c, 0x0e, 0x23, 0x81, 0xc0, 0x00,
     0x00, 0x38, 0x1c, 0x71, 0xc0, 0xe0, 0x00, 0x00, 0x70, 0x08, 0xf8, 0x80, 0x70, 0x00, 0x00, 0x70,
     0x01, 0xdc, 0x00, 0x70, 0x00, 0x00, 0x38, 0x03, 0x8e, 0x00, 0xe0, 0x00, 0x00, 0x1c, 0x07, 0x07,
     0x01, 0xc0, 0x00, 0x00, 0x0e, 0x06, 0x23, 0x03, 0x80, 0x00, 0x00, 0x07, 0x00, 0x70, 0x07, 0x00,
     0x00, 0x00, 0x03, 0x80, 0x78, 0x0e, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x7c, 0x1c, 0x00, 0x00, 0x00,
     0x00, 0xe0, 0x7e, 0x38, 0x00, 0x00, 0x00, 0x00, 0x70, 0x74, 0x70, 0x00, 0x00, 0x00, 0x00, 0x38,
     0x70, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x71, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x73, 0x80,
     0x00, 0x00, 0x00, 0x00, 0x06, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x7e, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c,
     0x33, 0x0d, 0xb1, 0xe1, 0x79, 0xe0, 0x7f, 0x33, 0x19, 0xf7, 0xf1, 0xff, 0xf0, 0xc3, 0x33, 0x31,
     0xc6, 0x19, 0x86, 0x18, 0xc1, 0xb3, 0x61, 0x84, 0x19, 0x06, 0x18, 0xff, 0xb3, 0xe1, 0x8f, 0xf9,
     0x06, 0x18, 0xc0, 0x33, 0xb1, 0x84, 0x01, 0x06, 0x18, 0xc0, 0x33, 0x19, 0x86, 0x01, 0x06, 0x18,
     0x77, 0x33, 0x0d, 0x87, 0x31, 0x06, 0x18, 0x3e, 0x33, 0x0d, 0x81, 0xe1, 0x06, 0x18};

void paymentCallback(EthereumAccountTransaction *transaction)
{
  float value = (float)transaction->getValue() / 1000000000ul;
  Serial9.println("Recieved ");
  Serial9.print(value);
  Serial9.print("Ether");

  if (value >= KWH_Price)
  {
    KwH_Purchased = value / KWH_Price; //Calculate the amount of KWH purchased
    /* Give access to electricity*/
    startFlag = true;
    digitalWrite(RelayPin, HIGH);
    lastMillis = millis();
  }
}

void statusContactCallback(StatusContact *statusContact)
{
  char from[133] = "";
  statusContact->getFrom(from);
  Status.send(from, welcomeMessage);
}

void getKWattage()
{
  float VPP;
  int readValue;       //value read from the sensor
  int maxValue = 0;    // store max value here
  int minValue = 4095; // store min value here
  int mVperAmp = 185;  // use 100 for 20A Module and 66 for 30A Module
  //double Voltage = 0;
  double VRMS = 0;

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) //sample for 1 Sec
  {
    readValue = analogRead(SensorValue);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the maximum sensor value*/
      minValue = readValue;
    }
  }
  VPP = ((maxValue - minValue) * 3.3) / 4095.0;
  VRMS = (VPP / 2.0) * 0.707;
  AmpsRMS = (VRMS * 1000) / mVperAmp;
  Kw = (AmpsRMS * 220) / 1000;
  Serial9.println(Kw);
}

void statusMessageCallback(StatusMessage *statusMessage)
{
  char payload[100] = "";
  statusMessage->getContent(payload);
  char from[133] = "";
  statusMessage->getFrom(from);
  if (!strcmp(payload, "On"))
  {
    digitalWrite(RelayPin, HIGH);
  }
  else if (!strcmp(payload, "Off"))
    digitalWrite(RelayPin, LOW);
  else if (!strcmp(payload, "Blink"))
  {
    digitalWrite(RelayPin, HIGH);
    Elkrem.delay(1000);
    digitalWrite(RelayPin, LOW);
  }
  else if (!strcmp(payload, "Help"))
  {
    Status.send(from, welcomeMessage);
  }
}

void setup()
{
  Elkrem.init();
  Ethereum.trackAccount(MACHINE_ACCOUNT);
  Ethereum.onAccountTransaction(&paymentCallback);
  Status.onMessage(&statusMessageCallback);
  Status.onContact(&statusContactCallback);
  Status.account("Elkrem Outlet Machine", MACHINE_PRIVATE_KEY);

  display.begin();
  display.clearDisplay();
  display.fillScreen(BLACK);
  display.drawBitmap(37, 3, logo16_glcd_bmp, 53, 50, WHITE);
  display.display();
  delay(10000);
  //display.clearDisplay();

  pinMode(Button1, INPUT_PULLUP);
  pinMode(Button2, INPUT_PULLUP);
  pinMode(Button3, INPUT_PULLUP);
  pinMode(SensorValue, INPUT);
  pinMode(RelayPin, OUTPUT);
  pinMode(LedPin, OUTPUT);

  digitalWrite(RelayPin, LOW);

  Serial9.println("System ON");
}

void loop()
{

  /* digitalWrite(RelayPin, HIGH);
  Serial9.println("ON");
  delay(1000);
  digitalWrite(RelayPin, LOW);
  Serial9.println("OFF");
  delay(1000); */
  if (startFlag)
  {

    /* getKWattage();
    if ((millis() - ScreenMillis) > 3000)
    {
      //display.clearDisplay();
      display.fillScreen(WHITE);
      display.setCursor(3, 10);
      display.setTextColor(BLACK, WHITE);
      display.setTextSize(1);
      display.print("Kw: ");
      display.print(Kw);
      display.setCursor(3, 42);
      display.print("AMP: ");
      display.print(AmpsRMS);
      display.display();
    } */

    /*     if (millis() - lastMillis >= 60000)
    {

      KwH += ((Kw * 1) / 60);
      lastMillis = millis();
      Percentage = (KwH / KwH_Purchased) * 100;
      Serial9.print("Percentage: ");
      Serial9.println(Percentage);
      //testFlag = true;
      if ((millis() - ScreenMillis) > 3000)
      {
        display.setCursor(3, 26);
        display.print("KwH: ");
        display.print(KwH);
        display.display();
      }
    } */
    if (millis() - lastMillis >= 3000)
    {
      startFlag = false;
      digitalWrite(RelayPin, LOW);
    }
  }

  if (!digitalRead(Button1))
  {
    //testFlag = false;
    screenFlag = true;
    ScreenMillis = millis();
    display.clearDisplay();
    qrcode.create("0xade6696F50ceC29063bF5D03a27c6F9DD1eDaC33");
    display.setCursor(3, 2);
    display.setTextColor(BLACK, WHITE);
    display.setTextSize(1);
    display.println("Pay me 0.1 Eth");
    display.setCursor(3, 18);
    display.println("for 3 sec");
    display.setCursor(3, 34);
    display.println("of power");
    display.setCursor(5, 55);
    display.println("Pay");
    display.setCursor(35, 55);
    display.println("Subscribe");
    display.setCursor(100, 55);
    display.print("Chat");
    display.display();
  }
  else if (!digitalRead(Button3))
  {
    //testFlag = false;
    screenFlag = true;
    ScreenMillis = millis();
    display.clearDisplay();
    qrcode.create("0x049ddf47e51dcc2bc9b7387b500141da15b455e516356d2d7c9d4b3a0e9e4d3e377562c6e9badac0b210b6922e59806ddd834c41e54fa7f570b1d124c7bbed2176");
    display.setCursor(3, 2);
    display.setTextColor(BLACK, WHITE);
    display.setTextSize(1);
    display.println("Chat with me");
    display.setCursor(3, 18);
    display.println("On status");
    display.setCursor(3, 34);
    display.println("app");
    display.setCursor(5, 55);
    display.println("Pay");
    display.setCursor(35, 55);
    display.println("Subscribe");
    display.setCursor(100, 55);
    display.print("Chat");
    display.display();
  }
  else if (!digitalRead(Button2))
  {
    //testFlag = false;
    screenFlag = true;
    ScreenMillis = millis();
    display.clearDisplay();
    qrcode.create("https://elkrem.io/");
    display.setCursor(3, 2);
    display.setTextColor(BLACK, WHITE);
    display.setTextSize(1);
    display.println("Subscribe to our");
    display.setCursor(3, 18);
    display.println("newsletter");
    display.setCursor(3, 34);
    display.println("at this");
    display.setCursor(5, 55);
    display.println("Pay");
    display.setCursor(35, 55);
    display.println("Subscribe");
    display.setCursor(100, 55);
    display.print("Chat");
    display.display();
  }

  if ((millis() - ScreenMillis) > 10000)
  {
    display.fillScreen(BLACK);
    display.drawBitmap(37, 3, logo16_glcd_bmp, 53, 50, WHITE);
    display.setTextColor(WHITE, BLACK);
    display.setCursor(5, 55);
    display.println("Pay");
    display.setCursor(35, 55);
    display.println("Subscribe");
    display.setCursor(100, 55);
    display.print("Chat");
    display.display();
  }

  Elkrem.run();
}