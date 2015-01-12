/***************************************************************************
            ARDUINO HAM RADIO WATCH v. 1.0 - IK8YFW - 2015
This is an ham radio watch with Monochrome OLEDs based on SSD1306 drivers
128x64 size display using I2C to communicate

Author:   GIUSEPPE CALLIPO - ik8yfw@libero.it
License:  The software is released under Creative Commons (CC) license.

NOTE: This software use the Adafruit SSD1606 and GFX dispay libraries to connect
the oled display. These libreries can be downloaded from Adafruit site.

All text above, must be included in any redistribution
****************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include "preferences.h"

/******************************************************************
              UNCOMMENT FOR DEBUG SERIAL
*******************************************************************/
//#define DEBUG_CLOCK
uint8_t demo = 0;
/******************************************************************
        DISPLAY OLED DEFINITION PARAMETERS
*******************************************************************/
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#define Adafruit_SSD1306_I2C_ADDRESS 0x3C  // the I2C address 0x3C (for the 128x64)

/**********************************************************************************
Tiny RTC Module functions
Realtime Clock modules with backup battery
details:   http://www.elecrow.com/wiki/index.php?title=Tiny_RTC
***********************************************************************************/
#define DS1307_I2C_ADDRESS 0x68  // the I2C address of Tiny RTC

/** WATCH VARIABLES **/
// FOR CLOCK
uint8_t ih = 0;              // Watch hour
uint8_t im = 0;              // Watch minutes
uint8_t is = 0;              // Watch seconds
uint8_t iDayWeek = 1;        // day of week
uint8_t day = 0;             // day of month
uint8_t month = 0;           // day of month
int     year = 0;            // year
int     tz = 1;              // time zone
uint8_t ih_utc= 0;           // UTC TIME
uint8_t bDST = 0;            // dst flag (also eprom storing)
uint8_t bTZ = 0;             // timezone for eeprom storing

// FOR ALARM
uint8_t ih_al0 = 6;          
uint8_t im_al0 = 10;
boolean ALM0_ON = false;

uint8_t ih_al1 = 6;
uint8_t im_al1 = 10;
boolean ALM1_ON = false;

uint8_t ih_al2 = 6;
uint8_t im_al2 = 10;
boolean ALM2_ON = false;

uint8_t ih_al3 = 6;
uint8_t im_al3 = 10;
boolean ALM3_ON = false;

uint8_t ih_al4 = 6;
uint8_t im_al4 = 10;
boolean ALM4_ON = false;

uint8_t ih_al5 = 6;
uint8_t im_al5 = 10;
boolean ALM5_ON = false;

// FOR RTC EXCHANGE DATA
byte bsecond, bminute, bhour, bdayOfWeek, bdayOfMonth, bmonth, byear;

/**************************************************
                      BUTTONS, SPEAKER & LED SETUP
**************************************************/
const uint8_t BTN_A = 3;            // the number of the pushbutton pin
const uint8_t BTN_B = 4;            // the number of the pushbutton pin
const uint8_t BTN_C = 5;            // the number of the pushbutton pin
const uint8_t BTN_D = 6;            // the number of the pushbutton pin
const uint8_t LED_A = 11;           // the number of the LED pin
const uint8_t LED_B = 12;           // the number of the LED pin
const uint8_t SPEAKER_OUT = 8;      // the number of SPEAKER pin        

uint8_t BTN_A_STATUS = 0;           // pushbutton status
uint8_t BTN_B_STATUS = 0;           // pushbutton status
uint8_t BTN_C_STATUS = 0;           // pushbutton status
uint8_t BTN_D_STATUS = 0;           // the number of the pushbutton pin

/**************************************************
            MENU - STATUS VARIABLE SUPPORT
**************************************************/
uint8_t MAIN_STATUS = 0;
uint8_t SET_STATUS = 0;
uint8_t SHOW_SAT =0;

boolean bDone_moon = false;
boolean bStop_RTC = false;
boolean SIG_ON = true;
boolean bDone = false;
uint8_t PREF_SAT=0;
boolean bPS = true;
boolean bPS_active = false;
boolean bPS_pause = false; 
boolean SIG_ON_PS = false;
uint8_t im_ps =0;
/***************************************************
                      MAIN SETUP                                     
***************************************************/
void setup(){
 
// Initialize Wire library 
 Wire.begin();
 
#ifdef DEBUG_CLOCK 
  Serial.begin(9600);
#endif  

  // initialize dispaly
  display.begin(SSD1306_SWITCHCAPVCC, Adafruit_SSD1306_I2C_ADDRESS);  

  // Clear the buffer.
  display.clearDisplay();
  
  // Splash screen
  display.setCursor(0,0); clearCursorY();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("");
  display.println("Open Ham Watch. v.1.0 ");
  display.println("    IK8YFW - 2015 ");
  display.display(); delay(500); 
  display.println("");
  display.println("    booting ... "); display.display();
  delay(1000);
  
  // Initialize IN/OUT
  // initialize the LED pin as an output:
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);  
  
  // initialize the pushbutton pin as an input:
  pinMode(BTN_A, INPUT);  
  pinMode(BTN_B, INPUT);  
  pinMode(BTN_C, INPUT);  
  pinMode(BTN_D, INPUT);  

  // read eeprom preset
  bDST = EEPROM.read(0);
  bTZ = EEPROM.read(1); tz = (bTZ-128);
  

  // Test controls
  play_signal();
  blinkLED_A();
  blinkLED_B();
}

