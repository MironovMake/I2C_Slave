#include <Arduino.h>
// подключаю библиотеку для i2c передачи
#include <Wire.h>
// задаю адрес подчиненного
#define SLAVE_ADDR 7
// переменные для передачи данных
int bcount = 0;
int counter = 0;
int key = 1;
int value = 0;
String adr;
// подаем высокий сигнал если есть, что сказать мастеру
int PinForMaster = 14;

//настраиваю экран LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
// количество строк в каждм подменю
//  MenuItems[] = {2, 15, 4, 2, 7, 4, 300, 200, 200, 200, 200, 360, 250, 23, 59, 23, 59, 31, 23, 59, 23, 59, 100, 12, 31, 23, 59};
int MenuItems[] = {2, 15, 4, 2, 5, 4, 300, 200, 200, 200, 200, 360, 250, 23, 59, 23, 59, 31, 23, 59, 23, 59, 100, 12, 32, 23, 59, 5};
// названия в меню
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
  0 Irrigation pattern
  1 Water pattern 
  2 Led pattern 
  3 Other devices
  4 Back
*/
String PatternsMenu[] = {"Irrigation pattern", "Led pattern", "Other devices", "Water pattern", "Back"};
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
String IrrigationMenu[] = {"Start at ", "Cycle duration ", "Angel rotation ",
                           "Irrigat mix ", "Irrigat duraion ", "Back"};

String WaterMenu[] = {"Start at ", "Cycle duration ", "Angel rotation ",
                      "Water mix ", "Water duraion ", "Back"};
/*
0 Distance 
1 Time on 
2 Time off 
3 Back"
*/
String LedMenu[] = {"Distance ", "Time on ", "Time off ", "Back"};

String ValueOfOneString;
String ValueOfLedString;
// MenuCount = -1;
int MenuCount = -1;
String DeviceMenu[] = {"SOLI", "LASER1", "LASER2",
                       "VALVE1", "VALVE2", "VALVE3", "VALVE4", "VALVE5", "VALVE6",
                       "PUMP1", "PUMP2", "PUMP3", "PUMP4", "BIG_PUMP",
                       "DEVICE1", "DEVICE2", "DEVICE3", "DEVICE4", "DEVICE5", "DEVICE6", "DEVICE7", "DEVICE8",
                       "LED", "DRIVE1", "DRIVE2", "DRIVE3", "DRIVE4", "STEP1", "STEP2", "STEP3", "STEP4", "Back"};

// настройка шаговиков
#include <AccelStepper.h>
#define motorInterfaceType 1
// Tank motor
AccelStepper stepper1_Tank = AccelStepper(motorInterfaceType, 2, 3); // stepPin, dirPin,enable for tank
// Led motor
AccelStepper stepper2_Led = AccelStepper(motorInterfaceType, 4, 8);
AccelStepper stepper3 = AccelStepper(motorInterfaceType, 11, 10);
AccelStepper stepper4 = AccelStepper(motorInterfaceType, 12, 13);
// пины для вкл/выкл моторов
int enable1_Tank = 58;
int enable2_Led = 59;
int enable3 = 60;
int enable4 = 61;

// объявление часов
#include <SPI.h>
#include "RTClib.h"
RTC_DS1307 rtc;
int number = 0;
// объявление энкодера
#include "GyverEncoder.h"
#define CLK 6
#define DT 7
#define SW 5
Encoder enc1(CLK, DT, SW);
int CurrentEncoderState[3];
int PreviousEncoderState[3];
// объявление температурного датчика
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 9
OneWire oneWire(ONE_WIRE_BUS);
float tempC;
DallasTemperature temper(&oneWire);

// NumberSpecialSensors=3
// hour, min, temper
int NumberSpecialSensors = 3;
// все переменные
const int leng = 195;
// pins of inputs sensors is 12
// int InputNumber = 12
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
bool OutputFlag[31];
const int OutputNumber = 31;
// pins of OUTPUTS
/* here outputs states:

  pin  24    OutputSensors[0]      "SOLI",     drain water from tank
  pin  25    OutputSensors[1]      "LASER1",   for LED mode
  pin  26    OutputSensors[2]      "LASER2", 
  pin  29    OutputSensors[3]      "VALVE1",   valve before tank
  pin  30    OutputSensors[4]      "VALVE2",   valve for irrigation
  pin  31    OutputSensors[5]      "VALVE3",   valve for waterring
  pin  32    OutputSensors[6]      "VALVE4",
  pin  33    OutputSensors[7]      "VALVE5", 
  pin  34    OutputSensors[8]      "VALVE6", 
  pin  39    OutputSensors[9]      "PUMP1",    pump before tank
  pin  40    OutputSensors[10]     "PUMP2",   mixer
  pin  41    OutputSensors[11]     "PUMP3",   pump for wattering and irrigation
  pin  42    OutputSensors[12]     "PUMP4", 
  pin  43    OutputSensors[13]     "BIG_PUMP"
  pin  44    OutputSensors[14]     "DEVICE1", 
  pin  45    OutputSensors[15]     "DEVICE2", 
  pin  46    OutputSensors[16]     "DEVICE3", 
  pin  47    OutputSensors[17]     "DEVICE4", 
  pin  48    OutputSensors[18]     "DEVICE5", 
  pin  49    OutputSensors[19]     "DEVICE6", 
  pin  50    OutputSensors[20]     "DEVICE7", 
  pin  51    OutputSensors[21]     "DEVICE8",
  pin  57    OutputSensors[22]     "LED",
  pin  15,16 OutputSensors[23,24]  "DRIVE1", 
  pin  17,18 OutputSensors[25,26]  "DRIVE2", 
  pin  19,20 OutputSensors[27,28]  "DRIVE3", 
  pin  21,56 OutputSensors[29,30]  "DRIVE4",
  
*/
int OutputSensors[] = {24, 25, 26, 29, 30, 31, 32, 33, 34, 39, 40, 41, 42, 43, 44, 45, 46,
                       47, 48, 49, 50, 51, 57, 15, 16, 17, 18, 19, 20, 21, 56};
