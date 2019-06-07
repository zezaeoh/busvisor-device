#include <SoftwareSerial.h>

#define STATUS_COUNTING 200
#define STATUS_SERIAL 201
#define RX 10
#define TX 11
SoftwareSerial mySerial(RX,TX); //RX, TX
//for flag
#define INIT 0
#define IN_1 1
#define IN_2 2
#define IN_3 3
#define IN_LAST 4
#define OUT_1 5
#define OUT_2 6
#define OUT_3 7
#define OUT_LAST 8

#define COUNT_TEST 200
#define COUNT_UP 1
#define COUNT_DOWN 2
#define COUNT_INIT 10
 
const static int THREHOLD = 50;
const int sensorPin0 = A1;
int sensorValue0 = 0;
const int sensorPin1 = A2;
int sensorValue1 = 0;
const int sensorPin2 = A3;
int sensorValue2 = 0;

const int LED0 = 2;
const int LED1 = 3;
const int LED2 = 4;
static int input = -1;

static unsigned char stat = 0b000;
static int cnt = 0;
static int flag = 0;

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);
  pinMode(sensorPin0, INPUT);
  pinMode(sensorPin1, INPUT);  
  pinMode(sensorPin2, INPUT);  

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}

void loop() {
  updateStatus();
  
  if(stat == 0b000 && flag != COUNT_TEST){
    if(flag == IN_LAST){
      countContol(COUNT_UP);
    }else if(flag == OUT_LAST){
      countContol(COUNT_DOWN);
    }
    flag = INIT;
  }
  
  switch(flag){
    case INIT:
    if(stat & 0b100){
      flag = IN_1;
    }
    else if(stat & 0b001){
      flag = OUT_1;
    }
    break;
    
    case IN_1:
    if(stat & 0b010){
      flag = IN_2;
    }
    break;
    
    case IN_2:
    if(stat & 0b001){
      flag = IN_3;
    }
    break;
    
    case IN_3:
    if(stat == 0b001){
      flag = IN_LAST;
    }
    break;
    
    case IN_LAST:
    if(stat & 0b010){
      flag = IN_3;
    }
    break;
    
    case OUT_1:
    if(stat & 0b010){
      flag = OUT_2;
    }
    break;
    
    case OUT_2:
    if(stat & 0b100){
      flag = OUT_3;
    }
    break;
    
    case OUT_3:
    if(stat == 0b100){
      flag = OUT_LAST;
    }
    break;

    case OUT_LAST:
    if(stat & 0b010){
      flag = OUT_3;
    }
    break;

    case COUNT_TEST:
    light();
    break;
    
    default:
    break;
  }

  if (mySerial.available() > 0){
    input = mySerial.read();
    
    switch(input){
      case COUNT_UP: case COUNT_DOWN: case COUNT_INIT:
      countContol(input);
      break;

      case COUNT_TEST:
      if(flag == COUNT_TEST){
        flag = INIT;
        lightOff();
      }
      else
        flag = COUNT_TEST;
      break;
      
      default:
      break;
    } 
  }
//  Serial.print("status: ");
//  Serial.println(stat);
//  Serial.print("\tflag: ");
//  Serial.println(flag);
//  Serial.print("\t\tcount: ");
//  Serial.println(cnt);
}

void updateStatus(){
  stat = 0b000;
  if(IsObjectInFrontOfSensor0()){
    stat |= 0b100;
  }
  if(IsObjectInFrontOfSensor1()){
    stat |= 0b010;
  }
  if(IsObjectInFrontOfSensor2()){
    stat |= 0b001;
  }
}


bool IsObjectInFrontOfSensor0(){
  sensorValue0 = analogRead(sensorPin0);
//  Serial.print("\tSensor0: ");
//  Serial.println(sensorValue0);
  delay(1);
  if(sensorValue0 <= THREHOLD){
    return true;
  }else{
    return false;
  }
}

bool IsObjectInFrontOfSensor1(){
  sensorValue1 = analogRead(sensorPin1);
//  Serial.print("\tSensor1: ");
//  Serial.println(sensorValue1);
  if(sensorValue1 <= THREHOLD){
    return true;
  }else{
    return false;
  }
}

bool IsObjectInFrontOfSensor2(){
  sensorValue2 = analogRead(sensorPin2);
//  Serial.print("\tSensor2: ");
//  Serial.println(sensorValue2);
  if(sensorValue2 <= THREHOLD){
    return true;
  }else{
    return false;
  }
}

void light(){
  Serial.println("light()");
  if(stat & 0b100){
    digitalWrite(LED0, LOW);
  }else{
    digitalWrite(LED0, HIGH);
  }
  if(stat & 0b010){
//    Serial.println("digitalWrite(LED0, LOW)");
    digitalWrite(LED1, LOW);
  }else{
//    Serial.println("digitalWrite(LED0, HIGH)");
    digitalWrite(LED1, HIGH);
  }
  if(stat & 0b001){
    digitalWrite(LED2, LOW);
  }else{
    digitalWrite(LED2, HIGH);
  }
}
void lightOff(){
  digitalWrite(LED0, LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
}

void countContol(int cc){
  switch(cc){
    case COUNT_UP:
    cnt++;
    break;

    case COUNT_DOWN:
    if(cnt > 0) cnt--;
    break;

    case COUNT_INIT:
    cnt = 0;
    break;

    default:
    break;
  }
  mySerial.write(cnt);
}
