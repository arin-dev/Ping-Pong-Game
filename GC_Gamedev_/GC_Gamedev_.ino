#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_SPI.h> //Hardware-specific library
#include <Wire.h> 

#define MODEL ST7735S128
#define CS   A2   
#define CD   A3
#define RST  A1
#define LED  A0

LCDWIKI_SPI my_lcd(MODEL,CS,CD,RST,LED);

uint8_t player_input = 64;
uint8_t player_pos = 64;
float pos_store = 0;

int ADXL345 = 0x53;

void get_input() {
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true);
  float X_out = ( Wire.read()| Wire.read() << 8);
  X_out = (0.3*X_out/256 + 0.7*pos_store);
  pos_store = X_out;
  player_input = 64 + (uint8_t)(500*X_out);
  Serial.print("Xa= ");
  Serial.println(X_out);
  Serial.println(player_input);   
}

void setup() 
{
  my_lcd.Init_LCD();
  my_lcd.Fill_Screen(0x0000);
  Serial.begin(9600); 
  Wire.begin(); 
 
  Wire.beginTransmission(ADXL345); 
  Wire.write(0x2D);
  Wire.write(8);
  Wire.endTransmission();

  Wire.beginTransmission(ADXL345); 
  Wire.write(0x1E);
  Wire.write(3);
  Wire.endTransmission();
  delay(10);
  
  Wire.beginTransmission(ADXL345); 
  Wire.write(0x1F);
  Wire.write(-3);
  Wire.endTransmission();
  delay(10);
  
  Wire.beginTransmission(ADXL345); 
  Wire.write(0x20);
  Wire.write(-11);
  Wire.endTransmission();
  delay(10);
}

void update_player() {
  
 
  for(int i=player_pos-8;i<=player_pos+8;i++) {
    if(player_input-i > 8 || player_input-i < -8) {
      my_lcd.Draw_Pixe(i, 10, 0x0000);
      my_lcd.Draw_Pixe(i, 11, 0x0000);
    }
  }
  for(int i=player_input-8;i<=player_input+8;i++) {
    if(i-player_pos > 8 || i-player_pos < -8) {
      my_lcd.Draw_Pixe(i, 10, 0xFFFF);
      my_lcd.Draw_Pixe(i, 11, 0xFFFF);
    }
  }
  
  player_pos = player_input;  
}

void loop() 
{
    get_input();
    update_player();
    delay(1);
}
