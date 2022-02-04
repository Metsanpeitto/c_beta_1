#include <DHT11.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "SoftwareSerial.h"



#define ONE_WIRE_BUS 5
#include "RTClib.h"

#define LED 13

#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection

  
const float minph = 5.5;
const float maxph = 7.0; 
int PHPin = A0; 
float Healthy1_mv = 1.96;  
float mvReading = 0;
float Vs = 5;
static float phValue = 7.2;        // dtoStrf double to string function
static char phStr[15];
int i= 0;
long reading = 0;
unsigned long sum = 0;
float average = 0;
float mvReading_7 = 2.1191406250;
float Slope = 0;
float mvReading_4 = 2.4365234375;
float offset= 0;;
 

int relay1 = 9;      // Water heater
int relay2 = 10;     //Water Pump
int relay3 = 11;     //Air Pump

int pin = 4;
DHT11 dht11(pin); 

OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//I2C pins declaration
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  
      
SoftwareSerial esp8266(3,2);         // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
                                      // This means that you need to connect the RX line from the esp to the Arduino's pin 2  

RTC_DS1307 RTC;  

int Second;
int Minute;
int Hour;      // Initialize them with the current value 
int Day;       
int Month;                            
int Year; 


//Post values -------------------------------------------------------------------------------
 
 int refHour; 
 /*
 int len;
 char inChar;
 char str[200]= ""; 
 boolean stringComplete;
 long postDelay = 60000;
 long minuteRef ;
 long lastPost ;
 */
 String tempStr;
 String humiStr;
 //String phStr;
 String waterStr;
 String distanceStr;
 String relay1Str;
 String relay2Str;
 String relay3Str;
 String relayStr[] = {relay1Str,relay2Str,relay3Str};
 String inputString ="";
 char* arr = "";      
 const String outPutStringTest =  "24,50,7,15,45,ON,ON,ON";


 //char* line = "";                     // This part receives the response line fromesp8266
 
 //Post values -------------------------------------------------------------------------------
 int postDelay = 10;
 int minuteRef ;
 int hourRef;
 int lastPost ;
 int lastPostHour;
 int len;
 char inChar;
 char str[200]= "";
 //char* arr = "";
// String inputString ="";
 boolean stringComplete;

                                                                  
 float temp,humi;                                   // Variables to store the values readed from the sensor DHT11
 int tempInt,humiInt,phValueInt,waterInt;           //  Variables in integer format,step before convert in String and send trough ESP8266
 boolean relay1State,relay2State,relay3State;
 boolean relayState[] = {relay1State,relay2State,relay3State}  ;     // Save here the current state of the relays 0 = Low ....
 
// defines  echo pins numbers
const int trigPin = 7;
const int echoPin = 6;

// defines variables
long duration;
int  distance; 


void setup() {
  
  Wire.begin();
  Serial.begin(9600);
  esp8266.begin(9600); 
  Slope_calc();

  while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    }

  delay(1000);    

 
  sensors.begin(); 
 
  lcd.begin(16,2);//Defining 16 columns and 2 rows of lcd display
  lcd.backlight();//To Power ON the back light
  //lcd.backlight();// To Power OFF the back light 
  lcd.print("      Setup");

  delay(1000);

       
  RTC.begin();
  //Si quitamos el comentario de la linea siguiente, se ajusta la hora y la fecha con la del ordenador
  // RTC.adjust(DateTime(__DATE__, __TIME__));

  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(relay3,OUTPUT);
  lcd.clear();
  lcd.print("   Relays ready");
  delay(1000);
  pinMode(trigPin,  OUTPUT);       // Sets the trigPin as an Output
  pinMode(echoPin,  INPUT);        // Sets the echoPin as an Input
  
  refHour = 43;    // Here a random hour is set. When checking the reference hour for the last post, the current hour will be always
                   // diferent than 43,then it will post the data online.
   digitalWrite(relay2,HIGH);
   digitalWrite(relay3,HIGH);
   relay2Str = "ON";
   relay3Str = "ON";
   lcd.clear();
   lcd.print("   Setup Done");
   delay(1000); 
}

void loop() { 
  
  dht();
  ds18B20();
  Time();    
  ReadPH();
  waterLevel();
  postData();
  serialEvent();
  
  }