/***************************************************
                      MAIN LOOP DEMO                                    
***************************************************/
void loop() {
  
  //get the time data from tiny RTC
  if (bStop_RTC == false){ getDateDs1307(); check_alarm(); }
  if (bPS == true) checkPS();
  if (bPS_active == true) { delay(3000); return;}

  // Read buttons
  readButtonsStatus();
  
  // Print clock 
  if (MAIN_STATUS == 0){
      display_main_clock();
  }
  
  // Print ALARM screen
  if (MAIN_STATUS == 1){
      display_alarm();
  }
  
  // Print Moon screen
  if (MAIN_STATUS == 2){
    if (bDone_moon==false){
      uint8_t fase = moon_phase(year, month , day);
      display_moon(fase);
    }
    bDone_moon = true;
  }
  
  if (MAIN_STATUS == 3){
    display_setup();
  }
  
  if (SHOW_SAT > 0){
      sat_tracker(); 
  }
  
  // Wait a bit
  delay(500);
    
}

void readButtonsStatus(){
  
  BTN_A_STATUS = digitalRead(BTN_A);
  if (BTN_A_STATUS == HIGH) { 
      if (SET_STATUS >0 && MAIN_STATUS==0) // On exit from set status, save rtc
      {
        setDateDs1307();
        delay(1000);
        bStop_RTC = false;
      }
      else{
        MAIN_STATUS ++;  // or normal transition
      }
      SET_STATUS = 0;
  }

  BTN_B_STATUS = digitalRead(BTN_B);
  if (BTN_B_STATUS == HIGH) { 
    if (MAIN_STATUS == 0 || MAIN_STATUS == 1 || MAIN_STATUS == 3){  // only set for 3 status
       SET_STATUS ++;
    }
  }
  
  BTN_C_STATUS = digitalRead(BTN_C);
  if (BTN_C_STATUS == HIGH) {
       // CLOCK
       if (MAIN_STATUS==0) {
           if (SET_STATUS==0){
             SHOW_SAT=1;
           } 
           if (SET_STATUS==1){
             ih_utc++;if (ih_utc>23) ih_utc = 0;
             ih = getTimeByZone(ih_utc, bDST, tz);
           }
           if (SET_STATUS==2){
             im++;if (im>59) im = 0;
           }
           if (SET_STATUS==3){
             iDayWeek = (iDayWeek+1)%7;
           }
           if (SET_STATUS==4){
             day++;if (day>31) day = 0;
           }
           if (SET_STATUS==5){
             month++;if (month>12) month = 1;
           }
           if (SET_STATUS==6){
             year++; if (year>2037) year = 2014;
           }
       }
       // ALARM
       if (MAIN_STATUS==1) {
           if (SET_STATUS==1){
             ih_al0++;if (ih_al0>23) ih_al0 = 0; 
           }
           if (SET_STATUS==2){
             im_al0++;if (im_al0>59) im_al0 = 0;
           }
           if (SET_STATUS==3){
             if (ALM0_ON == false) ALM0_ON = true; else ALM0_ON = false;  
           }
           if (SET_STATUS==4){
             ih_al1++;if (ih_al1>23) ih_al1 = 0;
           }
           if (SET_STATUS==5){
             im_al1++;if (im_al1>59) im_al1 = 0;
           }
           if (SET_STATUS==6){
             if (ALM1_ON == false) ALM1_ON = true; else ALM1_ON = false;
           }
           if (SET_STATUS==7){
             if (SIG_ON == false) SIG_ON = true; else SIG_ON = false;
           }
       }
       // SETUP
       if (MAIN_STATUS==3) {
           if (SET_STATUS==1){
             if (PREF_SAT==0) PREF_SAT=1; else PREF_SAT=0; 
           }
           if (SET_STATUS==2){
             if (bDST==0) bDST=1; else bDST=0;
             EEPROM.write(0,bDST); 
           }
           if (SET_STATUS==3){
             tz++;if (tz>12) tz = -11;
             bTZ = tz + 128; EEPROM.write(1,bTZ);
           }
           if (SET_STATUS==4){
             if (bPS==false) bPS=true; else bPS=false; 
           }
       }

  }

  BTN_D_STATUS = digitalRead(BTN_D);
  if (BTN_D_STATUS == HIGH) {
       if (MAIN_STATUS==0) {
           if (SET_STATUS==0){
             SHOW_SAT=2;
           }
           if (SET_STATUS==1){
             //ih--; if (ih<=0) ih = 23;
             if(ih_utc>0) {ih_utc--;} else { ih_utc=23; }
             ih = getTimeByZone(ih_utc, bDST, tz);
           }
           if (SET_STATUS==2){
             //im--; if (im<=0) im = 59;
             if(im>0) {im--;} else { im=59; }
           }
           if (SET_STATUS==3){
             if (iDayWeek >0) {iDayWeek--;} else { iDayWeek = 6;}
           }
           if (SET_STATUS==4){
             //day--; if (day<=0) im = 31;
             if(day>0) {day--;} else { day=31; }
           }
           if (SET_STATUS==5){
             //month--; if (month<=0) im = 12;
             if(month>0) {month--;} else { month=12; }
           }
           if (SET_STATUS==6){
             year--; if (year<=2014) year = 2014;
           }
       }
       // SETUP
       if (MAIN_STATUS==3) {
           if (SET_STATUS==3){
             tz--;if (tz<-11) tz = 12;
             bTZ = tz + 128; EEPROM.write(1,bTZ);
           }
       }
  } 
  
  // Decode buttons
   if (MAIN_STATUS == 0 && SET_STATUS >0) bStop_RTC = true; 

   if (MAIN_STATUS >= 3) {bDone_moon = false; } 
   
   if (MAIN_STATUS > 4) {MAIN_STATUS =0;} 
 
}

