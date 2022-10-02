//version includes bluetooth

//bluetooth imports (Software Serial)
#include <SoftwareSerial.h>
#define rxPin 8
#define txPin 7

#include <QueueList.h>
#include <avr/interrupt.h>
//#include <avr/sleep.h>
#include <LowPower.h>
//#include <avr/power.h>

// setting all variables
// constants won't change. They're used here to set pin numbers:
const int buttonPin = 2;    // the number of the pushbutton pin
const int unlockPin = 11;      // number of pin connected to *unlock* on remote
const int ledunlockPin = 6;      // number of pin connected to *unlock* on remote
const int lockPin = 12;      // number of pin connected to *lock* on remote
const int ledlockPin = 9;      // number of pin connected to *lock* on remote
const int ledPin = 3;      // the number of the LED pin
const int btPin1 = 10;      // one pin powering bluetooth (bt)
const int btPin2 = 4;      // one pin powering bluetooth (bt)
const int btPin3 = 5;      // one pin powering bluetooth (bt)

// the following variables are unsigned longs because the time, measured in
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 40;    // the debounce time; increase if the output flickers

// lists and other variables
//password goes backwards, read right to left
long passwordList[6] = {1000, 500, 500, 500, 500, 500};
long temp_password[6] = {0, 0, 0, 0, 0, 0};

//bluetooth software serial
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

// main temporary variables
char incoming_val = 0;
int button = 0;
int last_button = 0;
bool lock_timer_start = false;
unsigned long lock_clock_time = millis();
unsigned long idle_clock_time = millis();
int button_state = 0;
long button_press_value = 0;
bool unlockBool = true;
long passwordEnterList[6] = {100000, 100000, 100000, 100000, 100000, 100000};
int element_count = 0;
bool validBool = true;
int programmingMode = 0; //0 = false, 1 = true

//functions/procedures -----------------------------------------

//button detect function 
int GPIOBUTTONDETECT(){
  if (digitalRead(buttonPin) == HIGH){
    button = 1;
  }  
  
  else{
    button = 0;
  }

  if ((button == 1) && (last_button == 0)){
    last_button = button;
    return (1);
  }
   
  else if ((button == 0) && (last_button == 1)){
    last_button = button;
    return (2);
  }

  else {
    return (0);
  }
}

//push function
int pushEnterList(int newVal){
  for (int i = 5; i > 0; i--){
    passwordEnterList[i] = passwordEnterList[i-1];
  }
  passwordEnterList[0] = newVal;
  digitalWrite(ledPin, HIGH);
  delay(50);
  digitalWrite(ledPin, LOW);
}

//sleep function
void sleepNow(){
  digitalWrite(btPin1, LOW);
  digitalWrite(btPin2, LOW);
  digitalWrite(btPin3, LOW);
  pinMode(txPin, INPUT);
  delay(15);
  //sleep_enable();
  attachInterrupt(digitalPinToInterrupt(buttonPin), wakeUpNow, HIGH); //set button 2 to wake up arduino
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
//  power_spi_disable();    // Disable the Serial Peripheral Interface module.
//  power_timer0_disable(); // Disable the Timer 0 module.
//  power_timer1_disable(); // Disable the Timer 1 module.
//  power_timer2_disable(); // Disable the Timer 2 module  
//  power_twi_disable();    // Disable the Two Wire Interface module.  
//  power_usart0_disable(); // Disable the USART 0 module.
//  power_adc_disable();    // Disable the Analog to Digital Converter module
//  set_sleep_mode (SLEEP_MODE_PWR_DOWN); //set sleep mode
//  sleep_mode(); //actually sleeps
  
  //sleeping at this time, when inturruped by button, code below runs
  //sleep_disable();
  detachInterrupt(digitalPinToInterrupt(buttonPin));
//  power_spi_enable();    // Enable the Serial Peripheral Interface module.
//  power_timer0_enable(); // Enable the Timer 0 module..
//  power_timer1_enable(); // Enable the Timer 1 module.
//  power_timer2_enable(); // Enable the Timer 2 module  
//  power_twi_enable();    // Enable the Two Wire Interface module.  
//  power_usart0_enable(); // Enable the USART 0 module.
//  power_adc_enable();    // Enable the Analog to Digital Converter module
  

  // use leds to signal mat on
  digitalWrite(ledlockPin, HIGH);
  digitalWrite(ledunlockPin, HIGH);
  digitalWrite(ledPin, HIGH);

  delay(1000);

  digitalWrite(ledlockPin, LOW);
  digitalWrite(ledunlockPin, LOW);
  digitalWrite(ledPin, LOW);
  
  idle_clock_time = millis();
}

