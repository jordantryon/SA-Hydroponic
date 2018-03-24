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
#define waterToggle 7

//initialize the LCD with pins
#define RS 12
#define EN 11
#define D4 5
#define D5 4
#define D6 3
#define D7 2
LiquidCrystal lcd(RS,EN,D4,D5,D6,D7);

//temperature Sensors
#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);  //setup a oneWire instance to communicate with oneWire devices
DallasTemperature sensors(&oneWire);  //pass the oneWire reference to Dallas Temperature

//humidity Sensor
#define DHTPIN 9
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

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

//declare sensor value variables
double tempValA;
double tempValW;
double humidVal;

void setup(){
  //define outlet relay pins as outputs
  pinMode(outlet1,OUTPUT);
  pinMode(outlet2,OUTPUT);
  pinMode(outlet3,OUTPUT);
  pinMode(outlet4,OUTPUT);

  //ensure outlets are turned off
  outletsOff();

  //define limit switch pins as inputs
  pinMode(waterSwitch,INPUT);
  pinMode(leftSwitch,INPUT);
  pinMode(rightSwitch,INPUT);

  //define switches as inputs
  pinMode(eStop,INPUT);
  pinMode(waterLevelSwitch,INPUT);
  pinMode(waterToggle,INPUT_PULLUP);

  //start the oneWire library
  sensors.begin();

  //start the humidity sensor library
  dht.begin();

  //start the LCD library
  lcd.begin(20,4);
  lcd.noAutoscroll();

  //open the serial port at 9600 baud
  Serial.begin(9600);

  //print starting messages to clear buffer and start on new line
  Serial.println("Clear Buffer");
  Serial.println("Starting...");

  //p=rint starting message to LCD
  updateLCD("starting");
}

void loop(){
  /*
   * Watering routine
   */

  //forward
  digitalWrite(outlet3,on);

  if(digitalRead(waterSwitch) == HIGH){
    if(digitalRead(waterToggle) == HIGH){
      updateLCDstatus("watering");

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

      updateLCDsensors();

      //drive forward until microSwitch is no longer pressed
      digitalWrite(outlet3,on);
      while(digitalRead(waterSwitch) == HIGH){}

      //delay to move away from the switch, prevent double presses
      delay(200);

      updateLCDstatus("move to next");
    } else {
      updateLCDstatus("skip water");
    }
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

    updateLCDsensors();
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

    updateLCDsensors();
  }
  if(rightSwitchLockout == true && digitalRead(rightSwitch) == 0){
    rightSwitchLockout = false;
    delay(debounceTime);
  }

  /*
   * Unrecoverable Stop Conditions
   */

  //tray count differs by 2 or more
  if(abs(leftSwitchCount - rightSwitchCount) >= 2){
    outletsOff();

    Serial.println("ERROR: count");
    updateLCDstatus("ERROR count");

    while(true){
      blinkLight(500);
    }
  }

  /*
   * Recoverable Stop Conditions
   */

  //eStop is pressed
  //stops system while button is pressed, then resumes
  if(digitalRead(eStop) == LOW){
    outletsOff();

    Serial.println("ERROR: eStop");
    updateLCDstatus("ERROR eStop");
    delay(250);

    while(digitalRead(eStop) == LOW){
      blinkLight(200);
    }
    delay(250);
  }

  //water level too low, switch is high
  //stops system to refill res, then resumes
  if(digitalRead(waterLevelSwitch) == HIGH){
    outletsOff();

    Serial.println("ERROR: water");
    updateLCDstatus("ERROR water");

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

//blinks warning lamp at specified delay
void blinkLight(int d){
  digitalWrite(outlet2,on);
  delay(d);
  digitalWrite(outlet2,off);
  delay(d);
}

//turn all outlets off
void outletsOff(){
  digitalWrite(outlet1,off);
  digitalWrite(outlet2,off);
  digitalWrite(outlet3,off);
  digitalWrite(outlet4,off);
}

//update entire LCD, print a new status with specified string (max readable length = 12 chars)
void updateLCD(String stat){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("LC: " + (String)leftSwitchCount);
  lcd.setCursor(10,0);
  lcd.print("RC: " + (String)rightSwitchCount);
  lcd.setCursor(0,1);
  lcd.print("Ta: " + (String)tempValA);
  lcd.setCursor(10,1);
  lcd.print("Tw: " + (String)tempValW);
  lcd.setCursor(0,2);
  lcd.print("H%: " + (String)humidVal);
  lcd.setCursor(0,3);
  lcd.print("Status: " + stat);
}

//update status message on LCD with specified string (max readable length = 12 chars), leave other values unchanged
void updateLCDstatus(String stat){
  lcd.setCursor(0,3);
  lcd.print("                    ");

  lcd.setCursor(0,3);
  lcd.print("Status: " + stat);
}

//update sensor values on LCD, leave other values unchanged
void updateLCDsensors(){
  lcd.setCursor(0,1);
  lcd.print("                    ");
  lcd.setCursor(0,2);
  lcd.print("                    ");

  lcd.print("Ta: ");// + (String)tempValA);
  lcd.setCursor(10,1);
  lcd.print("Tw: ");// + (String)tempValW);
  lcd.setCursor(0,2);
  lcd.print("H%: ");// + (String)humidVal);
}

//update count values on LCD, leave other values unchanged
void updateLCDcount(){
  lcd.setCursor(0,0);
  lcd.print("                    ");

  lcd.setCursor(0,0);
  lcd.print("LC: ");// + (String)leftSwitchCount);
  lcd.setCursor(10,0);
  lcd.print("RC: ");// + (String)rightSwitchCount);
}
