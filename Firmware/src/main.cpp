#include <Arduino.h>
#include<Wire.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define MPU_ADD 0x68
#define SAMPLES_CALIBRATION 20
#define HALF_SAMPLES_CALIBRATION 10
#define  ACC_OUT_REGISTER 0x3B
#define ACC_CONFIG_REGISTER 0x1C
#define AFS_SEL_2G 0x00
#define AFS_SEL_4G 0x01
#define AFS_SEL_8G 0x10
#define AFS_SEL_16 0x11
#define  TH_ACC 3000
#define SAMPLES_ROLLING_SUM 50
#define SAMPLES_CHECK_AXIS 50
#define TH_ROLLING_SUM 500000


#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// this is the Width and Height of Display which is 128 xy 32
#define DISPLAY_HEIGHT 32
#define DISPLAY_WIDTH  128




// Variaveis para armazenar valores do sensor
uint8_t index, current_axis, i, current_register_addr;
int16_t acc,acc_old;
int16_t acc_arr[SAMPLES_ROLLING_SUM];
int32_t sum_acc, sum_acc_level;
uint16_t flag;

const float calib_coeffs[3][2] = {
          {1.0029002389052817, 607.9006397945009},
          {0.9949136997537035, -92.6093859967491},
          {0.9725727991611409, 12.820518761250241}
      };

void update_display();
void define_current_acc_register(void);
void config_mpu(void);
int16_t read_acc(uint8_t start_addr);
void print_values(void);

void setup() {
  // Inicializa Serial
  Serial.begin(9600);
  Serial.println("Setup has started");
  config_mpu();
  Serial.println("MPU has been configured");
  define_current_acc_register();
  Serial.println("Current acc register has been defined");
  sum_acc = 0; index = 0; acc_old = 0; memset(acc_arr, 0, SAMPLES_ROLLING_SUM*sizeof(int16_t)); flag = 0;
  Serial.println("Setup has finished");
  update_display();
}

void loop() {

  for(int i=0;i < SAMPLES_CHECK_AXIS; i++)
  { 
    // Read accelaeration
    acc = read_acc(current_register_addr);

    sum_acc -= acc_arr[index];
    acc_arr[index] = abs(acc);
    sum_acc += acc_arr[index];

    index ++;
    if(index == SAMPLES_ROLLING_SUM){
      index = 0 ;
    }

  
  if((i > 0) & (abs(acc - acc_old) > TH_ACC)){
      flag = 10000;
    }
    else{
      flag = 0;
    }

    acc_old = acc;

    print_values();

    //delay(50);
  }

  if(abs(sum_acc_level - sum_acc)> 10000){
    update_display();
    sum_acc_level = sum_acc;
  }


  if(sum_acc < TH_ROLLING_SUM){
     define_current_acc_register();
    sum_acc = 0; index = 0; acc_old = 0; memset(acc_arr, 0, SAMPLES_ROLLING_SUM*sizeof(int16_t)); flag = 0;
  }


}


void define_current_acc_register(void){

  float acc_xyz[3];
  uint8_t counter_xyz[3] = {0};
  
  while(1){

    for (uint8_t i = 0; i < SAMPLES_CALIBRATION; i++){
      Wire.beginTransmission(MPU_ADD);
      Wire.write(ACC_OUT_REGISTER);
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_ADD, 6, true); // Solicita os dados ao sensor
      
      acc_xyz[0] = fabs((Wire.read() << 8 | Wire.read())*calib_coeffs[0][0] - calib_coeffs[0][1]);
      acc_xyz[1] = fabs((Wire.read() << 8 | Wire.read())*calib_coeffs[1][0] - calib_coeffs[1][1]);
      acc_xyz[2] = fabs((Wire.read() << 8 | Wire.read())*calib_coeffs[2][0] - calib_coeffs[2][1]);

      uint8_t skip;
      for(uint8_t j = 0; j<3; j++){
        skip = 0;
        for(uint8_t k =0; k<3; k++){
          if(j!=k){
            if(acc_xyz[j] < acc_xyz[k]){
              skip = 1;
              break;          
            }
          }
        }

        if(skip == 0){
          counter_xyz[j]++;
          if (counter_xyz[j]>HALF_SAMPLES_CALIBRATION){
          current_register_addr = ACC_OUT_REGISTER + 2*j;
          current_axis = j;
          acc_old = read_acc(current_register_addr);
          sum_acc_level = acc_old*SAMPLES_ROLLING_SUM;
          return;
          }
        }
      }
    }
  }
}

int16_t read_acc(uint8_t start_addr){
  int16_t output;
  Wire.beginTransmission(MPU_ADD);
  Wire.write(start_addr);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADD, 2, true); // Solicita os dados ao sensor
  output =  (Wire.read() << 8 | Wire.read());
  return output;
}

void config_mpu(void){
// Inicializa o MPU-6050
  Wire.setClock(400000);
  Serial.println("Clock frequency has been set");
  Wire.beginTransmission(MPU_ADD);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.println("Handshake has been done");

  Wire.beginTransmission(MPU_ADD);
  Wire.write(ACC_CONFIG_REGISTER);
  Wire.write(AFS_SEL_2G);
  Wire.endTransmission();

  Wire.beginTransmission(MPU_ADD);
  Wire.write(0x1A);
  Wire.write(0x06);
  Wire.endTransmission();
}

void print_values(void){
  Serial.println(String(acc_old) + "," + String(sum_acc) + "," + String(current_register_addr) + "," + String(current_axis));

}



void update_display(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  float g = float(sum_acc)/ 860000.0;
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(1,1);
  display.println(String(g,2));
  display.display();

}