#include <LiquidCrystal.h>
#include "SoundData.h"
#include "XT_DAC_Audio.h"
#include "MusicDefinitions.h"
#include <RTClib.h>
#include <SPI.h>
#include <Wire.h> 
RTC_DS3231 rtc;
LiquidCrystal lcd(19, 23, 18, 17, 16, 5);

hw_timer_t * timer_1 = NULL;
portMUX_TYPE timerMux_1 = portMUX_INITIALIZER_UNLOCKED;

long delay_1=millis();
float temperature_samples[12];
float avg_temp;
volatile int interruptCounter;
volatile bool isInterruptCalled;

int alarmh=6;
int alarmm=30;
int alarm_am_pm = 0;
int n=0;
//classes
int8_t PROGMEM TwinkleTwinkle[] = {
  NOTE_C5,NOTE_C5,NOTE_G5,NOTE_G5,NOTE_A5,NOTE_A5,NOTE_G5,BEAT_2,
  NOTE_F5,NOTE_F5,NOTE_E5,NOTE_E5,NOTE_D5,NOTE_D5,NOTE_C5,BEAT_2,
  NOTE_G5,NOTE_G5,NOTE_F5,NOTE_F5,NOTE_E5,NOTE_E5,NOTE_D5,BEAT_2,
  NOTE_G5,NOTE_G5,NOTE_F5,NOTE_F5,NOTE_E5,NOTE_E5,NOTE_D5,BEAT_2,
  NOTE_C5,NOTE_C5,NOTE_G5,NOTE_G5,NOTE_A5,NOTE_A5,NOTE_G5,BEAT_2,
  NOTE_F5,NOTE_F5,NOTE_E5,NOTE_E5,NOTE_D5,NOTE_D5,NOTE_C5,BEAT_4,  
  NOTE_SILENCE,BEAT_5,SCORE_END
};
XT_MusicScore_Class Music(TwinkleTwinkle,TEMPO_ALLEGRO,INSTRUMENT_PIANO); 
XT_DAC_Audio_Class DacAudio(25, 0);         
XT_Wav_Class Am(AM);
XT_Wav_Class Pm(PM);
XT_Wav_Class TheTimeIs(THETIMEIS);
XT_Wav_Class TheAlarmIsSetTo(THEALARMISETO);
XT_Wav_Class Zero(ZERO);
XT_Wav_Class OClock(OCLOCK);
XT_Wav_Class One(ONE);
XT_Wav_Class Two(TWO);
XT_Wav_Class Three(THREE);
XT_Wav_Class Four(FOUR);
XT_Wav_Class Five(FIVE);
XT_Wav_Class Six(SIX);
XT_Wav_Class Seven(SEVEN);
XT_Wav_Class Eight(EIGHT);
XT_Wav_Class Nine(NINE);
XT_Wav_Class Ten(TEN);
XT_Wav_Class Eleven(ELEVEN);
XT_Wav_Class Twelve(TWELVE);
XT_Wav_Class Thirteen(THIRTEEN);
XT_Wav_Class Fourteen(FOURTEEN);
XT_Wav_Class Fifteen(FIFTEEN);
XT_Wav_Class Sixteen(SIXTEEN);
XT_Wav_Class Seventeen(SEVENTEEN);
XT_Wav_Class Eighteen(EIGHTEEN);
XT_Wav_Class Nineteen(NINTEEN);
XT_Wav_Class Twenty(TWENTY);
XT_Wav_Class Thirty(THIRTY);
XT_Wav_Class Forty(FOURTY);
XT_Wav_Class Fifty(FIFTY);
XT_Sequence_Class Sequence;  

void IRAM_ATTR onInterrupt() {
  portENTER_CRITICAL_ISR(&timerMux_1);
  isInterruptCalled=true;
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux_1);
 
}

void setup() { 
pinMode(15,INPUT); // TellTimeButton
pinMode(2,INPUT); // SetButton
pinMode(4,INPUT ); //UpButton
Serial.begin(115200);
lcd.begin(16, 2);
if (! rtc.begin()) {
   Serial.println("Couldn't find RTC");
   while (1);
}

  timer_1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer_1, &onInterrupt, true);
  timerAlarmWrite(timer_1, 5000000, true);
  timerAlarmEnable(timer_1);
 
  float x=rtc.getTemperature();
  avg_temp=x;
  for(int i=0;i<12;i++){
    temperature_samples[i]=x;
  }
}

