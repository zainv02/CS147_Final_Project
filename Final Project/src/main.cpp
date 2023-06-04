#include <Arduino.h>
#include <vector>
#include "nvs.h"
#include "nvs_flash.h"
#include <WiFi.h>
#include <HttpClient.h>


// States for the buzzer
#define BUZZER_HIGH_SECURITY_ON_STATE 0
#define BUZZER_HIGH_SECURITY_OFF_STATE 1
#define BUZZER_DOOR_OPEN_ON_STATE 2
#define BUZZER_DOOR_OPEN_OFF_STATE 3
#define BUZZER_OFF_STATE 4

// Times for the buzzer
#define BUZZER_HIGH_SECURITY_ON_MILLIS 250
#define BUZZER_HIGH_SECURITY_OFF_MILLIS 100
#define BUZZER_DOOR_OPEN_ON_MILLIS 100
#define BUZZER_DOOR_OPEN_OFF_MILLIS 100

int buzzer_state = BUZZER_OFF_STATE; // Buzzer state
unsigned long buzzer_timer = 10; // Buzzer timer
bool buzzerPlaying = false;


// function declarations here:
void buzzerFunction();
void lightLevelFunction(double lightAnalogVal);
void doorFunction(int doorState);
void motionFunction(int motionState);
void buttonFunction();

void activateAlarm(int alarmCause);
void deactivateAlarm(int deactivateCause);

//the tone and noTone in the library just don't work. this is alternative
void tone(int pin, int freq)
{
  ledcAttachPin(pin, 0);
  ledcWriteNote(0, NOTE_F, 4);
  
}

void noTone(int pin)
{
  ledcDetachPin(pin);
}

void connectToSite(std::string pathString);
void sendTrigger(int alarmStatus, int triggerCause);
void nvs_access();
void fetchOptions();

const int photoPin = 36;
const int doorPin = 12;
const int buzzerPin = 13;
const int motionSensorPin = 25;

// button constants and booleans ------------------
#define BLUE 0
#define RED 1
#define YELLOW 2
#define GREEN 3

const int blueButtonPin = 37;
const int redButtonPin = 38;
const int yellowButtonPin = 39;
const int greenButtonPin = 32;

bool bluePushed = false;
bool redPushed = false;
bool yellowPushed = false;
bool greenPushed = false;

const int buttonDelay = 300;
unsigned long buttonTimer = 0;
//timer for amount of time system will wait for the buttons to be pushed
unsigned long buttonWaitTimer = 0;
//how long it will wait
const int buttonWaitDelay = 4000;

const std::vector<int> buttonCode =  {RED, YELLOW, BLUE, GREEN};
std::vector<int> enteredButtonCode;

//for the calibration delay ----------------------------------
const int CALIBRATION_TIME = 10000;
unsigned int delayStart;
bool calibrating;

const int lightChangeThreshold = 400;
int lightLastRead;
int lastLightLevel;

//Alarm causes ---------------------------------------------
#define DOORCAUSE 0
#define LIGHTCAUSE 1
#define MOTIONCAUSE 2
#define MANUALCAUSE 3

//Deactivation causes
#define CODECAUSE 0
#define REMOTECAUSE 1

//For sensor states -----------------------------------------
bool lightEnabled = true;
bool motionEnabled = true;
bool doorEnabled = true;
bool highSecurityMode = true;
const int lightEnabledDelay = 1000;
unsigned int long lightEnabledTimer = 0; 


//values for getting the calibration data and mapping to light sensor
int high = 0;
int low = 0;
int mapped_high = 1000;
int mapped_low = 0;
double slope;

int networkTimer = 0;
char ssid[50]; // your network SSID (name)
char pass[50]; // your network password (use for WPA, or use
// as key for WEP)

// Name of the server we want to connect to
const char kHostname[] = "52.34.174.82";

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

