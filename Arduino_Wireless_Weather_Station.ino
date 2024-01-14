#include <TFT_HX8357_Due.h> // https://github.com/Bodmer/TFT_HX8357_Due
#include <Sodaq_DS3231.h>  //RTC Library https://github.com/SodaqMoja/Sodaq_DS3231
#include "DHT.h"          //https://github.com/adafruit/DHT-sensor-library
#include "RF24.h"        //https://github.com/TMRh20/RF24

#define DHTPIN 8  

#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

RF24 myRadio (6, 7);
byte addresses[][6] = {"0"};

TFT_HX8357_Due tft = TFT_HX8357_Due();       // Invoke custom library

float remoteHumidity = 0.0;
float remoteTemperature = 0.0;

String dateString;
String hours;
int minuteNow=0;
int minutePrevious=0;

struct package
{
  float temperature ;
  float humidity ;
};

float previousIndoorHumidity = 0;
float previousIndoorTemperature = 10;

float previousRemoteHumidity = 0.1;
float previousRemoteTemperature = 0.1;

float indoorHumidity = 0;
float indoorTemperature = 0;

typedef struct package Package;
Package data;

void setup() {

  Serial.begin(9600);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.invertDisplay(true);
  delay(100);

  rtc.begin();
  dht.begin();
  delay(2000);
  //setRTCTime();

  startWirelessCommunication();
  printUI();
}

void loop() {

   checkForWirelessData();

   getAndPrintTime();
   
   printIndoorTemperature();
   printIndoorHumidity();

   printRemoteTemperature();
   printRemoteHumidity();
}

void setRTCTime()
{
  DateTime dt(2016, 6, 7, 13, 40, 30, 2); // Year, Month, Day, Hour, Minutes, Seconds, Day of Week
  rtc.setDateTime(dt); //Adjust date-time as defined 'dt' above 
}

void startWirelessCommunication()
{
  myRadio.begin(); 
  myRadio.setChannel(115); 
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ; 
  myRadio.openReadingPipe(1, addresses[0]);
  myRadio.startListening();
  delay(100);
}

void checkForWirelessData()
{
    if ( myRadio.available()) 
  {
    while (myRadio.available())
    {
      myRadio.read( &data, sizeof(data) );
      previousRemoteTemperature = remoteTemperature;
      previousRemoteHumidity = remoteHumidity;
      remoteTemperature = data.temperature;
      remoteHumidity = data.humidity;
    }
    Serial.print("\nPackage:");
    Serial.print("\n");
    Serial.println(data.temperature);
    Serial.println(data.humidity);
  } 
}

void printUI()
{
  
tft.drawRoundRect(5,5,470,71,5,TFT_WHITE);
tft.drawRoundRect(6,6,470,71,5,TFT_WHITE);

tft.drawRoundRect(5,90,220,225,5,TFT_WHITE);
tft.drawRoundRect(6,91,220,225,5,TFT_WHITE);

tft.drawRoundRect(250,90,220,225,5,TFT_WHITE);
tft.drawRoundRect(251,91,220,225,5,TFT_WHITE);
  
tft.fillRect(26,90,180,40,TFT_GREEN);
tft.fillRect(270,90,180,40,TFT_CYAN);

tft.setCursor(62,100);
tft.setTextColor(TFT_BLACK);
tft.setTextSize(3);
tft.print("REMOTE");

tft.setCursor(312,100);
tft.setTextColor(TFT_BLACK);
tft.setTextSize(3);
tft.print("INDOOR");

tft.setCursor(162,165);
tft.setTextColor(TFT_GREEN);
tft.setTextSize(6);
tft.print("%");

tft.setCursor(412,165);
tft.setTextColor(TFT_CYAN);
tft.setTextSize(6);
tft.print("%");

tft.setCursor(162,230);
tft.setTextColor(TFT_GREEN);
tft.setTextSize(6);
tft.print("C");

tft.setCursor(145,230);
tft.setTextColor(TFT_GREEN);
tft.setTextSize(2);
tft.print("o");

tft.setCursor(412,230);
tft.setTextColor(TFT_CYAN);
tft.setTextSize(6);
tft.print("C");

tft.setCursor(395,230);
tft.setTextColor(TFT_CYAN);
tft.setTextSize(2);
tft.print("o");

}

