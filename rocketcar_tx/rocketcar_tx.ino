#include <PS3USB.h>
#include <EasyTransfer.h>

#define MAX_RESET 	7 //MAX3421E pin 12
#define MAX_GPX   	8 //MAX3421E pin 17

#define TRIM_MAX        100
#define TRIM_DEFAULT	0
#define TRIM_AMOUNT     5

#define LIGHT_MODE_IDLE        0
#define LIGHT_MODE_FIRE        1
#define LIGHT_MODE_TRIM_LEFT   2
#define LIGHT_MODE_TRIM_RIGHT  3

// Initialize USB Hooks
USB Usb;
PS3USB PS3(&Usb); // This will just create the instance
//PS3USB PS3(&Usb,0x00,0x15,0x83,0x3D,0x0A,0x57); // This will also store the bluetooth address - this can be obtained from the dongle when running the sketch

// Initialize EasyTransfer Object
EasyTransfer ET; 

struct SEND_DATA_STRUCTURE{
	int throttle;
	int reverse;
	int steering;
	boolean armESC;
	boolean breaking;
	boolean headlights;
	boolean engine1;
	boolean engine2;
};
SEND_DATA_STRUCTURE rocketcontrols;

volatile int trim = TRIM_DEFAULT;
volatile int controllerLightMode = LIGHT_MODE_IDLE;
bool idleReset = false;

void setup() {
  
	// Stupid Sparkfun USB Shield..
	pinMode(MAX_GPX, INPUT);
	pinMode(MAX_RESET, OUTPUT);
	digitalWrite(MAX_RESET, LOW);
	delay(20);
	digitalWrite(MAX_RESET, HIGH);
	delay(20);
	
	pinMode(13, OUTPUT);
	
	// Initialize Serial and Easy Transfer
	Serial.begin(9600);
	delay(1000);
	//Serial.println("Serial is ready.");
	
	// Initialize Easy-Transfer Lib
	ET.begin(details(rocketcontrols), &Serial);
	
	// Initialize USB
	if (Usb.Init() == -1) { // Halt!
		Serial.println("USB.Init() did not start..");
		while(Usb.Init() == -1) { 
			Serial.println("Waiting on USB.init()...");
			delay(500);
		}
	}
	
	//Serial.println("USB Initialized!");
	
	randomSeed(analogRead(0));
	
	resetSettings();
	delay(200);		
}

void loop() {

	Usb.Task();

	if( Usb.getUsbTaskState() != USB_STATE_RUNNING || !PS3.PS3Connected) {
            if (!idleReset) {
                idleReset = true;
                resetSettings();
	        ET.sendData();   
            }
            return;
	} else { idleReset = false; }

	// Steering
	rocketcontrols.steering = processSteering();
	
	// Steering Trim
	if (PS3.getButtonClick(RIGHT)) {
		adjustTrim(TRIM_AMOUNT);
	} else if (PS3.getButtonClick(LEFT)) {
		adjustTrim(-TRIM_AMOUNT);
	}

	// Throttle
	if(PS3.getAnalogButton(R2)) {
		rocketcontrols.throttle = int( PS3.getAnalogButton(R2) );
	} else {
		rocketcontrols.throttle = 0;
	}
	
	// Reverse
	if(PS3.getAnalogButton(L2)) {
		rocketcontrols.reverse = int( PS3.getAnalogButton(L2) );
	} else {
		rocketcontrols.reverse = 0;
	}

	// Breaking
	if(PS3.getButtonPress(CIRCLE)) {
		rocketcontrols.breaking = true;    
	} else {
		rocketcontrols.breaking = false;  
            }
	
	// Headlights
	if(PS3.getButtonClick(L3)) {
		rocketcontrols.headlights = !rocketcontrols.headlights;
	}
	
	// ENGINE 1  
        if (PS3.getButtonPress(L3) && PS3.getButtonPress(R3) && 
            PS3.getAnalogButton(L1) && PS3.getAnalogButton(R1)) 
        {
            controllerLightMode = LIGHT_MODE_FIRE;
            rocketcontrols.engine1 = true;
        } else {
            rocketcontrols.engine1 = false;
        }
		

/*
	Serial.print("throttle=");
	Serial.print(rocketcontrols.throttle);
	Serial.print(", steering=");
	Serial.print(rocketcontrols.steering);
	Serial.print(", reverse=");
	Serial.print(rocketcontrols.reverse);
	Serial.print(", breaking=");
	Serial.print(rocketcontrols.breaking);
	Serial.print(", headlights=");
	Serial.print(rocketcontrols.headlights);
	Serial.print(", engine1=");
	Serial.print(rocketcontrols.engine1);
	Serial.println("");
*/
	ET.sendData();
	
        processControllerLights();

	delay(20);
}


