#include <SPI.h>
#include <SD.h>
#include <RTCZero.h>
RTCZero rtc;

char* daynames[]={"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};


File myFile;
const uint8_t sdCS = 4;


const uint8_t eeg = A0;
const uint8_t eog = A1;
const uint8_t ekg = A2;

long lastTime = 0;
const int Fs = 3; //ends up being about 5ms by trial and error //200Hz sampling rate
const int samples = 2048;


uint16_t eegData[samples] = {0};
uint16_t eogData[samples] = {0};
uint16_t ekgData[samples] = {0};
long timer[samples] = {0};


const int statusLED = 8;

int i = 0;

String dateString = "";


/* Change these values to set the current initial time */
const byte seconds = 10;
const byte minutes = 55;
const byte hours = 3;

/* Change these values to set the current initial date */
const byte day = 9;
const byte month = 3;
const byte year = 18;


void setup() {
  delay(4000);
  
  Serial.begin(115200);
  SPI.begin();



  //Initializing internal RTC
  //*******************
  //DOES NOT GET UPDATED AUTOMATICALLY
  rtc.begin(); // initialize RTC
  rtc.setTime(hours, minutes, seconds); //hr, min, sec
  rtc.setDate(day, month, year); //day, month, year

  String yr = String(rtc.getYear());
  String mnth = String(rtc.getMonth());
  if(rtc.getMonth() < 10) mnth = "0" + mnth;
  
  String dy = String(rtc.getDay());
  if(rtc.getDay() < 10) dy = "0" + dy;

  dateString = yr+mnth+dy;
  
  
  
  analogReadResolution(12);
  pinMode(statusLED, OUTPUT);
  digitalWrite(statusLED, LOW);



  pinMode(sdCS,OUTPUT);
  if (!SD.begin(sdCS)) {
    //Serial.println("initialization failed!");
    for(int a = 0; a < 25; a++)
    {
      digitalWrite(statusLED, HIGH);
      delay(250);
      digitalWrite(statusLED, LOW);
      delay(250);
    }
    return;
  }
  Serial.println("initialization done.");
  SD.mkdir(dateString);

  

  lastTime = millis();
}

void loop()
{
  while(millis() - lastTime > Fs)
  {
    eegData[i] = analogRead(eeg);
    eogData[i] = analogRead(eog);
    ekgData[i] = analogRead(ekg);    
    timer[i] = millis();
    
    i++;
    lastTime = millis();
  }

  if(i > samples-1)
  {
    //Serial.println("now time to save to SD");
    saveToSD();
    i = 0;
  }

}


void saveToSD()
{
  digitalWrite(statusLED, HIGH);
  
  String hr = String(rtc.getHours());
  if(rtc.getHours() < 10) hr = "0" + hr;
  
  String mins = String(rtc.getMinutes());
  if(rtc.getMinutes() < 10) mins = "0" + mins;
  
  String sec = String(rtc.getSeconds());
  if(rtc.getSeconds() < 10) sec = "0" + sec;


  myFile = SD.open(dateString + "/" + hr+mins+sec + ".csv", FILE_WRITE);

  
  if(myFile)
  {
    myFile.println("Time(ms),EEG,EOG,EKG");
    
    for (int j = 0; j < samples; j++)
    {
      myFile.print(timer[j]);
      myFile.print(",");
      myFile.print(eegData[j]);
      myFile.print(",");
      myFile.print(eogData[j]);
      myFile.print(",");
      myFile.println(ekgData[j]);
    }
  }
  else
  {
    //Serial.println("error making file");
  }

  myFile.close();
  digitalWrite(statusLED, LOW);
}

