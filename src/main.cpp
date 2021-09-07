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
int MenuItems[] = {2, 15, 4, 2, 5, 3, 300, 200, 200, 200, 200, 360, 250, 23, 59, 23, 59, 31, 23, 59, 23, 59, 100, 12, 32, 23, 59, 5};
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
int previousMenuCount;
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
const int leng = 199;
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
int anotherCounter = 0;

bool LedFlag = 0;
bool LcdFlag;
// ограничитель по времени для экрана
unsigned long TimeFromBegin;
unsigned long sec;
// wait till esp send first datesadDS
int FirstTimeFlag = 0;
// вемя наступления цикла полива/орошения
bool after = 0;

int WaterEventMonth;
int WaterEventDay;
int WaterEventHour;
int WaterEventMin;

int IrrigatonEventMonth;
int IrrigatonEventDay;
int IrrigatonEventHour;
int IrrigatonEventMin;

int adjyear;
int adjmont;
int adjday;
int adjhour;
int adjmin;
int CurrentYear;
int CurrentMonth;
int CurrentDay;

// метка для цикла полива/орошения

int temporaryVar;
// получение строки для подменю орошения/полива
String IrrigationMenuValue(int j)
{
  switch (j)
  {
  case 0: // month/day/hour/min of begining
    ValueOfOneString = String(CurrentSensorState[171]) + "." + String(CurrentSensorState[170]) + "/" + String(CurrentSensorState[172]) + ":" + String(CurrentSensorState[173]);
    break;
  case 1: //cycle duration
    ValueOfOneString = String(CurrentSensorState[178] * 24 + CurrentSensorState[179]);
    break;
  case 2:
    ValueOfOneString = String(CurrentSensorState[194]);
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
    ValueOfOneString = String(CurrentSensorState[181]) + "." + String(CurrentSensorState[180]) + "/" + String(CurrentSensorState[182]) + ":" + String(CurrentSensorState[183]);
    break;
  case 1: //cycle duration
    ValueOfOneString = String(CurrentSensorState[188] * 24 + CurrentSensorState[189]);
    break;
  case 2:
    ValueOfOneString = String(CurrentSensorState[194]);
    break;
  case 3:
    ValueOfOneString = String(CurrentSensorState[185]);
    break;
  case 4:
    ValueOfOneString = String(CurrentSensorState[186]);
    break;
  case 5:
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
    ValueOfLedString = String(CurrentSensorState[190]) + ":" + String(CurrentSensorState[191]);
    break;
  case 2:
    ValueOfLedString = String(CurrentSensorState[192]) + ":" + String(CurrentSensorState[193]);
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
bool a, b, c, d;
// bool checkDate(int month, int day, int hour, int min)
bool checkDate(int month, int day, int hour, int min)
{
  bool result;
  bool one, two, tree, four;
  // если наступил след месяц
  (CurrentMonth > month || CurrentMonth == 1 && month == 12) ? one = true : one = false;
  // если наступил след день
  (CurrentDay > day) ? two = true : two = false;
  // если наступил след час в нужный день
  (CurrentDay == day && CurrentSensorState[0] > hour) ? tree = true : tree = false;
  // если наступил след час в нужный день
  (CurrentDay == day && hour == CurrentSensorState[0] && CurrentSensorState[1] >= min) ? four = true : four = false;
  result = (one || two || tree || four);
  if (one && month != 0)
  {
    Serial.println("month changed");
  }
  else if (two && day != 0)
    Serial.println("day changed");
  else if (tree && hour != 0)
    Serial.println("hour changed");
  else if (four && min != 0)
    Serial.println("min changed");
  return result;
}

#include "private_stuff.h"

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
        if (FirstTimeFlag != 1)
        {
          CurrentSensorState[NewSensorState[i]] = NewSensorState[i + 1];
          Serial.print(NewSensorState[i]);
          Serial.print("/");
          Serial.print(CurrentSensorState[NewSensorState[i]]);
          Serial.print(" ");
        }
        else if (FirstTimeFlag == 1 && NewSensorState[i] > 14)
        {
          CurrentSensorState[NewSensorState[i]] = NewSensorState[i + 1];
          Serial.print(NewSensorState[i]);
          Serial.print(".");
          Serial.print(CurrentSensorState[NewSensorState[i]]);
          Serial.print(" ");
        }
      }
      Serial.println();
      Serial.println("I've get data");
      FirstTimeFlag++;
      after = 0;
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
  delay(10);
  digitalWrite(PinForMaster, LOW);
  Serial.print(bval);
  Serial.print("|");
  bcount = bcount + 1;
  if (bcount > key)
  {
    bcount = 0;
  }
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

  /* CurrentSensorState[170] = 9;//month
  CurrentSensorState[171] = 5;//day
  CurrentSensorState[172] = 0;//hour
  CurrentSensorState[173] = 0; // date min
  CurrentSensorState[175] = 1;
  CurrentSensorState[176] = 1;
  CurrentSensorState[177] = 0;
  CurrentSensorState[178] = 2;

  CurrentSensorState[180] = 9; //month
  CurrentSensorState[181] = 5; //day
  CurrentSensorState[182] = 0; //hour
  CurrentSensorState[183] = 0; // date min
  CurrentSensorState[185] = 1;
  CurrentSensorState[186] = 1;
  CurrentSensorState[187] = 0;
  CurrentSensorState[188] = 2;
  CurrentSensorState[190] = 180;
    */
}

