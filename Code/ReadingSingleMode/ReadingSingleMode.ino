#include <EEPROM.h>

// FLASH ADDRESS Range 0x0000 - 0x7800

int ADDRESS1 = 0x77C0; //Calibration_Value_1
int ADDRESS2 = 0x77C8; //Calibration_Value_1
int ADDRESS3 = 0x77D0; //Dispense Weight

#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#include "HX711.h"
#define LOADCELL1_DOUT_PIN A2
#define LOADCELL1_SCK_PIN A3
#define LOADCELL2_DOUT_PIN A0
#define LOADCELL2_SCK_PIN A1
HX711 scale1, scale2;
float ReadEERPOM(int address);
void WriteEERPOM(float Value, int address);
void calibrateMode();
void initializeScale();
void splashScreen(int delayt);
void updateButton();
float calibration_factor1 = ReadEERPOM(ADDRESS1);
float calibration_factor2 = ReadEERPOM(ADDRESS2);

/*Buttons*/
int SW1 = 0;
int SWP1 = 1;
int SWPS1 = 1;
int SW2 = 0;
int SWPS2 = 1;
int SWP2 = 1;
int SW3 = 0;
int SWP3 = 1;
int SWPS3 = 1;
int SW4 = 0;
int SWP4 = 1;
int SWPS4 = 1;
int SW5 = 0;
int SWP5 = 1;
int SWPS5 = 1;
unsigned long SW1Time = 0; // the last time the input pin was toggled
unsigned long SW2Time = 0;
unsigned long SW3Time = 0;
unsigned long SW4Time = 0;
unsigned long SW5Time = 0;
int menu = 0;                     //1.Change Dispense weight,2)......
unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers
unsigned long lcd_last_refresh = 0;
unsigned long lcd_refresh = 250;
unsigned long menu_start_Or_lst_ButtonPress = 0;
unsigned long menu_Or_button_timeout = 15000;
unsigned long button_incrementdelay = 150;
unsigned long lastButton_time = 150;

int dispenseWeight = ReadEERPOM(ADDRESS3);
float startWeight = 0;
float threshold_weight = 2;

