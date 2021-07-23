#include <Arduino.h>
// very useful site https://voltiq.ru/nodemcu-v3-connecting-to-arduino-via-i2c/
#include <Wire.h>
#define SLAVE_ADDR 11

//LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
//  MenuItems[] = {2, 15, 3, 2, 7, 4, 300, 200, 200, 200, 200, 360, 250, 23, 59, 23, 59, 30, 23, 59, 23, 59, 100, 12, 31, 23, 59};
int MenuItems[] = {2, 15, 3, 2, 7, 4, 300, 200, 200, 200, 200, 360, 250, 23, 59, 23, 59, 30, 23, 59, 23, 59, 100, 12, 31, 23, 59};
String FirstMenu[] = {"Sensors", "Patterns", "Setup"};
String SensorsInMenu[] = {"Hour", "Minute", "Temperature", "Water leaks", "Light sensor",
                          "Bobber_Tank_min", "Bobber_Tank_max", "Bobber 3", "Bobber 4",
                          "Limit switch 1", "Limit switch 2", "Limit switch 3", "Limit switch 4",
                          "Obtacle1 ", "Obtacle2 ", "Back"};
/*
  0 adress
  1 time
  2 Back
*/
String SetupMenu[] = {"", "", "Back"};
/*
  0 Water pattern 
  1 Led pattern 
  2 Other devices
  3 Back
*/
String PatternsMenu[] = {"Water pattern", "Led pattern", "Other devices", "Back"};
int activeEncoder;
/*
 0 Start at 
 1 Cycle duration 
 2 Angel rotation 
 3 Irrigat mix time
 4 Irrigat duraion
 5 Water mix time
 6 Water duraion 
 7 Back" 
*/
String WaterMenu[] = {"Start at ", "Cycle duration ", "Angel rotation ",
                      "Irrigat mix ", "Irrigat duraion ", "Water mix ", "Water duraion ", "Back"};
/*
0 Distance 
1 Time on 
2 Time off 
3 Back"
*/
String LedMenu[] = {"Distance ", "Time on ", "Time off ", "Back"};
// MenuCount = -1;
int MenuCount = -1;
String DeviceMenu[] = {"SOLI", "LASER1", "LASER2",
                       "VALVE1", "VALVE2", "VALVE3", "VALVE4", "VALVE5", "VALVE6",
                       "PUMP1", "PUMP2", "PUMP3", "PUMP4", "BIG_PUMP",
                       "DEVICE1", "DEVICE2", "DEVICE3", "DEVICE4", "DEVICE5", "DEVICE6", "DEVICE7", "DEVICE8",
                       "DRIVE1", "DRIVE2", "DRIVE3", "DRIVE4", "STEP1", "STEP2", "STEP3", "STEP4", "Back"};

int ActiveDev;
int UnderMenu;
int DificultDevice[] = {15, 16, 17, 18, 19, 20, 21, 56};
// Stepper adjust
#include <AccelStepper.h>
#define motorInterfaceType 1

AccelStepper stepper1 = AccelStepper(motorInterfaceType, 2, 3); // stepPin, dirPin
// Led motor
AccelStepper stepper2 = AccelStepper(motorInterfaceType, 4, 8);
// Tank motor
AccelStepper stepper3 = AccelStepper(motorInterfaceType, 11, 10);
AccelStepper stepper4 = AccelStepper(motorInterfaceType, 12, 13);
int enable1 = 58; //58
int enable2 = 59;
int enable3 = 60;
int enable4 = 61;

// TRC adjust
#include <SPI.h>
#include "RTClib.h"
RTC_DS1307 rtc;
int number = 0;
// encoder
#include "GyverEncoder.h"
#define CLK 6
#define DT 7
#define SW 5
Encoder enc1(CLK, DT, SW);
int CurrentEncoderState[3];
int PreviousEncoderState[3];
// dallas tempereture
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 9
OneWire oneWire(ONE_WIRE_BUS);
float tempC;
DallasTemperature temper(&oneWire);
int bcount = 0;
int counter = 0;

// NumberSpecialSensors=3
// hour, min, temper
int NumberSpecialSensors = 3;
// all variables
const int leng = 183;
// pins of inputs sensors is 12
int InputNumber = 12;
int ZeroSensor;
bool translateflag = 1;
/* here 12 inputs sensors: 
  pin 22  InputSensors[0]  "LEAK", 
  pin 56  InputSensors[1]  "LIGHT", 
  pin 27  InputSensors[2]  "BOBBER1",   min level water of tank
  pin 28  InputSensors[3]  "BOBBER2",   max level water of tank
  pin 35  InputSensors[4]  "BOBBER3",   
  pin 36  InputSensors[5]  "BOBBER4",
  pin 37  InputSensors[6]  "SWITCH1",   max position of step motor LED
  pin 38  InputSensors[7]  "SWITCH2", 
  pin 52  InputSensors[8]  "SWITCH3", 
  pin 53  InputSensors[9]  "SWITCH4",
  pin 54  InputSensors[10]  "OBTACLE1",  cucher laser from LED mode
  pin 55  InputSensors[11]  "OBTACLE2",
*/
int InputSensors[] = {22, 56, 27, 28, 35, 36, 37, 38, 52, 53, 54, 55};