/*********************************************************************************************************/
//
//
/*********************************************************************************************************/
void check_alarm(){
  // User Alarm
  if (ALM0_ON == true){
     if((ih == ih_al0) && (im==im_al0))
     {
        play_melody();
        ALM0_ON = false;       
     } 
  }

  if (ALM1_ON == true){
     if((ih == ih_al1) && (im==im_al1))
     {
        play_melody();
        ALM1_ON = false;       
     } 
  }
  // Sat Alarm
  if (ALM2_ON == true){
     if((ih == ih_al2) && (im==im_al2))
     {
        play_alarm_sat();
        ALM2_ON = false;       
     } 
  }
 
  if (ALM3_ON == true){
     if((ih == ih_al3) && (im==im_al3))
     {
        play_alarm_sat();
        ALM3_ON = false;       
     } 
  }
 
  if (ALM4_ON == true){
     if((ih == ih_al4) && (im==im_al4))
     {
        play_alarm_sat();
        ALM4_ON = false;       
     } 
  }
 
  if (ALM5_ON == true){
     if((ih == ih_al5) && (im==im_al5))
     {
        play_alarm_sat();
        ALM5_ON = false;       
     } 
  }
 
  if (SIG_ON == true){
     if(im==0 && !bDone)
     {
        play_signal();
        bDone = true;  // Done only one beep     
     }
     if (im>0)
     {
       bDone = false;
     }
     
  }  
     
}


/*********************************************************************************************************/
//*
//*    DAY OF YEAR
//*
/*********************************************************************************************************/

static int MONTH_DAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*- Leap YEAR */
boolean isLeapYear( int y )
{
   return !(y%4) && (!(y%400) || (y%100));
}
                     
int daysInMonth( int m, int y )
{
   int num = MONTH_DAYS[m];
   if( m == 1 && isLeapYear( y ) ) num++;
   return num;
}

int dayNumber( int m, int d, int y )
{
  int idays = d;
  for (int i=0; i< m-1; i++){
    idays += daysInMonth( i, y );
  }
  return idays;
}