//accessing the username and password for wifi
void nvs_access() {
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
  err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  // Open
  Serial.printf("\n");
  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {
    Serial.printf("Done\n");
    Serial.printf("Retrieving SSID/PASSWD\n");
    size_t ssid_len;
    size_t pass_len;
    err = nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
    err |= nvs_get_str(my_handle, "pass", pass, &pass_len);
    switch (err) {
      case ESP_OK:
      Serial.printf("Done\n");
      Serial.printf("SSID = %s\n", ssid);
      Serial.printf("PASSWD = %s\n", pass);
      break;
      case ESP_ERR_NVS_NOT_FOUND:
      Serial.printf("The value is not initialized yet!\n");
      break;
      default:
      Serial.printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
  }
  // Close
  nvs_close(my_handle);
}

//connecting to server given by pathstring url
//returns the server's response
std::string connectToSite(char *pathString)
{
  int err = 0;
  WiFiClient c;
  HttpClient http(c);

  std::string response;


  //send request to server
  err = http.get(kHostname, 5000, pathString);
  if (err == 0) {
    Serial.println("startedRequest ok");
    err = http.responseStatusCode();
  if (err >= 0) {
    Serial.print("Got status code: ");
    Serial.println(err);
    // Usually you'd check that the response code is 200 or a
    // similar "success" code (200-299) before carrying on,
    // but we'll print out whatever response we get
    err = http.skipResponseHeaders();
    if (err >= 0) {
      int bodyLen = http.contentLength();
      Serial.print("Content length is: ");
      Serial.println(bodyLen);
      Serial.println();
      Serial.println("Body returned follows:");
      // Now we've got to the body, so we can print it out
      unsigned long timeoutStart = millis();
      char c;
      // Whilst we haven't timed out & haven't reached the end of the body
      while ((http.connected() || http.available()) &&
      ((millis() - timeoutStart) < kNetworkTimeout)) {
        if (http.available()) {
          c = http.read();
          // Print out this character
          Serial.print(c);
          response.push_back(c);
          bodyLen--;
          // We read something, reset the timeout counter
          timeoutStart = millis();
        } else {
          // We haven't got any data, so let's pause to allow some to
          // arrive
          delay(kNetworkDelay);
        }
      }
    } else {
      Serial.print("Failed to skip response headers: ");
      Serial.println(err);
      response.append(std::to_string(err));
    }
  } else {
    Serial.print("Getting response failed: ");
    Serial.println(err);
    response.append(std::to_string(err));
  }
  } else {
    Serial.print("Connect failed: ");
    Serial.println(err);
    response.append(std::to_string(err));
  }
  http.stop();

  return response;
}

//activates the buzzer and notifies server
void activateAlarm(int alarmCause)
{
  if(buzzer_state == BUZZER_OFF_STATE)
  {
    buzzer_state = BUZZER_HIGH_SECURITY_ON_STATE;
    Serial.print(alarmCause);
    Serial.print(" - Alarm Activated");
    if(alarmCause != MANUALCAUSE)
    {
      if(highSecurityMode)
      {
        sendTrigger(1, alarmCause);
      }
      else
      {
        sendTrigger(2, alarmCause);
      }
      
    }
    
  }
  
}

//notifies server of a sensor activation
void sendTrigger(int alarmStatus, int triggerCause)
{
  std::string path;

  //make url for trigger cause
  //url: ip/?trigger=[0, 1][0, 1, 2, 3]
  //0 is alarm off
  //1 is alarm on
  //0 door, 1 light, 2 motion, 3 manual
  path.append("/?trigger=");
  path.append(std::to_string(alarmStatus));
  path.append(std::to_string(triggerCause));
  char *pathString = &(path[0]);
  std::string response = connectToSite(pathString);
  //Serial.println(response);
}

//get the options from the server
void fetchOptions()
{

  std::string path;
  std::string response;

  path.append("/options");
  char *pathString = &(path[0]);
  response = connectToSite(pathString);
  //Serial.println(response);
  int fetchedAlarmState = int(response[0] - '0');
  int fetchedSecurityMode = int(response[1]- '0');
  int fetchedDoorState = int(response[2]- '0');
  int fetchedMotionState = int(response[3]- '0');
  int fetchedLightState = int(response[4]- '0');

  
  //do if checks for all of them to determine what should be enabled/disabled? or put them in global variables?
  Serial.println();
  Serial.println(fetchedAlarmState);
  if(fetchedAlarmState == 0)
  {
    deactivateAlarm(1);
    Serial.println("deactivating");
  }
  else
  {
    activateAlarm(3);
  }


  doorEnabled = fetchedDoorState;
  motionEnabled = fetchedMotionState;
  lightEnabled = fetchedLightState;
  highSecurityMode = fetchedSecurityMode;

}

//turns off alarm and notifies server
void deactivateAlarm(int deactivateCause)
{
  if(buzzer_state != BUZZER_OFF_STATE)
  {
    buzzer_state = BUZZER_OFF_STATE;
    Serial.print(deactivateCause);
    Serial.print(" - Alarm Deactivated");
    if(deactivateCause != REMOTECAUSE)
    {
      sendTrigger(0, deactivateCause);
    }
    
  }
  
}

//controls the buzzer
void buzzerFunction()
{
  //Serial.print(buzzer_state);
  switch (buzzer_state)
  {
    case BUZZER_HIGH_SECURITY_ON_STATE:
      if(!buzzerPlaying)
      {
        buzzerPlaying = true;
        tone(buzzerPin, 1000);
      }
      
      
      if(millis() >= buzzer_timer){
        buzzer_timer = millis() + BUZZER_HIGH_SECURITY_OFF_MILLIS;
        buzzer_state = BUZZER_HIGH_SECURITY_OFF_STATE;
      }

      break;

    case BUZZER_HIGH_SECURITY_OFF_STATE:
      if(buzzerPlaying)
      {
        noTone(buzzerPin);
        buzzerPlaying = false;
      }
      
      //Serial.println('-');
      if(millis() >= buzzer_timer)
      {
        buzzer_timer = millis() + BUZZER_HIGH_SECURITY_ON_MILLIS;
        buzzer_state = BUZZER_HIGH_SECURITY_ON_STATE;
      }


      break;

    case BUZZER_OFF_STATE:
      if(buzzerPlaying)
      {
        noTone(buzzerPin);
        buzzerPlaying = false;
      }
      break;

    default:
      break;
  }
}

//checks light level and activates alarm if the light level increases
// by a significant amount
void lightLevelFunction(double lightAnalogVal)
{
  //do calibration for CALIBRATION_TIME seconds
  if (!calibrating || millis() - delayStart > CALIBRATION_TIME)
  {
    //we are now done calibrating
    if(calibrating)
    {
      //this is run only when we finish calibrating and never again
      Serial.println("Finished Calibrating");
      Serial.print("High = ");
      Serial.println(high);
      Serial.print("Low = ");
      Serial.println(low);

      //converting our analog value to a value between mapped_low and mapped_high (0 to 179)
      slope = 1.0 * (mapped_high - mapped_low) / (high - low); //times 1.0 since we want it to be a double
      Serial.print("Slope = ");
      Serial.println(slope);
      Serial.println(slope * lightAnalogVal);
      double output = mapped_low + slope * (lightAnalogVal - low);
      Serial.println(output);
      
    }
    //make it so we don't run the slope calculation after we already did
    calibrating = false;

    //if it has been enough time since the last reading of the light level
    if(millis() - lightLastRead >= 1000)
    {
      double output = mapped_low + slope * (lightAnalogVal - low); //this is the servo value
      lightLastRead = millis();

      if(millis() >= lightEnabledTimer) //to ensure alarm doesn't go off immediately after sensor is enabled
      {
        //if the amount of light level since last recorded changes by the lightChangeThreshold, sound the alarm
        if( std::abs(output - lastLightLevel) >= lightChangeThreshold && lastLightLevel != 0)
        {
          Serial.print(output);
          Serial.print("  ");
          Serial.print(lastLightLevel);
          Serial.print(" ALARM");
          Serial.println("");
          activateAlarm(LIGHTCAUSE);
          
        }
        
      }
      lastLightLevel = output;
      
      
    }

  }
  else
  {
    //This is run during calibration. We get the highest detect light level and lowest detected light level
    //We will use these values to map the light level range to the servo angle range
    if(lightAnalogVal > high)
    {
      high = lightAnalogVal;
    }

    if(lightAnalogVal < low)
    {
      low = lightAnalogVal;
    }
  }
}

//activates alarm if door is opened
void doorFunction(int doorState)
{
  if(doorState == 1)
  {
    //Serial.println("Door Open");
    activateAlarm(DOORCAUSE);
  }
  else
  {
    //Serial.println("Door Closed");
    
  }
}

//activates alarm if motion is detected
void motionFunction(int motionState)
{
  //Serial.println(motionState);
  if(motionState == HIGH)
  {
    //Serial.println("Motion Detected");
    activateAlarm(MOTIONCAUSE);
  }
}

//controls the button for entering the security pin
void buttonFunction()
{
  if(millis() >= buttonTimer )
  {

    if(digitalRead(redButtonPin) == HIGH)
    {
      if(!redPushed)
      {  
        if(millis() > buttonWaitTimer && millis() >= buttonTimer)
          buttonWaitTimer = millis() + buttonWaitDelay;

        Serial.println("Red Button Pushed");
        redPushed = true;
        enteredButtonCode.push_back(RED);
        buttonTimer = millis() + buttonDelay;
      }
    }
    else
    {
      redPushed = false;
    }

    if(digitalRead(blueButtonPin) == HIGH)
    {
      if(!bluePushed && millis() >= buttonTimer)
      {
        if(millis() > buttonWaitTimer)
          buttonWaitTimer = millis() + buttonWaitDelay;

        Serial.println("Blue Button Pushed");
        bluePushed = true;
        enteredButtonCode.push_back(BLUE);
        buttonTimer = millis() + buttonDelay;
      }
    }
    else
    {
      bluePushed = false;
    }

    if(digitalRead(yellowButtonPin) == HIGH)
    {
      if(!yellowPushed && millis() >= buttonTimer)
      {
        if(millis() > buttonWaitTimer)
          buttonWaitTimer = millis() + buttonWaitDelay;

        Serial.println("Yellow Button Pushed");
        yellowPushed = true;
        enteredButtonCode.push_back(YELLOW);
        buttonTimer = millis() + buttonDelay;
      }
      
    }
    else
    {
      yellowPushed = false;
    }

    if(digitalRead(greenButtonPin) == HIGH)
    {
      if(!greenPushed && millis() >= buttonTimer)
        {
          if(millis() > buttonWaitTimer)
            buttonWaitTimer = millis() + buttonWaitDelay;

          Serial.println("Green Button Pushed");
          greenPushed = true;
          enteredButtonCode.push_back(GREEN);
          buttonTimer = millis() + buttonDelay;
        }
    }  
    else
    {
      greenPushed = false;
    }


  }


  if(enteredButtonCode.size() == buttonCode.size())
  {
    bool match = true;
    for (int i = 0; i < buttonCode.size(); i++)
    {
      if(enteredButtonCode.at(i) != buttonCode.at(i))
      {
        match = false;
        break;
      }
    }

    if(match)
    {
      Serial.println("Turn off alarm");
      deactivateAlarm(CODECAUSE);
    }
    else
    {
      Serial.println("Wrong Code");
    }
    enteredButtonCode.clear();
  }
    
  if(enteredButtonCode.size() > buttonCode.size())
  {
    Serial.println("Wrong code");
    enteredButtonCode.clear();
  }

}

void setup() {

  // Retrieve SSID/PASSWD from flash before anything else
  delay(3000);
  nvs_access();
  delay(3000);
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  pinMode(photoPin, INPUT);
  pinMode(doorPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(motionSensorPin, INPUT);

  pinMode(blueButtonPin, INPUT);
  pinMode(redButtonPin, INPUT);
  pinMode(yellowButtonPin, INPUT);
  pinMode(greenButtonPin, INPUT);
  
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
  

  //start the calibration timer
  delayStart = millis();
  calibrating = true;
  Serial.println();
  Serial.println("Calibrating light sensor");
  
}


void loop() {

  
  int lightAnalogVal =analogRead(photoPin);
  int doorState = digitalRead(doorPin);
  int motionVal = digitalRead(motionSensorPin);

  if(highSecurityMode)
    buzzerFunction();

  if(lightEnabled)
  {
    lightLevelFunction(lightAnalogVal);
  }
  else
  {
    lightEnabledTimer = millis() + lightEnabledDelay;
  }
    

  if(doorEnabled)
    doorFunction(doorState);

  if(motionEnabled)
    motionFunction(motionVal);

  buttonFunction();

  if(millis() > networkTimer && millis() > buttonWaitTimer)
  {
    fetchOptions();
    networkTimer = millis() + 5000;
  }

  
  

  


  
}