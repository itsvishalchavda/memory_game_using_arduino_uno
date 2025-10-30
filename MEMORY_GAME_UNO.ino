/*
Memory Game with 16x2 I2C LCD by Vishal Chavda
Hardware Setup:
4-Channel Relay Module: Pins 2, 3, 4, 5
4 Push Buttons: Pins 6, 7, 8, 9 (connected to GND)
1 Buzzer: Pin 10
16x2 I2C LCD: Pin A4 (SDA), Pin A5 (SCL)
4 AC Bulbs: Controlled by the relay module
Design by Vishal Chavda
 */

#include <Wire.h>                  
#include <LiquidCrystal_I2C.h>     

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int buzzerPin = 10;
const int relayPins[4] = {2, 3, 4, 5}; // Red, Green, Yellow, Blue
const int buttonPins[4] = {6, 7, 8, 9};

const int MAX_LEVEL = 25; // Maximum rounds
int sequence[MAX_LEVEL];  
int currentLevel = 0;     
int userStep = 0;         
int score = 0;

/*** Game State Manager ***/
// 0 == Idle - waiting to start
// 1 == Game Running 
// 2 == Game Over 
// 3 == Computer's Turn 
int gameState = 0;

#define RELAY_ON   LOW
#define RELAY_OFF  HIGH

// Tones for each color 
int toneFrequencies[4] = {262, 330, 392, 523};

void setup() 
{
  Serial.begin(9600);
  Serial.println("Game starting...");

  lcd.init();
  lcd.backlight();

  for (int i = 0; i < 4; i++) 
  {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], RELAY_OFF);
  }
  for (int i = 0; i < 4; i++) 
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(buzzerPin, OUTPUT);

  randomSeed(analogRead(A0));
  playStartupSequence();
  showIdleScreen();
  gameState = 0; // initiaLGame state
}

void loop() 
{

  int buttonPressed = checkButtonPress();

  switch (gameState) 
  {
    case 0: 
      if (buttonPressed != -1) 
      {
        startNewGame();
      }
      break;

    case 1: 
      if (buttonPressed != -1) 
      {
        flashLED(buttonPressed, 150);
        if (buttonPressed == sequence[userStep]) 
        {
          userStep++; 
          if (userStep == currentLevel) 
          {
            score = currentLevel; 
            playSuccessChime();
            delay(1000);
            nextRound(); 
          }
        } 
        else 
        {
          playGameOver();
        }
      }
      break;

    case 2: 
      if (buttonPressed != -1)
      {
        showIdleScreen();
        gameState = 0;
      }
      break;

    case 3: // Show color squence
      // This state is managed by functions (playSequence)
      // The main loop just waits
      break;
  }
}

void showIdleScreen() 
{
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("Press any key");
  lcd.setCursor(0, 1); 
  lcd.print("to start!");
}

void updateGameScreen() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Round: ");
  lcd.print(currentLevel);
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
}

void showGameOverScreen() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game Over!");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
}

void startNewGame() 
{
  Serial.println("NEW GAME");
  currentLevel = 0;
  score = 0;
  delay(500);
  nextRound();
}

void nextRound() 
{
  userStep = 0;
  currentLevel++;

  if (currentLevel > MAX_LEVEL) 
  {
    playGameWin(); 
    startNewGame();
    return;
  }
 
  Serial.print("Round ");
  Serial.println(currentLevel);

  updateGameScreen();

  sequence[currentLevel - 1] = random(0, 4);

  playSequence();

  gameState = 1;
  Serial.println("Your turn!");
}

void playSequence() 
{
  gameState = 3;
  delay(500);
  for (int i = 0; i < currentLevel; i++) 
  {
    int color = sequence[i];
    Serial.print(color);
    Serial.print(" ");
    flashLED(color, 400); 
    delay(150); 
  }
  Serial.println("");
}

int checkButtonPress() 
{
  for (int i = 0; i < 4; i++) 
  {
    if (digitalRead(buttonPins[i]) == LOW) 
    {
      delay(50); 
      while (digitalRead(buttonPins[i]) == LOW) 
      {
        delay(10);
      }
      Serial.print("User pressed: ");
      Serial.println(i);
      return i; 
    }
  }
  return -1; // No button pressed
}

void flashLED(int colorIndex, int duration) 
{
  digitalWrite(relayPins[colorIndex], RELAY_ON);
  tone(buzzerPin, toneFrequencies[colorIndex], duration);
  delay(duration);
  digitalWrite(relayPins[colorIndex], RELAY_OFF);
}

void playGameOver() 
{
  Serial.println("GAME OVER");
  gameState = 2;
  showGameOverScreen();

  for (int j = 0; j < 3; j++) 
  {
    for (int i = 0; i < 4; i++) 
    {
      digitalWrite(relayPins[i], RELAY_ON);
    }
    tone(buzzerPin, 150, 250);
    delay(250);
    for (int i = 0; i < 4; i++) 
    {
      digitalWrite(relayPins[i], RELAY_OFF);
    }
    delay(150);
  }
}

void playSuccessChime() 
{
  tone(buzzerPin, 600, 100);
  delay(120);
  tone(buzzerPin, 800, 150);
  delay(200);
}

void playGameWin() 
{
  Serial.println("YOU WIN!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("YOU WIN!");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);

  for (int i = 0; i < 3; i++) 
  {
    flashLED(0, 100);
    flashLED(1, 100);
    flashLED(2, 100);
    flashLED(3, 100);
  }
  delay(2000); 
}

void playStartupSequence() 
{
  lcd.setCursor(0, 0);
  lcd.print("Memory Game By ");
  lcd.setCursor(0, 1);
  lcd.print("Vishal Chavda, Loading...");
  
  flashLED(0, 150);
  flashLED(1, 150);
  flashLED(2, 150);
  flashLED(3, 150);
}
