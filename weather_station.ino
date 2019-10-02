



//---------------------------------------------------------------------------------------------------------------
//                                                  LIBRARIES
//---------------------------------------------------------------------------------------------------------------

#include <ESP8266WiFi.h>
#include<Wire.h>
#include <DHT.h>
#include <DHT_U.h>

//---------------------------------------------------------------------------------------------------------------
//                                                   DEFINES
//---------------------------------------------------------------------------------------------------------------

#define DHTTYPE DHT11                          // DHT 11
#define anInput     A0                        //analog feed from MQ135
#define digTrigger  2                        //digital feed from MQ135
#define co2Zero     140                     //calibrated CO2 0 level
#define led         9                      //led on pin 9
#define DHTPin      D7

#define Sensor1 D0
#define Sensor2 D1
#define Sensor3 D2
#define Sensor4 D6

//#define ssid "CIRKITREE"
//#define pass "8800379163"

//#define ssid "Redmi"
#define ssid "Deepanshu"
#define pass "A3E2E0E2"

#define server "api.thingspeak.com"

WiFiClient clt;
DHT dht(DHTPin, DHTTYPE);                

String apikey = "OILDMO9BT3UG7HQ0";
String quality;

float t;
float h;
int encoder=D5;
float rps;
float rpm,rpm1=0;
volatile byte pulses;
unsigned long timeold;
float s;
int r[5],q=0;
int dir;
    
int co2now[10];                               //int array for co2 readings
int co2raw = 0;                               //int for raw value of co2
int co2comp = 0;                              //int for compensated co2 
int co2ppm = 0;                               //int for calculated ppm
int zzz = 0;                                  //int for averaging
int grafX = 0; 

int S1 = 0;
int S2 = 0;
int S3 = 0;
int S4 = 0;

//---------------------------------------------------------------------------------------------------------------
//                                                   Functions
//---------------------------------------------------------------------------------------------------------------

void CalcCO2();
void CalcTemp();
void send_to_thingspeak();  
void readSensor();
void windDirection();
void counter();

//---------------------------------------------------------------------------------------------------------------
//                                                   Setup
//---------------------------------------------------------------------------------------------------------------
   
void setup() {
  
  Wire.begin(D2,D1);
  
  pinMode(anInput,INPUT);                     //MQ135 analog feed set for input
  pinMode(Sensor1, INPUT);
  pinMode(Sensor2, INPUT);
  pinMode(Sensor3, INPUT);
  pinMode(Sensor4, INPUT);
  
    pulses=0;
    rps=0;
    rpm=0;
    timeold=0;
    s=0;    
  
  Serial.begin(115200);                         //serial comms for debuging
  
  dht.begin();  
  WiFi.begin(ssid,pass);
  
  Serial.print("Connecting to Wifi ");
  Serial.print(ssid);
  
  delay(100);
  
  while(WiFi.status() !=WL_CONNECTED)

{
 Serial.println(".");
 delay(100); 
}

Serial.println("Connected to Wifi");  
if(clt.connect(server,80))  
Serial.println("connected to server");

delay(100);
}

//---------------------------------------------------------------------------------------------------------------
//                                                   LOOP
//---------------------------------------------------------------------------------------------------------------

void loop() 
{
  rpm=0;
//  lc.print("Waiting ...");
  if (clt.connect(server,80))
   {
  readSensor();
  CalcCO2();
  CalcTemp();
  
  //detachInterrupt(0);
  speed_calc();
  detachInterrupt(0);
    
  Serial.print(rpm);
  Serial.println("rpm ");
   
  send_to_thingspeak();
   }
  }
//---------------------------------------------------------------------------------------------------------------
//                                                   Thingspeak
//---------------------------------------------------------------------------------------------------------------

void send_to_thingspeak()
{
  String str;
  
str = apikey;

str +="&field1=";
str += String(dir);
str +="&field2=";
str += String(t) ;
str +="&field3=";
str += String(h) ;
str +="&field4=";
str += String(co2ppm) ;
str +="&field5=";
str += String(rpm) ;

str+="\r\n\r\n";
     clt.print("POST /update HTTP/1.1\n");
     clt.print("Host: api.thingspeak.com\n");
     clt.print("Connection: close\n");
     clt.print("X-THINGSPEAKAPIKEY: "+apikey+"\n");
     clt.print("Content-Type: application/x-www-form-urlencoded\n");
     clt.print("Content-Length: ");
     clt.print(str.length());
     clt.print("\n\n");  // the 2 carriage returns indicate closing of Header fields & starting of data
     
clt.print(str);
Serial.println("Waiting");
delay(27000);
}  