void loop() {
  if(isInterruptCalled==true){
      int index=interruptCounter%12;
      temperature_samples[index]=rtc.getTemperature();
      float tempSum=0;
      for(int i=0;i<12;i++){
        tempSum+=temperature_samples[i];
      }
      avg_temp=tempSum/12;
      isInterruptCalled=false;
    }
    
  DateTime now = rtc.now(); // used to get time from RTC(DS3231)
  int am_pm=0;//AM PM
  int h=now.hour();
  int m=now.minute();
  int s=now.second();
  
  if (h>11){
    am_pm=1;
  }
  if (h>12) {
   h=h-12;
  }
 lcd.setCursor(2,0);
    if(h>=0 && h<=9){
      lcd.print(" ");
    }
    lcd.print(h);
    lcd.print(':');
    if(m>=0 && m<=9){
      lcd.print("0");
    }
    lcd.print(m);
    lcd.print(':');
    if(s>=0 && s<=9){
      lcd.print("0");
    }
    lcd.print(s);
    
    if(am_pm==0){
      lcd.print(" AM");
    }
    else{
      lcd.print(" PM");
    }

    lcd.setCursor(0,1);
    int check_for_alarm_or_temp=s/10;
    if(check_for_alarm_or_temp%2!=0){
      lcd.print("Alarm: ");
      lcd.print(alarmh);
      lcd.print(':');
      lcd.print(alarmm);
      if(alarm_am_pm==0){
        lcd.print(" AM");
      }
      else{
        lcd.print(" PM");
      }
      if(s%10==9){
        while(millis()-delay_1<1000);
        delay_1=millis();
        lcd.clear();
      }
    }
    else{
      lcd.print("  Temp: ");
      lcd.print(avg_temp,1);
      lcd.print((char)223);
      lcd.print("C");
      if(s%10==9){
        while(millis()-delay_1<1000);
        delay_1=millis();
        lcd.clear();
      }
   }
     
  if (h==alarmh && m==alarmm && am_pm==alarm_am_pm) {
       digitalWrite (26,HIGH);
  }// alarm time set at 6:30 by default
  else{
     digitalWrite (26,LOW);//pin 26 to buzzer
  }
  
  DacAudio.FillBuffer(); 
 
  if(digitalRead(15)==HIGH){
    Sequence.RemoveAllPlayItems();            // Clear out any previous playlist
    delay(200);
    while(digitalRead(12));
    delay(200);
    is_Pressed(15,h,m,am_pm);
    DacAudio.Play(&Sequence);
    Serial.println(digitalRead(12));
  }
  
  if(digitalRead(2)==HIGH){
    delay(200);
    while(digitalRead(2));
    delay(200);
    n=(n+1)%3;
    Serial.println(n);
  }
  
  if(digitalRead(4)==HIGH){
    Sequence.RemoveAllPlayItems();
    delay(200);
    while(digitalRead(4));
    delay(200);
    setAlarm(4);
    DacAudio.Play(&Sequence);
    
  }
    
}

void setAlarm(int pinUp){
  if(n==0){
      ++alarmh;
      if(alarmh>=12){
        alarmh=0;
      }
//      Serial.println(hr);
    }
    else if(n==1){
       ++alarmm;
       if(alarmm>=60){
        alarmm=0;
        }
//       Serial.println(hr);
    }
      else if(n==2){
        alarm_am_pm=(alarm_am_pm+1)%2;
//        Serial.println(vm);
      }
      is_Alarm_Pressed(pinUp,alarmh,alarmm,alarm_am_pm);
}

