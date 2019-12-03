#include "./resources/sensor.h"
#include "./resources/eeprom.h"
#include <Wire.h>
#include <stdlib.h>

//LoRa  ------------------------------
#include <LoRa.h>
#include <SPI.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2
// ------------------------------------
//MOTOR SETUP ///////////////////////////////////////////
//Encoder Pins
#define encoder_a 32

#define STATUS_LED 33

int count = 0; // encoder count
int motor_status = 0;
int current_mode= 2; // default to autonomous

// Handle Rx commands
char rx_command  = '\0';
int  rx_argument = 0;
bool displayStatus = false;

// Motor A
int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 13; 

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 0; // was 200

// Motor ISR
// void EncoderEvent() {
//   count++;
//   //Serial.print(count);
//   if (count == 500){
//     // stop the motor
//     dutyCycle = 0;
//     ledcWrite(pwmChannel,dutyCycle);
//     motor_status = 0;
//     count = 0;
//     // Serial.println("Count is 500");
//     // //Serial.print(count);
//   }
//   if(count%100==0){
//     Serial.printf("Encoder count is %d\n",count);
//   }
// }

sensor myData; 
eeprom myEEPROM; //create instance of eeprom
int next_addr;   // next location to write to on EEPROM - must be global for 1 time init
int test_addr = 0;

void onLoRaRx(int packetSize){
  char rx_data[10];
  memset(rx_data,0,10*sizeof(char)); // initialize
  Serial.println("LoRa packet received.");
  while (LoRa.available()) {
    LoRa.readBytesUntil('\n',rx_data,10);
    rx_command  = rx_data[0];
    rx_argument = atoi(rx_data+1);
    //memcpy(&rx_argument,((char*)rx_data+1),4); // get int argument

    Serial.printf("LoRa Packet: %s\n", rx_data);
    
    //String LoRaData = LoRa.readString();
    //Serial.println(LoRaData); 
  }
}

void setup() {

  // Motor //////////////////////////////////////////////
  pinMode(encoder_a, INPUT);

  // sets the pins as outputs:
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  
  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(enable1Pin, pwmChannel);
  dutyCycle = 200;
  ledcWrite(pwmChannel, dutyCycle);
  // attachInterrupt(digitalPinToInterrupt(encoder_a), EncoderEvent, RISING);
  delay(10);
  // moves in clockwise direction (down)
  digitalWrite(motor1Pin1,HIGH);
  digitalWrite(motor1Pin2,LOW);

  // STATUS_LED init
  digitalWrite(STATUS_LED, LOW);

  Wire.begin();
  Serial.begin(9600);
  delay(10);
  Serial.println(); // get past the noise of monitor setup

  if(myEEPROM.init()){
    Serial.println("EEPROM Initialized!");
    //next_addr = myEEPROM.read_next_address();
    next_addr = 32;

    myEEPROM.read_config_raw();

        // TESTING THE EEPROM
        // byte temp_arr[] = {'4','8','2','6','0','a','b','\0'}; // 32 bytes of char format
        // //myEEPROM.page_write(0,temp_arr,32);
        // delay(500);
        // byte temp_arr2[32];
        // myEEPROM.page_read(0,temp_arr2,32);
        // for(int j = 0; j<7; j++){
        //   temp_arr2[j] += 1;
        // }
        //printf("Testing EEPROM page operations...\n%s\n",temp_arr2);

        // TEST INDIVIDUAL GETTERS and SETTERS
        // id
        //myEEPROM.write_id(73);
        //Serial.printf("EEPROM ID test (73 expected): %d\n",(int)myEEPROM.read_id());
        // mode
        //myEEPROM.write_mode(254);
        //Serial.printf("EEPROM mode test (254 expected): %d\n",(int)myEEPROM.read_mode());
        // addr
        myEEPROM.write_next_address(32);
        //Serial.printf("EEPROM next_addr test (32 expected): %d\n",(int)myEEPROM.read_next_address());
        // interval
        //myEEPROM.write_interval(498);
        //Serial.printf("EEPROM interval test (498 expected): %d\n",(int)myEEPROM.read_interval());
  } // end if EEPROM init

  //Initialize sensors - resets and calibrates sensors
  Serial.print("Initializing Sensors");
  if (myData.init()){
    Serial.print(".");
    delay(1000); // small delay between function calls
  }
  Serial.println("");
  Serial.println("Sensor Init Complete!");

  //Lets see what I2C devices are out there... after we have already initialized our sensors
  myData.i2c_scan();

  //LoRa Init ------------
  //setup LoRa transceiver module
  //915E6 for North America
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xF3);
  LoRa.receive();
  LoRa.onReceive(onLoRaRx);
  Serial.println("LoRa Initializing OK!");
  Serial.println("Starting Buoy Program");

  //printf("Sizeof int: %d\nSizeof long: %d\nSizeof unsigned long int: %d\n",sizeof(int),sizeof(long),sizeof(unsigned long int));
}