/********************************************************************************************************/
//* Function:  SAT TRACKER
//*
//* Description: Track orbital satellites flight 
/********************************************************************************************************/
void sat_tracker(){
/***************************************************
  ISS - ORBITAL ELEMENTS  - REFERENCE DATE: 12.02.2014                                     
***************************************************/
/*- Orbital Elements */
double t=0; //epoch
double i=0;// incl
double r=0;// raan
double e=0;// eccy
double w=0;//argp
double u=0;//ma
double m=0;//mm
double g=0;//decy
double o=0;//revn

if (PREF_SAT==0){
  t=t1; //epoch
  i=i1;// incl
  r=r1;// raan
  e=e1;// eccy
  w=w1;//argp
  u=u1;//ma
  m=m1;//mm
  g=g1;//decy
  o=o1;//revn

}
if (PREF_SAT==1){
  t=t2; //epoch
  i=i2;// incl
  r=r2;// raan
  e=e2;// eccy
  w=w2;//argp
  u=u2;//ma
  m=m2;//mm
  g=g2;//decy
  o=o2;//revn
}

/*- QTH */
double l=l_qth;//latitude
double n=n_qth;//longitude  

/*- Calculus Variables */
const static  double p=6.283185308;
double a,d,ys,yi,dy,h,f,q,b,s,c,k,j,x,y,z,op,di,qi,sq,sr,out1; 
double v=360/p;
//boolean bFirst = true;  
uint8_t iRow = 0;
int iY = 0;
boolean bDone = true;
double qiMax =-100;
double outMax =0;
double hMax=0;


/*- Reset Display */
display.clearDisplay();
  
// Prima Linea
display.setCursor(0,0); clearCursorY();
display.setTextSize(1);
display.setTextColor(WHITE);
//display.println(" ORBITS - Now to 24h");
display.print("SAT ");
display.print(PREF_SAT);
display.println(" - 24h Pass");
CR_LF(1);iY = getCursorY();display.setCursor(0,iY +2);

if (SHOW_SAT == 2){
  display.println(" UTC  |  AZ  |  EL");CR_LF(1);
}
else{
  display.println(" UTC  | MAX EL");CR_LF(1);
  resetSatAlarm();
}

iY = getCursorY();display.setCursor(0,iY +3);

display.display();

/*- Julian Date */
//ys=1*60;
ys=(ih-1)*60;
yi=23*60 + 59;
dy = (double)dayNumber( month, day, year );

/*- crunch number ... */
i=i/v;r=r/v;w=w/v;u=u/v;m=m*p;g=g*p;
l=l/v;n=n/v;
a=42220/(pow(m/p,2.0/3.0));
double sl=sin(l);double cl=cos(l);double sn=sin(n);
double cn=cos(n);double si=sin(i);double ci=cos(i);

/*- Seek each 5 minute */
double step_delta=5;

/*- Seek each 1 minute */
//if (SHOW_SAT == 1) step_delta =1;

for(double h=ys;h<=yi;h=h+step_delta){
    
    l=dy-t+h/1440;
    d=l/6*g/m;
    f=u+m*l*(1+3*d);
    n=a*(1-4*d);
    b= 66063.17*m/ pow(a*(1-e*e),2) *(1+7*d);  
    q=f; 
    
    /*- Solve main equation */
    do{
      s=sin(q);
      c=cos(q);
      d=(q-e*s-f)/(1-e*c);
      q=q-d;
      
      /*- chance to exit ...*/
      BTN_A_STATUS = digitalRead(BTN_A);
       if (BTN_A_STATUS == HIGH) { goto exit_cycle; }
       
    }
    while (fabs(d) > 1.0E-4);   

    j=n*(c-e); 
    k=n*s*sqrt(1-e*e);
    q=w+l*b*(5*ci*ci-1)/2;
    s=sin(q);c=cos(q);x=j*c-k*s;
    y=j*s+k*c;k=ci*y;
    q=-1.72541+r-6.300388*(l+t)-ci*l*b;
    s=sin(q);c=cos(q);
    z=y*si-6342.7384*sl;
    y=x*s+k*c-6385.4853*cl*sn;
    x=x*c-k*s-6385.4853*cl*cn;q=x*sn-y*cn;
    n=x*sl*cn+y*sl*sn-z*cl;
    d=v*atan(q/n); 
    if (n>0) {d =d-180;};
    if ((360 - abs(d))<0) {d=d-360;} 
    q=v*atan((x*cl*cn+y*cl*sn+z*sl)/sqrt(q*q+n*n));
    
    /*- Select only pass */
    if (q<-10){ 
              qiMax =-100; 
              outMax =0;
              hMax=0;
              bDone=false; 
              goto skip; 
            }

     op=(int)(o+int(f/p)); 
     di=abs(d+0.5);
     qi=int(q+0.5);
     sq=sqrt(x*x+y*y+z*z);sr=int(sq+0.5);
     out1= int(h/60)*0.4+(h/60)*0.6;

    /*- Print data !!! ... */
    /*- SHOW Small Details, just hour and max elevation for each pass. */
    if (SHOW_SAT == 1 && bDone==false){ 
    
       if (qi > qiMax) { 
                         qiMax= qi;
                         outMax =out1;
                         hMax=h;
                       }
       else
       { 
           iY = getCursorY();
           display.setCursor(0,iY +3);
   
           display.print(" ");     display.print(outMax); 
           display.print(" ");     display.println(qiMax);CR_LF(1);

           uint8_t myHour=int(hMax/60);
           int minutes = int((outMax-myHour) *100);
           uint8_t ihLocal= getTimeByZone(myHour, bDST, tz);
           addSatAlarm (ihLocal, minutes);
          
           display.display();
           iRow ++;
           if (iRow == 4){ goto exit_cycle;}  
    
           bDone=true;
       }
     }
     
      /*- SHOW FULL DETAILS SHEET: hour, azimuth and elevation. Also available Range, but not print it. */
     if (SHOW_SAT == 2)
     {
      iY = getCursorY();
      display.setCursor(0,iY +3);

      display.print(" ");     display.print(out1); // Hour
      display.print(" ");     display.print(di); // Az.
      display.print(" ");     display.println(qi); // Elev. 
      //display.print(" ");     display.println(sr); // Range
      CR_LF(1);
      display.display();
    
      /*- Pagination. Go to next Page ... */
      iRow ++;
      if (iRow == 4){ delay(5000);iRow =0;  
            display.clearDisplay();
            display.setCursor(0,0);
            clearCursorY();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.println("continue..."); CR_LF(1);
            display.println(""); CR_LF(1);
            display.display();

      }
     }
    
    skip:; 

      /*- chance to exit ...*/
      BTN_A_STATUS = digitalRead(BTN_A);
       if (BTN_A_STATUS == HIGH) { goto exit_cycle; }
  } 
  
  /*- Go to next status */
  exit_cycle:;

  display.println("done."); CR_LF(1);
  display.display();

  // Wait some seconds to display data.
  delay(6000);
  
  SHOW_SAT = 0; MAIN_STATUS = 0;

}

