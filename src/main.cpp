#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <TMRpcm.h>
#define DOOR_PIN 2
#define CLOSE_PIN 3
#define CRUISE_PIN 4
const int chipSelect = 10;
TMRpcm tmrpcm;
LiquidCrystal_I2C lcd(0x27,20,4);
int mode = 1;
File routefile;

char trigger;
int timer;
String soundName;
String stationCode;
int station;
int busy;
int endFlag;
int routeindex = -1;

unsigned long secondstarttime;
unsigned long currenttime;

void setup () {
  pinMode(DOOR_PIN,INPUT_PULLUP);
  pinMode(CLOSE_PIN,INPUT_PULLUP);
  pinMode(CRUISE_PIN,INPUT_PULLUP);
  tmrpcm.speakerPin = 9;
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HEVBSZ by ");
  lcd.setCursor(9, 1);
  lcd.print("BenceIT");
  if (!SD.begin(chipSelect)) {
     lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Sdcard not found");
    while (true);
  }
  tmrpcm.setVolume(5);
  delay(2000);
}

void routeRead(){
  int countindex = 0;
  routefile = SD.open("/routes/test.txt");
    if(routefile){
      while (routefile.available()){
        String line = routefile.readStringUntil('\n');
        if (countindex == routeindex + 1)
        {
          char str[line.length() + 1];
          line.toCharArray(str, sizeof(str)); // String konvertálása C-style karaktertömbbé

          char* token = strtok(str, ","); // Első token
          int index = 0;
          lcd.clear();
          while (token != NULL) {
              switch (index) {
                  case 0: lcd.setCursor(0, 0);lcd.print(String(token)); break;
                  case 1: trigger = token[1]; break;
                  case 2:
                    timer = atoi(token);
                    lcd.setCursor(13,1);
                    if (String(timer).length() == 3) lcd.print(String(timer));
                    else if(String(timer).length() == 2) lcd.print('0' + String(timer));
                    else lcd.print("00" + String(timer));
                    break;
                  case 3: soundName = String(token); break;
                  case 4: stationCode = String(token); break;
                  case 5: station = atoi(token); break;
                  case 6: busy = atoi(token); break;
                  case 7: endFlag = atoi(token); break;
              }
              token = strtok(NULL, ","); // Következő token
              index++;
          }
          routeindex++;
          mode = 2;
          break;
        }
        else countindex++;
        
      }
    }
  routefile.close();
}

void coutStarter(){
  if(digitalRead(CRUISE_PIN) == HIGH && trigger == 'M')mode = 3;
    else if(digitalRead(DOOR_PIN) == HIGH && trigger == 'A')mode = 3;
    else if(digitalRead(CLOSE_PIN) == HIGH && trigger == 'Z')mode = 3;
}


void loop () {
  switch (mode)
  {
  case 1:
    routeRead();
    break;

  case 2:
    coutStarter();
    secondstarttime = millis();
    break;
  
  case 3:
    if (timer != 0){
      currenttime = millis();
      if (currenttime - secondstarttime > 1000) {
        timer = timer - 1;
        secondstarttime = currenttime;
        lcd.setCursor(13,1);
        if (String(timer).length() == 3) lcd.print(String(timer));
        else if(String(timer).length() == 2) lcd.print('0' + String(timer));
        else lcd.print("00" + String(timer));
        Serial.println(timer);
      }
    }
    else mode = 4;
    break;
  
  case 4:
    tmrpcm.play(("/sounds/" + soundName + ".wav").c_str());
    mode = 1;
    break;
  }


  lcd.setCursor(8,1);
    if(digitalRead(CRUISE_PIN) == HIGH) lcd.print('M');
    else lcd.print('-');
    if(digitalRead(DOOR_PIN) == HIGH) lcd.print('A');
    else lcd.print('-');
    if(digitalRead(CLOSE_PIN) == HIGH) lcd.print('Z');
    else lcd.print('-');
}