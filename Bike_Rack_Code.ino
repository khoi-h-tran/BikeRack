// Calls in libraries I downloaded
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

// setting names for LED's

  const int redLEDPin=13; //red LED
  const int blueLEDPin=12; // blue LED
  const int greenLEDPin=11;// green LED
  const int yellowLEDPin=10; // yelow LED
  
//Setting name for alarm
  const int alarmPin=9;
  
// setting inputs for motor (H-Bridge ports)
  const int controlPin1=7; //controls direction motor spins. If pin 1 and pin 2 are both on or off, then motor does not spin. If one is on, other is off, it controls direction of spinning.
  const int controlPin2=8; //controls direction motor spins
  const int enablePin=5; //high=motor on, low=motor off.
  
// Setting variables to be used for motor control  
  int motorEnabled=0;
  int motorSpeed=0;
  int motorDirection=1;
  
  // setting sensors
  const int greenSensorPin=A1; //photoresistor sensor pin
  int greenSensorValue=0; // value for photoresistor sensor
  
  //setting variables for reservation button
  int reserveSwitchState=0; //turns on reservation feature of locking system
  int alarmOffSwitchState=0; //manual disable for the alarm
  const int alarmSwitch=4; //setting alarm switch to digital port 4
  const int reserveSwitch=6; //setting reserve switch to digital port 6
  
  int echoPin=A0; //ut input
  int trigPin=3; //ut output
  int sensorLOW=1023;
  int sensorHIGH=0;
  float duration; //setting the duration as a variable with decimal points for later
  float distance; //setting the distance as a variable with decimal points for later
  
// declaring variable i (which will be the counter)

   int i=0;
   int c=0;
   int count=0;
   
// declaring variable to determine if the system is locked
  int L;
  

void setup() {
  Serial.begin(9600);
   pinMode(echoPin,INPUT); //setting ut input 
   pinMode (trigPin,OUTPUT);   //setting ut output
   pinMode(controlPin1,OUTPUT);//motor direction control
   pinMode(controlPin2, OUTPUT);//motor direction control
   pinMode(enablePin,OUTPUT);//motor on and off
   digitalWrite(enablePin, LOW);//setting motor to be off initially
   pinMode(redLEDPin,OUTPUT); //setting red LED pin
   pinMode(blueLEDPin,OUTPUT); //setting blue LED pin
   pinMode(yellowLEDPin,OUTPUT); //setting yellow LED pin
   pinMode(greenLEDPin,OUTPUT); //setting green LED pin
   digitalWrite(redLEDPin, HIGH); //setting red LED as high
   digitalWrite(blueLEDPin, LOW); //seting blue LED as low
   digitalWrite(yellowLEDPin, LOW); //setting yellow LED as low
   digitalWrite(greenLEDPin, LOW); //setting green LED as low
   pinMode(alarmPin,OUTPUT); //setting alarm pin
   digitalWrite(alarmPin,LOW); //setting alarm to default off
   pinMode(reserveSwitch,6);
   pinMode(alarmSwitch,4);
}

