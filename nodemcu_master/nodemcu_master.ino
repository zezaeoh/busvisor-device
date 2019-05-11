#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Wire.h>
//D0 16 D7  13
//D1 O5 D8  15
//D2 4  D9  3
//D3 0  D10 1
//D4 2  D11 9
//D5 14 D12 10
//D6 12
#define BUTTON_CHECK 16  //D1
#define COUNT_IN 14     //D5
#define COUNT_OUT 12    //D6
#define POWER_CHECK 13  //D7
#define SPEAKER_OUT 15  //D8
#define EMERGE_1 1
#define EMERGE_2 2
#define INTERVAL 10000
#define INF 10000
#define ON 1
#define OFF 0

//for state control
#define ERROR_CONTROL -1
#define START_DRIVING 0
#define WHILE_DRIVING 1
#define END_DRIVING 2
#define CHECK_EMERGENCY 3
#define WAIT_FOR_BUTTON_INPUT 5
#define END_EMERGENCY 6
#define DISPLAY_EMERGENCY_2_STATUS 8
#define WAIT_BUTTON_UNTILE_BATTERY_DIE 10

#define SEND_EMERGENCY_1_STATUS_TO_SERVER 4
#define SEND_EMERGENCY_2_STATUS_TO_SERVER 7
#define SEND_DRIVING_END_STATUS_TO_SERVER 9


const static char* DEVICE_ID = "sc13150";

const static char* SSIDNAME = "sn";
const static char* PASSWORD = "pw";
const static int WIFI_NUM = 2;

// for display
const static char* NOT_CONNECTED = "Connecting..";
const static int NOT_CONNECTED_LEN = 12;
const static char* CONNECTED = "Connected!!";
const static int CONNECTED_LEN = 11;
const static char* NOT_REGISTERD_1 = "Not Registered";
const static int NOT_REGISTERD_1_LEN = 14;
const static char* NOT_REGISTERD_2 = "Device!!";
const static int NOT_REGISTERD_2_LEN = 8;
const static char* DRIVE_MODE_START_1 = "Register checked";
const static int DRIVE_MODE_START_1_LEN = 16;
const static char* DRIVE_MODE_START_2 = "Start Driving";
const static int DRIVE_MODE_START_2_LEN = 13;
const static char* CURRENT_CHILD = "Current Child";
const static int CURRENT_CHILD_LEN = 13;
const static char* DRIVE_MODE_END = "Driving End";
const static int DRIVE_MODE_END_LEN = 11;
const static char* NETWORK_ERROR_DISP_1 = "Network Error!!";
const static int NETWORK_ERROR_DISP_1_LEN = 15;
const static char* NETWORK_ERROR_DISP_2 = "Trying to Fix...";
const static int NETWORK_ERROR_DISP_2_LEN = 16;
const static char* EMERGE_1_DISP_1 = "EMERGENCY!!";
const static int EMERGE_1_DISP_1_LEN = 11;
const static char* EMERGE_1_DISP_2 = "Push the button";
const static int EMERGE_1_DISP_2_LEN = 15;
const static char* EMERGE_2_DISP_1 = "EMERGENCY!!";
const static int EMERGE_2_DISP_1_LEN = 11;
const static char* EMERGE_2_DISP_2 = "Alert Sent";
const static int EMERGE_2_DISP_2_LEN = 10;
const static char* NORMAL_END_1 = "Nice Driving!";
const static int NORMAL_END_1_LEN = 13;
const static char* NORMAL_END_2 = "Have a Nice Day";
const static int NORMAL_END_2_LEN = 15;
const static char* TIMEOUT_DISP = "TIME OUT!!!";
const static int TIMEOUT_DISP_LEN = 11;
const static char* EMERGE_END_DISP_1 = "EMERGENCY END!";
const static int EMERGE_END_DISP_1_LEN = 14;
const static char* EMERGE_END_DISP_2 = "Good Job :)";
const static int EMERGE_END_DISP_2_LEN = 11;
static char DISP_CHAR_BUFFER[17];

// for http request
static char REQ_CHAR_BUFFER[50];
static char PARAM_CHAR_BUFFER[30];
const static char* BASE_URL = "http://52.231.67.172:8088/device/";