/********************************************************************************************************/
//* Function:  ALARM DISPLAY
//*
//* Description: Display the alarm infos
/********************************************************************************************************/
void display_alarm(){
 
  display.clearDisplay();

//****************************************************************************************************  
//****************************************************************************************************  
{
  // First Alarm
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("ALM1");
  
  display.setCursor(19,5);
  display.setTextSize(2);

  char sih_al[]="00";
  sprintf(sih_al,"%.2d",ih_al0);
  char sim_al[]="00";
  sprintf(sim_al,"%.2d",im_al0);
  display.print(" ");

  if (SET_STATUS == 1) display.print(">");
  display.print(sih_al);
  display.print(F(":"));
  
  if (SET_STATUS == 2) display.print(">");
  display.print(sim_al);
  
  display.setCursor(92,8);
  display.setTextSize(1);
  display.print("  ");
  
  if (SET_STATUS == 3) display.print(">");
  if (ALM0_ON == true)  display.println(F("ON")); else display.println(F("OFF"));

  display.drawLine(0, 21, display.width(), 21, WHITE);
}
//****************************************************************************************************
//****************************************************************************************************
{
  // Second Alarm
  display.setCursor(0,26);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("ALM2");
  
  display.setCursor(19,31);
  display.setTextSize(2);

  char sih_al[]="00";
  sprintf(sih_al,"%.2d",ih_al1);
  char sim_al[]="00";
  sprintf(sim_al,"%.2d",im_al1);
  display.print(" ");
  
  if (SET_STATUS == 4) display.print(F(">"));
  display.print(sih_al);
  display.print(F(":"));
  
  if (SET_STATUS == 5) display.print(">");
  display.print(sim_al);
  
  display.setCursor(92,35);
  display.setTextSize(1);
  display.print("  ");
  
  if (SET_STATUS == 6) display.print(">");
  if (ALM1_ON == true)  display.println(F("ON")); else display.println(F("OFF"));

  display.drawLine(0, 50, display.width(), 50, WHITE);
}
//****************************************************************************************************  
//****************************************************************************************************  

  // Terza Linea
//  display.drawLine(0, 40, display.width(), 40, WHITE);
  display.drawLine((display.width()/2), 50, (display.width()/2), 64, WHITE);
  display.setCursor(display.width()/2+8,56);
  display.setTextSize(1);

  char sih[]="00";
  sprintf(sih,"%.2d",ih);
  char sim[]="00";
  sprintf(sim,"%.2d",im);
  char sis[]="00";
  sprintf(sis,"%.2d",is);
  display.print(sih);
  display.print(":");
  display.print(sim);
  display.print(".");
  display.print(sis);

// SIGNAL
  display.setCursor(0,56);
  display.print(F("SIG:  "));

  if (SET_STATUS == 7) display.print(">");
  if (SIG_ON == true)  display.println(F("ON")); else display.println(F("OFF"));

  display.display();
}