void setup()
{
  pinMode(A4, OUTPUT);
  digitalWrite(A4, 0);
  lcd.begin(16, 2);
  Serial.begin(9600);
  initializeScale();
  WriteEERPOM(ADDRESS3, 10.0);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(A7, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  pinMode(A6, INPUT_PULLUP);

  SW1 = digitalRead(6);
  SW2 = digitalRead(7);

  if ((SW1 + SW2) == 0)
  {
    delay(1000);
    // getValueDisp("Settings:1-3");
    //calibrateMode();
    calibrateMode();
  }

  splashScreen(1000);
}

void loop()
{
  mainDisplay();
  updateButton();
  if (SW5 == 0)
  {
    //tare Only
    scale1.tare();
  }
}

void dispense()
{
  float current_weight = 0;
  startWeight = getWeight();
  dispenseScreen(startWeight);
  if (startWeight >= dispenseWeight)
  {
    int i = 0;
    digitalWrite(A4,HIGH);
    while (i == 0)
    { updateButton();
      current_weight = getWeight();
      dispenseScreen(current_weight);
      if(SWPS3==0)
      { 
        digitalWrite(A4,LOW);
        i=1;
        lcd.clear();
        lcd.print("Emrgncy Stop");
        lcd.print("Weight:");
        lcd.print(current_weight - startWeight);
        delay(30000);
      }
      if (current_weight < ((startWeight - threshold_weight) - dispenseWeight))
      {
        digitalWrite(A4,LOW);
        i = 1;
      }
    }
  }
  else
  {
    lcd.clear();
    lcd.println("Not Enough Material");
    delay(1000);
  }
}

void mainDisplay()
{
  if (millis() > (lcd_last_refresh + lcd_refresh))
  {
    lcd_last_refresh = millis();
    lcd.clear();
    lcd.print("Cement");
    lcd.setCursor(0, 1);
    lcd.print("Weight= ");
    lcd.print(getWeight());
    lcd.print("Kgs");
  }
}

void dispenseScreen(float weight)
{
  if (millis() > (lcd_last_refresh + lcd_refresh))
  {
    lcd_last_refresh = millis();
    lcd.clear();
    lcd.print("Dispensed:");
    lcd.print((int)(weight - startWeight));
    lcd.setCursor(0, 1);
    lcd.print("Weight= ");
    lcd.print(weight);
    lcd.print("Kgs");
  }
}

float getWeight()
{
  float weight = scale1.get_units();
  weight = (int)((weight * 100) + 5);
  weight = weight / 100;
  return weight;
}

float ReadEERPOM(int address)
{

  union EPPROM
  {
    float cbvalue;
    char x[4];
  };
  EPPROM MYV;

  for (int i = 0; i < 4; i++)
  {
    MYV.x[i] = EEPROM.read(address + i);
  }

  //Serial.print("Read Vale: ");
  //Serial.println(MYV.cbvalue);

  return MYV.cbvalue;
}

void WriteEERPOM(float Value, int address)
{
  union EPPROM
  {
    float cbvalue;
    char x[4];
  };

  EPPROM MYV;

  MYV.cbvalue = Value;

  for (int i = 0; i < 4; i++)
  {
    EEPROM.update(address + i, MYV.x[i]);
  }

  //Serial.print("Write Vale: ");
  //Serial.println(Value);

  ReadEERPOM(address);
}

void calibrateMode()
{
  lcd.print("Calibration Mode");
  delay(2000);
  menu_start_Or_lst_ButtonPress = millis();
  scale1.tare();
  //scale2.tare();
  lcd.clear();
  int x = 0;
  while (x == 0)
  {
    if ((millis() - menu_start_Or_lst_ButtonPress) > menu_Or_button_timeout)
    {
      x = 2;
    }

    if ((millis() - lcd_last_refresh) > lcd_refresh)
    {
      lcd_last_refresh = millis();
      lcd.clear();
      lcd.print("C1: ");
      lcd.print(calibration_factor1);
      lcd.setCursor(0, 1);
      lcd.print("C2: ");
      lcd.print(calibration_factor2);
      lcd.print("1 Or 2");
      scale1.tare();
    }
    updateButton();
    if ((SWPS1 == 0) && (SWPS2 != 0))
    {
      menu_start_Or_lst_ButtonPress = millis();
      int j = 0;
      while (j == 0)
      {
        if ((millis() - menu_start_Or_lst_ButtonPress) > menu_Or_button_timeout)
        {
          j = 2;
          WriteEERPOM(calibration_factor1, ADDRESS1);
          scale1.set_scale(calibration_factor1); //Adjust to this calibration factor
          lcd.clear();
          lcd.print("Saved");
        }
        updateButton();

        if ((millis() - lastButton_time) > button_incrementdelay)
        {
          lastButton_time = millis();

          if ((SWPS1 == 0) && (SWPS2 != 0))
          {
            menu_start_Or_lst_ButtonPress = millis();
            calibration_factor1 = calibration_factor1 + 100;
          }

          if ((SWPS2 == 0) && (SWPS1 != 0))
          {
            menu_start_Or_lst_ButtonPress = millis();
            calibration_factor1 = calibration_factor1 - 100;
          }
        }

        if (millis() > (lcd_last_refresh + lcd_refresh))
        {
          lcd.clear();
          lcd.print("Val1:");
          lcd.print(calibration_factor1);
          lcd.setCursor(0, 1);
          lcd.print("Weight= ");
          lcd.print(scale1.get_units(), 2);
          Serial.println(scale1.get_units());
          lcd.print("Kgs");
        }
      }
    }

    if ((SWPS2 == 0) && (SWPS1 != 0))
    {
      menu_start_Or_lst_ButtonPress = millis();
      int j = 0;
      while (j == 0)
      {
        if ((millis() - menu_start_Or_lst_ButtonPress) > menu_Or_button_timeout)
        {
          j = 2;
          WriteEERPOM(calibration_factor2, ADDRESS2);
          scale2.set_scale(calibration_factor2); //Adjust to this calibration factor
          lcd.clear();
          lcd.print("Saved");
        }

        updateButton();

        if ((millis() - lastButton_time) > button_incrementdelay)
        {
          lastButton_time = millis();

          if ((SWPS1 == 0) && (SWPS2 != 0))
          {
            menu_start_Or_lst_ButtonPress = millis();
            calibration_factor2 = calibration_factor2 + 10;
          }

          if ((SWPS2 == 0) && (SWPS1 != 0))
          {
            menu_start_Or_lst_ButtonPress = millis();
            calibration_factor2 = calibration_factor2 - 10;
          }
        }

        if (millis() > (lcd_last_refresh + lcd_refresh))
        {
          lcd.clear();
          lcd.print("Val2:");
          lcd.print(calibration_factor1);
          lcd.setCursor(0, 1);
          lcd.print("Weight= ");
          lcd.print(scale2.get_units(), 2);
          lcd.print("Kgs");
        }
      }
    }
  }
}

void initializeScale()
{
  scale1.begin(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN);
  scale2.begin(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN);
  scale1.set_scale(calibration_factor1);
  scale2.set_scale(calibration_factor2);
  scale1.tare(); //Reset the scale to 0
  scale2.tare(); //Reset the scale to 0
  long zero_factor1 = scale1.read_average();
  long zero_factor2 = scale2.read_average();
  scale1.set_scale(calibration_factor1); //Adjust to this calibration factor
  scale2.set_scale(calibration_factor2); //Adjust to this calibration factor
}

void splashScreen(int delayt)
{

  lcd.setCursor(0, 1);
  lcd.print("Pittappillil");
  delay(delayt);
  lcd.clear();
  lcd.print("C1: ");
  lcd.print(calibration_factor1);
  lcd.setCursor(0, 1);
  lcd.print("C2: ");
  lcd.print(calibration_factor2);
  lcd.print("");
  delay(delayt);
  lcd.clear();
}

void updateButton()
{
  //Buttons SW1,SW2 INTERNAL
  //BUTTONS SW3,SW4,SW5->External
  SW1 = digitalRead(6);
  SW2 = digitalRead(7);
  SW3 = analogRead(A7);
  SW4 = digitalRead(A5);
  SW5 = analogRead(A6);
  if (SW1 != SWP1)
  {
    SW1Time = millis(); // reset the debouncing timer
    SWP1 = SW1;
  }

  if (SW2 != SWP2)
  {
    SW2Time = millis(); // reset the debouncing timer
    SWP2 = SW2;
  }

  if (SW3 != SWP3)
  {
    SW3Time = millis(); // reset the debouncing timer
    SWP3 = SW3;
  }

  if (SW4 != SWP4)
  {
    SW4Time = millis(); // reset the debouncing timer
    SWP4 = SW4;
  }

  if (SW5 != SWP5)
  {
    SW5Time = millis(); // reset the debouncing timer
    SWP5 = SW5;
  }

  if ((millis() - SW1Time) > debounceDelay)
  {
    if (SW1 != SWPS1)
    {
      SWPS1 = SW1;
    }
  }

  if ((millis() - SW2Time) > debounceDelay)
  {
    if (SW2 != SWPS2)
    {
      SWPS2 = SW2;
    }
  }

  if ((millis() - SW3Time) > debounceDelay)
  {
    if (SW3 != SWPS3)
    {
      SWPS3 = SW3;
    }
  }

  if ((millis() - SW4Time) > debounceDelay)
  {
    if (SW4 != SWPS4)
    {
      SWPS4 = SW4;
    }
  }

  if ((millis() - SW5Time) > debounceDelay)
  {
    if (SW5 != SWPS5)
    {
      SWPS5 = SW5;
    }
  }
  //Serial.print("SWPS3:");
  //Serial.print(SWPS3);
  //Serial.print(" SWPS4:");
  //Serial.print(SWPS4);
  //Serial.print(" SWPS5:");
  //Serial.println(SWPS5);
}

void calibrate2()
{
  scale1.set_scale(1);
  scale2.set_scale(1);
  float w1 = getValueDisp("Weight1", 0.1, 0);
  float a1 = scale1.get_units();
  float b1 = 0;
  lcd.clear();
  lcd.print("Done");
  updateButton();
  delay(1000);
  Serial.println("TEST");

  float w2 = getValueDisp("Weight2", 0.1, 0);
  lcd.println("Saving..");
  delay(5000);
  float a2 = scale1.get_units();
  float b2 = scale2.get_units();
  lcd.print("Done");
  delay(1500);
  lcd.clear();
  lcd.println("Saving..");
  calibration_factor1 = (((b1 * w2) - (b2 * w1)) / ((a2 * b1) - (a1 * b2)));
  calibration_factor2 = (((a1 * w2) - (a2 * w1)) / ((a1 * b2) - (a2 * b1)));
  //WriteEERPOM(ADDRESS1, calibration_factor1);
  //WriteEERPOM(ADDRESS2, calibration_factor2);
  delay(1000);
}

float getValueDisp(String text, float step, float start_value)
{
  float value = 0;
  int i = 0;
  while (i == 0)
  {
    updateButton();
    if (millis() > (lcd_last_refresh + lcd_refresh))
    {
      lcd_last_refresh = millis();
      lcd.clear();
      lcd.print(text);
      lcd.setCursor(0, 1);
      lcd.print(value);
    }

    updateButton();
    if ((millis() - lastButton_time) > button_incrementdelay)
    {
      if ((SWPS5 == 0))
      {
        lastButton_time = millis();
        menu_start_Or_lst_ButtonPress = millis();
        value = value + step;
      }

      if ((SWPS3 == 0))
      {
        lastButton_time = millis();
        menu_start_Or_lst_ButtonPress = millis();
        value = value - step;
      }

      if ((SWPS4 == 0))
      {
        lastButton_time = millis();
        menu_start_Or_lst_ButtonPress = millis();
        return value;
      }
    }
  }
}