const int OutputNumber = 30;
// pins of OUTPUTS
/* here outputs states:

  pin  24    OutputSensors[0]  "SOLI",     drain water from tank
  pin  25    OutputSensors[1]  "LASER1",   for LED mode
  pin  26    OutputSensors[2]  "LASER2", 
  pin  29    OutputSensors[3]  "VALVE1",   valve before tank
  pin  30    OutputSensors[4]  "VALVE2",   valve for irrigation
  pin  31    OutputSensors[5]  "VALVE3",   valve for waterring
  pin  32    OutputSensors[6]  "VALVE4",
  pin  33    OutputSensors[7]  "VALVE5", 
  pin  34    OutputSensors[8]  "VALVE6", 
  pin  39    OutputSensors[9]  "PUMP1",    pump before tank
  pin  40    OutputSensors[10]  "PUMP2",   mixer
  pin  41    OutputSensors[11]  "PUMP3",   pump for wattering and irrigation
  pin  42    OutputSensors[12]  "PUMP4", 
  pin  43    OutputSensors[13]  "BIG_PUMP"
  pin  44    OutputSensors[14]  "DEVICE1", 
  pin  45    OutputSensors[15]  "DEVICE2", 
  pin  46    OutputSensors[16]  "DEVICE3", 
  pin  47    OutputSensors[17]  "DEVICE4", 
  pin  48    OutputSensors[18]  "DEVICE5", 
  pin  49    OutputSensors[19]  "DEVICE6", 
  pin  50    OutputSensors[20]  "DEVICE7", 
  pin  51    OutputSensors[21]  "DEVICE8",
  pin  15,16    OutputSensors[22,23]  "DRIVE1", 
  pin  17,18    OutputSensors[24,25]  "DRIVE2", 
  pin  19,20    OutputSensors[26,27]  "DRIVE3", 
  pin  21,56    OutputSensors[28,29]  "DRIVE4",
*/
int OutputSensors[] = {24, 25, 26, 29, 30, 31, 32, 33, 34, 39, 40, 41, 42, 43, 44, 45, 46,
                       47, 48, 49, 50, 51, 15, 16, 17, 18, 19, 20, 21, 56};
unsigned long frucuenTrans;
int PreviousSensorState[leng];
int CurrentSensorState[leng];
int NewSensorState[leng];
int index[leng];
int key = 1;
int flag;
bool LedFlag = 0;
bool LcdFlag;
unsigned long TimeFromBegin;
int PinForMaster = 14;
unsigned long mil = 0;
unsigned long sec, sec1, sec2;
int n, m;
int value;
// wait till esp send first datesadDS
int FirstTimeFlag = 0;
// pin 57
int LED = 57;
int CurrentDay;
int EventHour;
int EventMin;
int EventDay;
int adjyear;
int adjmont;
int adjday;
int adjhour;
int adjmin;
int CurrentYear;
int CurrentMonth;
String adr;
String ValueOfOneString;
String ValueOfLedString;
int TankFlag = 50;
int temporaryVar;
// period of sensors checking
unsigned long SensorsCheck;

String WaterMenuValue(int j)
{
  switch (j)
  {
  case 0:
    ValueOfOneString = String(CurrentSensorState[166]) + "." + String(CurrentSensorState[165]) + "/" + String(CurrentSensorState[167]) + ":" + String(CurrentSensorState[168]);
    break;
  case 1:
    ValueOfOneString = String(CurrentSensorState[174] * 24 + CurrentSensorState[175]); //cycle
    break;
  case 2:
    ValueOfOneString = String(CurrentSensorState[178]);
    break;
  case 3:
    ValueOfOneString = String(CurrentSensorState[176]);
    break;
  case 4:
    ValueOfOneString = String(CurrentSensorState[170]);
    break;
  case 5:
    ValueOfOneString = String(CurrentSensorState[177]);
    break;
  case 6:
    ValueOfOneString = String(CurrentSensorState[169]);
    break;
  case 7:
    ValueOfOneString = "";
    break;
  }
  return ValueOfOneString;
}

String LedMenuValue(int j)
{
  switch (j)
  {
  case 0:
    ValueOfLedString = String(CurrentSensorState[leng - 4]);
    break;
  case 1:
    ValueOfLedString = String(CurrentSensorState[161]) + ":" + String(CurrentSensorState[162]); //cycle
    break;
  case 2:
    ValueOfLedString = String(CurrentSensorState[163]) + ":" + String(CurrentSensorState[164]);
    break;
  case 3:
    ValueOfLedString = "";
    break;
  }
  return ValueOfLedString;
}
int OneSensorsRequest(int temporary)
{
  int statistic[10];
  for (int j = 0; j < 10; j++)
  {
    (temporary == 0 || temporary == 6 || temporary == 7 || temporary == 8 || temporary == 9) ? statistic[j] = !digitalRead(InputSensors[temporary]) : statistic[j] = digitalRead(InputSensors[temporary]);
  }
  for (int k = 0; k < 10; k++) // sort
  {
    for (int m = 0; m < 10; m++)
    {
      if (statistic[m] > statistic[k])
      {
        temporaryVar = statistic[m];
        statistic[m] = statistic[k];
        statistic[k] = temporaryVar;
      }
    }
  }
  temporaryVar = 0;
  for (int j = 2; j < 8; j++)
  {
    temporaryVar += statistic[j];
  }
  return CurrentSensorState[NumberSpecialSensors + temporary] = round(temporaryVar / 6);
}

void SensorsInit()
{
  // pins for control step motors
  pinMode(enable1, OUTPUT);
  pinMode(enable2, OUTPUT);
  // Switch off power of step motors
  digitalWrite(enable1, HIGH);
  digitalWrite(enable2, HIGH);

  pinMode(PinForMaster, OUTPUT);
  digitalWrite(PinForMaster, LOW);
  for (int i = 0; i < InputNumber; i++)
  {
    pinMode(InputSensors[i], INPUT);
    (i == 6 || i == 7 || i == 8 || i == 9) ? pinMode(InputSensors[i], INPUT_PULLUP) : pinMode(InputSensors[i], INPUT);
    OneSensorsRequest(i);
  }
  for (int i = 0; i < OutputNumber; i++)
  {
    pinMode(OutputSensors[i], OUTPUT);
    CurrentSensorState[1 + NumberSpecialSensors + InputNumber + i] = 0;
  }
  pinMode(LED, OUTPUT);
}

void EventTimeUpdate(int some)
{
  DateTime future(rtc.now() + TimeSpan(0, 0, some, 0));
  EventDay = future.day();
  EventHour = future.hour();
  EventMin = future.minute();
}