void is_Pressed(int pinNumber, int h,int m,int am_pm){
    Sequence.AddPlayItem(&TheTimeIs);
     if (h==1){
        Sequence.AddPlayItem(&One);
     }
     else if (h==2){
      Sequence.AddPlayItem(&Two);
     }
     else if (h==3){
      Sequence.AddPlayItem(&Three);
     }
     else if (h==4){
      Sequence.AddPlayItem(&Four);
     }
     else if (h==5){
      Sequence.AddPlayItem(&Five);
     }
     else if (h==6){
      Sequence.AddPlayItem(&Six);
     }
     else if (h==7){
      Sequence.AddPlayItem(&Seven);
     }
     else if (h==8){
      Sequence.AddPlayItem(&Eight);
     }
     else if (h==9){
       Sequence.AddPlayItem(&Nine);
     }
     else if (h==10){
      Sequence.AddPlayItem(&Ten);
     }
     else if (h==11){
       Sequence.AddPlayItem(&Eleven);
     }
     else if (h==12){
       Sequence.AddPlayItem(&Twelve);
     }
     
    if (m==0){
       Sequence.AddPlayItem(&OClock);
    }
    else if (m==1){
     Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&One);
    }
    else if (m==2){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Two);
    }
    else if (m==3){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Three);
    }
    else if (m==4){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Four);
    }
    else if (m==5){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Five);
    }
    else if (m==6){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Six);
    }
    else if (m==7){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Seven);
    }
    else if (m==8){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Eight);
    }
    else if (m==9){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Nine);
    }
    else if (m==10){
     Sequence.AddPlayItem(&Ten);
    }
    else if (m==11){
     Sequence.AddPlayItem(&Eleven);
    }
    else if (m==12){
     Sequence.AddPlayItem(&Twelve);
    }
    else if (m==13){
     Sequence.AddPlayItem(&Thirteen);
    }
    else if (m==14){
     Sequence.AddPlayItem(&Fourteen);
    }
    else if (m==15){
     Sequence.AddPlayItem(&Fifteen);
    }
    else if (m==16){
     Sequence.AddPlayItem(&Sixteen);
    }
    else if (m==17){
     Sequence.AddPlayItem(&Seventeen);
    }
    else if (m==18){
     Sequence.AddPlayItem(&Eighteen);
    }
    else if (m==19){
     Sequence.AddPlayItem(&Nineteen);
    }
    else if (m==20){
     Sequence.AddPlayItem(&Twenty);
    }
    else if (m==21){
        Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&One);
    }
    else if (m==22){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Two);
    }
    else if (m==23){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Three);
    }
    else if (m==24){
       Sequence.AddPlayItem(&Twenty);
       Sequence.AddPlayItem(&Four);
    }
    else if (m==25){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Five);
    }
    else if (m==26){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Six);
    }
    else if (m==27){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Seven);
    }
    else if (m==28){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Eight);
    }
    else if (m==29){
       Sequence.AddPlayItem(&Twenty);
       Sequence.AddPlayItem(&Nine);
    }
    else if (m==30){
     Sequence.AddPlayItem(&Thirty);
    }
    else if (m==31){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&One);
    }
    else if (m==32){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Two);
    }
    else if (m==33){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Three);
    }
    else if (m==34){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Four);
    }
    else if (m==35){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Five);
    }
    else if (m==36){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Six);
    }
    else if (m==37){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Seven);
    }
    else if (m==38){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Eight);
    }
    else if (m==39){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Nine);
    }
    else if (m==40){
       Sequence.AddPlayItem(&Forty);
    }
    else if (m==41){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&One);
    }
    else if (m==42){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Two);
    }
    else if (m==43){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Three);
    }
    else if (m==44){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Four);
    }
    else if (m==45){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Five);
    }
    else if (m==46){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Six);
    }
    else if (m==47){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Seven);
    }
    else if (m==48){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Eight);
    }
    else if (m==49){
     Sequence.AddPlayItem(&Forty);
     Sequence.AddPlayItem(&Nine);
    }
    else if (m==50){
     Sequence.AddPlayItem(&Fifty);
    }
    else if (m==51){
     Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&One);
    }
    else if (m==52){
     Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Two);
    }
    else if (m==53){
     Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Three);
    }
    else if (m==54){
     Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Four);
    }
    else if (m==55){
       Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Five);
    }
    else if (m==56){
       Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Six);
    }
    else if (m==57){
       Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Seven);
    }
    else if (m==58){
       Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Eight);
    }
    else if (m==59){
      Sequence.AddPlayItem(&Fifty);
      Sequence.AddPlayItem(&Nine);
    }
    if (am_pm==0){ 
      Sequence.AddPlayItem(&Am); 
    }
    else if (am_pm==1){
      Sequence.AddPlayItem(&Pm);    
    }
}