// for logic
static int httpCode = 0;
static unsigned char child_cnt = 0;
static int status_code;
static int status_code_stack;
static unsigned char tmp;
static long button_inf = 10000;

static unsigned long previousMillis = 0;
static unsigned long currentMillis = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial countSerial(COUNT_IN, COUNT_OUT);


bool requestGET(const char addr[]) {
  HTTPClient http;
  sprintf(REQ_CHAR_BUFFER, "%s%s", BASE_URL, addr);
  http.begin(REQ_CHAR_BUFFER);
  return (http.GET() == 200) ? true : false;
}

void displayToLcd(const char cs[], int len, int c) {
  lcd.clear();
  lcd.setCursor(0, c);
  for (int i = 0; i < len; i++)
    lcd.print(cs[i]);
}

void displayToLcd(const char cs1[], int len1, const char cs2[], int len2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  for (int i = 0; i < len1; i++)
    lcd.print(cs1[i]);

  lcd.setCursor(0, 1);
  for (int i = 0; i < len2; i++)
    lcd.print(cs2[i]);
}

void updateChildCount(int cnt) {
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++)
    lcd.print(' ');

  sprintf(DISP_CHAR_BUFFER, "NUM: %d", cnt);
  lcd.setCursor(0, 1);
  int t = 0;
  while(DISP_CHAR_BUFFER[t] != '\0')
    lcd.print(DISP_CHAR_BUFFER[t++]);
}

bool isPowerOn() {
  if (digitalRead(POWER_CHECK))
    return true;
  else
    return false;
}

void controlSpeaker(int speakerStatus) {
  if (speakerStatus)
    digitalWrite(SPEAKER_OUT, HIGH);
  else
    digitalWrite(SPEAKER_OUT, LOW);
}

//bool isButtonPressed(){
//  if(digitalRead(BUTTON_CHECK))
//    return true;
//  else
//    return false;
//}

void setup () {
  Serial.begin(115200);
  countSerial.begin(9600);
  Wire.begin();
  WiFi.begin(SSIDNAME, PASSWORD);
  lcd.init();
  lcd.backlight();
  lcd.display();
  pinMode(BUTTON_CHECK, INPUT);
  pinMode(SPEAKER_OUT, OUTPUT);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    displayToLcd(NOT_CONNECTED, NOT_CONNECTED_LEN, 0);
  }

  displayToLcd(CONNECTED, CONNECTED_LEN, 0);
  delay(1000);

  while (!requestGET(DEVICE_ID)) {
    displayToLcd(NOT_REGISTERD_1, NOT_REGISTERD_1_LEN, NOT_REGISTERD_2, NOT_REGISTERD_2_LEN);
    delay(5000);
  }

  displayToLcd(DRIVE_MODE_START_1, DRIVE_MODE_START_1_LEN, DRIVE_MODE_START_2, DRIVE_MODE_START_2_LEN);
  delay(5000);

  status_code = START_DRIVING;
}

