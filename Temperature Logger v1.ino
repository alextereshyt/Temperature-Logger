#define USE_STANDARD_LCD 1
#include <ItemSubMenu.h>
#include <ItemCommand.h>
#include <ItemProgress.h>
#include <LcdMenu.h>
#include <utils/commandProccesors.h>
#include <AsyncTimer.h>
#include <RTClib.h>


#include "FS.h"
#include "SD.h"
#include "SPI.h"

File log_file;

RTC_DS3231 rtc;


#define CALIBRATE "83.4" //just increase or decrease when temperature doesnt match 
#define IS_INVERTED "1"  //invert thermistor
#define TEMP_SENSOR_DELAY "250" //interval between showing real time stats
#define LOGGING_DELAY "5000" //interval beetween logs 

#define RESISTOR "100000" //configure devider

#define LCD_ROWS 2
#define LCD_COLS 16

 #define outputA 15
 #define outputB 1
 #define outputS 3

  int counter = 0; 
  int aState;
  int aLastState;  
  int lastCounter = counter;

  bool isInCommand =0;



int ThermistorPin = 32;  //Thermistor config
int Vo;
float R1 = String(RESISTOR).toFloat();  // value of R1 on board
float logR2, R2, T;
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741;  //steinhart-hart coeficients for thermistor



AsyncTimer input;
AsyncTimer encoder;
AsyncTimer logger;
char current_command;

extern MenuItem* RTSMenu[];
extern MenuItem* Config_MENU[];

void tempStats();
void timeStats();
void dashboard();
void logSd();
void config();
void sdStats();

MAIN_MENU(
  ITEM_SUBMENU("Real-Time stat", RTSMenu),
 ITEM_COMMAND("SD Card info",*sdStats),
  ITEM_SUBMENU("Config",Config_MENU)
);

SUB_MENU(RTSMenu, mainMenu,
         ITEM_COMMAND("Dashboard", *dashboard),
         ITEM_COMMAND("Temperature", *tempStats),
         ITEM_COMMAND("Time",*timeStats));


#define TEMP_SENSOR_DELAY_C "rts_int="
#define LOGGING_DELAY_C "log_int="
#define CALIBRATE_C "calib="
#define IS_INVERTED_C "invert="
#define RESISTOR_C "r1="

const String config1 = String(TEMP_SENSOR_DELAY_C)+String(TEMP_SENSOR_DELAY);
const String config2 = String(LOGGING_DELAY_C)+String(LOGGING_DELAY);
const String config3 = String(CALIBRATE_C)+String(CALIBRATE);
const String config4 = String(IS_INVERTED_C)+String(IS_INVERTED);
const String config5 = String(RESISTOR_C)+String(RESISTOR);

LcdMenu menu(LCD_ROWS, LCD_COLS);

void menuBack(){
  menu.back();
}

SUB_MENU(Config_MENU, mainMenu,
         ITEM_COMMAND(config1.c_str(),*menuBack),
         ITEM_COMMAND(config2.c_str(),*menuBack),
         ITEM_COMMAND(config3.c_str(),*menuBack),
         ITEM_COMMAND(config4.c_str(),*menuBack),
          ITEM_COMMAND(config5.c_str(),*menuBack));

void setup() {
  Serial.begin(9600);
  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   pinMode (outputA,INPUT);
   pinMode (outputB,INPUT);
   pinMode (outputS,INPUT); 
   aLastState = digitalRead(outputA);
  menu.setupLcdWithMenu(13, 12, 25, 26, 27, 14, mainMenu);
  menu.lcd->clear();
  menu.lcd->setCursor(0, 0);
  menu.lcd->print("  Temperature  ");
  menu.lcd->setCursor(0, 1);
  menu.lcd->print("  Logger  v.1  ");
  delay(3000); 
  if(!SD.begin(5)){
     menu.lcd->clear();
     menu.lcd->setCursor(0, 0);
     menu.lcd->print("Card Mount Failed");
    for(;;){}
  }
  menu.resetMenu();
  encoder.setInterval([]() {
    updateEncoder();
  },0);
  input.setInterval([]() {
    processInput();
  },160);
   logger.setInterval([]() {
    logSd();
  },String(LOGGING_DELAY).toFloat());
 
                
}

void config(){
 isInCommand = 1;
menu.lcd->setCursor(0, 0);
    menu.lcd->print("rts_int=" + String(TEMP_SENSOR_DELAY)+" ms");
menu.lcd->setCursor(0,1);
 menu.lcd->print("sd_int=" + String(LOGGING_DELAY)+" ms");
 while (isInCommand) {  
    input.handle();
    logger.handle();
  }
  menu.back();
}