//---------------------------------------------------------------------------------------------------------------
//                                                   CO2 Calculation
//---------------------------------------------------------------------------------------------------------------
/*
 * Atmospheric CO2 Level..............400ppm
 * Average indoor co2.............350-450ppm
 * Maxiumum acceptable co2...........1000ppm
 * Dangerous co2 levels.............>2000ppm
 */

void CalcCO2()
  
  {   
 co2now[10];                               
 co2raw = 0;                               
 co2comp = 0;                               
 co2ppm = 0;                               
 zzz = 0;                                  
 grafX = 0;                                


 for (int x = 0;x<10;x++){                   
    co2now[x]=analogRead(A0);
    delay(200);
  }

for (int x = 0;x<10;x++){                    
    zzz=zzz + co2now[x];
    
  }
  co2raw = zzz/10;                           
  co2comp = co2raw - co2Zero;                
  co2ppm = map(co2comp,0,1023,350,1500);     
  if(co2ppm<=400)
  quality=("Good");
  
  else if(co2ppm>400&& co2ppm<550)
  quality=("Avg");
  
  else if (co2ppm>550 && co2ppm<1000)
  quality=("Poor");
  
  else
  quality=("Dangerous");
  
  Serial.print("CO2 Level :");               
  Serial.print(" ");                       
  Serial.print(co2ppm);                      
  Serial.print(" PPM    ");                     
  }

//---------------------------------------------------------------------------------------------------------------
//                                                   Temperature Sensor
//---------------------------------------------------------------------------------------------------------------
  void CalcTemp()
  {
    t = dht.readTemperature(); // Gets the values of the temperature
    h = dht.readHumidity(); // Gets the values of the humidity 
    if (isnan(h) || isnan(t)) 

                 {

                     Serial.println("Failed to read from DHT sensor!");

                      return;

                 }
  Serial.print("Temperature: ");              
  Serial.print(t);
  Serial.print(" C   ");
  Serial.print("Humidity: ");
  Serial.print("  ");
  Serial.print(h);
  Serial.println(" %");
}

//---------------------------------------------------------------------------------------------------------------
//                                                   Read IR Sensors
//---------------------------------------------------------------------------------------------------------------


void readSensor()
{
  Serial.println("Determining Direction");
  Serial.print("Direction: ");
  S1 = digitalRead(Sensor1);
  S2 = digitalRead(Sensor2);
  S3 = digitalRead(Sensor3);
  S4 = digitalRead(Sensor4);
  int j =0;
 
 
  if (S1 == 1 && S2==0 && S4==0 && S3==0)
 
  {
    dir=0;
    Serial.println("North  ");
   j=1; 
  }

  else if (S2 == 1 &&S1==0 &&S3==0 && S4==0)
  {
     dir=90;
   
    Serial.println("East");
   j=1;
  }
  else if (S3 == 1 && S2==0 && S4==0&& S1==0)
  {
     dir=180;
   
    Serial.println("South ");
   j=1;
  }
  else if (S4 == 1 && S1==0 && S3==0 && S2==0)
  {
     dir=270;
   
    Serial.println("West");
   j=1;
  }
  
  else if (S4 == 1 && S3 == 1 && S1==0 &&S2==0)
  {
     dir=225;
   
    Serial.println("South West");
   j=1;
  }
  else if (S4 == 1 && S1 == 1 && S3==0 &&S2==0)
  {
     dir=315;
  
    Serial.println("North West");
   j=1;
  }
  
  else if (S4 == 0 && S1 == 0 &&S2 == 1 && S3==1)
  {
     dir=135;
   
    Serial.println("South East");
   j=1;
  }
  else if (S2 == 1 && S1==1 && S4 == 0 && S3 == 0)
  {
     dir=45;
   
    Serial.println("North East");
   j=1;
  }
  else
  {
   Serial.println("Error reading direction");

   dir=400;   }
 
    
  }
//---------------------------------------------------------------------------------------------------------------
//                                                   RPM Calculator
//---------------------------------------------------------------------------------------------------------------

 void speed_calc()
  {
   Serial.println("Measuring RPM");
  attachInterrupt(digitalPinToInterrupt(D5),counter,FALLING);   
  delay(3000);
  if (pulses>=50) 
  { 
    rpm = 1000.0/(millis()-timeold)*pulses;
    rps =rpm/60.0;
    timeold = millis();
   
    pulses=0;
    int time1= timeold;
    
    Serial.print(time1);
    Serial.println(" msec");
    //detachInterrupt();   
   }
  
   } 
//---------------------------------------------------------------------------------------------------------------
//                                                   RPM Interrupt
//---------------------------------------------------------------------------------------------------------------

void counter()
{
  ++pulses;
  
 }

//---------------------------------------------------------------------------------------------------------------
//                                                   END
//---------------------------------------------------------------------------------------------------------------

 
