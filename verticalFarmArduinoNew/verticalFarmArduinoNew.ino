//temperature libraries
#include <OneWire.h>
#include <DallasTemperature.h>
//humidity libraries
#include <DHT.h>
//LCD library
#include <LiquidCrystal.h>

//declare var names for the outlets
#define outlet1 53  //secondary refill pump
#define outlet2 51  //warning lamp
#define outlet3 49  //drive motor
#define outlet4 47  //water pump

//inputs
#define eStop 45
#define rightSwitch 43
#define leftSwitch 41
#define waterSwitch 39
#define waterLevelSwitch 37

//booleans for outlet on and off states
bool on = LOW;
bool off = HIGH;

//logic for count sync
bool leftSwitchLockout = false;
bool rightSwitchLockout = false;

int debounceTime = 100;
int leftSwitchTime = millis();
int rightSwitchTime = millis();
int leftSwitchCount = 0;
int rightSwitchCount = 0;

//temperature Sensors
#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);  //setup a oneWire instance to communicate with oneWire devices
DallasTemperature sensors(&oneWire);  //pass the oneWire reference to Dallas Temperature

//humidity Sensor
#define DHTPIN 9
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//declare sensor value variables
double tempValA;
double tempValW;
double humidVal;

//initialize the LCD with pins
#define RS 12
#define EN 11
#define D4 5
#define D5 4
#define D6 3
#define D7 2
LiquidCrystal lcd(RS,EN,D4,D5,D6,D7);

void setup(){
  //open the serial port at 9600 baud
  Serial.begin(9600);

  //define outlet pins as outputs
  pinMode(outlet1,OUTPUT);
  pinMode(outlet2,OUTPUT);
  pinMode(outlet3,OUTPUT);
  pinMode(outlet4,OUTPUT);

  outletsOff();

  //limit switches
  pinMode(waterSwitch,INPUT);
  pinMode(leftSwitch,INPUT);
  pinMode(rightSwitch,INPUT);

  //switches
  pinMode(eStop,INPUT);
  pinMode(waterLevelSwitch,INPUT);

  //start the oneWire library
  sensors.begin();

  //humidity Sensor
  dht.begin();

  //LCD
  lcd.begin(20,4);

  Serial.println("Clear Buffer");
  Serial.println("Starting...");
}

void loop(){
  /*
   * Watering routine
   */

  //forward
  digitalWrite(outlet3,on);

  if(digitalRead(waterSwitch) == HIGH){
    //stop
    delay(250);
    digitalWrite(outlet3,off);

    //turn pump on
    digitalWrite(outlet4,on);

    //wait for pumping duration
    delay(14000);

    //turn pump off
    digitalWrite(outlet4,off);

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
    Serial.println(s1 + s2 + s3 + s4 + s5 + s6);

    //drive forward until microSwitch is no longer pressed
    digitalWrite(outlet3,on);
    while(digitalRead(waterSwitch) == HIGH){}

    //delay to move away from the switch, prevent double presses
    delay(200);
  }

  /*
   * Check Sync
   */

  //left switch
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

  //right switch
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

  //tray count differs by 2 or more
  if(abs(leftSwitchCount - rightSwitchCount) >= 2){
    outletsOff();

    Serial.println("ERROR: count");

    while(true){
      blinkLight(500);
    }
  }

  //eStop is pressed
  if(digitalRead(eStop) == LOW){
    outletsOff();

    Serial.println("ERROR: eStop");
    delay(250);

    while(digitalRead(eStop) == LOW){
      blinkLight(200);
    }
    delay(250);
  }

  //water level too low, switch is high
  //NOTE: stops system to refill res then resumes
  if(digitalRead(waterLevelSwitch) == HIGH){
    outletsOff();

    Serial.println("ERROR: water");

    while(digitalRead(waterLevelSwitch) == HIGH){
      digitalWrite(outlet1,on);
    }
    delay(10000);
    digitalWrite(outlet1,off);
  }
}

/*
 * Functions
 */

void blinkLight(int d){
  digitalWrite(outlet2,on);
  delay(d);
  digitalWrite(outlet2,off);
  delay(d);
}

void outletsOff(){
  digitalWrite(outlet1,off);
  digitalWrite(outlet2,off);
  digitalWrite(outlet3,off);
  digitalWrite(outlet4,off);
}

void updateLCD(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ta: " + (String)tempValA);
  lcd.setCursor(10,0);
  lcd.print("Tw: " + (String)tempValW);
  lcd.setCursor(0,1);
  lcd.print("H%: " + (String)humidVal);
}

/*
//time between trays exceeded 2000 ms
if(leftSwitchCount == rightSwitchCount && abs(leftSwitchTime - rightSwitchTime) > 2000){
  outletsOff();

  Serial.println("ERROR: time");

  while(true){
    blinkLight(1000);
  }
}
*/