//int temp_address = 0; 

float temp_ambient     = 0.0;
float pressure_ambient = 0.0;
float wind_speed       = 0.0;
float lux              = 0.0;
float temp_water       = 0.0;
float pressure_water   = 0.0;
float dissolved_oxygen = 0.0;
int timestamp          = 0;
byte current_data_package[33]; // one extra character, which will NULL terminate when this array is used as a string
byte eeprom_data_package[33];

int motor_time = 0;

int loopTime = 0;

void loop() {

// if(in_history_mode) // burst transmit one history cell at a time
// {
  
// }

  // act on current command
  switch (rx_command)
  {
    case '\0': // no command present
      break;
    case 'c':  // c [scope] - Clip stored data to the most recent # entries
      Serial.printf("Command %c is not yet implemented.\n",rx_command);
      rx_command  = '\0';
      rx_argument = 0;
      break;
    case 'f': // get a frame
      myEEPROM.page_read((rx_argument*32)+32,eeprom_data_package,32);
      Serial.printf("Retrieved data: %s\n", eeprom_data_package);
      Serial.printf("Command %c is not yet implemented.\n",rx_command);
      rx_command  = '\0';
      rx_argument = 0;
      break;
    case 'g':
      Serial.printf("Command %c is not yet implemented.\n",rx_command);
      rx_command  = '\0';
      rx_argument = 0;
      break;
    case 'm':
      current_mode = rx_argument;
      switch (rx_argument)
      {
        case 0: // stationary
          Serial.printf("Switching to stationary mode...\n");
          dutyCycle = 0;
          ledcWrite(pwmChannel, dutyCycle);
          motor_time = 0;
          break;
        case 1: // upward
          Serial.printf("Switching to upward mode...\n");
          digitalWrite(motor1Pin1,LOW);
          digitalWrite(motor1Pin2,HIGH); // moves in counterclockwise direction (up)
          dutyCycle = 500;
          ledcWrite(pwmChannel, dutyCycle);
          motor_status = 1; // motor should be moving
          break;
        case 2: // downward
          Serial.printf("Switching to downward mode...\n");
          
          dutyCycle = 200;
          ledcWrite(pwmChannel, dutyCycle);
          delay(10);
          digitalWrite(motor1Pin1,HIGH);
          digitalWrite(motor1Pin2,LOW); // moves in counterclockwise direction (up)
          motor_status = 1; // motor should be moving
          break;
        case 3: // auto
        default:
          Serial.printf("This mode (%d) does not exist!\n", rx_argument);
          break;
      }
      rx_command  = '\0';
      rx_argument = 0;
      break;
    case 'p':
      // toggle LED
      digitalWrite(STATUS_LED,!displayStatus);
      displayStatus = !displayStatus;
      // send most recent data
      loopTime = 0; // forces loop to read sensors and transmit now
      rx_command  = '\0';
      rx_argument = 0;
      break;
    default:
      Serial.printf("FLUSHING LoRa BUFFER - Invalid command '%c' arg: %d\n", rx_command, rx_argument);
      LoRa.flush();
      rx_command  = '\0';
      rx_argument = 0;
      break;
  }

  // control motor - loop_time and motor_time [0] stationary [1] upward [2] downward [3] auto
  // if(current_mode==1){ // start motor upward

  //   motor_status = 1;
  // }

  // motor timer
  if(motor_status == 1) // motor running
  {
    motor_time = motor_time+1;
    if(motor_time % 1000 == 0)
    {
      Serial.printf("Motor Time is %d\n", motor_time);
    }
    // configure stop times
    if(current_mode == 1 && motor_time>300000) // upward timing
    {
      dutyCycle = 0;
      ledcWrite(pwmChannel, dutyCycle);
      motor_status = 0;
      motor_time = 0;
    } else if(current_mode == 2 && motor_time>150000) // downward timing
    {
      dutyCycle = 0;
      ledcWrite(pwmChannel, dutyCycle);
      motor_status = 0;
      motor_time = 0;
    }
  }



  // check sensors and transmit on a timer
  if(loopTime%(7*1000) == 0)
  { // transmit every 7 seconds
    // sleep when not doing things
    //delay(7*1000); // 7 second delay between sensor checks
    // read ALL the SENSORS
    temp_ambient     = 23.45;
    pressure_ambient = 1011.66;
    wind_speed       = 4.5;
    lux              = 0.0;
    temp_water       = 19.3456433;
    pressure_water   = 990.0;
    dissolved_oxygen = myData.dissolved_oxygen();
    timestamp        = millis();

      // DEBUG
      myEEPROM.read_config_raw();
      //Serial.printf("temp_ambient: %f \t pressure_ambient: %f \t wind_speed: %f \t %f %f %f %f %d\n", 
      //              temp_ambient, pressure_ambient, wind_speed, lux, temp_water, pressure_water, dissolved_oxygen, timestamp);

    // construct packet
    memcpy(current_data_package+0,&temp_ambient,sizeof(float));
    memcpy(current_data_package+4,&pressure_ambient,sizeof(float));
    memcpy(current_data_package+8,&wind_speed,sizeof(float));
    memcpy(current_data_package+12,&lux,sizeof(float));
    memcpy(current_data_package+16,&temp_water,sizeof(float));
    memcpy(current_data_package+20,&pressure_water,sizeof(float));
    memcpy(current_data_package+24,&dissolved_oxygen,sizeof(float));
    memcpy(current_data_package+28,&timestamp,sizeof(int));
    // send packet

        // packet to serial
        //Serial.println((char*)&current_data_package);
        // packet to EEPROM
        // String data = "";
        // for(int i =0; i < 32; i++){
        //   data += current_data_package[i];
        // }

        // packet to EEPROM
        myEEPROM.page_write(next_addr,current_data_package,32);

        // packet to LoRa
        LoRa.beginPacket();
        LoRa.write((uint8_t*)&current_data_package, 32);
        LoRa.endPacket();
        LoRa.receive();

        // retrieve the EEPROM data and double check for clarity
        myEEPROM.page_read(next_addr,eeprom_data_package,32);
        Serial.printf("FLASH  now contains: %s\nEEPROM now contains: %s\n", current_data_package,eeprom_data_package);
        
        // echo sent data to Serial, for visual feedback
        //Serial.write((uint8_t*)&current_data_package, 32);
        //Serial.println();
    
    // test next address location before using
    test_addr = next_addr + 32;
    if(test_addr % 32 != 0)
    {
      Serial.println("Bad address for frame! Attempting to revert to last 32 byte chunk.");
      test_addr = next_addr -(next_addr % 32);
    }
    // test data limit
    if(test_addr >= 0 && test_addr <= 32) // test if test_addr is in system zone
    {
      Serial.println("DATA LIMIT REACHED!!! CANNOT STORE DATA!!!");
      // don't increment! Keep overwriting last address
    } else {
      next_addr = (next_addr + 32) % MAX_EEPROM_SIZE;
      myEEPROM.write_next_address(next_addr);
    }
    printf("Next address is %d (should be %d)\n",myEEPROM.read_next_address(),next_addr);
    // next_addr += 32;
    // while(next_addr%32!=0){
    //   Serial.println("Bad address for frame! Skipping to next 32 byte chunk.");
    //   next_addr = next_addr + (32-(next_addr % 32));
    // }
    // if(next_addr >=0 && next_addr <32){
    //   Serial.println("DATA LIMIT REACHED!!! CANNOT STORE DATA!!!");
    //   next_addr -= 32; // get stuck!!!
    // }
    //Serial.printf("DO: %.2f mg/L\n", myData.dissolved_oxygen());
    //delay(7*1000); // 7 second delay between DO checks
  } // end if every 7 seconds
  loopTime = millis(); // can also be altered in ping
}