// Here I override CurrentSensorState according sensors
void SensorsRequest()
{
  // sensors setup
  for (int i = 53; i < 154; i = i + 4)
  {

    if (CurrentSensorState[i] != CurrentSensorState[i + 2] || CurrentSensorState[i + 1] != CurrentSensorState[i + 3])
    {
      if (CurrentSensorState[i] == CurrentSensorState[0] && CurrentSensorState[i + 1] == CurrentSensorState[1])
      {
        CurrentSensorState[19 + (i - 53) / 4] = 1;
        digitalWrite(OutputSensors[(i - 53) / 4], HIGH);
        flag = 1;
        Serial.println("two");
      }
      else if (CurrentSensorState[i + 2] == CurrentSensorState[0] && CurrentSensorState[i + 3] == CurrentSensorState[1])
      {
        CurrentSensorState[19 + (i - 53) / 4] = 0;
        digitalWrite(OutputSensors[(i - 53) / 4], LOW);
        flag = 1;
        Serial.println("tree");
      }
    }
  }
  for (int i = 0; i < InputNumber; i++)
  {
    CurrentSensorState[NumberSpecialSensors + i] = OneSensorsRequest(i);
    if (CurrentSensorState[NumberSpecialSensors + i] != PreviousSensorState[NumberSpecialSensors + i])
    {
      flag = 1;
      Serial.println("four  ");
    }
  }
  // tank adjust
  if (CurrentSensorState[177] > 0)
  {
    if (digitalRead(InputSensors[2]) && TankFlag == 0 && CurrentSensorState[170] > 0) // drain water
    {
      digitalWrite(OutputSensors[0], HIGH); // soli on
      TankFlag++;
      Serial.println("drain water");
    }
    if (!digitalRead(InputSensors[2]) && TankFlag < 2 && CurrentSensorState[170] > 0)
    {
      digitalWrite(OutputSensors[0], LOW); // soli off
      TankFlag = 2;
      delay(500);
    }
    // fill tank
    if (!digitalRead(InputSensors[3]) && TankFlag == 2 && CurrentSensorState[170] > 0) // if max bobber show low level
    {
      digitalWrite(OutputSensors[3], HIGH); // open valve before tank
      delay(500);
      digitalWrite(OutputSensors[9], HIGH); // pump1 on
      Serial.println("fill tank");
      TankFlag++; // 3
    }
    if (digitalRead(InputSensors[3]) && TankFlag == 3 && CurrentSensorState[170] > 0) // when tank is fill
    {
      digitalWrite(OutputSensors[9], LOW); // off pump
      digitalWrite(OutputSensors[3], LOW); // close valve
      TankFlag++;                          //4
    }
    // turn reel
    if (TankFlag == 4 && CurrentSensorState[170] > 0)
    {

      Serial.println("turn stepper");

      digitalWrite(enable1, LOW);
      delay(500);
      stepper1.setCurrentPosition(0);
      while (stepper1.currentPosition() != round(CurrentSensorState[leng - 5] / 1.8))
      {
        stepper1.setSpeed(500);
        stepper1.runSpeed();
      }
      delay(300);
      Serial.println("Start mix");
      digitalWrite(enable1, HIGH);
      delay(300);
      digitalWrite(OutputSensors[10], HIGH); // start mixing
      EventTimeUpdate(CurrentSensorState[176]);
      TankFlag++; // 5
    }
    if (CurrentSensorState[1] == EventMin && CurrentSensorState[0] == EventHour && CurrentDay == EventDay && TankFlag == 5 && CurrentSensorState[170] > 0)
    {
      digitalWrite(OutputSensors[10], LOW); // stop mixing
      EventTimeUpdate(CurrentSensorState[170]);
      delay(300);
      digitalWrite(OutputSensors[4], HIGH); // open valve for irrigation
      delay(300);
      digitalWrite(OutputSensors[11], HIGH); // water all
      TankFlag++;
      delay(300);
      // 7
    }
    if (CurrentSensorState[1] == EventMin && CurrentSensorState[0] == EventHour && CurrentDay == EventDay && TankFlag == 6 && CurrentSensorState[170] > 0)
    {
      Serial.println("irrigation");
      digitalWrite(OutputSensors[11], LOW); // stop water
      digitalWrite(OutputSensors[4], LOW);  // close valve
      TankFlag++;
      delay(300);

      // 7
    }
    if (digitalRead(InputSensors[2]) && TankFlag == 7 && CurrentSensorState[170] > 0) // drain water
    {
      delay(300);
      digitalWrite(OutputSensors[0], HIGH); // soli on
      TankFlag++;

      Serial.println("draun tank after irrigation");
    }
    if (!digitalRead(InputSensors[2]) && (TankFlag == 7 || TankFlag == 8) && CurrentSensorState[170] > 0)
    {
      digitalWrite(OutputSensors[0], LOW); // soli off
      TankFlag = 9;
      delay(300);
    }
  }
  if (CurrentSensorState[170] == 0 && TankFlag == 0)
    TankFlag = 9;

  // I finised irrigation
  if (digitalRead(InputSensors[2]) && TankFlag == 9 && CurrentSensorState[169] > 0) // drain water
  {
    delay(300);
    digitalWrite(OutputSensors[0], HIGH); // soli on
    Serial.println("drain tank before watering");
    TankFlag++;
  }
  if (!digitalRead(InputSensors[2]) && (TankFlag == 9 || TankFlag == 10) && CurrentSensorState[169] > 0)
  {
    digitalWrite(OutputSensors[0], LOW); // soli off
    TankFlag = 11;
    delay(100);
  }
  // fill tank
  if (!digitalRead(InputSensors[3]) && TankFlag == 11 && CurrentSensorState[169] > 0)
  {
    Serial.println("fill tank for watering");
    delay(300);
    digitalWrite(OutputSensors[3], HIGH); // open valve before tank
    delay(300);
    digitalWrite(OutputSensors[9], HIGH); // pump1 on
    TankFlag++;                           // 3
  }
  if (digitalRead(InputSensors[3]) && TankFlag == 12 && CurrentSensorState[169] > 0) // when tank is fill
  {
    digitalWrite(OutputSensors[9], LOW); // off pump
    digitalWrite(OutputSensors[3], LOW); // close valve
    TankFlag++;                          //4
  }

  if ((TankFlag == 13 && CurrentSensorState[169] > 0))
  {
    Serial.println("turn stepper");
    delay(100);
    digitalWrite(enable1, LOW);
    stepper1.setCurrentPosition(0);
    while (stepper1.currentPosition() != round(CurrentSensorState[leng - 5] / 1.8))
    {
      stepper1.setSpeed(600);
      stepper1.runSpeed();
    }
    delay(500);
    digitalWrite(enable1, HIGH);
    delay(500);
    digitalWrite(OutputSensors[10], HIGH); // start mixing

    Serial.println("start mixing");
    EventTimeUpdate(CurrentSensorState[177]);
    delay(300);
    TankFlag++;
  }
  if (CurrentSensorState[1] == EventMin && CurrentSensorState[0] == EventHour && CurrentDay == EventDay && TankFlag == 14 && CurrentSensorState[169] > 0)
  {
    delay(500);
    Serial.println("waterring");
    digitalWrite(OutputSensors[10], LOW);  // stop mixing
    digitalWrite(OutputSensors[11], HIGH); // water all
    delay(500);
    digitalWrite(OutputSensors[5], HIGH); // open correct valve
    EventTimeUpdate(CurrentSensorState[169]);
    TankFlag++;
    delay(500);
    // 13
  }
  if (CurrentSensorState[1] == EventMin && CurrentSensorState[0] == EventHour && CurrentDay == EventDay && TankFlag == 15 && CurrentSensorState[169] > 0)
  {
    delay(500);
    digitalWrite(OutputSensors[11], LOW); // stop water
    digitalWrite(OutputSensors[5], LOW);  // close valve
    TankFlag++;
    delay(500);
    // 14
  }
  if (digitalRead(InputSensors[2]) && TankFlag == 16 && CurrentSensorState[169] > 0) // drain water
  {
    delay(500);
    digitalWrite(OutputSensors[0], HIGH); // soli on
    TankFlag++;
    delay(500);
    Serial.println("drain after watering");
  }
  if (!digitalRead(InputSensors[2]) && (TankFlag == 17 || TankFlag == 16) && CurrentSensorState[169] > 0)
  {
    digitalWrite(OutputSensors[0], LOW); // soli off
    TankFlag = 18;
    delay(500);
  }
  // second step for LED adjust
  // if it's time for checking
  if (CurrentSensorState[161] == CurrentSensorState[0] && CurrentSensorState[162] == CurrentSensorState[1] && (CurrentSensorState[161] != CurrentSensorState[163] || CurrentSensorState[162] != CurrentSensorState[164]) && !LedFlag)
  {
    digitalWrite(OutputSensors[1], HIGH);
    delay(500);
    while (!digitalRead(InputSensors[10]) && !OneSensorsRequest(6)) // while the is obtacle and no switch
    {
      // move step

      digitalWrite(enable2, LOW);
      delay(300);
      stepper2.setCurrentPosition(0);
      CurrentSensorState[16] += CurrentSensorState[leng - 4];
      while (stepper2.currentPosition() != round(CurrentSensorState[leng - 4] / 0.2 * 200))
      {
        stepper2.setSpeed(700);
        stepper2.runSpeed();
      }

      digitalWrite(enable2, HIGH);
      delay(1000);
    }

    if (!digitalRead(InputSensors[6])) // if switc don't reach
      digitalWrite(LED, HIGH);         //led on
    LedFlag = 1;
    digitalWrite(OutputSensors[1], LOW); //laser off
  }
  if (CurrentSensorState[163] == CurrentSensorState[0] && CurrentSensorState[164] == CurrentSensorState[1] && (CurrentSensorState[161] != CurrentSensorState[163] || CurrentSensorState[162] != CurrentSensorState[164]))
  {
    digitalWrite(LED, LOW);
    LedFlag = 0;
  }
}