/********************************************************************************************************/
//* Function:  ALARM DISPLAY
//*
//* Description: Display the alarm infos
/********************************************************************************************************/
void display_setup(){
 
  display.clearDisplay();

  // First Alarm
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("     SETUP SCREEN");
  display.println(""); 

  display.print("SATELLITE: ");
  if (SET_STATUS == 1) display.print(">");
  display.println(PREF_SAT);
  int iY = display.getCursorY();
  display.setCursor(0,iY +3);
  
  display.print("DST: ");
  if (SET_STATUS == 2) display.print(">");
  if (bDST == 1)  display.println(F("ON")); else display.println(F("OFF"));
  iY = display.getCursorY();
  display.setCursor(0,iY +3);
  
  display.print("TIMEZONE: ");
  if (SET_STATUS == 3) display.print(">");
  display.println(tz);
  iY = display.getCursorY();
  display.setCursor(0,iY +3);
  
  display.print("POWER SAFE: ");
  if (SET_STATUS == 4) display.print(">");
  if (bPS == true)  display.println(F("ON")); else display.println(F("OFF"));
  
  display.display();
}


/********************************************************************************************************/
//* Function:  Set and reset ALARM SAT
//*
//* Description: The function get Local time from utc, TZ and DST 
/********************************************************************************************************/

void resetSatAlarm(){
  ALM2_ON = false;
  ALM3_ON = false;
  ALM4_ON = false;
  ALM5_ON = false;
}

void addSatAlarm(uint8_t h, uint8_t m){
  if (ALM2_ON == false) {ih_al2 = h; im_al2=m; ALM2_ON = true; return; }
  if (ALM3_ON == false) {ih_al3 = h; im_al3=m; ALM3_ON = true; return; }
  if (ALM4_ON == false) {ih_al4 = h; im_al4=m; ALM4_ON = true; return; }
  if (ALM5_ON == false) {ih_al5 = h; im_al5=m; ALM5_ON = true; return; }
}


/********************************************************************************************************/
//* Function:  GET TIME BY ZONE
//*
//* Description: The function get Local time from utc, TZ and DST 
/********************************************************************************************************/
uint8_t getTimeByZone(uint8_t local, uint8_t bDST, int mytz) {
     	        uint8_t ih_gmt = 0;
                if (bDST == 1) mytz ++;
                uint8_t iTemp = (local+mytz)%24;
		if ( iTemp >0) ih_gmt = iTemp;
		   else  
		ih_gmt = 24-abs(iTemp);
		if (ih_gmt == 24) ih_gmt = 0;
        return ih_gmt;
}

/********************************************************************************************************/
//* Function:  MAIN CLOCK DISPLAY
//*
//* Description: Display the main clock infos
/********************************************************************************************************/
void display_main_clock(){
  // Day week
  String dayWeeks [] ={"SU","MO","TU","WE","TH","FR","SA" }; 
  
  //**************************************
  char sih[4]="00";
  sprintf(sih,"%.2d:",ih);
  
  char sih_gmt[4]="00";
  sprintf(sih_gmt,"%.2d:",ih_utc);

  char sim[3]="00";
  sprintf(sim,"%.2d",im);

  char sis[4]="00";
  sprintf(sis," %.2d",is);  
  //**************************************
  
  display.clearDisplay();
  
  // First Line
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  //display.println("  [AL] - SIG - [PS]");
  display.print(F(" UTC "));display.print(sih_gmt);display.print(sim);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(F("    ")); 
  if ((ALM0_ON == true)||(ALM1_ON == true)) {display.print(F("ALM "));}//else {display.print(F("    "));} 
  if (SIG_ON == true) display.print(F("SIG"));
  display.drawLine(0, 13, display.width(), 13, WHITE);
  display.drawLine(display.width()-1, 0, display.width()-1, 13, WHITE);
  display.drawLine(display.width()/2, 0, display.width()/2, 13, WHITE);
  
  // Second Line
  display.setCursor(5,28);
  display.setTextSize(1);
  if (bDST == 1) display.print(F("DST"));

  display.setCursor(19,23);
  display.setTextSize(2);
  display.print(" ");

  if (SET_STATUS == 1) display.print(">");
  display.print(sih);
  
  if (SET_STATUS == 2) display.print(">");
  display.print(sim);

  if ((SET_STATUS != 2) && (SET_STATUS != 1)) {
    display.setCursor(92,29);
    display.setTextSize(1);
    display.println(sis);
  }
  
  // Terza Linea
  display.drawLine(0, 49, display.width(), 49, WHITE);
  display.setCursor(0,56);
  display.setTextSize(1);

  if (SET_STATUS == 3) display.print(">");
  display.print(dayWeeks[iDayWeek]);
  display.print(" ");
  if (SET_STATUS == 4) display.print(">");
  display.print(day);
  display.print("-");
  if (SET_STATUS == 5) display.print(">");
  display.print(month);
  display.print("-");
  if (SET_STATUS == 6) display.print(">");
  display.println(year);
  
  display.drawLine(display.width()-1, 49, display.width()-1, 64, WHITE);
  display.drawLine((display.width()/3)*2-4, 49, (display.width()/3)*2-4, 64, WHITE);

  display.setCursor(((display.width()/3)*2) + 2,56);
  display.setTextSize(1);
  if (bPS == true) display.print(F("PS "));
  if( ALM2_ON || ALM3_ON || ALM4_ON || ALM5_ON) display.print(F("SAT")); 

  display.display();
}

