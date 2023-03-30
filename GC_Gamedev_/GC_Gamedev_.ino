#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_SPI.h> //Hardware-specific library
#include <Wire.h> 

#define MODEL ST7735S128
#define CS   A2   
#define CD   A3
#define RST  A1
#define LED  A0

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

LCDWIKI_SPI my_lcd(MODEL,CS,CD,RST,LED);

uint8_t player_input = 64;
uint8_t player_pos = 64;
uint8_t bot_pos = 64;
uint8_t new_bot_pos = 0;
uint8_t ball_pos[2] = {64,15};
int8_t ball_speed[2] = {0, 3};
float pos_store = 0;
uint8_t speed = 3;
int player_score = 0;
int bot_score = 0;


int ADXL345 = 0x53;

void get_input() {
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true);
  float X_out = -( Wire.read()| Wire.read() << 8);
  X_out = (0.05*X_out/256 + 0.95*pos_store);
  pos_store = X_out;
  player_input = min(max(15, 64 - (int)(500*X_out)), 115);
}

void start_game() {

  player_score = 0;
  bot_score = 0;  
  player_input = 64;
  player_pos = 64;
  bot_pos = 64;
  my_lcd.Fill_Screen(BLACK);

  my_lcd.Set_Text_colour(CYAN);
  my_lcd.Set_Text_Size(2);
  my_lcd.Print_String("START", 40, 50);
  
  delay(2000);

  my_lcd.Set_Text_colour(BLACK);
  my_lcd.Set_Text_Size(2);
  my_lcd.Print_String("START", 40, 50);

  delay(200);

  for(int i=64-8;i<=64+8;i++) {
    my_lcd.Draw_Pixe(i, 110, BLUE);
    my_lcd.Draw_Pixe(i, 111, BLUE);
  }

  for(int i=64-8;i<=64+8;i++) {
    my_lcd.Draw_Pixe(i, 10, RED);
    my_lcd.Draw_Pixe(i, 11, RED);
  }
  delay(1000);
}

void setup() {
  my_lcd.Init_LCD();
  my_lcd.Fill_Screen(BLACK);
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

  my_lcd.Set_Rotation(2);
  my_lcd.Set_Draw_color(WHITE);
  my_lcd.Set_Text_Back_colour(BLACK);

  delay(1000);
}

void render_player() {
  for(int i=player_pos-8;i<=player_pos+8;i++) {
    if(player_input-i > 8 || player_input-i < -8) {
      my_lcd.Draw_Pixe(i, 110, BLACK);
      my_lcd.Draw_Pixe(i, 111, BLACK);
    }
  }
  for(int i=player_input-8;i<=player_input+8;i++) {
    if(i-player_pos > 8 || i-player_pos < -8) {
      my_lcd.Draw_Pixe(i, 110, BLUE);
      my_lcd.Draw_Pixe(i, 111, BLUE);
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
  if(!(y == 110 || y == 111)) return false;
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
        my_lcd.Draw_Pixe(i, j, WHITE);        
      }
    }
  }
  for(int i=prev_pos[0]-3;i<=prev_pos[0]+3;i++) {
    for(int j=prev_pos[1]-3;j<=prev_pos[1]+3;j++) {
      if(!point_in_ball(i, j) && !point_in_player(i, j)) {
        my_lcd.Draw_Pixe(i, j, BLACK);        
      }
    }
  }
}

void collision_with_player() {
  ball_speed[0] = constrain(ball_pos[0]-player_pos, -2, 2); 
  if(ball_speed[1] > 0)
    ball_speed[1] = -(speed - abs(ball_speed[0]));
  else ball_speed[1] = (speed - abs(ball_speed[0]));

  // // To remake the player after collision 
  // for(int i=player_pos-8;i<=player_pos+8;i++) {
  //     my_lcd.Draw_Pixe(i, 110, BLUE);
  //     my_lcd.Draw_Pixe(i, 111, BLUE);
  // }

}

void collision_with_bot() {
  ball_speed[1] = -ball_speed[1];
}