void logSd(){
DateTime now = rtc.now();

File log_file = SD.open("/"+String(now.day())+"_"+String(now.month())+"_"+String(now.year())+"_log.txt", FILE_APPEND); 
 if(!log_file){
   log_file = SD.open("/"+String(now.day())+"_"+String(now.month())+"_"+String(now.year())+"_log.txt", FILE_WRITE); 
  }
messTemp();

if(!log_file.println(String(now.hour())+":"+String(now.minute())+":"+String(now.second()) + "  "+String(T)+"C")){ menu.lcd->clear();
     menu.lcd->setCursor(0, 0);
     menu.lcd->print("SD Card FAIL!");}
log_file.close();
};

void updateEncoder(){
aState = digitalRead(outputA);
   if (aState != aLastState){     
     if (digitalRead(outputB) != aState) { 
       counter ++;
     } else {
       counter --;
     }
    
   } 
   aLastState = aState;
   
}

void processInput() {
  if (!digitalRead(outputS)) {if(!isInCommand) menu.enter(); else isInCommand = 0; delay(160);}
  
   if (counter > lastCounter){     
      menu.up();
     
     } 
   if (counter < lastCounter) {
       menu.down();
      
     }
    
   lastCounter = counter;
}

void loop() {
  input.handle();
  encoder.handle();
  logger.handle();
}

void messTemp(){
 Vo = analogRead(ThermistorPin);
    if(String(IS_INVERTED).toFloat())Vo = 4095 - Vo;
    R2 = R1 * ((4095 / (float)Vo ));
    logR2 = log(R2);
    T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
    T = T - 273.15;
    T = T + String(CALIBRATE).toFloat();
     
}

void dashboard() {
  isInCommand = 1;
  AsyncTimer temp_stat;
  int temp_timer_id = temp_stat.setInterval([]() {
    messTemp();
    menu.lcd->setCursor(0, 1);
    menu.lcd->print(T);
    menu.lcd->setCursor(5, 1);
    menu.lcd->print(" C");
  },String(TEMP_SENSOR_DELAY).toFloat());

  menu.lcd->clear();
  menu.lcd->setCursor(10, 1);
  menu.lcd->print(String(TEMP_SENSOR_DELAY) + "ms");

  while (isInCommand) {
    DateTime now = rtc.now();
    menu.lcd->setCursor(0, 0);
    menu.lcd->print(String(now.hour())+":"+String(now.minute())+":"+String(now.second()));
    menu.lcd->setCursor(10, 0);
     menu.lcd->print(String(now.day())+"/"+String(now.month()));
    temp_stat.handle();
    input.handle();
    logger.handle();
  }
  temp_stat.cancel(temp_timer_id);
  menu.back();
  menu.back();
}

void timeStats() {
  isInCommand = 1;
  menu.lcd->clear();
  menu.lcd->setCursor(0,0);
  menu.lcd->print("Time:");

  while (isInCommand) {
    DateTime now = rtc.now();
    menu.lcd->setCursor(0, 1);
    menu.lcd->print(String(now.hour())+":"+String(now.minute())+":"+String(now.second()));
    menu.lcd->setCursor(10, 1);
    menu.lcd->print(String(now.day())+"/"+String(now.month()));
    input.handle();
    
  }
  menu.back();menu.back();
}

void tempStats() {
  isInCommand = 1;
  AsyncTimer temp_stat;
  int temp_timer_id = temp_stat.setInterval([]() {
    messTemp();
    menu.lcd->setCursor(0, 1);
    menu.lcd->print(T);
    menu.lcd->setCursor(5, 1);
    menu.lcd->print(" C");
  },String(TEMP_SENSOR_DELAY).toFloat());

  menu.lcd->clear();
  menu.lcd->setCursor(0, 0);
  menu.lcd->print("Temperature:");
  menu.lcd->setCursor(10, 1);
  menu.lcd->print(String(TEMP_SENSOR_DELAY) + "ms");

  while (isInCommand) {
    temp_stat.handle();
    input.handle();
  }
  temp_stat.cancel(temp_timer_id);
  menu.back();menu.back();
}

void sdStats(){
  isInCommand = 1;

  int sd_total = SD.totalBytes() / (1024 * 1024);
  int sd_used =  SD.usedBytes() / (1024 * 1024);

  menu.lcd->clear();
  menu.lcd->setCursor(0, 0);

uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    menu.lcd->print("No SD card attached");
    return;
  }

  menu.lcd->print("Type: ");
  if(cardType == CARD_MMC){
   menu.lcd->print("MMC");
  } else if(cardType == CARD_SD){
    menu.lcd->print("SDSC");
  } else if(cardType == CARD_SDHC){
    menu.lcd->print("SDHC");
  } else {
    menu.lcd->print("UNKNOWN");
  }
menu.lcd->print(" FAT32");
  menu.lcd->setCursor(0, 1);
  menu.lcd->print("Used: "+String(sd_used)+"/"+String(sd_total)+ " Mb");
    while (isInCommand) {
    input.handle();
  }
  menu.back();
}