void LCD_request()
{
  lcd.setCursor(0, 0);
  switch (MenuCount)
  {
  case -1: // main menu
    lcd.clear();
    for (int i = 0; i < 3; i++)
    {
      lcd.setCursor(0, i);
      (CurrentEncoderState[0] == i) ? lcd.print(">") : lcd.print(" ");
      lcd.print(FirstMenu[i]);
    }
    break;
  case 0: //Sensors submenu
    lcd.clear();
    for (int i = -1; i < 3; i++)
    {
      lcd.setCursor(0, 1 + i);
      (i == 0) ? lcd.print(">") : lcd.print(" ");
      if (CurrentEncoderState[0] + i == 1)
      {
        lcd.print("Time ");
        lcd.print(CurrentSensorState[0]);
        lcd.print(":");
        lcd.print(CurrentSensorState[1]);
      }
      else if (CurrentEncoderState[0] + i == MenuItems[MenuCount + 1] || CurrentEncoderState[0] + i == 0)
      {
        lcd.print("Back");
      }
      else if (CurrentEncoderState[0] + i > 1 && CurrentEncoderState[0] + i < MenuItems[MenuCount + 1])
      {
        lcd.print(SensorsInMenu[CurrentEncoderState[0] + i]);
        lcd.print(" ");
        lcd.print(CurrentSensorState[CurrentEncoderState[0] + i]);
      }
      else if (CurrentEncoderState[0] + i < 1)
      {
        lcd.print(SensorsInMenu[CurrentEncoderState[0] - i]);
        lcd.print(" ");
        lcd.print(CurrentSensorState[CurrentEncoderState[0] - i]);
      }
      else if (CurrentEncoderState[0] + i > MenuItems[MenuCount + 1])
      {
        if (CurrentEncoderState[0] + i - MenuItems[MenuCount + 1] == 1)
        {
          lcd.print("Time ");
          lcd.print(CurrentSensorState[0]);
          lcd.print(":");
          lcd.print(CurrentSensorState[1]);
        }
        else
        {
          // тут печатаются минуты
          lcd.print(SensorsInMenu[CurrentEncoderState[0] + i - MenuItems[MenuCount + 1]]);
          lcd.print(" ");
          lcd.print(CurrentSensorState[CurrentEncoderState[0] + 1 + i - MenuItems[MenuCount + 1]]);
        }
      }
    }
    break;

  case 1: // main menu
    lcd.clear();
    for (int i = -1; i < 3; i++)
    {
      lcd.setCursor(0, 1 + i);
      (i == CurrentEncoderState[0] - 1) ? lcd.print(">") : lcd.print(" ");
      lcd.print(PatternsMenu[i + 1]);
    }
    break;
  case 2: // main menu
    lcd.clear();
    for (int i = 0; i < 3; i++)
    {
      lcd.setCursor(0, i);
      (CurrentEncoderState[0] == i) ? lcd.print(">") : lcd.print(" ");
      lcd.print(SetupMenu[i]);
    }
    break;
  case 3: // water menu
    lcd.clear();
    for (int i = -1; i < 3; i++)
    {
      lcd.setCursor(0, 1 + i);
      (i == 0) ? lcd.print(">") : lcd.print(" ");
      if (CurrentEncoderState[0] + i >= 0 && CurrentEncoderState[0] + i <= MenuItems[MenuCount + 1])
      {
        lcd.print(WaterMenu[CurrentEncoderState[0] + i]);
        lcd.print(WaterMenuValue(CurrentEncoderState[0] + i));
      }
      else if (CurrentEncoderState[0] + i > MenuItems[MenuCount + 1])
      {
        lcd.print(WaterMenu[CurrentEncoderState[0] + i - 8]);
        lcd.print(WaterMenuValue(CurrentEncoderState[0] + i - 8));
      }
      else if (CurrentEncoderState[0] + i == -1)
      {
        lcd.print(WaterMenu[MenuItems[MenuCount + 1]]);
        lcd.print(WaterMenuValue(MenuItems[MenuCount + 1]));
      }
    }
    break;

  case 4: // main menu
    lcd.clear();
    for (int i = 0; i < 4; i++)
    {
      lcd.setCursor(0, i);
      (CurrentEncoderState[0] == i) ? lcd.print(">") : lcd.print(" ");
      lcd.print(LedMenu[i]);
      lcd.print(LedMenuValue(i));
    }
    break;
  case 5: // main menu
    lcd.setCursor(16, 1);
    lcd.print("   ");
    lcd.setCursor(16, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 6: // main menu
    lcd.setCursor(16, 1);
    lcd.print("   ");
    lcd.setCursor(16, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 7: // main menu
    lcd.setCursor(13, 1);
    lcd.print("   ");
    lcd.setCursor(13, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 8: // main menu
    lcd.setCursor(17, 1);
    lcd.print("   ");
    lcd.setCursor(17, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 9: // main menu
    lcd.setCursor(11, 1);
    lcd.print("   ");
    lcd.setCursor(11, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 10: // main menu
    lcd.setCursor(15, 1);
    lcd.print("   ");
    lcd.setCursor(15, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 11: // main menu
    lcd.setCursor(10, 0);
    lcd.print("   ");
    lcd.setCursor(10, 0);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 12: // main menu
    lcd.setCursor(9, 1);
    lcd.print("  ");
    lcd.setCursor(9, 1);
    lcd.print(CurrentEncoderState[0]);
    lcd.print(":");
    lcd.print(CurrentSensorState[162]);
    break;
  case 13: // main menu
    lcd.setCursor(9, 1);
    lcd.print("       ");
    lcd.setCursor(9, 1);
    lcd.print(CurrentSensorState[161]);
    lcd.print(":");
    lcd.print(CurrentEncoderState[0]);
    break;
  case 14: // main menu
    lcd.setCursor(10, 2);
    lcd.print("  ");
    lcd.setCursor(10, 2);
    lcd.print(CurrentEncoderState[0]);
    lcd.print(":");
    lcd.print(CurrentSensorState[164]);
    break;
  case 15: // main menu
    lcd.setCursor(10, 2);
    lcd.print("       ");
    lcd.setCursor(10, 2);
    lcd.print(CurrentSensorState[163]);
    lcd.print(":");
    lcd.print(CurrentEncoderState[0]);
    break;
  case 16: // main menu
    lcd.clear();
    for (int i = -1; i < 1; i++)
    {
      lcd.setCursor(0, 1 + i);
      (i == 0) ? lcd.print(">") : lcd.print(" ");
      if (CurrentEncoderState[0] + i >= 0 && CurrentEncoderState[0] + i <= 30)
      {
        lcd.print(DeviceMenu[CurrentEncoderState[0] + i]);
      }
      else if (CurrentEncoderState[0] + i == -1)
      {
        lcd.print(DeviceMenu[MenuItems[MenuCount + 1]]);
      }
    }
    lcd.setCursor(1, 2);
    if (CurrentEncoderState[0] < 28)
    {
      lcd.print("on ");
      lcd.print(CurrentSensorState[53 + CurrentEncoderState[0] * 4]);
      lcd.print(":");
      lcd.print(CurrentSensorState[53 + CurrentEncoderState[0] * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[53 + CurrentEncoderState[0] * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[53 + CurrentEncoderState[0] * 4 + 3]);
    }
    else if (CurrentEncoderState[0] != 30)
    {
      lcd.print("on ");
      lcd.print(CurrentSensorState[45 + (CurrentEncoderState[0] - 28) * 4]);
      lcd.print(":");
      lcd.print(CurrentSensorState[45 + (CurrentEncoderState[0] - 28) * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[45 + (CurrentEncoderState[0] - 28) * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[45 + (CurrentEncoderState[0] - 28) * 4 + 3]);
    }
    else
      lcd.print(DeviceMenu[0]);
    lcd.setCursor(1, 3);
    if (CurrentEncoderState[0] != 30)
      lcd.print(DeviceMenu[CurrentEncoderState[0] + 1]);
    else
      lcd.print(DeviceMenu[1]);

    break;
  case 17: // main menu
    //lcd.setCursor(1, 2);
    //lcd.print("                              ");
    lcd.setCursor(1, 2);
    lcd.print("on ");
    lcd.print(CurrentEncoderState[0]);
    lcd.print(":");
    if (activeEncoder < 28)
    {
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 3]);
    }
    else
    {
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 3]);
    }
    break;
  case 18: // main menu
           // lcd.setCursor(1, 2);
           // lcd.print("                              ");
    lcd.setCursor(1, 2);
    lcd.print("on ");

    if (activeEncoder < 28)
    {
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 3]);
    }
    else
    {
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 3]);
    }
    break;
  case 19: // main menu
           // lcd.setCursor(1, 2);
           // lcd.print("                              ");
    lcd.setCursor(1, 2);
    lcd.print("on ");

    if (activeEncoder < 28)
    {
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 3]);
    }
    else
    {
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 3]);
    }
    break;
  case 20: // main menu
           // lcd.setCursor(1, 2);
           // lcd.print("                              ");
    lcd.setCursor(1, 2);
    lcd.print("on ");

    if (activeEncoder < 28)
    {
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[53 + activeEncoder * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(" ");
    }
    else
    {
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[45 + (activeEncoder - 28) * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(" ");
    }
    break;
  case 21: // change year
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.print(SetupMenu[0]);
    lcd.setCursor(0, 1);
    lcd.print(">");
    lcd.print(CurrentEncoderState[0] + 2021);
    lcd.print(".");
    lcd.print(CurrentMonth);
    lcd.print(".");
    lcd.print(CurrentDay);
    lcd.print("  ");
    lcd.print(CurrentSensorState[0]);
    lcd.print(":");
    lcd.print(CurrentSensorState[1]);
    lcd.setCursor(0, 2);
    lcd.print(" ");
    lcd.print(SetupMenu[2]);
    break;
  case 22: // change mounth
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.print(SetupMenu[0]);
    lcd.setCursor(0, 1);
    lcd.print(">");
    lcd.print(adjyear);
    lcd.print(".");
    lcd.print(CurrentEncoderState[0]);
    lcd.print(".");
    lcd.print(CurrentDay);
    lcd.print("  ");
    lcd.print(CurrentSensorState[0]);
    lcd.print(":");
    lcd.print(CurrentSensorState[1]);
    lcd.setCursor(0, 2);
    lcd.print(" ");
    lcd.print(SetupMenu[2]);
    break;
  case 23: // change hour
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.print(SetupMenu[0]);
    lcd.setCursor(0, 1);
    lcd.print(">");
    lcd.print(adjyear);
    lcd.print(".");
    lcd.print(adjmont);
    lcd.print(".");
    lcd.print(CurrentEncoderState[0]);
    lcd.print("  ");
    lcd.print(CurrentSensorState[0]);
    lcd.print(":");
    lcd.print(CurrentSensorState[1]);
    lcd.setCursor(0, 2);
    lcd.print(" ");
    lcd.print(SetupMenu[2]);
    break;
  case 24: // change minute
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.print(SetupMenu[0]);
    lcd.setCursor(0, 1);
    lcd.print(">");
    lcd.print(adjyear);
    lcd.print(".");
    lcd.print(adjmont);
    lcd.print(".");
    lcd.print(adjday);
    lcd.print("  ");
    lcd.print(CurrentEncoderState[0]);
    lcd.print(":");
    lcd.print(CurrentSensorState[1]);
    lcd.setCursor(0, 2);
    lcd.print(" ");
    lcd.print(SetupMenu[2]);
    break;
  case 25: // change minute
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.print(SetupMenu[0]);
    lcd.setCursor(0, 1);
    lcd.print(">");
    lcd.print(adjyear);
    lcd.print(".");
    lcd.print(adjmont);
    lcd.print(".");
    lcd.print(adjday);
    lcd.print("  ");
    lcd.print(adjhour);
    lcd.print(":");
    lcd.print(CurrentEncoderState[0]);
    lcd.setCursor(0, 2);
    lcd.print(" ");
    lcd.print(SetupMenu[2]);
    break;
  }
}