void clear_ball()
{
  for(int i=ball_pos[0]-3;i<=ball_pos[0]+3;i++) {
    for(int j=ball_pos[1]-3;j<=ball_pos[1]+3;j++) {
        my_lcd.Draw_Pixe(i, j, BLACK);        
      }
    }
    ball_pos[0] = 64;
    ball_pos[1] = 15;
    ball_speed[0] = 0;
    ball_speed[1] = 3;
}

void playerscore(){
  clear_ball();
  player_score++;
}

void botscore(){
    clear_ball();
    bot_score++;
}
void check_collision() {
  if(ball_speed[0] > 0 && ball_pos[0] >= 125) ball_speed[0] = -ball_speed[0]; // Checking collisions with wall and reflecting
  if(ball_speed[0] < 0 && ball_pos[0] <= 3) ball_speed[0] = -ball_speed[0]; // Checking collisions with wall and reflecting

  if(ball_pos[1] < 40) {
    if(ball_speed[1] < 0 && (ball_pos[1] <=16 && ball_pos[1] > 12) && abs(ball_pos[0]-bot_pos) <= 13) collision_with_bot(); // Checking collisions with bot and reflecting
  }
  if(ball_pos[1] > 80 ) {
    if(ball_speed[1] > 0 && (ball_pos[1] >=104 && ball_pos[1] < 108) && abs(ball_pos[0]-player_pos) <= 13) collision_with_player(); // Checking collisions with player and reflecting
  }
  if(ball_pos[1]<4)
  {
    playerscore(); //If miss by player will increase bot's score
  }
  if(ball_pos[1]>122)
  {
    botscore(); //If miss by bot will increase bot's score
  }
  else{

    Serial.println(127-ball_pos[1]);
  }
}

void render_score() {
  my_lcd.Set_Text_colour(BLUE);
  my_lcd.Set_Text_Size(1);
  my_lcd.Print_String(String(player_score), 115, 60);

  my_lcd.Set_Text_colour(RED);
  my_lcd.Set_Text_Size(1);
  my_lcd.Print_String(String(bot_score), 5, 60);
}

int if_win(){
  if( player_score == 5 || bot_score == 5 )
    return 0;
  else
    return 1;
}


//Making bot
int run = 0;
void render_bot() {
  if(run<1) // To reduce reaction time of bot
  {
    run++;
  }
  else{
    run = 0;
    if (ball_pos[0] - bot_pos > 3 && ball_pos[0] < 118 && bot_pos + 3 < 115)
      new_bot_pos = bot_pos + 4 ;
    else if (ball_pos[0] - bot_pos < 4 && ball_pos[0] > 5 && bot_pos - 5 > 8)
      new_bot_pos = bot_pos - 3;
    else
      return;
    for(int i=bot_pos-8;i<=bot_pos+8;i++) {
      if(new_bot_pos-i > 8 || new_bot_pos-i < -8) {
        my_lcd.Draw_Pixe(i, 10, BLACK);
        my_lcd.Draw_Pixe(i, 11, BLACK);
      }
    }
    for(int i=new_bot_pos-8;i<=new_bot_pos+8;i++) {
        if(i-bot_pos > 8 || i-bot_pos < -8) {
          my_lcd.Draw_Pixe(i, 10, RED);
          my_lcd.Draw_Pixe(i, 11, RED);
        }
      }

      bot_pos = new_bot_pos;  
  }
  }
//Ending bot

void loop() {
  //tick();
  start_game();
  while(if_win())
  {  
  render_score();
  get_input();
  render_player();
  render_ball(ball_pos[0]+ball_speed[0], ball_pos[1]+ball_speed[1]);
  render_bot();
  check_collision();

  delay(10);}

  render_score();

  my_lcd.Fill_Screen(BLACK);

  if(player_score>bot_score)
  { my_lcd.Set_Text_colour(CYAN);
    my_lcd.Set_Text_Size(2);
    my_lcd.Print_String("YOU WIN!", 15, 50);
  }
  else
  {
  my_lcd.Set_Text_colour(CYAN);
  my_lcd.Set_Text_Size(2);
  my_lcd.Print_String("YOU LOSE!", 15, 50);
  }
  delay(2000);
  player_input = 64;

  my_lcd.Fill_Screen(BLACK);

}