void dht()
{
  int err; 
  if((err=dht11.read(humi, temp))==0)
  {
    Serial.print("temperature:");
    Serial.print(temp);
    Serial.print(" humidity:");
    Serial.print(humi);
    Serial.println();

    lcd.clear();//Clean the screen
    lcd.setCursor(0,0); 
    lcd.print("   Temperature ");
    lcd.setCursor(5,2);
    lcd.print(temp);
    delay(2000); 

    lcd.clear();//Clean the screen
    lcd.setCursor(0,0); 
    lcd.print("    Humidity ");
    lcd.setCursor(6,2);
    lcd.print(humi);
    delay(2000); 
   
  }
  else
  {
    lcd.clear();//Clean the screen
    lcd.setCursor(0,0); 
    lcd.print("Error No: ");
    lcd.setCursor(0,1);
    lcd.print(err);
    delay(1000); 
    Serial.println();
    Serial.print("Error No :");
    Serial.print(err);
    Serial.println();    
  }
  delay(DHT11_RETRY_DELAY); //delay for reread
}

void ds18B20(void) 
{ 
 // call sensors.requestTemperatures() to issue a global temperature 
 // request to all devices on the bus 
 /********************************************************************/
 sensors.requestTemperatures(); // Send the command to get temperature readings 

 Serial.println("DONE"); 
/********************************************************************/
 lcd.clear();//Clean the screen
 lcd.print("  Water temp:"); 
 lcd.setCursor(6,2); 
 lcd.print(sensors.getTempCByIndex(0)); 
 waterInt = (int)(sensors.getTempCByIndex(0));
 delay(2000);

   if ((waterInt <= -5)&&(waterInt >= -15)) 
     {       
      sensors.requestTemperatures();
      lcd.clear();//Clean the screen
      lcd.print("  Water temp:"); 
      lcd.setCursor(6,2); 
      lcd.print(sensors.getTempCByIndex(0)); 
      waterInt = (int)(sensors.getTempCByIndex(0));
      delay(2000);
       }
       
 if (waterInt <= 9) 
     { digitalWrite(relay1,HIGH);
       lcd.clear();
       lcd.print("Water heating up");
       relay1Str= "ON";
       }
       else
          { digitalWrite(relay1,LOW);
            lcd.clear();
            lcd.print("Water no heating");
            relay1Str= "OFF";
           } 
 if (waterInt < -15) 
     {
       waterInt = -14;
       lcd.clear();
       lcd.print("Water temp Error");
       
      }
 
 Serial.print(sensors.getTempCByIndex(0)); 
 Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"?  
   // You can have more than one DS18B20 on the same bus.  
   // 0 refers to the first IC on the wire 
   delay(2000); 
} 


void relayTest() {

  Serial.print("testing");
  lcd.clear();
  lcd.print(" Relay test");
    digitalWrite(relay1,HIGH);
    delay(1000);
    digitalWrite(relay2,HIGH);
    delay(1000);
    digitalWrite(relay3,HIGH);
    delay(1000);
    digitalWrite(relay1,LOW);
    delay(1000);
    digitalWrite(relay2,LOW);
    delay(1000);
    digitalWrite(relay3,LOW);
    delay(1000);

  }  

  
void ReadPH(){

  i = 0;
  sum = 0;
  
  while(i<=20){
                reading=analogRead(PHPin);
                sum=sum+reading;
                delay(10);
                i++;
               }

  average=sum/i; 

  //Converting to mV reading and then to pH
  mvReading=average*Vs/1024;
  //phValue=mvReading*K_PH; 
  phValue=(7-((mvReading_7-mvReading)*Slope));

 if ((phValue > 1)&&(phValue < 10)) {
   lcd.clear();
   lcd.print("PH:   ");
   lcd.print(phValue,DEC);      
   delay(2000);
  } else {
    phValue = 11.1 ;
    lcd.clear();
    lcd.print("   PH Error"); 
    lcd.setCursor(0,2);
    lcd.print(phValue);        
    delay(2000);
    }
   dtostrf (phValue,  4,  1,  phStr);
      
   Serial.print("PH String = ");
   Serial.println(phStr);   
 
}

void Slope_calc(){ 

                    offset=Healthy1_mv-mvReading_7;
                    Slope=3/(Healthy1_mv-mvReading_4-offset); 
                 } 
                 

void Time () {
  
    DateTime now = RTC.now(); 
    delay(1000);    
    
    Year = now.year();
    Month = now.month();
    Day = now.day();
    Hour = now.hour();
    Minute = now.minute();
    Second = now.second();
    lcd.clear();   
    lcd.print(" Time : ");
    lcd.print(Hour);
    lcd.print(":");  
    lcd.print(Minute);
    lcd.print(":");
    lcd.print(Second);     
    delay(2000);    
    lcd.clear();
}  


