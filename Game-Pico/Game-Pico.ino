#include "mbed.h"
#include "USBSerial.h"
#include "TextLCD.h"
#define WAIT_US 1000000 // multiplier to fix use of wait_us instead of wait 
using namespace mbed;
USBSerial pc; // tx, rx
//initialising and setting up hardware and defining variables
TextLCD lcd(p0, p1, p12, p13, p14, p15);
BusOut rows(p20, p19, p18);
BusIn cols(p11,p10,p9,p8);
SPI sw(p3, NC, p2); 
DigitalOut cs(p5);
PwmOut buzzer(p27);
float startFrequency[]={523.25, 587.33, 659.25, 698.46, 783.99};
float startBeat[]={0.2, 0.2, 0.2, 0.2, 0.2, 0.2};
float successFrequency[] = {784.00, 880.00, 987.77}; 
float successBeat[] = {0.3, 0.3, 0.3}; 
float failFrequency[] = {329.63, 369.99, 392.00}; 
float failBeat[] = {0.2, 0.2, 0.2};
float closeFrequency[] = {392.00, 440.00, 493.88};
float closeBeat[] = {0.2, 0.2, 0.2};
char Keytable[] = { 'F', 'E', 'D', 'C', 
'3', '6', '9', 'B', 
'2', '5', '8', '0', 
'1', '4', '7', 'A'
};
char getKey(){ 
int i,j;
char ch=' ';
for (i = 0; i <= 3; i++) { 
rows = i; 
for (j = 0; j <= 3; j++) { 
if (((cols ^ 0x00FF) & (0x0001<<j)) != 0) { 
ch = Keytable[(i * 4) + j]; 
}
} 
} 
return ch;
}

int within(int value, int goal, int n) {
    return (abs(value - goal) <= n);
}

boolean set = false; 
boolean verif;
char guess[2] = {' ',' '};
int setnum;
int score = 0;
char repeat[] = {'A'};
int gamecount=0;
int failcount=0;
char buffer[2];
char temp[2] = {' ',' '};
int main()
{
    //play start sound and set up variables to start game, switch off LEDs and print LCD pattern
    for (int i = 0; i < 5; i++) {
        buzzer.period(1.0 / startFrequency[i]);
        buzzer.write(0.5);
        wait_us(1000000*startBeat[i]);
    }
    buzzer.write(0);
    while(1){
    gamecount++;
    failcount = 0;
    score = 30;
    char numberKey[2] = {' ', ' '};
    int keyIndex = 0;
    set = false;
    verif = false;
    guess[1] = ' ';
    guess[0] = ' ';
    cs=0;
    sw.format(8,0);
    sw.write(0x0000);
    sw.write(0x0000);
    cs = 1;
    cs = 0;
    lcd.printf("[==][==][==][==]");
    while (!set)
    {
        //take user inputs for secret number on keypad and ensure user can see what they type
        char key = getKey();
        if (key >= '0' && key <= '9')
        {
            numberKey[keyIndex] = key;
            keyIndex = (keyIndex + 1) % 2;
            lcd.cls(); // Clear the LCD
            lcd.printf("Secret Number:%c%c", numberKey[0], numberKey[1]);
            lcd.locate(0,0);
            lcd.printf("Secret Number:%c%c", numberKey[0], numberKey[1]);
            wait_us(200000); 
        }
        else if (key == 'A')
        {
            //confirm the number is valid, under 30
            if (numberKey[0] != ' ' && numberKey[1] != ' ')
            {
                if((numberKey[0] - '0') * 10 + (numberKey[1] - '0')>30){
                lcd.locate(0,0);
                lcd.cls();
                lcd.printf("Invalid number!");
                numberKey[0] = ' ';
                numberKey[1] = ' ';
                keyIndex = 0;
            }else{
              pc.printf("%c%c\r", numberKey[0], numberKey[1]); // Send the valid two-digit number over USBSerial to the PC
              wait_us(300000);                                    
              lcd.cls(); // Clear the LCD
              lcd.locate(0,0);
              lcd.printf("Secret Number:%c%c", numberKey[0], numberKey[1]);
              set = true;
              setnum = (numberKey[0] - '0') * 10 + (numberKey[1] - '0');
            }}
        }
        wait_us(3000);
    }
    while(1){
      //recieve guess from pc
    while(guess[1] == ' '){
      pc.gets(guess,3);
    }
    int guessno = (guess[0] - '0') * 10 + (guess[1] - '0');
    if (guessno == setnum){
      //if correct print success message on lcd, play success sound and change lights green
      score = score-failcount;
      if(score < 0){
        score = 0;}
      lcd.locate(0,1);
      lcd.printf("Well done!Pts:%d", score);
      pc.printf("101");
      sw.write(0x00AA);
      sw.write(0x00AA);
      cs = 1;
      cs = 0;
      for (int i = 0; i < 3; i++) {
      buzzer.period(1.0 / successFrequency[i]);
      buzzer.write(0.5);
      wait_us(1000000*successBeat[i]);
      }
      buzzer.write(0.0);
      wait_us(5000000);
      lcd.cls();
      break;
    }
    else if(within(guessno, setnum, 3)){
      //if within 3 print close message on lcd, play close sound and change lights orange
      lcd.locate(0,0);
      lcd.cls();
      lcd.printf("Secret Number:%d", setnum);
      lcd.locate(0,1);
      lcd.printf("Guessed: %c%c", guess[0], guess[1]);
      pc.printf("102");
      failcount++;
      sw.write(0x00FF);
      sw.write(0x00FF);
      cs = 1;
      cs = 0;
      for (int i = 0; i < 3; i++) {
        buzzer.period(1.0 / closeFrequency[i]);
        buzzer.write(0.5);
        wait_us(1000000*closeBeat[i]);
      }
      buzzer.write(0.0);
      }else{
        //if wrong print fail message on lcd, play fail sound and change lights red
      lcd.cls(); // Clear the LCD
      lcd.printf("Secret Number:%d", setnum);
      lcd.locate(0,1);
      lcd.printf("Guessed: %c%c", guess[0], guess[1]);
      pc.printf("100");
      failcount++;
      sw.write(0x0055);
      sw.write(0x0055);
      cs = 1;
      cs = 0;
      for (int i = 0; i < 3; i++) {
      buzzer.period(1.0 / failFrequency[i]);
      buzzer.write(0.5);
      wait_us(1000000*failBeat[i]);
      }
      buzzer.write(0.0);
    }
    guess[0] = ' ';
    guess[1] = ' ';
        
  }
  lcd.cls();

  
}
}