void getAndPrintTime()
{
  
   delay(100);
   DateTime now = rtc.now(); //get the current date-time
   minuteNow = now.minute();
   if(minuteNow!=minutePrevious)
   {
      readSensor();
      dateString = getDayOfWeek(now.dayOfWeek())+" ";
      dateString = dateString+String(now.date())+"/"+String(now.month());
      dateString= dateString+"/"+ String(now.year()); 
      minutePrevious = minuteNow;
      hours = String(now.hour());
    if(now.minute()<10)
    {
      hours = hours+":0"+String(now.minute());
    }else
    {
      hours = hours+":"+String(now.minute());
    }

    printTime();
   }
}

void printTime()
{
  String dateAndTime = dateString+" "+hours;
  
  tft.setTextSize(2);
  char charBuf[25];
  dateAndTime.toCharArray(charBuf, 25);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString(charBuf,240,25,2);

  Serial.println(dateAndTime);
}

String getDayOfWeek(int i)
{
  switch(i)
  {
    case 1: return "Mon";break;
    case 2: return "Tue";break;
    case 3: return "Wed";break;
    case 4: return "Thu";break;
    case 5: return "Fri";break;
    case 6: return "Sat";break;
    case 7: return "Sun";break;
    default: return "Mon";break;
  }
}

void readSensor()
{
  previousIndoorTemperature = indoorTemperature;
  previousIndoorHumidity = indoorHumidity;
  
  indoorHumidity = dht.readHumidity();
  indoorTemperature = dht.readTemperature();
  Serial.println(indoorTemperature);
  Serial.println(indoorHumidity);

  previousRemoteTemperature = remoteTemperature;
  previousRemoteHumidity = remoteHumidity;
  remoteTemperature = dht.readTemperature() - 0.6;
  remoteHumidity = dht.readHumidity() - 0.6;
}

void printIndoorTemperature()
{
  if(indoorTemperature != previousIndoorTemperature)
  {

    String temperature = String(indoorTemperature,1);

    tft.fillRect(270,232,120,40,TFT_BLACK);
  
    tft.setCursor(270,234);
    tft.setTextColor(TFT_CYAN);
    tft.setTextSize(5);
    tft.print(temperature);

    previousIndoorTemperature = indoorTemperature;
  }
}

void printRemoteHumidity()
{
  String humidity;
  if(remoteHumidity != previousRemoteHumidity)
  {
    if(remoteHumidity == 0.0 && remoteTemperature == 0.0) //We just booted up
    {
      humidity = "---";
    }else
    {
          humidity = String(remoteHumidity,1);
    }
    
    tft.fillRect(20,167,120,40,TFT_BLACK);
  
    tft.setCursor(20,167);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(5);
    tft.print(humidity);

    previousRemoteHumidity = remoteHumidity;
  }
}

void printRemoteTemperature()
{
  String temperature;
  if(remoteTemperature != previousRemoteTemperature)
  {
    if(remoteHumidity == 0.0 && remoteTemperature == 0.0) //We just booted up
    {
      temperature = "---";
    }else
    {
          temperature = String(remoteTemperature,1);
    }
    
    tft.fillRect(20,232,120,40,TFT_BLACK);
  
    tft.setCursor(20,234);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(5);
    tft.print(temperature);

    previousRemoteTemperature = remoteTemperature;
  }
}

void printIndoorHumidity()
{
   if(indoorHumidity != previousIndoorHumidity)
  {

    String humidity = String(indoorHumidity,1);

    tft.fillRect(270,167,120,40,TFT_BLACK);
  
    tft.setCursor(270,167);
    tft.setTextColor(TFT_CYAN);
    tft.setTextSize(5);
    tft.print(humidity);

    previousIndoorHumidity = indoorHumidity; 
  }
}




