#include <Wire.h>
#include <AntaresESP8266MQTT.h> 
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#define ACCESSKEY "0f1859c20d87b077:afcab3db1a3ab4a1"
#define WIFISSID "ice blood"
#define PASSWORD "tigaroda"

const char* serverName = "http://api.thebigbox.id/sms-broadcast/1.0.0/send";

#define projectName "PosFic"
#define deviceName "Posfic"
// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;

// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D1;
const uint8_t sda = D2;

// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

// MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;
//rain
int rainsensor= A0; // analog sensor pin A0 untuk input raindrop
int Data_Sensorhujan;
int led1 = D7; // digital pin nomor 7 untuk output buzzer
//Getar
const int pinSensor = D0;
int Data_Sensorgetar;

int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;
AntaresESP8266MQTT antares(ACCESSKEY);
void setup() {
  Serial.begin(9600);
  antares.setDebug(true);
  antares.wifiConnection(WIFISSID,PASSWORD);
  antares.setMqttServer();  // Inisiasi server MQTT Antares
  pinMode(pinSensor, INPUT);
  pinMode(led1, OUTPUT);
   pinMode(rainsensor, INPUT);
  Wire.begin(sda, scl);
  MPU6050_Init();
}

void loop() {
  antares.checkMqttConnection();
  double Ax, Ay, Az, T, Gx, Gy, Gz;
     Data_Sensorhujan = analogRead(rainsensor);
   Serial.println(Data_Sensorhujan);
   delay(250);
   
   if (Data_Sensorhujan > 351){ 
   
      Serial.print("Tidak Hujan = ");Serial.println(Data_Sensorhujan);
      digitalWrite(led1, LOW);
      antares.add("Cuaca", "Tidak Hujan");
    
   }
   else if (Data_Sensorhujan < 350 && Data_Sensorhujan > 301){ 
    
      Serial.print("Hujan = ");Serial.println(Data_Sensorhujan);
      digitalWrite(led1, LOW); 
      antares.add("Cuaca", "Hujan");
      
   }
   else if (Data_Sensorhujan < 300){ 
 
      Serial.print("Hujan Deras = ");Serial.println(Data_Sensorhujan);
      digitalWrite(led1, HIGH);
      antares.add("Cuaca", "Hujan Deras");
  
   }
   Data_Sensorgetar = digitalRead(pinSensor);
  Serial.println(Data_Sensorgetar);
    if (Data_Sensorgetar == 1)
  {
    Serial.print("Ada Getaran : ");
     antares.add("Getaran", "Ada Getaran");
    delay(500);
  }
  else
  {
    Serial.print("Tidak Ada Getaran : ");
     antares.add("Getaran", "Tidak ada Getaran");
    delay(500);
  }
   delay(1000);
   
  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
  
  //divide each with their sensitivity scale factor
  Ax = (double)AccelX/AccelScaleFactor;
  Ay = (double)AccelY/AccelScaleFactor;
  Az = (double)AccelZ/AccelScaleFactor;
  T = (double)Temperature/340+36.53; //temperature formula
  Gx = (double)GyroX/GyroScaleFactor;
  Gy = (double)GyroY/GyroScaleFactor;
  Gz = (double)GyroZ/GyroScaleFactor;

  Serial.print("Ax: "); Serial.print(Ax);
  Serial.print(" Ay: "); Serial.print(Ay);
  Serial.print(" Az: "); Serial.print(Az);
  Serial.print(" T: "); Serial.print(T);
  Serial.print(" Gx: "); Serial.print(Gx);
  Serial.print(" Gy: "); Serial.print(Gy);
  Serial.print(" Gz: "); Serial.println(Gz);

  Serial.print("HTTP Response code: ");
if (Gx > 4)
{
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("x-api-key", "b8iVz65KbbK2m0KjiPXJVh910Ri7GRju");
  String httpRequestData = "msisdn=085156530932&content=Berpotensi%20Longsor%20LM-001";
  int httpResponseCode = http.POST(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);  
  // Free resources
  http.end();
}
  delay(100);
  antares.add("Gx", Gx);
  antares.add("Gy", Gy);
  antares.add("T", T);
 antares.publish(projectName, deviceName);
  delay(500);
}

void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

// read all 14 register
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}

//configure MPU6050
void MPU6050_Init(){
  delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}
