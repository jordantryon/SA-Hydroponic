//Declare var names for the pins/outlets
#define outlet1 53
#define outlet2 51  //Warning lamp
#define outlet3 49  //drive motor
#define outlet4 47  //water pump

//Inputs
#define waterSwitch 39
#define leftSwitch 41
#define rightSwitch 43

//Booleans for outlet on and off states
bool on = LOW;
bool off = HIGH;

bool leftSwitchLockout = false;
bool rightSwitchLockout = false;

int printDelay = 500;
int printPrevTime = millis();
int printCount = 0;

int debounceTime = 50;
int leftSwitchTime = millis();
int rightSwitchTime = millis();
int leftSwitchCount = 0;
int rightSwitchCount = 0;

void setup(){
  //Open the serial port at 9600 bps:
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

  Serial.println("Clear Buffer");
  Serial.println("Starting...");
}

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
    
    //Turn on pump till full (time)
    digitalWrite(outlet4,on);
    delay(14000);
    
    //Turn pump off
    digitalWrite(outlet4,off);
    
    //Wait for dripping to stop (time)
    delay(3000);
    
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

  //Print info
  int timeDiff = millis() - printPrevTime;
  if(timeDiff >= printDelay){
    if(printCount++ % 10 == 0){
      Serial.println("Time between trays\tLeft count\tRight count");
    }
    Serial.print(abs(leftSwitchTime - rightSwitchTime));
    Serial.print(" ms\t\t\t");
    Serial.print(leftSwitchCount);
    Serial.print("\t\t");
    Serial.print(rightSwitchCount);
    Serial.println();

    printPrevTime = millis();
  }

  /*
   * Stop Conditions
   */

  //Time between trays exceeded 1000 ms
  if(leftSwitchCount == rightSwitchCount && abs(leftSwitchTime - rightSwitchTime) > 1000){
    //!!! STOP !!!
    //Set all outlets to off
    digitalWrite(outlet1,off);
    digitalWrite(outlet2,off);
    digitalWrite(outlet3,off);
    digitalWrite(outlet4,off);
    
    Serial.println("Time between trays\tLeft count\tRight count");
    Serial.print(abs(leftSwitchTime - rightSwitchTime));
    Serial.print(" ms\t\t\t");
    Serial.print(leftSwitchCount);
    Serial.print("\t\t");
    Serial.print(rightSwitchCount);
    Serial.println();

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
    
    Serial.println("Time between trays\tLeft count\tRight count");
    Serial.print(abs(leftSwitchTime - rightSwitchTime));
    Serial.print(" ms\t\t\t");
    Serial.print(leftSwitchCount);
    Serial.print("\t\t");
    Serial.print(rightSwitchCount);
    Serial.println();

    while(true){
      digitalWrite(outlet2,on);
      delay(500);
      digitalWrite(outlet2,off);
      delay(500);
    }
  }
}
