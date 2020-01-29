/**
 * A Legoino example to control a Lego train with a rotary encoder.
 * 
 * 1) Power up the ESP32
 * 2) Power up the Train Hub
 * 
 * You can change the motor speed with the left (A) remote buttons
 * 
 * (c) Copyright 2019 - Cornelius Munz
 * Released under MIT License
 * 
 */

#include "PoweredUpRemote.h"
#include "PoweredUpHub.h"

#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"

#define ROTARY_ENCODER_A_PIN 33
#define ROTARY_ENCODER_B_PIN 32
#define ROTARY_ENCODER_BUTTON_PIN 25
#define ROTARY_ENCODER_VCC_PIN -1 /*put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN);

// create a hub instance
PoweredUpHub myHub;

PoweredUpHub::Port _portA = PoweredUpHub::Port::A;

int currentSpeed = 0;
int updatedSpeed = 0;
bool isInitialized = false;


void rotary_loop() {
  //first lets handle rotary encoder button click
  if (rotaryEncoder.currentButtonState() == BUT_DOWN) {
    //we can process it here or call separate function like:
    updatedSpeed = 0;
    rotaryEncoder.reset();
  }

  //lets see if anything changed
  int16_t encoderDelta = rotaryEncoder.encoderChanged();
  
  //optionally we can ignore whenever there is no change
  if (encoderDelta == 0) return;
  
  //for some cases we only want to know if value is increased or decreased (typically for menu items)
  if (encoderDelta>0) Serial.print("+");
  if (encoderDelta<0) Serial.print("-");

  //for other cases we want to know what is current value. Additionally often we only want if something changed
  //example: when using rotary encoder to set termostat temperature, or sound volume etc
  
  //if value is changed compared to our last read
  if (encoderDelta!=0) {
    //now we need current value
    int16_t encoderValue = rotaryEncoder.readEncoder();
    //process new value. Here is simple output.
    Serial.print("Value: ");
    Serial.println(encoderValue);
    updatedSpeed = encoderValue * 5;
  } 
  
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting sketch...");

    myHub.init(); // after connecting the remote, try to connect the hub

    rotaryEncoder.begin();
    rotaryEncoder.setup([]{rotaryEncoder.readEncoder_ISR();});
    rotaryEncoder.setBoundaries(-20, 20, false); //minValue, maxValue, cycle values (when max go to min and vice versa)
} 

// main loop
void loop() {
  // connect flow

  if (myHub.isConnecting()) {
    myHub.connectHub();
    if (myHub.isConnected()) {
      Serial.println("Connected to Hub");
    } else {
      Serial.println("Failed to connect to Hub");
    }
  }

  if ( myHub.isConnected() && !isInitialized) {
     Serial.println("System is initialized");
      isInitialized = true;
      // both activations are needed to get status updates
      myHub.setLedColor(WHITE);
  }

  // if connected we can control the train motor on Port A with the remote
  if (isInitialized) {
    rotary_loop();
    if (currentSpeed != updatedSpeed) {
      myHub.setMotorSpeed(_portA, updatedSpeed);
      currentSpeed = updatedSpeed;
      if ( updatedSpeed == 0 )
         myHub.setLedColor( WHITE );
      else if ( updatedSpeed > 0 )
         myHub.setLedColor( GREEN );
      else
         myHub.setLedColor( RED );
      Serial.print("Current speed:");
      Serial.println(currentSpeed, DEC);
    }
    delay(50);
  }
  
} // End of loop
