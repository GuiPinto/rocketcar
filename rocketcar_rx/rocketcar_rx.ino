/**
  ROCKET CAR!!
  Receiver
**/

#include <Servo.h> 
#include <EasyTransfer.h>

#define ENGINE1_PIN  10
#define THROTTLE_PIN 8
#define STEERING_PIN 9
#define ESCFAN_PIN  5

#define escArmVal 89

Servo throttle;
Servo steering;

EasyTransfer ET; 

struct CONTROLLER_DATA_STRUCTURE{
	int throttle;
	int reverse;
	int steering;
	boolean armESC;
	boolean breaking;
	boolean headlights;
	boolean engine1;
	boolean engine2;
};

CONTROLLER_DATA_STRUCTURE controller;

boolean engine1Fireing = false;
unsigned long engine1FireingTime = 2000;
unsigned long engine1FiredTime = 0;

void setup(){
  
	Serial.begin(9600);
	
	ET.begin(details(controller), &Serial);
	
	steering.attach(STEERING_PIN, 1000, 2000);
	throttle.attach(THROTTLE_PIN, 1000, 2000);
	
	pinMode(13, OUTPUT);
	
	pinMode(THROTTLE_PIN, OUTPUT);
	pinMode(STEERING_PIN, OUTPUT); 
	pinMode(ENGINE1_PIN, OUTPUT);  
	pinMode(ESCFAN_PIN, OUTPUT);
	
	randomSeed(analogRead(0));

	// Arm ESC
	//throttle.write(89);
	//delay(3000);

}

int motorInit = 0;

void loop(){
	
    
	/*
	I have all values i need: 
	- arm with a value between 89 to 97 during 2s
	- one direction:99 to 138 and the other 87 to 48
	- brake but is too bad for the motor, i don´t use them in my project:0 and 254.
	*/
	
	if(ET.receiveData()) {
		
		// Arming Electronic Speed Controller
		if (controller.armESC) {
			throttle.write( escArmVal );
			delay(2000);
		}
		
		// Steering
                //steering.write( map(controller.steering, 0, 255, 20 + trim, 175 - trim) );
                int steeringVal = controller.steering;
                // Invert steering because servo is mounted backwards
		steering.write(map(steeringVal, 0, 255, 255, 0));

//-----Foward: 95-255
//-----Reverse: 84-0

		// Throttle
		if (controller.breaking) {
			throttle.write(88);
		} else if (controller.reverse > 5) {
			throttle.write( map(controller.reverse, 5, 255, 15, 84) );
		} else if (controller.throttle > 5) {  
			throttle.write( map(controller.throttle, 5, 255, 125, 255) );
		} else {
			throttle.write(92);
		}	

		// Rocket Engine
		if (controller.engine1 && !engine1Fireing) {
			engine1Fireing = true;
			engine1FiredTime = millis();
		}
		if (engine1Fireing) {
			if (millis() > engine1FiredTime + engine1FireingTime) {
				digitalWrite(ENGINE1_PIN, LOW);
				engine1Fireing = false;
			} else {
				digitalWrite(ENGINE1_PIN, HIGH);
			}
		}
		
		delay(20);	
	}

}

