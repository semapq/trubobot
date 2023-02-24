//include block
#include <Arduino.h>
#include <Servo.h>
#include <IBusBM.h>
#include "MPU9250.h"


//define block
#define MAX_SIGNAL 2000
#define MIN_SIGNAL 1000
#define MOTORA_PIN 24
#define MOTORB_PIN 25

//variables block
int speed = MIN_SIGNAL;
int speedA = MIN_SIGNAL;
int speedB = MIN_SIGNAL;
int turn = 0;
int max_speed = 0;
int u_turn = 0;
int u_speed = 0;
float p_k_t = 1;
float p_k_s = 1;
float k_mxspd = 1;

bool stop = 0;
bool calibration_boot = 0;

//init block
IBusBM ibus;
Servo motorA, motorB;
MPU9250 mpu;

//func block
int readChannel(byte channelInput, int minLimit, int maxLimit, int defaultValue);
bool readSwitch(byte channelInput, bool defaultValue);
void ESC_calibration();

void setup() {
  pinMode(13, OUTPUT);

  Serial.begin(9600);
  ibus.begin(Serial3);
  Wire.begin();

  mpu.setup(0x68);
  motorA.attach(MOTORA_PIN);
  motorB.attach(MOTORB_PIN);

  motorA.writeMicroseconds(speedA);
  motorB.writeMicroseconds(speedB);

  if(calibration_boot == 1){
    ESC_calibration();
  }

  delay(5000);

  mpu.calibrateAccelGyro();
  mpu.calibrateMag();


}

void loop() {

  digitalWrite(13, HIGH);

/*   Serial.print("ch1: "); Serial.print(ibus.readChannel(1));
  Serial.print("\tch3: "); Serial.print(map(ibus.readChannel(3), 1000, 2000, -1000, 1000));
  Serial.print("\tch9: "); Serial.println(ibus.readChannel(9)); */

    if (mpu.update())
    {
      Serial.print(mpu.getYaw()); Serial.print(", ");
      Serial.print(mpu.getPitch()); Serial.print(", ");
      Serial.println(mpu.getRoll());
    }

  speed = ibus.readChannel(1);
  max_speed = ibus.readChannel(4);
  turn = map(ibus.readChannel(3), 1000, 2000, -1000, 1000);
  stop = map(ibus.readChannel(9), 1000, 2000, 1, 0);

  if(speed < 1020) speed = 1000;
  if(speed > 1950) speed = 2000;

  if(turn < 20 && turn > -20) turn = 0;
  if(turn > 950) turn = 1000;
  if(turn < -950) turn = -1000;

  u_turn = pow(turn, 3)/1000000;

  k_mxspd = 1 - ((2000 - max_speed)/2000.);

  speedA = k_mxspd*(p_k_s*speed + p_k_t*u_turn);
  speedB = k_mxspd*(p_k_s*speed - p_k_t*u_turn);

  if(speedA > max_speed) speedA = max_speed;
  if(speedA < 1000) speedA = 1000;

  if(speedB > max_speed) speedB = max_speed;
  if(speedB < 1000) speedB = 1000;

 /*  Serial.print("spd: "); Serial.print(speed);
  Serial.print("\ttrn: "); Serial.print(turn);
  Serial.print("\tmxspd: "); Serial.print(max_speed);
  Serial.print("\tk_mxspd: "); Serial.print(k_mxspd);

  Serial.print("\tspdA: "); Serial.print(speedA);
  Serial.print("\tspdB: "); Serial.print(speedB);
  
  Serial.print("\tstp: "); Serial.println(stop); */

  if(stop)
  {
    speedA = 1000;
    speedB = 1000;
  }

    motorA.writeMicroseconds(speedA);
    motorB.writeMicroseconds(speedB);

    digitalWrite(13, LOW);
}

int readChannel(byte channelInput, int minLimit, int maxLimit, int defaultValue) {
  uint16_t ch = ibus.readChannel(channelInput);
  if (ch < 100) return defaultValue;
  return map(ch, 1000, 2000, minLimit, maxLimit);
}

bool readSwitch(byte channelInput, bool defaultValue) {
  int intDefaultValue = (defaultValue) ? 100 : 0;
  int ch = readChannel(channelInput, 0, 100, intDefaultValue);
  return (ch > 50);
}

void ESC_calibration(){
    Serial.println("ESC calibration program begin...");
    delay(1000);
    Serial.println("This program will start the ESC.");
    
    Serial.print("Now writing maximum output: (");Serial.print(MAX_SIGNAL);Serial.print(" us in this case)");Serial.print("\n");
    Serial.println("Turn on power source, then wait 2 seconds and press any key.");

    motorA.writeMicroseconds(MAX_SIGNAL);
    motorB.writeMicroseconds(MAX_SIGNAL);

    while (!Serial.available());Serial.read();

    Serial.println("\n");
    Serial.println("\n");
    Serial.print("Sending minimum output: (");Serial.print(MIN_SIGNAL);Serial.print(" us in this case)");Serial.print("\n");

    motorA.writeMicroseconds(MIN_SIGNAL);
    motorB.writeMicroseconds(MIN_SIGNAL);

    Serial.println("The ESC is calibrated");
    Serial.println("----");
}