void is_Alarm_Pressed(int pinNumber, int h,int m,int am_pm){

    Sequence.AddPlayItem(&TheAlarmIsSetTo);
     if (h==1){
        Sequence.AddPlayItem(&One);
     }
     else if (h==2){
      Sequence.AddPlayItem(&Two);
     }
     else if (h==3){
      Sequence.AddPlayItem(&Three);
     }
     else if (h==4){
      Sequence.AddPlayItem(&Four);
     }
     else if (h==5){
      Sequence.AddPlayItem(&Five);
     }
     else if (h==6){
      Sequence.AddPlayItem(&Six);
     }
     else if (h==7){
      Sequence.AddPlayItem(&Seven);
     }
     else if (h==8){
      Sequence.AddPlayItem(&Eight);
     }
     else if (h==9){
       Sequence.AddPlayItem(&Nine);
     }
     else if (h==10){
      Sequence.AddPlayItem(&Ten);
     }
     else if (h==11){
       Sequence.AddPlayItem(&Eleven);
     }
     else if (h==12){
       Sequence.AddPlayItem(&Twelve);
     }
     
    if (m==0){
       Sequence.AddPlayItem(&OClock);
    }
    else if (m==1){
     Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&One);
    }
    else if (m==2){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Two);
    }
    else if (m==3){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Three);
    }
    else if (m==4){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Four);
    }
    else if (m==5){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Five);
    }
    else if (m==6){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Six);
    }
    else if (m==7){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Seven);
    }
    else if (m==8){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Eight);
    }
    else if (m==9){
      Sequence.AddPlayItem(&Zero);
      Sequence.AddPlayItem(&Nine);
    }
    else if (m==10){
     Sequence.AddPlayItem(&Ten);
    }
    else if (m==11){
     Sequence.AddPlayItem(&Eleven);
    }
    else if (m==12){
     Sequence.AddPlayItem(&Twelve);
    }
    else if (m==13){
     Sequence.AddPlayItem(&Thirteen);
    }
    else if (m==14){
     Sequence.AddPlayItem(&Fourteen);
    }
    else if (m==15){
     Sequence.AddPlayItem(&Fifteen);
    }
    else if (m==16){
     Sequence.AddPlayItem(&Sixteen);
    }
    else if (m==17){
     Sequence.AddPlayItem(&Seventeen);
    }
    else if (m==18){
     Sequence.AddPlayItem(&Eighteen);
    }
    else if (m==19){
     Sequence.AddPlayItem(&Nineteen);
    }
    else if (m==20){
     Sequence.AddPlayItem(&Twenty);
    }
    else if (m==21){
        Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&One);
    }
    else if (m==22){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Two);
    }
    else if (m==23){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Three);
    }
    else if (m==24){
       Sequence.AddPlayItem(&Twenty);
       Sequence.AddPlayItem(&Four);
    }
    else if (m==25){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Five);
    }
    else if (m==26){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Six);
    }
    else if (m==27){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Seven);
    }
    else if (m==28){
       Sequence.AddPlayItem(&Twenty);
        Sequence.AddPlayItem(&Eight);
    }
    else if (m==29){
       Sequence.AddPlayItem(&Twenty);
       Sequence.AddPlayItem(&Nine);
    }
    else if (m==30){
     Sequence.AddPlayItem(&Thirty);
    }
    else if (m==31){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&One);
    }
    else if (m==32){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Two);
    }
    else if (m==33){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Three);
    }
    else if (m==34){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Four);
    }
    else if (m==35){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Five);
    }
    else if (m==36){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Six);
    }
    else if (m==37){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Seven);
    }
    else if (m==38){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Eight);
    }
    else if (m==39){
       Sequence.AddPlayItem(&Thirty);
       Sequence.AddPlayItem(&Nine);
    }
    else if (m==40){
       Sequence.AddPlayItem(&Forty);
    }
    else if (m==41){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&One);
    }
    else if (m==42){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Two);
    }
    else if (m==43){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Three);
    }
    else if (m==44){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Four);
    }
    else if (m==45){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Five);
    }
    else if (m==46){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Six);
    }
    else if (m==47){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Seven);
    }
    else if (m==48){
       Sequence.AddPlayItem(&Forty);
       Sequence.AddPlayItem(&Eight);
    }
    else if (m==49){
     Sequence.AddPlayItem(&Forty);
     Sequence.AddPlayItem(&Nine);
    }
    else if (m==50){
     Sequence.AddPlayItem(&Fifty);
    }
    else if (m==51){
     Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&One);
    }
    else if (m==52){
     Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Two);
    }
    else if (m==53){
     Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Three);
    }
    else if (m==54){
     Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Four);
    }
    else if (m==55){
       Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Five);
    }
    else if (m==56){
       Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Six);
    }
    else if (m==57){
       Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Seven);
    }
    else if (m==58){
       Sequence.AddPlayItem(&Fifty);
     Sequence.AddPlayItem(&Eight);
    }
    else if (m==59){
      Sequence.AddPlayItem(&Fifty);
      Sequence.AddPlayItem(&Nine);
    }
    if (am_pm==0){ 
      Sequence.AddPlayItem(&Am); 
    }
    else if (am_pm==1){
      Sequence.AddPlayItem(&Pm);    
    }
}
