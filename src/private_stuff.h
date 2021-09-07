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
        flag = 1;
      }
      else if (CurrentSensorState[i + 2] == CurrentSensorState[0] && CurrentSensorState[i + 3] == CurrentSensorState[1] && OutputFlag[(i - 54) / 4] == 1)
      {
        CurrentSensorState[19 + (i - 54) / 4] = 0;
        digitalWrite(OutputSensors[(i - 54) / 4], LOW);
        flag = 1;
        OutputFlag[(i - 54) / 4] = 0;
      }
    }
  }
  for (int i = 0; i < InputNumber; i++)
  {
    CurrentSensorState[NumberSpecialSensors + i] = OneSensorsRequest(i);
    if (CurrentSensorState[NumberSpecialSensors + i] != PreviousSensorState[NumberSpecialSensors + i])
    {
      flag = 1;
    }
  }

  // после проверки датчиков смотрю не надо ли в контуре бака что то изменить
  // бак работает в соответствии с ТЗ

  if (CurrentSensorState[177] > 0 && CurrentSensorState[177] < 10) // если флаг в пределах нормы
  {

    if (digitalRead(InputSensors[2]) && CurrentSensorState[177] == 1) // сливаем воду перед орошением
    {
      digitalWrite(OutputSensors[0], HIGH); // soli on
      CurrentSensorState[177]++;
      Serial.println("drain water from tank  ");
      Serial.println(CurrentSensorState[177]);
    }
    if (!digitalRead(InputSensors[2]) && CurrentSensorState[177] < 3)
    {
      digitalWrite(OutputSensors[0], LOW); // soli off
      CurrentSensorState[177] = 3;
      Serial.print("tank was empty  ");
      Serial.println(CurrentSensorState[177]);
    }
    // fill tank
    if (!digitalRead(InputSensors[3]) && CurrentSensorState[177] == 3) // if max bobber show low level
    {
      digitalWrite(OutputSensors[3], HIGH); // open valve before tank
      digitalWrite(OutputSensors[9], HIGH); // pump1 on
      Serial.print("FILLING tank  ");

      CurrentSensorState[177]++; // 4
      Serial.println(CurrentSensorState[177]);
    }
    if (digitalRead(InputSensors[3]) && CurrentSensorState[177] == 4) // when tank is fill
    {
      digitalWrite(OutputSensors[9], LOW); // off pump
      digitalWrite(OutputSensors[3], LOW); // close valve
      CurrentSensorState[177]++;           //4
      Serial.print("tank is FULL  ");
      Serial.println(CurrentSensorState[177]);
    }
    // turn reel
    if (CurrentSensorState[177] == 5)
    {
      Serial.print("turn stepper  ");
      Serial.println(CurrentSensorState[177]);
      digitalWrite(enable1_Tank, LOW);
      stepper1_Tank.setCurrentPosition(0);
      stepper1_Tank.moveTo(round(CurrentSensorState[leng - 5] / 1.8));
      stepper1_Tank.runToPosition();

      digitalWrite(enable1_Tank, HIGH);
      digitalWrite(OutputSensors[10], HIGH); // start mixing
                                             // count time when I need to stop mixing
      DateTime future(rtc.now() + TimeSpan(0, 0, CurrentSensorState[175], 0));
      IrrigatonEventMonth = future.month();
      IrrigatonEventDay = future.day();    //IrrigatonEventDay = future.day();
      IrrigatonEventHour = future.hour();  //IrrigatonEventHour = future.hour();
      IrrigatonEventMin = future.minute(); //IrrigatonEventMin = future.minute();
      CurrentSensorState[177]++;           // 6
      Serial.print("Start mix  ");
      Serial.println(CurrentSensorState[177]);
    }
    if (CurrentSensorState[177] == 6)
      a = checkDate(IrrigatonEventMonth, IrrigatonEventDay, IrrigatonEventHour, IrrigatonEventMin);
    if (a)
    {
      a = 0;
      Serial.print("Stop mixing  ");
      Serial.println(CurrentSensorState[177]);
      digitalWrite(OutputSensors[10], LOW); // stop mixing
                                            //count time when i need to stop irrigation
      DateTime future(rtc.now() + TimeSpan(0, 0, CurrentSensorState[176], 0));
      IrrigatonEventMonth = future.month();
      IrrigatonEventDay = future.day();      //IrrigatonEventDay = future.day();
      IrrigatonEventHour = future.hour();    //IrrigatonEventHour = future.hour();
      IrrigatonEventMin = future.minute();   //IrrigatonEventMin = future.minute();
      digitalWrite(OutputSensors[4], HIGH);  // open valve for irrigation
      digitalWrite(OutputSensors[11], HIGH); // water all
      CurrentSensorState[177]++;             //7
      Serial.print("Begin Irrigation  ");
      Serial.println(CurrentSensorState[177]);
    }
    if (CurrentSensorState[177] == 7)
      b = checkDate(IrrigatonEventMonth, IrrigatonEventDay, IrrigatonEventHour, IrrigatonEventMin);
    if (b) // если наступил след час в день события
           // если наступил след минута в час события соответствующего дня
    {
      b = 0;

      digitalWrite(OutputSensors[11], LOW); // stop water
      digitalWrite(OutputSensors[4], LOW);  // close valve
      CurrentSensorState[177]++;            // 8
      Serial.println("irrigation stop  ");
      Serial.println(CurrentSensorState[177]);
    }
    if (digitalRead(InputSensors[2]) && CurrentSensorState[177] == 8) // drain water
    {
      digitalWrite(OutputSensors[0], HIGH); // soli on
      CurrentSensorState[177]++;
      Serial.println("draun tank after irrigation  ");
      Serial.println(CurrentSensorState[177]);
    }
    if (!digitalRead(InputSensors[2]) && (CurrentSensorState[177] == 9 || CurrentSensorState[177] == 8)) // drain water
    {
      digitalWrite(OutputSensors[0], LOW); // soli on
      CurrentSensorState[177] = 10;
      Serial.print("I finished Irrigation tank empty  ");
      Serial.println(CurrentSensorState[177]);
    }
  }

  // Орошение закончено
  // делаем полив

  if (CurrentSensorState[187] < 10 && CurrentSensorState[187] > 0) // если флаг в пределах нормы
  {
    if (digitalRead(InputSensors[2]) && CurrentSensorState[187] == 1) // сливаем воду перед орошением
    {
      digitalWrite(OutputSensors[0], HIGH); // soli on
      CurrentSensorState[187]++;
      Serial.print("drain water  ");
      Serial.println(CurrentSensorState[187]);
    }
    if (!digitalRead(InputSensors[2]) && CurrentSensorState[187] < 3)
    {
      digitalWrite(OutputSensors[0], LOW); // soli off
      CurrentSensorState[187] = 3;
      Serial.print("Tank is empty  ");
      Serial.println(CurrentSensorState[187]);
    }
    // fill tank
    if (!digitalRead(InputSensors[3]) && CurrentSensorState[187] == 3) // if max bobber show low level
    {
      digitalWrite(OutputSensors[3], HIGH); // open valve before tank
      digitalWrite(OutputSensors[9], HIGH); // pump1 on
      Serial.print("fill tank  ");
      Serial.println(CurrentSensorState[187]);
      CurrentSensorState[187]++; // 3
    }
    if (digitalRead(InputSensors[3]) && CurrentSensorState[187] == 4) // when tank is fill
    {
      digitalWrite(OutputSensors[9], LOW); // off pump
      digitalWrite(OutputSensors[3], LOW); // close valve
      CurrentSensorState[187]++;           //5
      Serial.print("Tank is full  ");
      Serial.println(CurrentSensorState[187]);
    }
    // turn reel
    if (CurrentSensorState[187] == 5)
    {
      Serial.print("turn stepper  ");
      Serial.println(CurrentSensorState[187]);
      digitalWrite(enable1_Tank, LOW);
      stepper1_Tank.setCurrentPosition(0);
      stepper1_Tank.moveTo(round(CurrentSensorState[leng - 5] / 1.8));
      stepper1_Tank.runToPosition();
      Serial.print("Start mix  ");
      Serial.println(CurrentSensorState[187]);
      digitalWrite(enable1_Tank, HIGH);
      digitalWrite(OutputSensors[10], HIGH); // start mixing
                                             // count time when I need to stop mixing
      DateTime future(rtc.now() + TimeSpan(0, 0, CurrentSensorState[185], 0));
      WaterEventMonth = future.month(); //IrrigatonEventDay = future.day();
      WaterEventDay = future.day();     //IrrigatonEventDay = future.day();
      WaterEventHour = future.hour();   //IrrigatonEventHour = future.hour();
      WaterEventMin = future.minute();  //IrrigatonEventMin = future.minute();
      CurrentSensorState[187]++;        // 6
    }
    if (CurrentSensorState[187] == 6)
      c = checkDate(WaterEventMonth, WaterEventDay, WaterEventHour, WaterEventMin);
    if (c)
    {
      c = 0;
      Serial.print("stop mixing  ");
      Serial.println(CurrentSensorState[187]);
      digitalWrite(OutputSensors[10], LOW); // stop mixing
                                            //count time when i need to stop irrigation
      DateTime future(rtc.now() + TimeSpan(0, 0, CurrentSensorState[186], 0));
      WaterEventMonth = future.month(); //IrrigatonEventDay = future.day();
      WaterEventDay = future.day();     //IrrigatonEventDay = future.day();
      WaterEventHour = future.hour();   //IrrigatonEventHour = future.hour();
      WaterEventMin = future.minute();  //IrrigatonEventMin = future.minute();
      Serial.print("Start watering  ");
      Serial.println(CurrentSensorState[187]);
      digitalWrite(OutputSensors[11], HIGH); // water all
      digitalWrite(OutputSensors[5], HIGH);  // open correct valve
      CurrentSensorState[187]++;
      // 7
    }
    if (CurrentSensorState[187] == 7)
      d = checkDate(WaterEventMonth, WaterEventDay, WaterEventHour, WaterEventMin);
    if (d) // если наступил след час в день события
           // если наступил след минута в час события соответствующего дня
    {
      d = 0;
      Serial.print("Stop watering  ");
      Serial.println(CurrentSensorState[187]);
      digitalWrite(OutputSensors[11], LOW); // stop water
      digitalWrite(OutputSensors[5], LOW);  // close valve
      CurrentSensorState[187]++;
      // 7
    }
    // осушение бака после полива
    if (digitalRead(InputSensors[2]) && CurrentSensorState[187] == 8) // drain water
    {
      digitalWrite(OutputSensors[0], HIGH); // soli on
      CurrentSensorState[187]++;
      Serial.print("draun tank after watering  ");
      Serial.println(CurrentSensorState[187]);
    }

    // бак пуст и полив завершен
    if (!digitalRead(InputSensors[2]) && (CurrentSensorState[187] == 9 || CurrentSensorState[187] == 8)) // drain water
    {
      digitalWrite(OutputSensors[0], LOW); // soli on
      CurrentSensorState[187] = 10;
      Serial.print("I drained tank after watering, tank empty  ");
      Serial.println(CurrentSensorState[187]);
    }
  }

  // проверяю надо ли что то делать с контуром света
  if (CurrentSensorState[190] == CurrentSensorState[0] && CurrentSensorState[191] == CurrentSensorState[1] && (CurrentSensorState[190] != CurrentSensorState[192] || CurrentSensorState[191] != CurrentSensorState[193]) && !LedFlag)
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
  if (CurrentSensorState[192] == CurrentSensorState[0] && CurrentSensorState[193] == CurrentSensorState[1] && (CurrentSensorState[190] != CurrentSensorState[192] || CurrentSensorState[191] != CurrentSensorState[193]))
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
    if (FirstTimeFlag == 0)
    {
      lcd.setCursor(0, 3);
      lcd.print("WiFi not connected");
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
  case 3: // меню орошения
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
        lcd.print(IrrigationMenu[CurrentEncoderState[0] + i - 6]);
        lcd.print(IrrigationMenuValue(CurrentEncoderState[0] + i - 6));
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
    lcd.print(CurrentSensorState[191]);
    break;
  case 13:
    lcd.setCursor(9, 1);
    lcd.print("       ");
    lcd.setCursor(9, 1);
    lcd.print(CurrentSensorState[190]);
    lcd.print(":");
    lcd.print(CurrentEncoderState[0]);
    break;
  case 14:
    lcd.setCursor(10, 2);
    lcd.print("  ");
    lcd.setCursor(10, 2);
    lcd.print(CurrentEncoderState[0]);
    lcd.print(":");
    lcd.print(CurrentSensorState[193]);
    break;
  case 15:
    lcd.setCursor(10, 2);
    lcd.print("       ");
    lcd.setCursor(10, 2);
    lcd.print(CurrentSensorState[192]);
    lcd.print(":");
    lcd.print(CurrentEncoderState[0]);
    break;
  case 16: // тут будут видны управляемые устройства
    lcd.clear();
    for (int i = -1; i < 1; i++)
    {
      lcd.setCursor(0, 1 + i);
      (i == 0) ? lcd.print(">") : lcd.print(" ");
      if (CurrentEncoderState[0] + i >= 0 && CurrentEncoderState[0] + i <= 31)
      {
        lcd.print(DeviceMenu[CurrentEncoderState[0] + i]);
      }
      else if (CurrentEncoderState[0] + i == -1)
      {
        lcd.print(DeviceMenu[MenuItems[MenuCount + 1]]);
      }
    }
    lcd.setCursor(1, 2);
    if (CurrentEncoderState[0] < 29)
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
    else if (CurrentEncoderState[0] != 31)
    {
      lcd.print("on ");
      lcd.print(CurrentSensorState[46 + (CurrentEncoderState[0] - 29) * 4]);
      lcd.print(":");
      lcd.print(CurrentSensorState[46 + (CurrentEncoderState[0] - 29) * 4 + 1]);
      lcd.print(" off ");
      lcd.print(CurrentSensorState[46 + (CurrentEncoderState[0] - 29) * 4 + 2]);
      lcd.print(":");
      lcd.print(CurrentSensorState[46 + (CurrentEncoderState[0] - 29) * 4 + 3]);
    }
    else
      lcd.print(DeviceMenu[0]);
    lcd.setCursor(1, 3);
    if (CurrentEncoderState[0] != 31)
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
    if (previousMenuCount == 2)
    {
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
    }
    else if (previousMenuCount == 3)
    {
      lcd.setCursor(0, 1);
      lcd.print(">Start at ");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(".");
      lcd.print(CurrentSensorState[171]);
      lcd.print("/");
      lcd.print(CurrentSensorState[172]);
      lcd.print(".");
      lcd.print(CurrentSensorState[173]);
      lcd.print(" ");
    }
    else if (previousMenuCount == 26)
    {
      lcd.setCursor(0, 1);
      lcd.print(">Start at ");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(".");
      lcd.print(CurrentSensorState[181]);
      lcd.print("/");
      lcd.print(CurrentSensorState[182]);
      lcd.print(".");
      lcd.print(CurrentSensorState[183]);
      lcd.print(" ");
    }
    break;
  case 23: // смена часа
    if (previousMenuCount == 2)
    {
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
    }
    else if (previousMenuCount == 3)
    {
      lcd.setCursor(0, 1);
      lcd.print(">Start at ");
      lcd.print(CurrentSensorState[170]);
      lcd.print(".");
      lcd.print(CurrentEncoderState[0]);
      lcd.print("/");
      lcd.print(CurrentSensorState[172]);
      lcd.print(".");
      lcd.print(CurrentSensorState[173]);
      lcd.print(" ");
    }
    else if (previousMenuCount == 26)
    {
      lcd.setCursor(0, 1);
      lcd.print(">Start at ");
      lcd.print(CurrentSensorState[180]);
      lcd.print(".");
      lcd.print(CurrentEncoderState[0]);
      lcd.print("/");
      lcd.print(CurrentSensorState[182]);
      lcd.print(".");
      lcd.print(CurrentSensorState[183]);
      lcd.print(" ");
    }
    break;
  case 24: // изменение минуты
    if (previousMenuCount == 2)
    {
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
    }
    else if (previousMenuCount == 3)
    {
      lcd.setCursor(0, 1);
      lcd.print(">Start at ");
      lcd.print(CurrentSensorState[170]);
      lcd.print(".");
      lcd.print(CurrentSensorState[171]);
      lcd.print("/");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(".");
      lcd.print(CurrentSensorState[173]);
      lcd.print(" ");
    }
    else if (previousMenuCount == 26)
    {
      lcd.setCursor(0, 1);
      lcd.print(">Start at ");
      lcd.print(CurrentSensorState[180]);
      lcd.print(".");
      lcd.print(CurrentSensorState[181]);
      lcd.print("/");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(".");
      lcd.print(CurrentSensorState[183]);
      lcd.print(" ");
    }
    break;
  case 25: // change minute
    if (previousMenuCount == 2)
    {
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
    }
    else if (previousMenuCount == 3)
    {
      lcd.setCursor(0, 1);
      lcd.print(">Start at ");
      lcd.print(CurrentSensorState[170]);
      lcd.print(".");
      lcd.print(CurrentSensorState[171]);
      lcd.print("/");
      lcd.print(CurrentSensorState[172]);
      lcd.print(".");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(" ");
    }
    else if (previousMenuCount == 26)
    {
      lcd.setCursor(0, 1);
      lcd.print(">Start at ");
      lcd.print(CurrentSensorState[180]);
      lcd.print(".");
      lcd.print(CurrentSensorState[181]);
      lcd.print("/");
      lcd.print(CurrentSensorState[182]);
      lcd.print(".");
      lcd.print(CurrentEncoderState[0]);
      lcd.print(" ");
    }
    break;
  case 26: // меню полива

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
        lcd.print(WaterMenu[CurrentEncoderState[0] + i - 6]);
        lcd.print(WaterMenuValue(CurrentEncoderState[0] + i - 6));
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

void MenuControl()
{

  // скачим по страницам экрана
  if (MenuCount == 3 || MenuCount == 4 || MenuCount == 16 || MenuCount == 26)
  {
    activeEncoder = CurrentEncoderState[0];
  }
  else if (MenuCount == 1)
    activeEncoder = 0;

  CurrentEncoderState[1] = CurrentEncoderState[0];
  if (MenuCount == -1) // go out from first page
  {
    MenuCount = CurrentEncoderState[1];
    previousMenuCount = MenuCount;
  }
  else if (MenuCount == 2 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // go out from first page
  {
    MenuCount = -1;
  }
  else if (MenuCount == 2 && CurrentEncoderState[0] == 1) // go to the Devices page
  {
    MenuCount = 21;
  }
  else if (MenuCount == 21 && previousMenuCount == 2) // go to the Devices page
  {
    MenuCount++;
    adjyear = CurrentEncoderState[1] + 2021;
  }
  else if (MenuCount == 22) // go to the Devices page
  {
    MenuCount++;
    adjmont = CurrentEncoderState[1];
    if (previousMenuCount == 3)
    {
      CurrentSensorState[170] = adjmont;
    }
    else if (previousMenuCount == 26)
    {
      CurrentSensorState[180] = adjmont;
    }
  }
  else if (MenuCount == 23) // go to the Devices page
  {
    MenuCount++;

    adjday = CurrentEncoderState[1];
    if (previousMenuCount == 3)
    {
      CurrentSensorState[171] = adjday;
    }
    else if (previousMenuCount == 26)
    {
      CurrentSensorState[181] = adjday;
    }
  }
  else if (MenuCount == 24) // go to the Devices page
  {

    MenuCount++;
    adjhour = CurrentEncoderState[1];
    if (previousMenuCount == 3)
    {
      CurrentSensorState[172] = adjhour;
    }
    else if (previousMenuCount == 26)
    {
      CurrentSensorState[182] = adjhour;
    }
  }
  else if (MenuCount == 25 && previousMenuCount == 2) // go to the Devices page
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
  else if (MenuCount == 0 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // go to the first page
  {
    MenuCount = -1;
  }
  else if (MenuCount == 1 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // go to the first page
  {
    MenuCount = -1;
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

  /* параметры контура орошения MenuCount = 3;*/

  else if (MenuCount == 1 && CurrentEncoderState[0] == 0) // переход в меню орошения
  {
    MenuCount = 3;
    previousMenuCount = MenuCount;
  }
  else if (MenuCount == 3 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // возврат в меню шаблонов
  {
    MenuCount = 1;
  }
  else if (MenuCount == 3 && CurrentEncoderState[0] == 0) // возврат в меню шаблонов
  {
    MenuCount = 22;
  }
  else if (MenuCount == 25 && previousMenuCount == 3) // go to the Devices page
  {
    MenuCount = 3;
    adjmin = CurrentEncoderState[1];
    CurrentSensorState[170] = adjmont;
    CurrentSensorState[171] = adjday;
    CurrentSensorState[172] = adjhour;
    CurrentSensorState[173] = adjmin;
    Serial.print("PreviousSensorState[173]");
    Serial.println(PreviousSensorState[173]);
    flag = 1;
  }
  else if (MenuCount == 3 && CurrentEncoderState[0] == 1) // меняю длительность
  {
    MenuCount = 5;
  }
  else if (MenuCount == 5 && previousMenuCount == 3) // back to water menu
  {
    CurrentSensorState[179] = CurrentEncoderState[1] % 24;
    CurrentSensorState[178] = (CurrentEncoderState[1] - CurrentSensorState[178]) / 24;
    DateTime future(rtc.now() + TimeSpan(CurrentSensorState[178], CurrentSensorState[179], 0, 0));
    CurrentSensorState[171] = future.day();
    CurrentSensorState[172] = future.hour();
    CurrentSensorState[173] = future.minute();
    MenuCount = 3;
    flag = 1;
    Serial.println("twelve");
  }
  else if (MenuCount == 3 && CurrentEncoderState[0] == 2) // меняю угол поворота
  {
    MenuCount = 6;
  }
  else if (MenuCount == 6 && previousMenuCount == 3)
  {
    CurrentSensorState[194] = CurrentEncoderState[1];
    MenuCount = 3;
    flag = 1;
  }
  else if (MenuCount == 3 && CurrentEncoderState[0] == 3) // время перемешивания
  {
    MenuCount = 7;
  }
  else if (MenuCount == 7 && previousMenuCount == 3) // go to the WaterPattern page
  {
    CurrentSensorState[175] = CurrentEncoderState[1];
    MenuCount = 3;
    flag = 1;
  }

  else if (MenuCount == 3 && CurrentEncoderState[0] == 4) // длительность полива
  {
    MenuCount = 8;
  }
  else if (MenuCount == 8 && previousMenuCount == 3) // go to the WaterPattern page
  {
    CurrentSensorState[176] = CurrentEncoderState[1];
    MenuCount = 3;
    flag = 1;
  }
  /* параметры контура полива MenuCount = 26;*/
  else if (MenuCount == 1 && CurrentEncoderState[0] == 3) // go to the Water page
  {
    MenuCount = 26;
    previousMenuCount = MenuCount;
  }
  else if (MenuCount == 26 && CurrentEncoderState[0] == MenuItems[MenuCount + 1]) // возврат в меню шаблонов
  {
    MenuCount = 1;
  }
  else if (MenuCount == 26 && CurrentEncoderState[0] == 0) // возврат в меню шаблонов
  {
    MenuCount = 22;
  }
  else if (MenuCount == 25 && previousMenuCount == 26) // go to the Devices page
  {
    MenuCount = 26;
    adjmin = CurrentEncoderState[1];
    CurrentSensorState[180] = adjmont;
    CurrentSensorState[181] = adjday;
    CurrentSensorState[182] = adjhour;
    CurrentSensorState[183] = adjmin;
    Serial.print("PreviousSensorState[173]");
    Serial.println(PreviousSensorState[173]);

    flag = 1;
  }
  else if (MenuCount == 26 && CurrentEncoderState[0] == 1) // меняю длительность
  {
    MenuCount = 5;
  }
  else if (MenuCount == 5 && previousMenuCount == 26) // back to water menu
  {
    CurrentSensorState[189] = CurrentEncoderState[1] % 24;
    CurrentSensorState[188] = (CurrentEncoderState[1] - CurrentSensorState[188]) / 24;
    DateTime future(rtc.now() + TimeSpan(CurrentSensorState[188], CurrentSensorState[189], 0, 0));
    CurrentSensorState[181] = future.day();
    CurrentSensorState[182] = future.hour();
    CurrentSensorState[183] = future.minute();
    MenuCount = 26;
    flag = 1;
    Serial.println("twelve");
  }
  else if (MenuCount == 26 && CurrentEncoderState[0] == 2) // меняю угол поворота
  {
    MenuCount = 6;
  }
  else if (MenuCount == 6 && previousMenuCount == 26)
  {
    CurrentSensorState[194] = CurrentEncoderState[1];
    MenuCount = 26;
    flag = 1;
  }
  else if (MenuCount == 26 && CurrentEncoderState[0] == 3) // время перемешивания
  {
    MenuCount = 7;
  }
  else if (MenuCount == 7 && previousMenuCount == 26) // go to the WaterPattern page
  {
    CurrentSensorState[185] = CurrentEncoderState[1];
    MenuCount = 26;
    flag = 1;
  }

  else if (MenuCount == 26 && CurrentEncoderState[0] == 4) // длительность полива
  {
    MenuCount = 8;
  }
  else if (MenuCount == 8 && previousMenuCount == 26) // go to the WaterPattern page
  {
    CurrentSensorState[186] = CurrentEncoderState[1];
    MenuCount = 26;
    flag = 1;
  }

  /* параметры контура света MenuCount = 4;*/

  else if (MenuCount == 1 && CurrentEncoderState[0] == 1) // переход в меню настройки контура освещения
  {
    MenuCount = 4;
  }
  else if (MenuCount == 4 && CurrentEncoderState[0] == 3) // вернуться в меню шаблонов
  {
    MenuCount = 1;
  }
  else if (MenuCount == 4 && CurrentEncoderState[0] == 0) // поменять дистанцию на которую отъедет мотор
  {
    MenuCount = 11;
  }
  else if (MenuCount == 11) // go to the WaterPattern page
  {
    CurrentSensorState[leng - 4] = CurrentEncoderState[1];
    MenuCount = 4;
    flag = 1;
  }
  else if (MenuCount == 4 && CurrentEncoderState[0] == 1) // изменить время включения света
  {
    MenuCount = 12;
  }
  else if (MenuCount == 12) // меняю часы включения света
  {
    CurrentSensorState[190] = CurrentEncoderState[1];
    MenuCount = 13;
  }
  else if (MenuCount == 13) // меняю минуты включения света
  {
    CurrentSensorState[191] = CurrentEncoderState[1];
    MenuCount = 4;
    flag = 1;
  }
  else if (MenuCount == 4 && CurrentEncoderState[0] == 2) // меняю время выключения света
  {
    MenuCount = 14;
  }
  else if (MenuCount == 14) // меняю часы выключения света
  {
    CurrentSensorState[192] = CurrentEncoderState[1];
    MenuCount = 15;
  }
  else if (MenuCount == 15) // меняю минуты выключения света
  {
    CurrentSensorState[193] = CurrentEncoderState[1];
    MenuCount = 4;
    flag = 1;
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

  if (MenuCount == 3 || MenuCount == 4 || MenuCount == 16 || MenuCount == 26)
  {
    CurrentEncoderState[0] = activeEncoder;
  }
  LcdFlag = 1;
  Serial.print("MenuCount  ");
  Serial.println(MenuCount);
}