int raceLightIndex = 0;
int raceLightCounter = 0;
int lightFireCounter = 0;
int lightTrimCounter = 0;
LEDEnum LEDs[4] = {LED4, LED3, LED2, LED1};
void processControllerLights() {
    
    switch(controllerLightMode) {
  case LIGHT_MODE_FIRE:

      lightFireCounter++;
      if (lightFireCounter == 200) {
          lightFireCounter = 0;
          controllerLightMode = LIGHT_MODE_IDLE;
      } else {
           if (lightFireCounter % 2 == 0) {
                for (int led = 0; led < 4; led++) { PS3.setLedOn(LEDs[led]); }
           } else {
                for (int led = 3; led > 0; led--) { PS3.setLedOff(LEDs[led]); }
           }
      }
  
  break;
  case LIGHT_MODE_IDLE:
     // Race Lights
    raceLightCounter++;
    if (raceLightCounter != 5) return;
    raceLightCounter = 0;
    for (int led = 0; led < 4; led++) {
        if (led == raceLightIndex) {
            PS3.setLedOn(LEDs[led]);
        } else {
            PS3.setLedOff(LEDs[led]);
        }
    }  
    raceLightIndex++;
    if (raceLightIndex > 3) raceLightIndex = 0;
 break;
  case LIGHT_MODE_TRIM_LEFT:
  
      PS3.setLedOn(LED4);
      PS3.setLedOn(LED3);
      PS3.setLedOff(LED2);
      PS3.setLedOff(LED1);

    lightTrimCounter++;
    if (lightTrimCounter == 18) {
         lightTrimCounter = 0;   
        controllerLightMode = LIGHT_MODE_IDLE;
    }

 break;
  case LIGHT_MODE_TRIM_RIGHT:
  
      PS3.setLedOff(LED4);
      PS3.setLedOff(LED3);
      PS3.setLedOn(LED2);
      PS3.setLedOn(LED1);
  
    lightTrimCounter++;
    if (lightTrimCounter == 18) {
         lightTrimCounter = 0;   
        controllerLightMode = LIGHT_MODE_IDLE;
    }

 break;
    }
    
}

void adjustTrim(int direction) {
        trim = constrain(trim + direction, -(TRIM_MAX), TRIM_MAX);
        if (direction > 0) {
         controllerLightMode = LIGHT_MODE_TRIM_RIGHT;   
        } else {
         controllerLightMode = LIGHT_MODE_TRIM_LEFT;   
        }
}

int processSteering() { 
       int steering =  PS3.getAnalogHat(LeftHatX) + trim; 	 
        steering = constrain(steering, 0, 255);
        return steering;
}


void resetSettings() {
	rocketcontrols.throttle = 0;
	rocketcontrols.reverse = 0;
	rocketcontrols.steering = 125 + trim;
	rocketcontrols.armESC = false;
	rocketcontrols.breaking = false;
	rocketcontrols.headlights = false;
	rocketcontrols.engine1 = false;
	rocketcontrols.engine1 = false;

	//PS3.setRumbleOn(200, 100, 0, 0);
	//PS3.setRumbleOn(RumbleHigh );
}