/********************************************************************************************************/
//* Function:  MOON PHASE CALCULUS
//*
//* Description:
//*      MOON PHASE Calculus and DISPLAY, working from 1900 to 2199 inclusive
//*      Calculates the moon phase (0-7), accurate to 1 segment.
//*      0 = > new moon.
//*      4 => Full moon.
/********************************************************************************************************/
int moon_phase(int year_m,int month_m,int day_m)
{
    int g, e;
    int dy = dayNumber( month_m, day_m, year_m );
    
    g = (year_m-1900)%19 + 1;
    e = (11*g + 18) % 30;
    if ((e == 25 && g > 11) || e == 24) e++;
    return ((((e + dy)*6+11)%177)/22 & 7);
}

/********************************************************************************************************/
//* Function:  MOON PHASE DISPLAY
//*
//* Description: Display info of moon phase
//*             phase:  0 - 7
//*             0: new moon (dark)
//*             4: full moon
//*             1,2,3: Rise 1/4 
//*             5,6,7: Set  1/4
/********************************************************************************************************/
void display_moon(uint8_t phase){
 
  uint8_t eclipse = 0;
  uint8_t age = phase * 4;
  
  display.clearDisplay();
  
  // First Line
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(F("MOON PHASES - "));
  display.print("Age("); display.print(age);display.println(")");
  display.drawLine(0, 15, display.width(), 15, WHITE);
  
  // Second Line
  display.setCursor(5,25);
  display.setTextSize(1);
  if (phase>0 && phase<4) { display.print(F("   Moon 1/4 Rise ")); eclipse = 27 ;}
  if (phase==4) { display.print(F("      Full Moon ")); eclipse = 0;}
  if (phase==0) { display.print(F("      New Moon ")); eclipse = 19;}
  if (phase>4 && phase<=7) { display.print(F("   Moon 1/4 Set ")); eclipse = 10;}
 
  // Date Line
  display.setCursor(0,46);
  display.setTextSize(1);
  
  display.print("  ");
  display.print(day);
  display.print("-");
  display.print(month);
  display.print("-");
  display.println(year);
   
  // Animation
  for (uint8_t i= 0; i<=eclipse; i++){
   
    // draw a white circle
    display.fillCircle((display.width()/2)+35, 48, 10, WHITE);
 
    // draw a black circle for eclipse
    display.fillCircle(((display.width()/2)+35+19)-i, 48, 8, BLACK);
    delay(100);
    display.display();

  }
  
}

/************************************************************************************************************/

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
 return ( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
 return ( (val/16*10) + (val%16) );
}
// Function to set the currnt time, change the second&minute&hour to the right time
void setDateDs1307()                
{
  bsecond= is;
  bminute =im;
  bhour =ih_utc;
  bmonth =month;
  bdayOfMonth=day;
  byear = year-2000;
  bdayOfWeek = iDayWeek; 

  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(decToBcd(0));
  Wire.write(decToBcd(bsecond));    // 0 to bit 7 starts the clock
  Wire.write(decToBcd(bminute));
  Wire.write(decToBcd(bhour));      // If you want 12 hour am/pm you need to set
                                  // bit 6 (also need to change readDateDs1307)
  Wire.write(decToBcd(bdayOfWeek));
  Wire.write(decToBcd(bdayOfMonth));
  Wire.write(decToBcd(bmonth));
  Wire.write(decToBcd(byear));
  Wire.endTransmission();
}
// Function to gets the date and time from the ds1307 and prints result
void getDateDs1307()
{
 // Reset the register pointer
 Wire.beginTransmission(DS1307_I2C_ADDRESS);
 Wire.write(decToBcd(0));
 Wire.endTransmission();
 Wire.requestFrom(DS1307_I2C_ADDRESS, 7);
 bsecond     = bcdToDec(Wire.read() & 0x7f);
 bminute     = bcdToDec(Wire.read());
 bhour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
 bdayOfWeek  = bcdToDec(Wire.read());
 bdayOfMonth = bcdToDec(Wire.read());
 bmonth      = bcdToDec(Wire.read());
 byear       = bcdToDec(Wire.read());  
 
 /* copy back values */
 is=(uint8_t) bsecond;
 im=(uint8_t) bminute;
 ih_utc=(uint8_t) bhour;
 month=(uint8_t) bmonth;
 day=(uint8_t) bdayOfMonth;
 year=((int) byear + 2000);
 iDayWeek = (uint8_t) bdayOfWeek;
 
/*******************  LOCAL TIME *********************************/
  ih= getTimeByZone(ih_utc, bDST, tz);
/****************************************************************/  
 
 #ifdef DEBUG_CLOCK
   Serial.print(bhour, DEC);
   Serial.print(":");
   Serial.print(bminute, DEC);
   Serial.print(":");
   Serial.print(bsecond, DEC);
   Serial.print("  ");
   Serial.print(bmonth, DEC);
   Serial.print("/");
   Serial.print(bdayOfMonth, DEC);
   Serial.print("/");
   Serial.print(byear,DEC);
   Serial.print("  ");
   Serial.println();
 #endif
 //Serial.print("Day of week:");
}

