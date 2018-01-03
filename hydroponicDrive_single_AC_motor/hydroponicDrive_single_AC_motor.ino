//Temperature libraries
#include <OneWire.h>
#include <DallasTemperature.h>
//Humidity libraries
#include "DHT.h"

//Declare var names for the outlets
#define outlet1 53
#define outlet2 51  //warning lamp
#define outlet3 49  //drive motor
#define outlet4 47  //water pump

//Inputs
#define waterSwitch 39
#define leftSwitch 41
#define rightSwitch 43
#define eStop 45
#define waterLevelSwitch 37

//Booleans for outlet on and off states
bool on = LOW;
bool off = HIGH;

bool leftSwitchLockout = false;
bool rightSwitchLockout = false;
bool allowPump = true;

int printDelay = 500;
int printPrevTime = millis();
int printCount = 0;

int debounceTime = 50;
int leftSwitchTime = millis();
int rightSwitchTime = millis();
int leftSwitchCount = 0;
int rightSwitchCount = 0;

//Temperature Sensors
#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);  //Setup a oneWire instance to communicate with oneWire devices
DallasTemperature sensors(&oneWire);  //Pass the oneWire reference to Dallas Temperature

//Humidity Sensor
#define DHTPIN 9
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup(){
  //Open the serial port at 9600 baud
  Serial.begin(9600);
  
  //Define outlet pins as outputs
  pinMode(outlet1,OUTPUT);
  pinMode(outlet2,OUTPUT);
  pinMode(outlet3,OUTPUT);
  pinMode(outlet4,OUTPUT);

  //Set all outlets to off
  digitalWrite(outlet1,off);
  digitalWrite(outlet2,off);
  digitalWrite(outlet3,off);
  digitalWrite(outlet4,off);

  //Limit switches
  pinMode(waterSwitch,INPUT);
  pinMode(leftSwitch,INPUT);
  pinMode(rightSwitch,INPUT);

  //Switches
  pinMode(eStop,INPUT);
  pinMode(waterLevelSwitch,INPUT);

  //Start the oneWire library
  sensors.begin();

  //Humidity Sensor
  dht.begin();
  
  Serial.println("Clear Buffer");
  Serial.println("Starting...");
}

double tempValA;
double tempValW;
double humidVal;

void loop(){
  /*
   * Watering routine
   */
   
  //Forward
  digitalWrite(outlet3,on);
  
  if(digitalRead(waterSwitch) == HIGH){
    //Stop
    delay(100);
    digitalWrite(outlet3,off);
    
    //Wait for swing to stop
    delay(300);

    if(allowPump == true){
      //Turn on pump till full (time)
      digitalWrite(outlet4,on);
      delay(14000);
    
      //Turn pump off
      digitalWrite(outlet4,off);
    }
    
    //Wait for dripping to stop (time)
    delay(3000);

    /*
     * Get sensor data
     */
  
    sensors.requestTemperatures();  //Send command to get temperature readings
    tempValA = sensors.getTempCByIndex(1);
    tempValW = sensors.getTempCByIndex(0);
    humidVal = dht.readHumidity();
  
    /*
     * Print sensor data
     */
  
    String s1 = "0000";
    String s2 = "," + (String)tempValA;
    String s3 = "," + (String)tempValW;
    String s4 = "," + (String)humidVal;
    String s5 = "," + (String)rightSwitchCount;
    String s6 = "," + (String)leftSwitchCount;
    String s7 = "," + (String)allowPump;
    Serial.println(s1 + s2 + s3 + s4 + s5 + s6 + s7);
    
    //Drive forward until microSwitch is no longer pressed
    while(digitalRead(waterSwitch) == HIGH){
      digitalWrite(outlet3,on);
    }
  
    //delay to move away from the switch
    delay(800);
      
    //Turn off drive motor
    digitalWrite(outlet3,off);
  }
  
  /*
   * Check Sync
   */
  
  //Left Switch
  if(leftSwitchLockout == false && digitalRead(leftSwitch) == 1){
    leftSwitchTime = millis();
    leftSwitchCount++;
    delay(debounceTime);
    leftSwitchLockout = true;
  }
  if(leftSwitchLockout == true && digitalRead(leftSwitch) == 0){
    leftSwitchLockout = false;
    delay(debounceTime);
  }

  //Right Switch
  if(rightSwitchLockout == false && digitalRead(rightSwitch) == 1){
    rightSwitchTime = millis();
    rightSwitchCount++;
    delay(debounceTime);
    rightSwitchLockout = true;
  }
  if(rightSwitchLockout == true && digitalRead(rightSwitch) == 0){
    rightSwitchLockout = false;
    delay(debounceTime);
  }
  
  /*
   * Stop Conditions
   */

  //Time between trays exceeded 2000 ms
  if(leftSwitchCount == rightSwitchCount && abs(leftSwitchTime - rightSwitchTime) > 2000){
    //!!! STOP !!!
    //Set all outlets to off
    digitalWrite(outlet1,off);
    digitalWrite(outlet2,off);
    digitalWrite(outlet3,off);
    digitalWrite(outlet4,off);
    
    Serial.println("ERROR: time");

    while(true){
      digitalWrite(outlet2,on);
      delay(1000);
      digitalWrite(outlet2,off);
      delay(1000);
    }
  }

  //Tray count differed by 2 or more
  if(abs(leftSwitchCount - rightSwitchCount) >= 2){
    //!!! STOP !!!
    //Set all outlets to off
    digitalWrite(outlet1,off);
    digitalWrite(outlet2,off);
    digitalWrite(outlet3,off);
    digitalWrite(outlet4,off);
    
    Serial.println("ERROR: count");

    while(true){
      digitalWrite(outlet2,on);
      delay(500);
      digitalWrite(outlet2,off);
      delay(500);
    }
  }

  //eStop was pressed
  if(digitalRead(eStop) == LOW){
    //!!! STOP !!!
    //Set all outlets to off
    digitalWrite(outlet1,off);
    digitalWrite(outlet2,off);
    digitalWrite(outlet3,off);
    digitalWrite(outlet4,off);
    
    Serial.println("ERROR: eStop");
    delay(250);
    
    while(digitalRead(eStop) == LOW){
      digitalWrite(outlet2,on);
      delay(200);
      digitalWrite(outlet2,off);
      delay(200);
    }
    delay(250);
  }

  //water level too low, switch no longer pressed
  //NOTE: does not stop system, just disables water pump
  if(allowPump == true && digitalRead(waterLevelSwitch) == LOW){
    Serial.println("ERROR: water");
    
    allowPump = false;
    delay(250);
  }
  if(allowPump == false && digitalRead(waterLevelSwitch) == HIGH){
    allowPump = true;
    delay(250);
  }
}