bool trans;
void loop()
{
  // если не осущевтвляется общение с мастером каждые 0,2 секунды проверять все датчики
  if (millis() - sec > 200 && (value == 254 || FirstTimeFlag == 0))
  {
    sec = millis();
    DateTime now = rtc.now();
    CurrentDay = now.day();
    ZeroSensor = now.second();
    CurrentSensorState[0] = now.hour();
    if (CurrentSensorState[0] > 23)
      CurrentSensorState[0] = PreviousSensorState[0];
    CurrentSensorState[1] = now.minute();
    if (CurrentSensorState[0] != 66 && CurrentSensorState[1] != 66)
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
      digitalWrite(PinForMaster, LOW);
      flag = 1;

      // чекнуть не первое ли сегодня число
      // (CurrentEncoderState[0] == 0 && CurrentEncoderState[1] == 0 && CurrentDay == 1) ? CurrentSensorState[38] = 1 : CurrentSensorState[38] = 0;

      /*проверяю не проебался ли флаг из-за отключения электричества*/
      // если настала дата осуществлять орошение
      if ((CurrentSensorState[178] * 24 + CurrentSensorState[179] > 0) && checkDate(CurrentSensorState[170], CurrentSensorState[171], CurrentSensorState[172], CurrentSensorState[173]))
      {
        Serial.println();
        Serial.print("date is ");
        Serial.print(CurrentSensorState[170]);
        Serial.print(".");
        Serial.print(CurrentSensorState[171]);
        Serial.print(" / ");
        Serial.print(CurrentSensorState[172]);
        Serial.print(":");
        Serial.println(CurrentSensorState[173]);
        Serial.print("now  is ");
        Serial.print(CurrentSensorState[170]);
        Serial.print(".");
        Serial.print(CurrentDay);
        Serial.print(" / ");
        Serial.print(CurrentSensorState[0]);
        Serial.print(":");
        Serial.println(CurrentSensorState[1]);
        DateTime future(rtc.now() + TimeSpan(CurrentSensorState[178], CurrentSensorState[179], 0, 0));
        CurrentSensorState[170] = future.month();
        CurrentSensorState[171] = future.day();
        CurrentSensorState[172] = future.hour();
        CurrentSensorState[173] = future.minute();
        CurrentSensorState[177] = 1;
        Serial.print("Its time for Irrigation  ");
        Serial.println(CurrentSensorState[177]);
      }
      if ((CurrentSensorState[188] * 24 + CurrentSensorState[189] > 0) && checkDate(CurrentSensorState[180], CurrentSensorState[181], CurrentSensorState[182], CurrentSensorState[183]))
      {
        DateTime future(rtc.now() + TimeSpan(CurrentSensorState[188], CurrentSensorState[189], 0, 0));
        CurrentSensorState[180] = future.month();
        CurrentSensorState[181] = future.day();
        CurrentSensorState[182] = future.hour();
        CurrentSensorState[183] = future.minute();
        CurrentSensorState[187] = 1;
        Serial.print("Its time for Watering  ");
        Serial.println(CurrentSensorState[187]);
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
    MenuControl();
  }
  // если гдет поднялся flag, значит какое то значение поменяло свое состояние
  // и надо об этом сказать мастеру
  if ((flag && value == 254 && FirstTimeFlag > 1 && after) || (flag && FirstTimeFlag == 0 && after))
  {
    after = 0;
    Serial.println("Something has changed  ");
    key = 0;
    for (int i = 0; i < leng - 1; i++)
    {
      if (CurrentSensorState[i] != PreviousSensorState[i] && i != 174 && i != 184 || (anotherCounter < 4 && i < 15))
      {
        Serial.print("i  ");
        Serial.print(i);
        Serial.print("  before/after  ");
        Serial.print(PreviousSensorState[i]);
        Serial.print("/");
        Serial.println(CurrentSensorState[i]);
        PreviousSensorState[i] = CurrentSensorState[i];
        index[key] = i;
        key++;
        LcdFlag = 1;
        trans = 1;
      }
    }
    if (FirstTimeFlag > 1)
      anotherCounter++;
    key = key * 2 + 1;
    Serial.print("key/value  ");
    Serial.print(key);
    Serial.print("/");
    Serial.println(value);
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
  if (key > 1 && value == 254 && trans && FirstTimeFlag > 0)
  {
    Serial.println("Master I have something to say");
    digitalWrite(PinForMaster, HIGH);
    trans = 0;
    delay(600);
    digitalWrite(PinForMaster, LOW);
  }
}
