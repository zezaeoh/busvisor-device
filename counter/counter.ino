#include <SoftwareSerial.h>

#define STATUS_COUNTING 0
#define STATUS_SERIAL 1
#define RX 10
#define TX 11

SoftwareSerial mySerial(RX,TX); //RX, TX

const static int sensorPin = A1;
static int sensorValue = 0;
const static int sensorPin2 = A2;
static int sensorValue2 = 0;
static int flag1[2] = {1,};
static int flag2[2] = {1,};
static int previous_count = 0;
static int count = 0;
static int status_code = 0;


void setup(){
  Serial.begin(115200);
  mySerial.begin(9600);
  pinMode(sensorPin, INPUT);
  pinMode(sensorPin2, INPUT);
}

void loop(){
  if (mySerial.available() > 0){
    mySerial.read();
    count = 0;
  }
  
  switch(status_code){
    case STATUS_COUNTING:
      counting();
      if(isCountChanged()){
        //send count
        status_code = 1;
      }
//      Serial.print("COUNTER: ");
//      Serial.println(count);
      break;
    case STATUS_SERIAL: //send master the count through sw serial
      mySerial.write(count);
//      Serial.print("COUNTER: ");
//      Serial.println(count);
      previous_count = count;
      status_code = 0;
      break;
    default:
    break;
  }

  
}

void counting(){
  if(IsObjectInFrontOfSensor1()){
    flag1[0] = flag1[1];
    flag1[1] = 0; 
  }else{
    flag1[0] = flag1[1];
    flag1[1] = 1;
  }
  
  if(IsObjectInFrontOfSensor2()){
    flag2[0] = flag2[1];
    flag2[1] = 0;
  }else{
    flag2[0] = flag2[1];
    flag2[1] = 1;
  }
  delay(10);

  if( (flag1[0]+flag2[0]) == 0){
    if(flag1[1] + flag2[1] == 1){
      if(flag1[1] == 1){
        previous_count = count;
        count++;
      }else {
        if(count > 0){
          previous_count = count;
          count--;
        }
      }
    }
  }
}

bool isCountChanged(){
  if(previous_count == count)
    return false;
  else
    return true;
}
bool IsObjectInFrontOfSensor1(){
  sensorValue = analogRead(sensorPin);
//  Serial.print("Sensor1: ");
//  Serial.println(sensorValue);
  if(sensorValue <= 50){
    return true;
  }else{
    return false;
  }
}

bool IsObjectInFrontOfSensor2(){
  sensorValue2 = analogRead(sensorPin2);
//  Serial.print("\tSensor2: ");
//  Serial.println(sensorValue2);
  if(sensorValue2 <= 50){
    return true;
  }else{
    return false;
  }
}