//wake up function
void wakeUpNow()  //This is the code that runs when the interrupt button is pressed and interrupts are enabled
{
  idle_clock_time = millis();
  digitalWrite(btPin1, HIGH);
  digitalWrite(btPin2, HIGH);
  digitalWrite(btPin3, HIGH);
  pinMode(txPin, OUTPUT);
  digitalWrite(txPin, HIGH);
}


//----------------------------------------------------------------//

void setup() {
  pinMode(btPin1, OUTPUT);
  pinMode(btPin2, OUTPUT);
  pinMode(btPin3, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(lockPin, OUTPUT);
  pinMode(unlockPin, OUTPUT);
  pinMode(ledlockPin, OUTPUT);
  pinMode(ledunlockPin, OUTPUT);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
    
  // Set the baud rate for the SoftwareSerial object
  mySerial.begin(9600);
  Serial.begin(9600);

  digitalWrite(btPin1, HIGH);
  digitalWrite(btPin2, HIGH);
  digitalWrite(btPin3, HIGH);
  
  // set LED state to high to signla mat is on
  digitalWrite(ledlockPin, HIGH);
  digitalWrite(ledunlockPin, HIGH);
  digitalWrite(ledPin, HIGH);

  delay(500);

  digitalWrite(ledlockPin, LOW);
  digitalWrite(ledunlockPin, LOW);
  digitalWrite(ledPin, LOW);

  //mySerial.print("AT");
  //delay(1000);
  //mySerial.print("AT+NAMECONDUITS_Mat");

}

void loop() {
  if (programmingMode != 0){
    digitalWrite(ledlockPin, HIGH);
    digitalWrite(ledunlockPin, HIGH);
  }
  else{
    digitalWrite(ledlockPin, LOW);
    digitalWrite(ledunlockPin, LOW);
  }
  //changing passcode sequence
  if (programmingMode == 1){
    // use leds to signal mat on
    //input CURRENT password ---------------------------------------------
    button_state = GPIOBUTTONDETECT();
    if (button_state == 1){
      lock_clock_time = millis();
    }
    
    if (button_state == 2){
      pushEnterList(millis()-lock_clock_time);
    }
    
    if ((millis()-lock_clock_time >= 5000) && (button_state == 0)){
      //tx 'd' - timeout
      Serial.print("timeout: ");
      Serial.println(millis()-lock_clock_time);
      mySerial.print('d');
      programmingMode = 0;
      idle_clock_time = millis();
      lock_clock_time = millis();
      Serial.println(programmingMode);
    }

    unlockBool = true;
    for(int i = 0; i < 6; i++){
      if ((abs(passwordList[i] - passwordEnterList[i]) > 500) && (passwordList[i] != 0)){ //changes error margin
        unlockBool = false;
      }
    }

    if (unlockBool){
      //tx 'p'
      mySerial.print('p');

      //reset passwordEnterList
      for(int i = 0; i < 6; i++){
        passwordEnterList[i] = 0;
      }
      programmingMode = 2;
      Serial.println(programmingMode);
    }
    
    //if cancel is sent then exit both loop
    if(mySerial.available()>0){
      incoming_val = mySerial.read();
      if (incoming_val == '2'){
        programmingMode = 0;
        lock_clock_time = millis();
        Serial.println(programmingMode);
      }
    }
  }
    
    //input NEW password ----------------------------------------------
  if (programmingMode == 2){
    button_state = GPIOBUTTONDETECT();
    
    if (button_state == 1){
      lock_clock_time = millis();
    }
    
    if (button_state == 2){
      if (millis()-lock_clock_time < 500){
        pushEnterList(550);
      }
      else{
        pushEnterList(millis()-lock_clock_time);
      } 
    }
    
    validBool = true;
    if ((millis()-lock_clock_time >= 5000) && (button_state == 0)){
      element_count = 0;
      for (int i = 0; i<6; i++){
        if (passwordEnterList[i] != 0){
          element_count++;
        }
        if (passwordEnterList[i] >= 2500){
          mySerial.print('d');
          programmingMode = 0;
          idle_clock_time = millis();
          lock_clock_time = millis();
          Serial.println(programmingMode);
          validBool = false;
        }
      }

      if ((element_count>4) && (validBool)){
        Serial.println("temp password");
        //save password to temp_password if password is valid
        for (int i=0; i<6; i++){
          temp_password[i] = passwordEnterList[i];
          Serial.print(temp_password[i]);
          Serial.print(", ");
        }
        mySerial.print('a');
        //reset
        lock_clock_time = millis();
        for (int i=0; i<6; i++){
          passwordEnterList[i] = 0;
        }
        programmingMode = 3;
        Serial.print("");
        Serial.print(programmingMode);
      }
    }
    //check for cancel
    if(mySerial.available()>0){
      incoming_val = mySerial.read();
      if (incoming_val == '2'){
        programmingMode = 0;
        lock_clock_time = millis();
        Serial.println(programmingMode);
      }
    }
  }

   //CONFRIM new password ----------------------------------------------
    if (programmingMode == 3){
      //"unlock" new password ----------------------------------------------
      button_state = GPIOBUTTONDETECT();
      if (button_state == 1){
        lock_clock_time = millis();
      }
      
      if (button_state == 2){
        pushEnterList(millis()-lock_clock_time);
      }

      if ((millis()-lock_clock_time >= 5000) && (button_state == 0)){
        //tx 'd' - timeout
        mySerial.print('d');
        programmingMode = 0;
        lock_clock_time = millis();
        idle_clock_time = millis();
        Serial.println(programmingMode);
      }
      
      unlockBool = true;
      for(int i = 0; i < 6; i++){
        if ((abs(temp_password[i] - passwordEnterList[i]) > 500) && (temp_password[i] != 0)){ //changes error margin, ignores zeroes in password
          unlockBool = false;
        }
      }
  
      if (unlockBool){
        //tx 'p'
        Serial.println("validation input");
        for(int i = 0; i < 6; i++){
          Serial.print(passwordEnterList[i]);
          Serial.print(", ");
        }
        for(int i = 0; i < 6; i++){
          passwordList[i] = temp_password[i];
        }
        //reset passwordEnterList
        for(int i = 0; i < 6; i++){
          passwordEnterList[i] = 0;
        }
        mySerial.print('c');
        delay(2000);
        Serial.print("");
        Serial.println("PASSWORD ACCEPTED");
        programmingMode = 0;
        lock_clock_time = millis();
        idle_clock_time = millis();
        if (programmingMode == 0){
        digitalWrite(ledlockPin, LOW);
        digitalWrite(ledunlockPin, LOW);
        }
      }
  }

//lock and unlock sequences -----------------------------------------------------------
  if (programmingMode == 0){
    if(mySerial.available()>0)
    {
      Serial.println("INCOMING");
      incoming_val = mySerial.read();
      if (incoming_val == '1'){
        programmingMode = 1;
        Serial.println(programmingMode);
        lock_clock_time = millis();
      }
      idle_clock_time = millis();
    
   }
    button_state = GPIOBUTTONDETECT();
    
    if (button_state == 2){
      if (lock_timer_start){
        button_press_value = millis() - lock_clock_time;
      }
  
        //lock code
        if (button_press_value >= 3000){
          digitalWrite(lockPin, HIGH);
          digitalWrite(ledlockPin, HIGH);
          delay(3000);
          digitalWrite(lockPin, LOW);
          digitalWrite(ledlockPin, LOW);
          for(int i = 0; i < 6; i++){
            passwordEnterList[i] = 0;
          }
        }
  
        else {
          //if there are 6 digits, add new digit, remove earliest chosen digit
          //uses function pushEnterList
          pushEnterList(button_press_value);
        }
  
        lock_timer_start = false;
        idle_clock_time = millis();
    }
  
    if (button_state == 1){
      lock_timer_start = true;
      lock_clock_time = millis();
      idle_clock_time = millis();
    }
  
    if (button_state == 0){
      if (millis() - idle_clock_time >= 7500){ //idle time before sleeping
        sleepNow();
      }
    }
  
    unlockBool = true;
    for(int i = 0; i < 6; i++){
      if ((abs(passwordList[i] - passwordEnterList[i]) > 500) && (passwordList[i] != 0)){ //changes error margin
        unlockBool = false;
      }
    }
  
    if (unlockBool){
      digitalWrite(unlockPin, HIGH);
      digitalWrite(ledunlockPin, HIGH);
      delay(3000);
      digitalWrite(unlockPin, LOW);
      digitalWrite(ledunlockPin, LOW);
      for(int i = 0; i < 6; i++){
        passwordEnterList[i] = 100000;
      }
    }
      
      delay(50);
  }
}
