
#define SEND_RAW_SENSOR_VALUES 0x86
#define SEND_CALIBRATED_SENSOR_VALUES 0x87
#define SEND_BATTERY_MILLIVOLTS 0xB1
#define DO_PRINT 0xB8
#define M1_FORWARD 0xC1
#define M1_BACKWARD 0xC2
#define M2_FORWARD 0xC5
#define M2_BACKWARD 0xC6
#define AUTO_CALIBRATE 0xBA
#define CLEAR 0xB7

#include <Ultrasonic.h>

float m1Speed;
float m2Speed;

void initialize() {

  Serial1.begin(115200);
  Serial.begin(9600);
  m1Speed = 0;
  m2Speed = 0;

}


void pololuReset() {

  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);
  delay(10);
  digitalWrite(5, HIGH);
  delay(100);  

}

void stopSmooth() {

  while(m1Speed > 0 || m2Speed > 0) {
    if((m1Speed - 0.05) >= 0) {
      m1Speed -= 0.05;
    } else {
      m1Speed = 0;    
    }
    if((m2Speed - 0.05) >= 0) {
      m2Speed -= 0.05;        
    } else {
      m2Speed = 0;
  }

  activateMotor(0, m1Speed);
  activateMotor(1, m2Speed);
  delay(35);
  }  
  
}

void activateMotor(int motor, float speed) {

  char opcode = 0x0;
  if (speed > 0.0) {
    if (motor == 0) {
      opcode = M1_FORWARD;
      m1Speed = speed;
    } else {
      opcode = M2_FORWARD;
      m2Speed = speed;
    }
  } else {
    if (motor == 0) {
      opcode = M1_BACKWARD;
      m1Speed = speed;
    } else {
      opcode = M2_BACKWARD;
      m2Speed = speed;
    }
  }
  unsigned char arg = 0x7f * abs(speed);
  Serial1.write(opcode);
  Serial1.flush(); 
  Serial1.write(arg);
  Serial1.flush();
  
}

float battery() {

  Serial1.write(SEND_BATTERY_MILLIVOLTS);
  Serial1.flush();
  char lowbyte = Serial1.read();
  char hibyte = Serial1.read();
  float v = ((lowbyte + (hibyte << 8))/1000.0);
  return(v);
  
}

char sensorAutoCalibrate() {

  Serial1.write(AUTO_CALIBRATE);
  Serial1.flush();
  return Serial1.read();    
    
}

void calibratedSensors(int sensors[5]) {
  
  int sensor1 = 0;
  int sensor2 = 0;
  int sensor3 = 0;
  int sensor4 = 0;
  int sensor5 = 0;
  
  Serial1.write(SEND_CALIBRATED_SENSOR_VALUES);
  Serial1.flush();

  char sensor1High = Serial1.read();
  char sensor1Low = Serial1.read();
  sensor1 = sensor1Low + (sensor1High << 8);   
  char sensor2High = Serial1.read();
  char sensor2Low = Serial1.read();
  sensor2 = sensor2Low + (sensor2High << 8);   
  char sensor3High = Serial1.read();
  char sensor3Low = Serial1.read();
  sensor3 = sensor3Low + (sensor3High << 8);   
  char sensor4High = Serial1.read();
  char sensor4Low = Serial1.read();
  sensor4 = sensor4Low + (sensor4High << 8);   
  char sensor5High = Serial1.read();
  char sensor5Low = Serial1.read();
  sensor5 = sensor5Low + (sensor5High << 8);   

  sensors[0] = sensor1;
  sensors[1] = sensor2;
  sensors[2] = sensor3;
  sensors[3] = sensor4;
  sensors[4] = sensor5;

}

void setup() {
  initialize();
  
  if (battery() < 25) {
    Serial.print("Insufficient power: ");
    Serial.println(battery());
    return;
  }

  // Passive opto-sensor
  pinMode(A4, INPUT);

  // LED
  pinMode(20, OUTPUT);

  sensorAutoCalibrate();
}

void loop() {
  int sensors[5];
  calibratedSensors(sensors);

  char displayBuf[50];
  sprintf(displayBuf, "Active Optosensors: %d; %d; %d; %d; %d\n", sensors[0], sensors[1], sensors[2], sensors[3], sensors[4]);
   //Serial.println(displayBuf);

  float output = analogRead(A4);

  Serial.println(output);

  int lightpercent = (1023-output)/1023 * 100;
  Serial.println(lightpercent);
  if (lightpercent < 50) {
    digitalWrite(20, HIGH);
  } else {
    digitalWrite(20, LOW);
  }

  Ultrasonic ultrasonic(10);
  int distance = ultrasonic.read();
  Serial.print("Distance(CM): ");
  Serial.println(distance);
  if (distance < 5) {
    stopSmooth();
    delay(2000);
  }

  int maxSensorIdx = 0;
  for (int i = 0; i < 5; i++) {
    if (sensors[i] > sensors[maxSensorIdx]) {
      maxSensorIdx = i;
    }
  }

  // Sensors from left to right: [0 - 4]
  switch (maxSensorIdx) {
    case 0:
        stopSmooth();
        activateMotor(1, 0.3);
        activateMotor(0, -0.15);
        delay(15);
        stopSmooth();
        break;
    case 1:
        activateMotor(1, 0.2);
        activateMotor(0, -0.1);
        break;
    case 2:
        activateMotor(0, 0.12);
        activateMotor(1, 0.12);
        break;
    case 3:
        activateMotor(0, 0.2);
        activateMotor(1, -0.1);
        break;
    case 4:
        stopSmooth();
        activateMotor(0, 0.3);
        activateMotor(1, -0.15);
        delay(15);
        stopSmooth();
        break;
    default:
        break;
  }

  // Meet endpoint, stop!
  // TODO: change threshold?
  int trshd = 0.8 * sensors[maxSensorIdx];
  if (sensors[1] > trshd && sensors[2] > trshd && sensors[3] > trshd && sensors[0] > trshd && sensors[4] > trshd) {
    stopSmooth();
  }

  // TODO: change delay based on 'maxSensorIdx'?
  delay(5);

}