// Get date from Master
void receiveEvent()
{
  value = Wire.read();
  if (FirstTimeFlag == 0 && value != 254 && value != 255)
  {
    SetupMenu[0] += char(value);
  }
  if (value == 255)
  {
    number = -1;
  }
  if (number != -1)
  {
    NewSensorState[number] = value;
  }
  if (number == 0)
  {
    Serial.println();
    Serial.println("I start to getting date from Master");
  }
  Serial.print(value);
  Serial.print(" ");
  if (value == 254)
  {
    if (FirstTimeFlag > 0)
    {
      // i increase value ig degree becouse i afraid its gona be too big
      for (int i = 0; i < number; i = i + 2)
      {
        CurrentSensorState[NewSensorState[i]] = NewSensorState[i + 1];
        if (FirstTimeFlag == 1)
          PreviousSensorState[NewSensorState[i]] = CurrentSensorState[NewSensorState[i]];
      }
      Serial.println();
      Serial.println("CurrentSensorState after master");
      for (int i = 0; i < leng; i++)
      {
        Serial.print(" ");
        Serial.print(CurrentSensorState[i]);
      }
      Serial.println();
      Serial.println("I success finished getting date from Master");
      FirstTimeFlag++;
      //flag = 0;
    }
    else if (FirstTimeFlag == 0)
    {
      FirstTimeFlag = 1;
      Serial.println();
      Serial.print("adr  ");
      Serial.println(SetupMenu[0]);
      delay(1000);

      //TimeFromBegin = millis();
    }
    TimeFromBegin = millis();
    flag = 1;
    Serial.println("thix");
  }
  (number == leng - 1) ? number = 0 : number++;
}

