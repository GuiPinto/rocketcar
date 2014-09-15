/**
 * ROCKET CAR!!
 * Transmitter
 *
 * PS3 BT Reference: https://github.com/felis/USB_Host_Shield_2.0/blob/master/examples/Bluetooth/PS3BT/PS3BT.ino
 **/

#include <PS3USB.h>
#include <EasyTransfer.h>

boolean printAngle;
uint8_t state = 0;

//Revision 1.2 (DEV-09628)
#define MAX_RESET 8 //MAX3421E pin 12
#define MAX_GPX   7 //MAX3421E pin 17
//Revision 1.3 (DEV-09947)
#define MAX_RESET 7 //MAX3421E pin 12
#define MAX_GPX   8 //MAX3421E pin 17


// Initialize USB Hooks
USB Usb;
PS3USB PS3(&Usb); // This will just create the instance
//PS3USB PS3(&Usb,0x00,0x15,0x83,0x3D,0x0A,0x57); // This will also store the bluetooth address - this can be obtained from the dongle when running the sketch

// Initialize EasyTransfer Object
EasyTransfer ET; 

struct SEND_DATA_STRUCTURE{
  int throttle;
  int breaks;
  int stearing;
  boolean armESC;
  boolean breaking;
  boolean headlights;
  boolean engine1;
  boolean engine2;  
};
SEND_DATA_STRUCTURE rocketcontrols; // Name it


void setup() {

  // Stupid Sparkfun USB Shield..
  pinMode(MAX_GPX, INPUT);
  pinMode(MAX_RESET, OUTPUT);
  digitalWrite(MAX_RESET, LOW);
  delay(20);
  digitalWrite(MAX_RESET, HIGH);
  delay(20);

  // Initialize Serial and Easy Transfer
  Serial.begin(9600);

  // Init Easy-Transfer Lib
  ET.begin(details(rocketcontrols), &Serial);

  // Initialize USB
  if (Usb.Init() == -1) {
    while(1) { 
      // Halt!
    }
  }


  pinMode(13, OUTPUT);
  randomSeed(analogRead(0));

}
void loop() {

  Usb.Task();

  // Stearing
  rocketcontrols.stearing = int( PS3.getAnalogHat(LeftHatX) );

  // Throttle
  if(PS3.getAnalogButton(R2)) {
    rocketcontrols.throttle = int( PS3.getAnalogButton(R2) );
    rocketcontrols.breaking = false;
  } else {
    rocketcontrols.throttle = 0;
  }

  // Breaking
  if(PS3.getAnalogButton(L2)) {
    rocketcontrols.breaks = int( PS3.getAnalogButton(L2) );
    rocketcontrols.breaking = false;
  } else {
    rocketcontrols.breaks = 0;
  }
  
  // Breaking
  if(PS3.getButtonClick(CIRCLE)) {
    rocketcontrols.breaking = true;
  }
  
  // Headlights
  if(PS3.getButtonClick(L3)) {
   rocketcontrols.headlights = !rocketcontrols.headlights;
  }
  
  // Arm ESC
  rocketcontrols.armESC = PS3.getButtonClick(SELECT);
  
  // ENGINE 1  
  rocketcontrols.engine1 = PS3.getButtonClick(L1) && PS3.getButtonClick(R1);// && PS3.getButtonClick(CROSS);
  
  ET.sendData();

  delay(50);
  
}