/********************************************************************************************************/
//* Function:  PLAY SIMPLE MELODY
//*
//* Description:
//*              The function play a simple melody.
//*              Connect 8 ohm speaker on digital pin 8.
//*              Note: Source based from arduino examples.
//*
/********************************************************************************************************/
void play_melody() {
  for (int iRepeat =0; iRepeat < 500;iRepeat ++){
    play_signal();
    if (digitalRead(BTN_A) == HIGH) { break; }
    delay(100);
  }
}

void play_alarm_sat() {

  digitalWrite(LED_B, HIGH);   // turn the LED on (HIGH is the voltage level)

  int NOTE_G3 = 193;
  // Repeat tone 3 times
  for (int iRepeat =0; iRepeat < 10 ;iRepeat ++){
    tone(SPEAKER_OUT, NOTE_G3,1000/8);
    delay((1000/8)* 1.30);
    noTone(SPEAKER_OUT);
    
    tone(SPEAKER_OUT, NOTE_G3,1000/8);
    delay((1000/8)* 1.30);
    noTone(SPEAKER_OUT);

    tone(SPEAKER_OUT, NOTE_G3,1000/8);
    delay((1000/8)* 1.30);
    noTone(SPEAKER_OUT);

    delay(1500);
   }
   
  digitalWrite(LED_B, LOW);   // turn the LED off (LOW is the 0 level)

}

void play_signal() {
    tone(SPEAKER_OUT, 247,1000/4);
    delay((1000/8)* 1.30);
    noTone(SPEAKER_OUT);
}

/********************************************************************************************************/
//* Function:  CHECK PS
//*
//* Description:
//*
/********************************************************************************************************/
void checkPS(){
  
  /*- Reactive a bPS in pause */
  if (bPS_pause){
    if (im==im_ps){            
          bPS_pause = false; 
          SIG_ON =SIG_ON_PS;
    }    
    
  }else{
    /*- Default activation */
    if (ih > 22 || ih < 7) {
       if (bPS_active == false){
         display.clearDisplay();
         display.display();
         resetSatAlarm();   
         SIG_ON_PS = SIG_ON;
         SIG_ON = false;
         bPS_active = true;   
       }
    }else{
      bPS_active = false;
    }
  }
  
  /*- suspend PS */
  if (bPS_active == true){
       // Blink Led during active sattus PS
       blinkLED_A();
       
       if ((digitalRead(BTN_A) == HIGH) ||
           (digitalRead(BTN_B) == HIGH) ||
           (digitalRead(BTN_C) == HIGH) ||
           (digitalRead(BTN_D) == HIGH) 
       ) 
       { 
           bPS_active = false;
           bPS_pause = true; 
           SIG_ON =SIG_ON_PS;
           im_ps = (im + 1)%59;
           MAIN_STATUS = 0; 
      }
  }

}

/***********************************************************
 SCOPE: it is used to create a simple support to make a interline
 
 HOW IT WORK:
     FOLLOW the movement of cursor Y in DISPLAY LIBRARY
     for each println, we need to call CR_LF function and
     read the real position of cursor with getCursorY function.
***********************************************************/
uint8_t cursor_y =0;
void clearCursorY(){
  cursor_y =0;
}
uint8_t getCursorY(){
  return cursor_y;
}
void CR_LF(uint8_t textSize){
  cursor_y= cursor_y + textSize*8 + 1;
}

/***********************************************************/

/***********************************************************
                  BLINK LED A AND LED B
************************************************************/
void blinkLED_A(){
  digitalWrite(LED_A, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(250);                  // wait for a bit
  digitalWrite(LED_A, LOW);    // turn the LED off by making the voltage LOW
}

void blinkLED_B(){
  digitalWrite(LED_B, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(250);                  // wait for a bit
  digitalWrite(LED_B, LOW);    // turn the LED off by making the voltage LOW
}