void postData() {          

    DateTime now = RTC.now(); 
    delay(1000);    
    
    Year = now.year();
    Month = now.month();
    Day = now.day();
    Hour = now.hour();
    Minute = now.minute();
    Second = now.second();
    
    if (refHour != Hour)
      { 
        String outPutString = "";
        String outPutAlarmString = "";
        const char* come = ","; 

        //hValueInt = (int)phValue;
     
        tempInt = (int)temp;
        humiInt = (int)humi; 
               
        //phStr = (String)phValueInt;
        //phStr = "";
        //phStr += (String)(phValue, 1); 
        // phStr =  (String)(phValue,1);
        tempStr  = (String)tempInt;
        humiStr  = (String)humiInt;                
        waterStr = (String)waterInt;   
        distanceStr = (String)distance;
     
        lcd.clear();
        lcd.print("   refHour:");
        lcd.print(refHour);
        lcd.setCursor(3,2);
        lcd.print("Hour:");
        lcd.print(Hour);
        delay(1500); 
        
        lcd.clear();   
        lcd.print("  Parsing data");        
        delay(1000);
        lcd.clear();  

          

        lcd.print("Values: ");    
        lcd.setCursor(0,2);
        outPutString += tempStr;
        lcd.print(tempStr);
        lcd.print(",");
        delay(1000);
        outPutString += come;
        outPutString += humiStr;
        lcd.print(humiStr);
        lcd.print(",");
        delay(1000);
        outPutString += come;
        outPutString += phStr;
        lcd.print(phStr);
        lcd.print(",");
        delay(1000);
        outPutString += come;
        outPutString += waterStr;
        lcd.print(waterStr); 
        lcd.print(",");       
        delay(1000);
        outPutString += come;
        outPutString += distanceStr;
        lcd.print(distanceStr); 
        lcd.print(",");               
        delay(1000);
        outPutString += come;
        outPutString += relay1Str;
        //lcd.print(relay1Str);         
        lcd.print(",");              
        delay(1000);
        outPutString += come;
        outPutString += relay2Str;
        //lcd.print(relay2Str);        
        lcd.print(",");               
        delay(1000);
        outPutString += come;
        outPutString += relay3Str;
        //lcd.print(relay3Str);               
        
        delay(1000);
         
        lcd.clear();
         
        lcd.print(outPutString);
        delay(1000); 
        //outPutString = outPutStringTest;
        esp8266.println(outPutString);

        delay(1000);
        lcd.clear(); 
        
        refHour = Hour;     
        lcd.print("   ESP print"); 
        delay(2000);
        lcd.clear();      
        while(esp8266.available()) lcd.print(esp8266.read());
        delay(2000);
       /* String line;
           while(esp8266.available())
           { 
               line += esp8266.read();
              delay(1000);
              Serial.print(line);                
            }  */
            // readWifi();       
  //     line = "";
       outPutString = "";   
                           
       }
 }
 

void serialEvent() { 
   lcd.clear();
   lcd.print("Serial event");     
    // get the new byte:
    inChar = (char)esp8266.read();
    lcd.println("inChar :");
    lcd.setCursor(0,2);
    lcd.print(inChar);
    delay(1000);
   // Serial.print(inChar);                                    // add it to the inputString:    
    inputString += inChar;
   // Serial.println(inputString);     

    inputString.toCharArray(arr, len) ;
  //Serial.println("arr ready");                               // if the incoming character is a newline, set a flag
    if (inChar == '\n') {                                      // so the main loop can do something about it:  
    stringComplete = true;    
    delay(1000);  
    lcd.clear();
    lcd.print("Done !");      
    lcd.setCursor(0,2);
    lcd.println(arr);  
    delay(1000);
    lcd.print("this was arr");
    delay(1000);  

       inChar = 0;
       inputString = "";
   }   
}  

  void waterLevel() {               //test it with the final water repository    
      
                     digitalWrite(trigPin, LOW);
                     delayMicroseconds(2);
                     digitalWrite(trigPin, HIGH);
                     delayMicroseconds(10); 
                     digitalWrite(trigPin, LOW);
                     duration = pulseIn(echoPin, HIGH); 
                     distance= duration*0.034/2;
                     lcd.clear();
                     lcd.setCursor(0,0);               // Sets the location at which subsequent text written to the LCD will be displayed
                     lcd.print("  Distance: ");          // Prints string "Distance" on the LCD
                     lcd.setCursor(7,2);
                     lcd.print(distance);              // Prints the distance value from the sensor
                     lcd.print(" cm");
                     delay(2000);

                     if (distance < 50) 
                                 { digitalWrite(relay2,HIGH);
                                   lcd.clear();
                                   lcd.print("Pump Working");
                                   relay2Str= "ON";
                                   }
                                    else
                                        { digitalWrite(relay2,LOW);
                                          lcd.clear();
                                          lcd.print("Pump Stopped");
                                          relay2Str= "OFF";
                                         }
}