void loop() {
  switch (status_code) {
    case ERROR_CONTROL: // error (maybe network)
      displayToLcd(NETWORK_ERROR_DISP_1, NETWORK_ERROR_DISP_1_LEN, NETWORK_ERROR_DISP_2, NETWORK_ERROR_DISP_2_LEN);
      delay(5000);
      if (requestGET(DEVICE_ID))
        status_code = status_code_stack;
      break;

    case START_DRIVING: // driving start
      displayToLcd(CURRENT_CHILD, CURRENT_CHILD_LEN, 0);
      sprintf(PARAM_CHAR_BUFFER, "%s/start", DEVICE_ID);

      if (requestGET(PARAM_CHAR_BUFFER)){
        status_code = 1;
        updateChildCount(0);
        countSerial.write(1);
      } else {
        status_code_stack = status_code;
        status_code = ERROR_CONTROL;
      }
      break;

    case WHILE_DRIVING: // while driving
      if (!isPowerOn()) {
        // check power: if power off then, status_code = 2;
        status_code = END_DRIVING;
      } else {
        if (countSerial.available() > 0) {
          tmp  = countSerial.read();
          if (tmp != child_cnt) {
            child_cnt = tmp;
            updateChildCount(child_cnt);
            sprintf(PARAM_CHAR_BUFFER, "%s/check/%d", DEVICE_ID, child_cnt);

            if (!requestGET(PARAM_CHAR_BUFFER)) {
              status_code_stack = status_code;
              status_code = ERROR_CONTROL;
            }
          }
        }
      }
      break;

    case END_DRIVING: // driving end
      displayToLcd(DRIVE_MODE_END, DRIVE_MODE_END_LEN, 0);
      status_code = SEND_DRIVING_END_STATUS_TO_SERVER;
      delay(5000);
      break;

    case CHECK_EMERGENCY: // check emergency
      if (child_cnt > 0) {
        displayToLcd(EMERGE_1_DISP_1, EMERGE_1_DISP_1_LEN, EMERGE_1_DISP_2, EMERGE_1_DISP_2_LEN);
        // turn on the speaker
        controlSpeaker(ON);
        status_code = SEND_EMERGENCY_1_STATUS_TO_SERVER;
      } else {
        displayToLcd(NORMAL_END_1, NORMAL_END_1_LEN, NORMAL_END_2, NORMAL_END_2_LEN);
        status_code = INF;
      }
      break;

    case SEND_EMERGENCY_1_STATUS_TO_SERVER: // send emergency status to server
      sprintf(PARAM_CHAR_BUFFER, "%s/emerg/%d", DEVICE_ID, EMERGE_1);

      if (!requestGET(PARAM_CHAR_BUFFER)) {
        status_code_stack = CHECK_EMERGENCY;
        status_code = ERROR_CONTROL;
      } else {
        previousMillis = millis();
        status_code = WAIT_FOR_BUTTON_INPUT;
      }
      break;

    case WAIT_FOR_BUTTON_INPUT: // wait for button input, check timeout
      currentMillis = millis();
      if (currentMillis - previousMillis >= INTERVAL) { // timeout
        displayToLcd(TIMEOUT_DISP, TIMEOUT_DISP_LEN, 0);
        status_code = SEND_EMERGENCY_2_STATUS_TO_SERVER;
      } else {
        // wait button input
        if (0) { // button pressed
          // stop the speaker
          controlSpeaker(OFF);
          status_code = END_EMERGENCY;
        }
      }
      break;

    case END_EMERGENCY: // emergency end
      displayToLcd(EMERGE_END_DISP_1, EMERGE_END_DISP_1_LEN, EMERGE_END_DISP_2, EMERGE_END_DISP_2_LEN);
      status_code = INF;
      break;

    case SEND_EMERGENCY_2_STATUS_TO_SERVER:
      sprintf(PARAM_CHAR_BUFFER, "%s/emerg/%d", DEVICE_ID, EMERGE_2);

      if (!requestGET(PARAM_CHAR_BUFFER)) {
        status_code_stack = status_code;
        status_code = ERROR_CONTROL;
      } else {
        status_code = DISPLAY_EMERGENCY_2_STATUS;
      }
      break;

    case DISPLAY_EMERGENCY_2_STATUS:
      displayToLcd(EMERGE_2_DISP_1, EMERGE_2_DISP_1_LEN, EMERGE_2_DISP_2, EMERGE_2_DISP_2_LEN);
      status_code = WAIT_BUTTON_UNTILE_BATTERY_DIE;
      break;

    case SEND_DRIVING_END_STATUS_TO_SERVER:
      sprintf(PARAM_CHAR_BUFFER, "%s/end", DEVICE_ID);

      if (!requestGET(PARAM_CHAR_BUFFER)) {
        status_code_stack = status_code;
        status_code = ERROR_CONTROL;
      } else
        status_code = CHECK_EMERGENCY;
      break;

    case WAIT_BUTTON_UNTILE_BATTERY_DIE: // wait for button until battery burn out
      // wait button input
      if (0) { // button Pressed
        // stop the speaker
        controlSpeaker(OFF);
        status_code = INF;
      }
      break;

    default:
      break;
  }
}
