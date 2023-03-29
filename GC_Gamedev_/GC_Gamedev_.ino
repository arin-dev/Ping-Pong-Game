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
uint8_t ball_pos[2] = {64, 16};
int8_t ball_speed[2] = {0, 4};
float pos_store = 0;
uint8_t clock = 0;
uint8_t speed = 3;
uint8_t blocks[8] = {0};
int score = 0;


int ADXL345 = 0x53;

void get_input() {
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true);
  float X_out = ( Wire.read()| Wire.read() << 8);
  X_out = (0.05*X_out/256 + 0.95*pos_store);
  pos_store = X_out;
  player_input = min(max(15, 64 - (int)(500*X_out)), 115);
  // Serial.print("Xa= ");
  // Serial.println(X_out);
  // Serial.println(player_input);   
}

void setup() {
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

  // my_lcd.Set_Rotation(2);
  my_lcd.Set_Draw_color(0xFFFF);
  my_lcd.Set_Text_Back_colour(0x0000);

  blocks[0] = 0b00000000;
  blocks[1] = 0b00000000;
  blocks[2] = 0b00000000;
  blocks[3] = 0b01001000;
  blocks[4] = 0b00000000;

  delay(1000);
  start_game();
  
}

void render_player() {
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

bool point_in_ball(uint8_t x, uint8_t y) {
  if(x == ball_pos[0] || y == ball_pos[1]) {
    if(abs(x-ball_pos[0])+abs(y-ball_pos[1]) <= 3) {
      return true;
    } else return false;
  } else {
    if(abs(x-ball_pos[0])+abs(y-ball_pos[1]) <= 4) {
      return true;
    } else return false;
  }
}

bool point_in_player(uint8_t x, uint8_t y) {
  if(!(y == 10 || y == 11)) return false;
  if(abs(player_pos-x) <= 8) return true;
  return false;
}

void render_ball(uint8_t x, uint8_t y) {
  uint8_t prev_pos[2];
  prev_pos[0] = ball_pos[0];
  prev_pos[1] = ball_pos[1];
  ball_pos[0] = x;
  ball_pos[1] = y;
  for(int i=ball_pos[0]-3;i<=ball_pos[0]+3;i++) {
    for(int j=ball_pos[1]-3;j<=ball_pos[1]+3;j++) {
      if(point_in_ball(i, j)) {
        my_lcd.Draw_Pixe(i, j, 0xFFFF);        
      }
    }
  }
  for(int i=prev_pos[0]-3;i<=prev_pos[0]+3;i++) {
    for(int j=prev_pos[1]-3;j<=prev_pos[1]+3;j++) {
      if(!point_in_ball(i, j) && !point_in_player(i, j)) {
        my_lcd.Draw_Pixe(i, j, 0x0000);        
      }
    }
  }
}

void collision_with_player() {
  ball_speed[0] = constrain(ball_pos[0]-player_pos, -2, 2); 
  if(ball_speed[1] > 0)
    ball_speed[1] = -(speed - abs(ball_speed[0]));
  else ball_speed[1] = (speed - abs(ball_speed[0]));

}

void check_collision() {
  if(ball_speed[1] > 0 && ball_pos[1] >= 125) ball_speed[1] = -ball_speed[1];
  if(ball_speed[0] > 0 && ball_pos[0] >= 125) ball_speed[0] = -ball_speed[0];
  if(ball_speed[0] < 0 && ball_pos[0] <= 3) ball_speed[0] = -ball_speed[0];

  if(ball_pos[1] < 40) {
    //check player collision
    if(ball_speed[1] < 0 && (ball_pos[1] <=16 && ball_pos[1] > 12) && abs(ball_pos[0]-player_pos) <= 13) collision_with_player();
  } else {
    //check block collision
    // bool tl=false, tr=false, bl=false, br=false;
    // if(blocks[(127-ball_pos[1]-3)/8]&(1<<((ball_pos[0]-3)/8))) {
    //   blocks[(127-ball_pos[1]-3)/8] &= (1<<9 - 1) - (1<<((ball_pos[0]-3)/8));
    // }
    Serial.println(127-ball_pos[1]);
  }
}

void render_block(uint8_t x, uint8_t y) {
  my_lcd.Set_Draw_color(0xFFFF);
  my_lcd.Draw_Rectangle(112-x, 120-y, 127-x, 127-y);
}

void remove_block(uint8_t x, uint8_t y) {
  my_lcd.Set_Draw_color(0x0000);
  my_lcd.Draw_Rectangle(112-x, 120-y, 127-x, 127-y);
}

void render_grid() {
  for(uint8_t i=0;i<8;i++) {
    for(uint8_t j=0;j<8;j++) {
      if(blocks[i]&(1<<j)) render_block(16*j, 8*i);
    }
  }
}

void render_score() {
  
  my_lcd.Print_Number_Int(score, 0, 0, 0, ' ',10);
}

void start_game() {
  for(int i=64-8;i<=64+8;i++) {
    my_lcd.Draw_Pixe(i, 10, 0xFFFF);
    my_lcd.Draw_Pixe(i, 11, 0xFFFF);
  }
  delay(1000);
}

void tick() {
  if(!clock) {
    score++;
  }
  clock++;
}


void loop() {
  tick();
  
  // render_grid();
  get_input();
  render_ball(ball_pos[0]+ball_speed[0], ball_pos[1]+ball_speed[1]);
  render_player();
  check_collision();
  // render_score();
  clock++;
  delay(10);
}

