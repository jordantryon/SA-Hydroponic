//outlet pin definitions
#define outlet1 53  //
#define outlet2 51  //warning lamp for low nutrient tank
#define outlet3 49  //drive motor
#define outlet4 47  //water pump

//input button definitions
#define toggleSwitch 41     //switch to toggle between auto and operator control
#define forwardBtn 43       //button to manually move belt forwards in operator control
#define backwardBtn 45      //button to manually move belt backwards in operator control
#define trayLimitSwitch 39  //limit switch to detect incoming tray for watering
#define waterLevelSwitch 38 //switch to detect water level

//variables
int prevTime = millis();  //variable to keep track of the previous trays time

//booleans for outlet on and off states
bool on = LOW;
bool off = HIGH;

void setup(){
  //define outlet pins as outputs
  pinMode(outlet1,OUTPUT);
  pinMode(outlet2,OUTPUT);
  pinMode(outlet3,OUTPUT);
  pinMode(outlet4,OUTPUT);
  
  //set all outlets to off
  digitalWrite(outlet1,off);
  digitalWrite(outlet2,off);
  digitalWrite(outlet3,off);
  digitalWrite(outlet4,off);

  //input buttons
  pinMode(toggleSwitch,INPUT);
  pinMode(forwardBtn,INPUT);
  pinMode(backwardBtn,INPUT);
  
  //switches
  pinMode(trayLimitSwitch,INPUT);
  pinMode(waterLevelSwitch,INPUT);
}

void loop(){
  //check the nutrient tank level
  if(digitalRead(waterLevelSwitch) == LOW){ //if level is low
    digitalWrite(outlet2,on);               //turn on lamp
  }else{                                    //when level is back up
    digitalWrite(outlet2,off);              //turn off lamp
  }

  //check state of the auto/manual toggle switch
  if(digitalRead(toggleSwitch) == HIGH){  //auto mode
    if(millis()-prevTime < 90000){  //if time since last tray < 90 secs
      //turn on drive motor
      digitalWrite(outlet3,HIGH);
  
      //wait until switch is pressed
      while(digitalRead(trayLimitSwitch) == LOW){}
  
      //stop drive belt
      digitalWrite(outlet3,off);
  
      //wait for swing to stop
      delay(500);
  
      //turn on pump until full (time)
      digitalWrite(outlet4,on);
      delay(6000);
  
      //turn off pump
      digitalWrite(outlet4,off);
  
      //wait for driping to stop
      delay(2000);
  
      //drive forward
      digitalWrite(outlet3,on);
      
      //wait until limit switch is no longer pressed
      while(digitalRead(trayLimitSwitch) == HIGH){}
      
      //delay to move away from the switch
      delay(400);
      
      //Turn off drive motor;
      digitalWrite(outlet3,off);
      
      //reset previous time
      prevTime = millis();
    }else{  //if time since last tray >= 90 secs
      digitalWrite(outlet3,off);  //turn off drive belt
      digitalWrite(outlet4,off);  //turn off pump
      while(true){}
    }
  }else{  //manual mode
    //turn off water pump
    digitalWrite(outlet4,LOW);
    
    if(digitalRead(forwardBtn) == HIGH){  //if forward button is pressed
      digitalWrite(outlet3,HIGH);         //drive belt forward
    }else{                                //otherwise
      digitalWrite(outlet3,LOW);          //stop belt
    }
    
    //when exiting manual mode, reset time since last tray
    prevTime = millis();
  }
}