unsigned long frucuenTrans;
// предыдущее значение данных
int PreviousSensorState[leng];
// текущее значение данных
int CurrentSensorState[leng];
// буфер
int NewSensorState[leng];
int index[leng];
// влаги на изменение событий, стстояние экрана
int flag;
bool LedFlag = 0;
bool LcdFlag;
// ограничитель по времени для экрана
unsigned long TimeFromBegin;
unsigned long sec;
// wait till esp send first datesadDS
int FirstTimeFlag = 0;
// вемя наступления цикла полива/орошения
int CurrentDay;

int WaterEventHour;
int WaterEventMin;
int WaterEventDay;

int IrrigatonEventHour;
int IrrigatonEvenMin;
int IrrigatonEventDay;

int adjyear;
int adjmont;
int adjday;
int adjhour;
int adjmin;
int CurrentYear;
int CurrentMonth;
// метка для цикла полива/орошения
int WateringTankFlag = 50;
int IrrigationTankFlag = 50;

int temporaryVar;
// получение строки для подменю орошения/полива
String IrrigationMenuValue(int j)
{
  switch (j)
  {
  case 0: // month/day/hour/min of begining
    ValueOfOneString = String(CurrentSensorState[170]) + "." + String(CurrentSensorState[171]) + "/" + String(CurrentSensorState[172]) + ":" + String(CurrentSensorState[173]);
    break;
  case 1: //cycle duration
    ValueOfOneString = String(CurrentSensorState[179] * 24 + CurrentSensorState[180]);
    break;
  case 2:
    ValueOfOneString = String(CurrentSensorState[190]);
    break;
  case 3:
    ValueOfOneString = String(CurrentSensorState[175]);
    break;
  case 4:
    ValueOfOneString = String(CurrentSensorState[176]);
    break;
  case 5:
    ValueOfOneString = "";
    break;
  }
  return ValueOfOneString;
}
// получение строки для подменю орошения/полива
String WaterMenuValue(int j)
{
  switch (j)
  {
  case 0:
    ValueOfOneString = String(CurrentSensorState[171]) + "." + String(CurrentSensorState[170]) + "/" + String(CurrentSensorState[172]) + ":" + String(CurrentSensorState[173]);
    break;
  case 1:
    ValueOfOneString = String(CurrentSensorState[179] * 24 + CurrentSensorState[180]); //cycle
    break;
  case 2:
    ValueOfOneString = String(CurrentSensorState[183]);
    break;
  case 3:
    ValueOfOneString = String(CurrentSensorState[181]);
    break;
  case 4:
    ValueOfOneString = String(CurrentSensorState[175]);
    break;
  case 5:
    ValueOfOneString = String(CurrentSensorState[182]);
    break;
  case 6:
    ValueOfOneString = String(CurrentSensorState[174]);
    break;
  case 7:
    ValueOfOneString = "";
    break;
  }
  return ValueOfOneString;
}
// получение строки для подменю контур света
String LedMenuValue(int j)
{
  switch (j)
  {
  case 0:
    ValueOfLedString = String(CurrentSensorState[leng - 4]);
    break;
  case 1:
    ValueOfLedString = String(CurrentSensorState[166]) + ":" + String(CurrentSensorState[167]);
    break;
  case 2:
    ValueOfLedString = String(CurrentSensorState[168]) + ":" + String(CurrentSensorState[169]);
    break;
  case 3:
    ValueOfLedString = "";
    break;
  }
  return ValueOfLedString;
}
// обрабатываю показания датчика под номером temporary
int OneSensorsRequest(int temporary)
{ // сначала считаю статистику во избежание ошибок
  int statistic[10];
  for (int j = 0; j < 10; j++)
  {
    (temporary == 0 || temporary > 6) ? statistic[j] = !digitalRead(InputSensors[temporary]) : statistic[j] = digitalRead(InputSensors[temporary]);
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
  // возвращаю значение
  return CurrentSensorState[NumberSpecialSensors + temporary] = round(temporaryVar / 6);
}
// подключаю все к пинам и говорю как все будет работать
void SensorsInit()
{
  // подключение часов
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
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  temper.begin();
  enc1.setType(TYPE2);
  // настройка шаговиков
  stepper1_Tank.setMaxSpeed(200);
  stepper2_Led.setMaxSpeed(200);
  stepper3.setMaxSpeed(200);
  stepper4.setMaxSpeed(200);
  stepper1_Tank.setAcceleration(40);
  stepper2_Led.setAcceleration(40);

  // pins for control step motors
  pinMode(enable1_Tank, OUTPUT);
  pinMode(enable2_Led, OUTPUT);
  // Switch off power of step motors
  digitalWrite(enable1_Tank, HIGH);
  digitalWrite(enable2_Led, HIGH);

  pinMode(PinForMaster, OUTPUT);
  digitalWrite(PinForMaster, LOW);
  for (int i = 0; i < InputNumber; i++)
  {
    pinMode(InputSensors[i], INPUT);
    (i > 6) ? pinMode(InputSensors[i], INPUT_PULLUP) : pinMode(InputSensors[i], INPUT);
    OneSensorsRequest(i);
  }
  for (int i = 0; i < OutputNumber; i++)
  {
    pinMode(OutputSensors[i], OUTPUT);
    CurrentSensorState[1 + NumberSpecialSensors + InputNumber + i] = 0;
  }
  pinMode(OutputSensors[22], OUTPUT);
}

// проверяю все датчики
void SensorsRequest()
{
  // sensors setup
  for (int i = 54; i < 159; i = i + 4)
  {
    if (CurrentSensorState[i] != CurrentSensorState[i + 2] || CurrentSensorState[i + 1] != CurrentSensorState[i + 3])
    {
      if (CurrentSensorState[i] == CurrentSensorState[0] && CurrentSensorState[i + 1] == CurrentSensorState[1] && OutputFlag[(i - 54) / 4] == 0)
      {
        CurrentSensorState[19 + (i - 54) / 4] = 1;
        digitalWrite(OutputSensors[(i - 54) / 4], HIGH);
        OutputFlag[(i - 54) / 4] = 1;
        Serial.println("i - 54) / 4  ");
        Serial.println((i - 54) / 4);
        flag = 1;
        Serial.println("two");
      }
      else if (CurrentSensorState[i + 2] == CurrentSensorState[0] && CurrentSensorState[i + 3] == CurrentSensorState[1] && OutputFlag[(i - 54) / 4] == 1)
      {
        CurrentSensorState[19 + (i - 54) / 4] = 0;
        digitalWrite(OutputSensors[(i - 54) / 4], LOW);
        flag = 1;
        OutputFlag[(i - 54) / 4] = 0;
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
      Serial.print("four  ");
      Serial.println(i);
    }
  }

  // после проверки датчиков смотрю не надо ли в контуре бака что то изменить
  // бак работает в соответствии с ТЗ

  if (CurrentSensorState[178] * 24 + CurrentSensorState[179] > 0) // если установлено время орошения
  {
    if (digitalRead(InputSensors[2]) && IrrigationTankFlag == 0) // сливаем воду перед орошением
    {
      digitalWrite(OutputSensors[0], HIGH); // soli on
      IrrigationTankFlag++;
      Serial.println("drain water");
    }
    if (!digitalRead(InputSensors[2]) && IrrigationTankFlag < 2)
    {
      digitalWrite(OutputSensors[0], LOW); // soli off
      IrrigationTankFlag = 2;
    }
    // fill tank
    if (!digitalRead(InputSensors[3]) && IrrigationTankFlag == 2) // if max bobber show low level
    {
      digitalWrite(OutputSensors[3], HIGH); // open valve before tank
      digitalWrite(OutputSensors[9], HIGH); // pump1 on
      Serial.println("fill tank");
      IrrigationTankFlag++; // 3
    }
    if (digitalRead(InputSensors[3]) && IrrigationTankFlag == 3) // when tank is fill
    {
      digitalWrite(OutputSensors[9], LOW); // off pump
      digitalWrite(OutputSensors[3], LOW); // close valve
      IrrigationTankFlag++;                //4
    }
    // turn reel
    if (IrrigationTankFlag == 4)
    {

      Serial.println("turn stepper");
      digitalWrite(enable1_Tank, LOW);
      stepper1_Tank.setCurrentPosition(0);
      stepper1_Tank.moveTo(round(CurrentSensorState[leng - 5] / 1.8));
      stepper1_Tank.runToPosition();
      Serial.println("Start mix");
      digitalWrite(enable1_Tank, HIGH);
      digitalWrite(OutputSensors[10], HIGH); // start mixing
                                             // count time when I need to stop mixing
      DateTime future(rtc.now() + TimeSpan(0, 0, CurrentSensorState[175], 0));
      IrrigatonEventDay = future.day();   //IrrigatonEventDay = future.day();
      IrrigatonEventHour = future.hour(); //IrrigatonEventHour = future.hour();
      IrrigatonEvenMin = future.minute(); //IrrigatonEvenMin = future.minute();

      IrrigationTankFlag++; // 5
    }
    if (CurrentSensorState[1] == IrrigatonEvenMin && CurrentSensorState[0] == IrrigatonEventHour && CurrentDay == IrrigatonEventDay && IrrigationTankFlag == 5)
    {
      digitalWrite(OutputSensors[10], LOW); // stop mixing
                                            //count time when i need to stop irrigation
      DateTime future(rtc.now() + TimeSpan(0, 0, CurrentSensorState[176], 0));
      IrrigatonEventDay = future.day();   //IrrigatonEventDay = future.day();
      IrrigatonEventHour = future.hour(); //IrrigatonEventHour = future.hour();
      IrrigatonEvenMin = future.minute(); //IrrigatonEvenMin = future.minute();

      digitalWrite(OutputSensors[4], HIGH);  // open valve for irrigation
      digitalWrite(OutputSensors[11], HIGH); // water all
      IrrigationTankFlag++;
      // 7
    }
    if (CurrentSensorState[1] == IrrigatonEvenMin && CurrentSensorState[0] == IrrigatonEventHour && CurrentDay == IrrigatonEventDay && IrrigationTankFlag == 6)
    {
      Serial.println("irrigation");
      digitalWrite(OutputSensors[11], LOW); // stop water
      digitalWrite(OutputSensors[4], LOW);  // close valve
      IrrigationTankFlag++;
      // 7
    }
    if (digitalRead(InputSensors[2]) && IrrigationTankFlag == 7) // drain water
    {
      digitalWrite(OutputSensors[0], HIGH); // soli on
      IrrigationTankFlag++;
      Serial.println("draun tank after irrigation");
    }
  }

  // Орошение закончено
  // делаем полив

  if (CurrentSensorState[188] * 24 + CurrentSensorState[189] > 0) // если установлено время орошения
  {
    if (digitalRead(InputSensors[2]) && WateringTankFlag == 0) // сливаем воду перед орошением
    {
      digitalWrite(OutputSensors[0], HIGH); // soli on
      WateringTankFlag++;
      Serial.println("drain water");
    }
    if (!digitalRead(InputSensors[2]) && WateringTankFlag < 2)
    {
      digitalWrite(OutputSensors[0], LOW); // soli off
      WateringTankFlag = 2;
    }
    // fill tank
    if (!digitalRead(InputSensors[3]) && WateringTankFlag == 2) // if max bobber show low level
    {
      digitalWrite(OutputSensors[3], HIGH); // open valve before tank
      digitalWrite(OutputSensors[9], HIGH); // pump1 on
      Serial.println("fill tank");
      WateringTankFlag++; // 3
    }
    if (digitalRead(InputSensors[3]) && WateringTankFlag == 3) // when tank is fill
    {
      digitalWrite(OutputSensors[9], LOW); // off pump
      digitalWrite(OutputSensors[3], LOW); // close valve
      WateringTankFlag++;                  //4
    }
    // turn reel
    if (WateringTankFlag == 4)
    {

      Serial.println("turn stepper");
      digitalWrite(enable1_Tank, LOW);
      stepper1_Tank.setCurrentPosition(0);
      stepper1_Tank.moveTo(round(CurrentSensorState[leng - 5] / 1.8));
      stepper1_Tank.runToPosition();
      Serial.println("Start mix");
      digitalWrite(enable1_Tank, HIGH);

      digitalWrite(OutputSensors[10], HIGH); // start mixing
                                             // count time when I need to stop mixing
      DateTime future(rtc.now() + TimeSpan(0, 0, CurrentSensorState[185], 0));
      IrrigatonEventDay = future.day();   //IrrigatonEventDay = future.day();
      IrrigatonEventHour = future.hour(); //IrrigatonEventHour = future.hour();
      IrrigatonEvenMin = future.minute(); //IrrigatonEvenMin = future.minute();

      WateringTankFlag++; // 5
    }
    if (CurrentSensorState[1] == IrrigatonEvenMin && CurrentSensorState[0] == IrrigatonEventHour && CurrentDay == IrrigatonEventDay && WateringTankFlag == 5)
    {
      digitalWrite(OutputSensors[10], LOW); // stop mixing
                                            //count time when i need to stop irrigation
      DateTime future(rtc.now() + TimeSpan(0, 0, CurrentSensorState[186], 0));
      IrrigatonEventDay = future.day();   //IrrigatonEventDay = future.day();
      IrrigatonEventHour = future.hour(); //IrrigatonEventHour = future.hour();
      IrrigatonEvenMin = future.minute(); //IrrigatonEvenMin = future.minute();

      digitalWrite(OutputSensors[11], HIGH); // water all
      digitalWrite(OutputSensors[5], HIGH);  // open correct valve
      WateringTankFlag++;
      // 7
    }
    if (CurrentSensorState[1] == IrrigatonEvenMin && CurrentSensorState[0] == IrrigatonEventHour && CurrentDay == IrrigatonEventDay && WateringTankFlag == 6)
    {
      Serial.println("irrigation");
      digitalWrite(OutputSensors[11], LOW); // stop water
      digitalWrite(OutputSensors[5], LOW);  // close valve
      WateringTankFlag++;
      // 7
    }
    if (digitalRead(InputSensors[2]) && WateringTankFlag == 7) // drain water
    {
      digitalWrite(OutputSensors[0], HIGH); // soli on
      WateringTankFlag++;
      Serial.println("draun tank after irrigation");
    }
  }

  // проверяю надо ли что то делать с контуром света
  if (CurrentSensorState[166] == CurrentSensorState[0] && CurrentSensorState[167] == CurrentSensorState[1] && (CurrentSensorState[166] != CurrentSensorState[168] || CurrentSensorState[167] != CurrentSensorState[169]) && !LedFlag)
  {
    digitalWrite(OutputSensors[1], HIGH);
    while (!CurrentSensorState[9] && !CurrentSensorState[13]) // while the is obtacle and no switch
    {
      // move step
      digitalWrite(enable2_Led, LOW);
      stepper2_Led.setCurrentPosition(0);
      CurrentSensorState[16] += CurrentSensorState[leng - 4];
      stepper2_Led.setCurrentPosition(0);
      stepper2_Led.moveTo(round(CurrentSensorState[leng - 4] / 0.2 * 200));
      stepper2_Led.runToPosition();
      digitalWrite(enable2_Led, HIGH);
      CurrentSensorState[9] = OneSensorsRequest(6);
      CurrentSensorState[13] = OneSensorsRequest(10);
    }

    if (!digitalRead(InputSensors[6]))       // if switc don't reach
      digitalWrite(OutputSensors[22], HIGH); //led on
    LedFlag = 1;
    digitalWrite(OutputSensors[1], LOW); //laser off
  }
  if (CurrentSensorState[168] == CurrentSensorState[0] && CurrentSensorState[169] == CurrentSensorState[1] && (CurrentSensorState[166] != CurrentSensorState[168] || CurrentSensorState[167] != CurrentSensorState[169]))
  {
    digitalWrite(OutputSensors[22], LOW);
    LedFlag = 0;
  }
}

void LCD_request()
{
  lcd.setCursor(0, 0);
  switch (MenuCount)
  {
  case -1: // Главное меню
    lcd.clear();
    for (int i = 0; i < 3; i++)
    {
      lcd.setCursor(0, i);
      (CurrentEncoderState[0] == i) ? lcd.print(">") : lcd.print(" ");
      lcd.print(FirstMenu[i]);
    }
    break;
  case 0: // Меню с датчиками
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
  case 1: // Меню с контурами
    lcd.clear();
    if (CurrentEncoderState[0] != 4)
    {
      for (int i = -1; i < 3; i++)
      {
        lcd.setCursor(0, 1 + i);
        (i == CurrentEncoderState[0] - 1) ? lcd.print(">") : lcd.print(" ");
        lcd.print(PatternsMenu[i + 1]);
      }
    }
    else
    {
      for (int i = -1; i < 3; i++)
      {
        lcd.setCursor(0, 1 + i);
        (i == CurrentEncoderState[0] - 2) ? lcd.print(">") : lcd.print(" ");
        lcd.print(PatternsMenu[i + 2]);
      }
    }
    break;
  case 2: //
    lcd.clear();
    for (int i = 0; i < 3; i++)
    {
      lcd.setCursor(0, i);
      (CurrentEncoderState[0] == i) ? lcd.print(">") : lcd.print(" ");
      lcd.print(SetupMenu[i]);
    }
    break;
  case 3: // Контур бака
    lcd.clear();
    for (int i = -1; i < 3; i++)
    {
      lcd.setCursor(0, 1 + i);
      (i == 0) ? lcd.print(">") : lcd.print(" ");
      if (CurrentEncoderState[0] + i >= 0 && CurrentEncoderState[0] + i <= MenuItems[MenuCount + 1])
      {
        lcd.print(IrrigationMenu[CurrentEncoderState[0] + i]);
        lcd.print(IrrigationMenuValue(CurrentEncoderState[0] + i));
      }
      else if (CurrentEncoderState[0] + i > MenuItems[MenuCount + 1])
      {
        lcd.print(IrrigationMenu[CurrentEncoderState[0] + i - 8]);
        lcd.print(IrrigationMenuValue(CurrentEncoderState[0] + i - 8));
      }
      else if (CurrentEncoderState[0] + i == -1)
      {
        lcd.print(IrrigationMenu[MenuItems[MenuCount + 1]]);
        lcd.print(IrrigationMenuValue(MenuItems[MenuCount + 1]));
      }
    }
    break;

  case 4: // Контур света
    lcd.clear();
    for (int i = 0; i < 4; i++)
    {
      lcd.setCursor(0, i);
      (CurrentEncoderState[0] == i) ? lcd.print(">") : lcd.print(" ");
      lcd.print(LedMenu[i]);
      lcd.print(LedMenuValue(i));
    }
    break;
    // с 5 по 15 изменение значений CurrentEncoderState
  case 5:
    lcd.setCursor(16, 1);
    lcd.print("   ");
    lcd.setCursor(16, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 6:
    lcd.setCursor(16, 1);
    lcd.print("   ");
    lcd.setCursor(16, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 7:
    lcd.setCursor(13, 1);
    lcd.print("   ");
    lcd.setCursor(13, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 8:
    lcd.setCursor(17, 1);
    lcd.print("   ");
    lcd.setCursor(17, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 9:
    lcd.setCursor(11, 1);
    lcd.print("   ");
    lcd.setCursor(11, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 10:
    lcd.setCursor(15, 1);
    lcd.print("   ");
    lcd.setCursor(15, 1);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 11:
    lcd.setCursor(10, 0);
    lcd.print("   ");
    lcd.setCursor(10, 0);
    lcd.print(CurrentEncoderState[0]);
    break;
  case 12:
    lcd.setCursor(9, 1);
    lcd.print("  ");
    lcd.setCursor(9, 1);
    lcd.print(CurrentEncoderState[0]);
    lcd.print(":");
    lcd.print(CurrentSensorState[167]);
    break;
  case 13:
    lcd.setCursor(9, 1);
    lcd.print("       ");
    lcd.setCursor(9, 1);
    lcd.print(CurrentSensorState[166]);
    lcd.print(":");
    lcd.print(CurrentEncoderState[0]);
    break;
  case 14:
    lcd.setCursor(10, 2);
    lcd.print("  ");
    lcd.setCursor(10, 2);
    lcd.print(CurrentEncoderState[0]);
    lcd.print(":");
    lcd.print(CurrentSensorState[169]);
    break;
  case 15:
    lcd.setCursor(10, 2);
    lcd.print("       ");
    lcd.setCursor(10, 2);
    lcd.print(CurrentSensorState[168]);
    lcd.print(":");
    lcd.print(CurrentEncoderState[0]);
    break;
  case 16: // тут будут видны управляемые устройства
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
      lcd.print(CurrentSensorState[54 + CurrentEncoderState[0] * 4]);
      lcd.print(":");
      lcd.print(CurrentSensorState[54 + CurrentEncoderState[0] * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[54 + CurrentEncoderState[0] * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[54 + CurrentEncoderState[0] * 4 + 3]);
    }
    else if (CurrentEncoderState[0] != 30)
    {
      lcd.print("on ");
      lcd.print(CurrentSensorState[46 + (CurrentEncoderState[0] - 28) * 4]);
      lcd.print(":");
      lcd.print(CurrentSensorState[46 + (CurrentEncoderState[0] - 28) * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[46 + (CurrentEncoderState[0] - 28) * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[46 + (CurrentEncoderState[0] - 28) * 4 + 3]);
    }
    else
      lcd.print(DeviceMenu[0]);
    lcd.setCursor(1, 3);
    if (CurrentEncoderState[0] != 30)
      lcd.print(DeviceMenu[CurrentEncoderState[0] + 1]);
    else
      lcd.print(DeviceMenu[1]);

    break;
  case 17:
    lcd.setCursor(1, 2);
    lcd.print("on ");
    lcd.print(CurrentEncoderState[0]);
    lcd.print(":");
    if (activeEncoder < 28)
    {
      lcd.print(CurrentSensorState[54 + activeEncoder * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[54 + activeEncoder * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[54 + activeEncoder * 4 + 3]);
    }
    else
    {
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 3]);
    }
    break;
  case 18:
    lcd.setCursor(1, 2);
    lcd.print("on ");
    if (activeEncoder < 28)
    {
      lcd.print(CurrentSensorState[54 + activeEncoder * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[54 + activeEncoder * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[54 + activeEncoder * 4 + 3]);
    }
    else
    {
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 3]);
    }
    break;
  case 19:
    lcd.setCursor(1, 2);
    lcd.print("on ");

    if (activeEncoder < 28)
    {
      lcd.print(CurrentSensorState[54 + activeEncoder * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[54 + activeEncoder * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[543 + activeEncoder * 4 + 3]);
    }
    else
    {
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 3]);
    }
    break;
  case 20:
    lcd.setCursor(1, 2);
    lcd.print("on ");

    if (activeEncoder < 28)
    {
      lcd.print(CurrentSensorState[54 + activeEncoder * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[54 + activeEncoder * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[54 + activeEncoder * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(" ");
    }
    else
    {
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 0]);
      lcd.print(":");
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[46 + (activeEncoder - 28) * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(" ");
    }
    break;
  case 21: // смена года
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
  case 22: // смена месяца
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
  case 23: // смена часа
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
  case 24: // изменение минуты
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
  case 26: // изменение минуты
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
  }
}
// получение данных от мастера
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
    Serial.println("I begin get data from Master");
  }
  if (number != -1)
  {
    NewSensorState[number] = value;
  }
  if (number == 0)
  {
  }
  if (value == 254) // данные закончились
  {
    Serial.print("FirstTimeFlag ");
    Serial.println(FirstTimeFlag);
    if (FirstTimeFlag > 0)
    {
      // i increase value ig degree becouse i afraid its gona be too big
      for (int i = 0; i < number; i = i + 2)
      {
        CurrentSensorState[NewSensorState[i]] = NewSensorState[i + 1];
        Serial.print(NewSensorState[i]);
        Serial.print("/");
        Serial.print(CurrentSensorState[NewSensorState[i]]);
        Serial.print(" ");
        if (FirstTimeFlag == 1)
          PreviousSensorState[NewSensorState[i]] = CurrentSensorState[NewSensorState[i]];
      }
      Serial.println();
      Serial.println("I've get data");
      FirstTimeFlag++;
      //flag = 0;
    }
    else if (FirstTimeFlag == 0)
    {
      FirstTimeFlag = 1;
    }
    TimeFromBegin = millis();
    flag = 1;
  }
  (number == leng - 1) ? number = 0 : number++;
}

// Отправка данных мастеру
void requestEvent()
{
  digitalWrite(PinForMaster, LOW);
  // Define a byte to hold data
  byte bval;
  // Cycle through data
  // First response is always 255 to mark beginning
  switch (bcount)
  {
  case 0:
    bval = 255; // start key
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
      key = 1;
      bval = 254;
    }

    break;
  }
  Wire.write(bval);
  bcount = bcount + 1;
  if (bcount > key)
    bcount = 0;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Mega begin work");
  SensorsInit();

  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(receiveEvent); // Get date from Master
  Wire.onRequest(requestEvent); // Send date to Master
  PreviousSensorState[0] = 77;
  PreviousSensorState[1] = 77;
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 1);
  lcd.print("  PLEASE  WAIT");
  // delay(2000);
  LCD_request();
}
bool after = 0;
bool trans;
void loop()
{
  // если не осущевтвляется общение с мастером каждые 0,2 секунды проверять все датчики
  if (millis() - sec > 200 && value == 254)
  {
    sec = millis();
    DateTime now = rtc.now();
    CurrentDay = now.day();
    ZeroSensor = now.second();
    CurrentSensorState[0] = now.hour();
    if (CurrentSensorState[0] > 23)
      CurrentSensorState[0] = PreviousSensorState[0];
    CurrentSensorState[1] = now.minute();
    if (FirstTimeFlag > 1)
      after = 1;
    if (CurrentSensorState[1] > 59)
    {
      CurrentSensorState[1] = PreviousSensorState[1];
      flag = 1;
    }
    CurrentYear = now.year();
    CurrentMonth = now.month();
    SetupMenu[1] = String(now.year()) + "." + String(CurrentDay) + "." + String(now.month()) + "  " + String(PreviousSensorState[0]) + ":" + String(PreviousSensorState[1]);
    SensorsRequest();
    // каждую минуту проверять температуру и время событий
    if (CurrentSensorState[1] != PreviousSensorState[1] && CurrentSensorState[1] < 60 && CurrentSensorState[1] > -1 && ZeroSensor < 60)
    {
      flag = 1;
      // если настала дата осуществлять орошение
      if (CurrentSensorState[1] == CurrentSensorState[173] && CurrentSensorState[0] == CurrentSensorState[174] && CurrentDay == CurrentSensorState[175])
      {
        DateTime future(rtc.now() + TimeSpan(CurrentSensorState[178], CurrentSensorState[179], 0, 0));
        CurrentSensorState[175] = future.day();
        CurrentSensorState[174] = future.hour();
        CurrentSensorState[173] = future.minute();
        IrrigationTankFlag = 0;
      }
      if (CurrentSensorState[1] == CurrentSensorState[183] && CurrentSensorState[0] == CurrentSensorState[184] && CurrentDay == CurrentSensorState[185])
      {
        DateTime future(rtc.now() + TimeSpan(CurrentSensorState[178], CurrentSensorState[179], 0, 0));
        CurrentSensorState[185] = future.day();
        CurrentSensorState[184] = future.hour();
        CurrentSensorState[183] = future.minute();
        WateringTankFlag = 0;
      }
      temper.requestTemperatures();                      // Send the command to get temperatures
      CurrentSensorState[2] = temper.getTempCByIndex(0); // temperature
    }
  }
  // слушаем энкодер
  enc1.tick();
  // если крутим вправо
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

    Serial.println(CurrentEncoderState[0]);
  }

  // если крутим влево
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
    Serial.println(CurrentEncoderState[0]);
  }

  // если энкодер зажат, выйти на начальный жкран
  if (enc1.isHolded())
  {
    MenuCount = -1;
    LcdFlag = 1;
  }
  // если нажата кнопка энкодера
  if (enc1.isPress())
  {

    // скачим по страницам экрана
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
      flag = 1;
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
    else if (MenuCount == 1 && CurrentEncoderState[0] == 0) // go to the Irrigation
    {
      MenuCount = 3;
    }
    else if (MenuCount == 1 && CurrentEncoderState[0] == 1) // go to the Led page
    {
      MenuCount = 4;
    }
    else if (MenuCount == 1 && CurrentEncoderState[0] == 2) // go to the Devices page
    {
      MenuCount = 16;
    }
    else if (MenuCount == 1 && CurrentEncoderState[0] == 3) // go to the Water page
    {
      MenuCount = 26;
    }
    else if (MenuCount == 26 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // go to the Pattern page
    {

      MenuCount = 1;
    }
    else if (MenuCount == 16 && CurrentEncoderState[0] != MenuItems[MenuCount + 1]) // go to the Devices page
    {
      MenuCount = 17;
    }
    else if (MenuCount == 17) // go to the Devices page
    {
      MenuCount = 18;
      (activeEncoder < 28) ? CurrentSensorState[54 + activeEncoder * 4 + 0] = CurrentEncoderState[0] : CurrentSensorState[46 + (activeEncoder - 28) * 4 + 0] = CurrentEncoderState[0];
    }
    else if (MenuCount == 18) // go to the Devices page
    {
      MenuCount = 19;
      (activeEncoder < 28) ? CurrentSensorState[54 + activeEncoder * 4 + 1] = CurrentEncoderState[0] : CurrentSensorState[46 + (activeEncoder - 28) * 4 + 1] = CurrentEncoderState[0];
    }
    else if (MenuCount == 19) // go to the Devices page
    {

      (activeEncoder < 28) ? CurrentSensorState[54 + activeEncoder * 4 + 2] = CurrentEncoderState[0] : CurrentSensorState[46 + (activeEncoder - 28) * 4 + 2] = CurrentEncoderState[0];
      MenuCount = 20;
    }
    else if (MenuCount == 20) // go to the Devices page
    {
      (activeEncoder < 28) ? CurrentSensorState[54 + activeEncoder * 4 + 3] = CurrentEncoderState[0] : CurrentSensorState[46 + (activeEncoder - 28) * 4 + 3] = CurrentEncoderState[0];
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
      CurrentSensorState[188] = CurrentEncoderState[1] % 24;
      CurrentSensorState[189] = (CurrentEncoderState[1] - CurrentSensorState[188]) / 24;
      DateTime future(rtc.now() + TimeSpan(CurrentSensorState[179], CurrentSensorState[180], 0, 0));
      CurrentSensorState[181] = future.day();
      CurrentSensorState[182] = future.hour();
      CurrentSensorState[183] = future.minute();
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
      CurrentSensorState[183] = CurrentEncoderState[1];
      MenuCount = 3;
    }
    else if (MenuCount == 3 && CurrentEncoderState[0] == 3) // change cycle
    {
      MenuCount = 7;
    }
    else if (MenuCount == 7) // go to the WaterPattern page
    {
      CurrentSensorState[181] = CurrentEncoderState[1];
      MenuCount = 3;
    }

    else if (MenuCount == 3 && CurrentEncoderState[0] == 4) // change cycle
    {
      MenuCount = 8;
    }
    else if (MenuCount == 8) // go to the WaterPattern page
    {
      CurrentSensorState[175] = CurrentEncoderState[1];
      MenuCount = 3;
    }

    else if (MenuCount == 3 && CurrentEncoderState[0] == 5) // change cycle
    {
      MenuCount = 9;
    }
    else if (MenuCount == 9) // go to the WaterPattern page
    {
      CurrentSensorState[182] = CurrentEncoderState[1];
      MenuCount = 3;
    }

    else if (MenuCount == 3 && CurrentEncoderState[0] == 6) // change cycle
    {
      MenuCount = 10;
    }
    else if (MenuCount == 10) // go to the WaterPattern page
    {
      CurrentSensorState[174] = CurrentEncoderState[1];
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
      CurrentSensorState[166] = CurrentEncoderState[1];
      MenuCount = 13;
    }
    else if (MenuCount == 13) // go to the WaterPattern page
    {
      CurrentSensorState[167] = CurrentEncoderState[1];
      MenuCount = 4;
    }

    else if (MenuCount == 4 && CurrentEncoderState[0] == 2) // go to the WaterPattern page
    {
      MenuCount = 14;
    }
    else if (MenuCount == 14) // go to the WaterPattern page
    {
      CurrentSensorState[168] = CurrentEncoderState[1];
      MenuCount = 15;
    }
    else if (MenuCount == 15) // go to the WaterPattern page
    {
      CurrentSensorState[169] = CurrentEncoderState[1];
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
    Serial.print("MenuCount  ");
    Serial.println(MenuCount);
  }
  // если гдет поднялся flag, значит какое то значение поменяло свое состояние
  // и надо об этом сказать мастеру
  if (flag && value == 254 && FirstTimeFlag > 1 && after)
  {
    after = 0;
    Serial.println("Something has changed  ");
    key = 0;
    for (int i = 0; i < leng - 1; i++)
    {
      if (CurrentSensorState[i] != PreviousSensorState[i])
      {
        Serial.print("i  ");
        Serial.print(i);
        Serial.print("  SensorState  ");
        Serial.println(CurrentSensorState[i]);
        PreviousSensorState[i] = CurrentSensorState[i];
        index[key] = i;
        key++;
        LcdFlag = 1;
        trans = 1;
      }
    }
    if (FirstTimeFlag == 1)
      Serial.println("FirstTime  ");
    key = key * 2 + 1;
    Serial.print("key  ");
    Serial.println(key);

    flag = 0;
  }
  // обновляем экран
  if (LcdFlag && (millis() - TimeFromBegin) > 150)
  {
    LCD_request();
    LcdFlag = 0;
    TimeFromBegin = millis();
  }
  // подаем еденичку мастеру, чтоб он нас спросил, то изменилось
  if (key > 1 && value == 254 && trans)
  {
    digitalWrite(PinForMaster, HIGH);
    trans = 0;
  }
}