// Send date to Master
void requestEvent()
{
  // Define a byte to hold data
  byte bval;
  // Cycle through data
  // First response is always 255 to mark beginning
  switch (bcount)
  {
  case 0:
    digitalWrite(PinForMaster, LOW);
    bval = 255; // start key
    if (key != 1)
      Serial.println("Translation begin");
    break;
  default:
    if ((bcount - 1) % 2 == 0 && bcount != key) //0,2,4,6,8
    {
      counter = (bcount - 1) / 2;
      bval = index[counter];
    }
    else if (bcount != key) //  1,3,5,7
    {
      counter = ((bcount - 1) - 1) / 2;
      bval = CurrentSensorState[index[counter]];
    }
    else if (bcount == key)
    {
      if (key != 1)
      {
        Serial.println(254);
        Serial.println("Translation stop");
      }
      key = 1;
      bval = 254;
    }

    break;
  }
  if (key != 1)
  {
    Serial.print(bval);
    Serial.print(" ");
    if (bcount == key)
      Serial.println("Translation stop");
  }
  Wire.write(bval);
  bcount = bcount + 1;
  if (bcount > key)
    bcount = 0;
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("arduino Mega Start");

  SensorsInit();

  while (!Serial)
    ; // for Leonardo/Micro/Zero
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }
  if (!rtc.isrunning())
  {
    Serial.println("RTC is NOT running!");
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  temper.begin();
  enc1.setType(TYPE2);
  stepper1.setMaxSpeed(600);
  stepper2.setMaxSpeed(600);
  stepper3.setMaxSpeed(600);
  stepper4.setMaxSpeed(600);
  // stepper.step(STEPS*8);
  // initialize the lcd
  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(receiveEvent); // Get date from Master
  Wire.onRequest(requestEvent); // Send date to Master
  CurrentSensorState[leng - 2] = 233;
  CurrentSensorState[leng - 1] = 254;
  for (int i = 0; i < leng; i++)
  {
    Serial.print(CurrentSensorState[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("Setup end");
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 1);
  lcd.print("  PLEASE  WAIT");
}

void loop()
{
  if (FirstTimeFlag > 1)
  {

    if (millis() - sec > 200 && value == 254)
    {
      sec = millis();
      DateTime now = rtc.now();
      CurrentDay = now.day();
      ZeroSensor = now.second();
      CurrentSensorState[0] = now.hour();
      CurrentSensorState[1] = now.minute();
      CurrentYear = now.year();
      CurrentMonth = now.month();
      SetupMenu[1] = String(now.year()) + "." + String(CurrentDay) + "." + String(now.month()) + "  " + String(PreviousSensorState[0]) + ":" + String(PreviousSensorState[1]);
      SensorsRequest();
      if (CurrentSensorState[1] != PreviousSensorState[1] && CurrentSensorState[1] < 60 && CurrentSensorState[1] > -1 && ZeroSensor < 60)
      {
        flag = 1;
        Serial.println("ethgh");
        // if cycle start
        if (CurrentSensorState[1] == CurrentSensorState[168] && CurrentSensorState[0] == CurrentSensorState[167] && CurrentDay == CurrentSensorState[166])
        {
          DateTime future(rtc.now() + TimeSpan(CurrentSensorState[174], CurrentSensorState[175], 0, 0));
          CurrentSensorState[166] = future.day();
          CurrentSensorState[167] = future.hour();
          CurrentSensorState[168] = future.minute();
          TankFlag = 0;
        }
        delay(100);
        temper.requestTemperatures();                      // Send the command to get temperatures
        CurrentSensorState[2] = temper.getTempCByIndex(0); // temperature
        delay(100);
      }
    }

    enc1.tick();
    if (enc1.isRight())
    {
      CurrentEncoderState[0]--;
      if (MenuCount == 0 && CurrentEncoderState[0] < 1)
      {
        CurrentEncoderState[0] = MenuItems[MenuCount + 1];
      }
      else if (CurrentEncoderState[0] < 0 && MenuCount != 0)
      {
        CurrentEncoderState[0] = MenuItems[MenuCount + 1];
      }
      LcdFlag = 1;
    }

    if (enc1.isLeft())
    {
      CurrentEncoderState[0]++;
      if (CurrentEncoderState[0] > MenuItems[MenuCount + 1])
      {
        if (MenuCount == 0)
          CurrentEncoderState[0] = 1;
        else
          CurrentEncoderState[0] = 0;
      }
      LcdFlag = 1;
    }

    if (enc1.isHolded())
    {
      MenuCount = -1;
      LcdFlag = 1;
    }

    if (enc1.isPress()) // push button
    {

      if (MenuCount == 3 || MenuCount == 4 || MenuCount == 16)
      {
        activeEncoder = CurrentEncoderState[0];
      }
      else if (MenuCount == 1)
        activeEncoder = 0;

      CurrentEncoderState[1] = CurrentEncoderState[0];
      if (MenuCount == -1) // go out from first page
      {
        MenuCount = CurrentEncoderState[1];
      }
      else if (MenuCount == 2 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // go out from first page
      {
        MenuCount = -1;
      }
      else if (MenuCount == 2 && CurrentEncoderState[0] == 1) // go to the Devices page
      {
        MenuCount = 21;
      }
      else if (MenuCount == 21) // go to the Devices page
      {
        MenuCount++;
        adjyear = CurrentEncoderState[1] + 2021;
      }
      else if (MenuCount == 22) // go to the Devices page
      {
        MenuCount++;
        adjmont = CurrentEncoderState[1];
      }
      else if (MenuCount == 23) // go to the Devices page
      {
        MenuCount++;
        adjday = CurrentEncoderState[1];
      }
      else if (MenuCount == 24) // go to the Devices page
      {
        MenuCount++;
        adjhour = CurrentEncoderState[1];
      }
      else if (MenuCount == 25) // go to the Devices page
      {
        MenuCount = 2;
        adjmin = CurrentEncoderState[1];
        rtc.adjust(DateTime(adjyear, adjmont, adjday, adjhour, adjmin, 0));
        DateTime now = rtc.now();
        CurrentDay = now.day();
        ZeroSensor = now.second();
        PreviousSensorState[0] = now.hour();
        PreviousSensorState[1] = now.minute();
        CurrentYear = now.year();
        CurrentMonth = now.month();
        SetupMenu[1] = String(now.year()) + "." + String(CurrentDay) + "." + String(now.month()) + "  " + String(PreviousSensorState[0]) + ":" + String(PreviousSensorState[1]);
      }
      else if (MenuCount == 21) // go to the Devices page
      {
        MenuCount++;
        adjyear = CurrentEncoderState[1];
      }
      else if (MenuCount == 0 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // go to the first page
      {
        MenuCount = -1;
      }
      else if (MenuCount == 1 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // go to the first page
      {
        MenuCount = -1;
      }
      else if (MenuCount == 1 && CurrentEncoderState[0] != MenuItems[MenuCount + 1] && CurrentEncoderState[1] != 2) // go to the WaterPattern page
      {
        MenuCount = CurrentEncoderState[1] + 3;
      }
      else if (MenuCount == 1 && CurrentEncoderState[0] == 2) // go to the Devices page
      {
        MenuCount = 16;
      }
      else if (MenuCount == 16 && CurrentEncoderState[0] != MenuItems[MenuCount + 1]) // go to the Devices page
      {
        MenuCount = 17;
      }
      else if (MenuCount == 17) // go to the Devices page
      {
        MenuCount = 18;
        (activeEncoder < 28) ? CurrentSensorState[53 + activeEncoder * 4 + 0] = CurrentEncoderState[0] : CurrentSensorState[45 + (activeEncoder - 28) * 4 + 0] = CurrentEncoderState[0];
      }
      else if (MenuCount == 18) // go to the Devices page
      {
        MenuCount = 19;
        (activeEncoder < 28) ? CurrentSensorState[53 + activeEncoder * 4 + 1] = CurrentEncoderState[0] : CurrentSensorState[45 + (activeEncoder - 28) * 4 + 1] = CurrentEncoderState[0];
      }
      else if (MenuCount == 19) // go to the Devices page
      {

        (activeEncoder < 28) ? CurrentSensorState[53 + activeEncoder * 4 + 2] = CurrentEncoderState[0] : CurrentSensorState[45 + (activeEncoder - 28) * 4 + 2] = CurrentEncoderState[0];
        MenuCount = 20;
      }
      else if (MenuCount == 20) // go to the Devices page
      {
        (activeEncoder < 28) ? CurrentSensorState[53 + activeEncoder * 4 + 3] = CurrentEncoderState[0] : CurrentSensorState[45 + (activeEncoder - 28) * 4 + 3] = CurrentEncoderState[0];
        MenuCount = 16;
      }
      else if (MenuCount == 16 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // go to the Devices page
      {
        MenuCount = 1;
      }
      else if (MenuCount == 2 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // go out from first page
      {
        MenuCount = -1;
      }
      else if (MenuCount == 3 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // go to the Pattern page
      {
        MenuCount = 1;
      }
      else if (MenuCount == 3 && CurrentEncoderState[0] == 1) // change cycle
      {
        MenuCount = 5;
      }
      else if (MenuCount == 5) // back to water menu
      {
        CurrentSensorState[175] = CurrentEncoderState[1] % 24;
        CurrentSensorState[174] = (CurrentEncoderState[1] - CurrentSensorState[175]) / 24;
        DateTime future(rtc.now() + TimeSpan(CurrentSensorState[174], CurrentSensorState[175], 0, 0));
        CurrentSensorState[166] = future.day();
        CurrentSensorState[167] = future.hour();
        CurrentSensorState[168] = future.minute();
        MenuCount = 3;
        flag = 1;
        Serial.println("twelve");
      }
      else if (MenuCount == 3 && CurrentEncoderState[0] == 2) // change cycle
      {
        MenuCount = 6;
      }
      else if (MenuCount == 6) // go to the WaterPattern page
      {
        CurrentSensorState[178] = CurrentEncoderState[1];
        MenuCount = 3;
      }
      else if (MenuCount == 3 && CurrentEncoderState[0] == 3) // change cycle
      {
        MenuCount = 7;
      }
      else if (MenuCount == 7) // go to the WaterPattern page
      {
        CurrentSensorState[176] = CurrentEncoderState[1];
        MenuCount = 3;
      }

      else if (MenuCount == 3 && CurrentEncoderState[0] == 4) // change cycle
      {
        MenuCount = 8;
      }
      else if (MenuCount == 8) // go to the WaterPattern page
      {
        CurrentSensorState[170] = CurrentEncoderState[1];
        MenuCount = 3;
      }

      else if (MenuCount == 3 && CurrentEncoderState[0] == 5) // change cycle
      {
        MenuCount = 9;
      }
      else if (MenuCount == 9) // go to the WaterPattern page
      {
        CurrentSensorState[177] = CurrentEncoderState[1];
        MenuCount = 3;
      }

      else if (MenuCount == 3 && CurrentEncoderState[0] == 6) // change cycle
      {
        MenuCount = 10;
      }
      else if (MenuCount == 10) // go to the WaterPattern page
      {
        CurrentSensorState[169] = CurrentEncoderState[1];
        MenuCount = 3;
      }

      else if (MenuCount == 4 && CurrentEncoderState[0] == 3) // change cycle
      {
        MenuCount = 1;
      }
      else if (MenuCount == 4 && CurrentEncoderState[0] == 0) // go to the WaterPattern page
      {
        MenuCount = 11;
      }
      else if (MenuCount == 11) // go to the WaterPattern page
      {
        CurrentSensorState[leng - 4] = CurrentEncoderState[1];
        MenuCount = 4;
      }

      else if (MenuCount == 4 && CurrentEncoderState[0] == 1) // go to the WaterPattern page
      {
        MenuCount = 12;
      }
      else if (MenuCount == 12) // go to the WaterPattern page
      {
        CurrentSensorState[161] = CurrentEncoderState[1];
        MenuCount = 13;
      }
      else if (MenuCount == 13) // go to the WaterPattern page
      {
        CurrentSensorState[162] = CurrentEncoderState[1];
        MenuCount = 4;
      }

      else if (MenuCount == 4 && CurrentEncoderState[0] == 2) // go to the WaterPattern page
      {
        MenuCount = 14;
      }
      else if (MenuCount == 14) // go to the WaterPattern page
      {
        CurrentSensorState[163] = CurrentEncoderState[1];
        MenuCount = 15;
      }
      else if (MenuCount == 15) // go to the WaterPattern page
      {
        CurrentSensorState[164] = CurrentEncoderState[1];
        MenuCount = 4;
      }
      // here i change variables
      if (CurrentEncoderState[0] > MenuItems[MenuCount])
        CurrentEncoderState[0] = 1;
      else
        CurrentEncoderState[0] = 0;
      if (MenuCount == 0)
        CurrentEncoderState[0] = 1;
      else if (MenuCount == -1)
        CurrentEncoderState[0] = 0;

      if (MenuCount == 3 || MenuCount == 4 || MenuCount == 16)
      {
        CurrentEncoderState[0] = activeEncoder;
      }
      LcdFlag = 1;
    }
  }
  if (flag && value == 254 && FirstTimeFlag > 1)
  {
    Serial.println("Something has changed  ");
    key = 0;
    for (int i = 0; i < leng - 1; i++)
    {
      if (CurrentSensorState[i] != PreviousSensorState[i])
      {
        Serial.print(i);
        Serial.print("_");
        Serial.print(CurrentSensorState[i]);
        Serial.print("_");
        Serial.print(PreviousSensorState[i]);
        Serial.print("  ");
        PreviousSensorState[i] = CurrentSensorState[i];
        index[key] = i;
        key++;
        LcdFlag = 1;
      }
    }
    if (FirstTimeFlag == 1)
      Serial.println();
    Serial.print("key  ");
    Serial.println(key);
    key = key * 2 + 1;
    flag = 0;
  }
  if (LcdFlag && (millis() - TimeFromBegin) > 150)
  {
    LCD_request();
    LcdFlag = 0;
    TimeFromBegin = millis();
  }
  if (key > 1 && value == 254)
    digitalWrite(PinForMaster, HIGH);
}