void loop() {
  
  //Setting up Ultra Sonic Sensor
  digitalWrite(trigPin,LOW);//no pulse sent
  delay(2);//wait 2 ms
  digitalWrite(trigPin,HIGH);//sends a sonar pulse wave
  delay(10); //wait 10ms
  digitalWrite(trigPin,LOW); //no pulse
  duration=pulseIn(echoPin,HIGH);//pulse in starts timing when pulse is high and stops when it is low
  distance=(duration/2.0)/29.1; //calculation to find distance in cm
  
  if (distance >= 4 || distance <=0) { //stating if object is out of range
    Serial.println("Out of Range");
  }

  else{
    Serial.print(distance); //outputting distance
    Serial.println("cm"); ///outputting unit in cm
  }
  delay(1000); 
  
  //Setting up Photoresistor
    greenSensorValue=analogRead(greenSensorPin); //pulling analog signal in and storing it
    delay(100); //analog reading takes some time, this is to allow for a buffer
    Serial.print("Analog light sensor value: "); //outputs the signal title
    Serial.println(greenSensorValue); //outputs the signal
 
 //Setting up reservation switch
  reserveSwitchState=digitalRead(reserveSwitch);//checking if it is reserved
 if(reserveSwitchState==HIGH) { //while it is reserved, lock in this while loop
     ++count;
     if (count==3) {
       count=1; 
      }
     Serial.print("Reservation Status:");
     if (count==1) {
       Serial.println("RESERVED");
     }
     else if (count==2){
       Serial.println("NOT RESERVED");
     }
     
     delay(1000);
 }
 
 if (reserveSwitchState==LOW) {

   if (count==2) {
     digitalWrite(blueLEDPin,LOW); //turn blue light off from reservation
     digitalWrite(redLEDPin,HIGH); //turn on red light to show it is available
     }
   else if (count==1){
      digitalWrite(blueLEDPin,HIGH); //turn light blue to indicate that it is reserved 
      digitalWrite(redLEDPin,LOW); //turn off red light to show it is available
      reserveSwitchState=digitalRead(reserveSwitch);
      if(reserveSwitchState==HIGH) { //while it is reserved, lock in this while loop
      break;
      }

 }
 
  if (nfc.tagPresent()) { // Checking to see if tag is present 
    c=++i; // counting
    delay(5000);
  }
  if (c==3){ // resets counter to 0 so it can loop through again
    i==1;
    c==i;
    }
    
  else if (c==1) {
    while(distance>4 && greenSensorValue<800){ //while the actuator is not locked
      digitalWrite(enablePin,HIGH); //turn motor on
      digitalWrite(controlPin1,HIGH); //spin motor one way
      digitalWrite(controlPin2,LOW);//spin motor one way
    }
    digitalWrite(enablePin,LOW); //turn off motor
    digitalWrite(controlPin1,LOW); //do not spin motor
    digitalWrite(controlPin2,LOW); //do not spin motor
      
    if (distance>4 || greenSensorValue<800) { //checking to see if sensors are broken
      digitalWrite(redLEDPin, LOW);
      digitalWrite(yellowLEDPin, HIGH);
      L=0; //setting L=0 keeps track that the system it is not locked
    }
    else if (distance<4 || greenSensorValue>800){//checking to see if sensors indicate locking
      digitalWrite(redLEDPin,LOW);
      digitalWrite(greenLEDPin, HIGH);
      L=1; //setting L=1 keeps track that the system is locked
    }
    while (L==1) {
       alarmOffSwitchState=digitalRead(alarmSwitch); //reads in state of switch to turn of alarm
       if (nfc.tagPresent()) { // Checking to see if tag is present
         c=++i; //increment counter if tag is tapped
         break; //break out of while loop and sift through code to find when counter is satisfied
       }
       else if (distance>4 || greenSensorValue<800){ //if not locked try to set off alarm
         if (alarmOffSwitchState==LOW) { //set alarm of alarm off switch is not pressed
           digitalWrite(alarmPin,HIGH); //turn alarm on
         }
          else if (alarmOffSwitchState==HIGH) {//if alarm of switch is pressed turn off alarm
           digitalWrite(alarmPin,LOW); //turn alarm off
          }
       }
       else {
         digitalWrite(alarmPin,LOW);//if locked and nothing is out of ordinary, do not set off alarm
       }
       }
    }
 
  
  else if(c==2) {
    L=0;
    while (L==0) {
      digitalWrite(enablePin,HIGH); //turn motor on
      digitalWrite(controlPin1,LOW); //spin motor one way
      digitalWrite(controlPin2,HIGH); //spin motor one way
      digitalWrite(redLEDPin,HIGH);
      delay (5000);
      ++L;
    }
    while (L==1) {
      digitalWrite(enablePin,LOW); //turn motor on
      digitalWrite(controlPin1,LOW); //spin motor one way
      digitalWrite(controlPin2,LOW); //spin motor one way
      digitalWrite(greenLEDPin,HIGH);
      digitalWrite(redLEDPin,LOW);
      if (nfc.tagPresent()) { // Checking to see if tag is present
         c=++i; //increment counter if tag is tapped
         break; //break out of while loop and sift through code to find when counter is satisfied
      }
    }
    
    
    }
    
}


