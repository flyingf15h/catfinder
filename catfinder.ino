#include <Arduino.h>

const int BUTTON_PINS[] = {2, 3, 4, 5}; 
const int LED_PINS[] = {9, 10, 11};  

enum GameState { IDLE, STARTING, PLAYING, ENDING, SHOW_SCORE };
GameState gameState = IDLE;

// Game variables
unsigned long startTime = 0; 
unsigned long lastFlashTime = 0;
int currentSpeed = 1;  
const int SPEED_DELAYS[] = {500, 350, 200};
int score = 0;
bool ledsActive[3] = {false, false, false};
unsigned long ledStartTimes[3] = {0, 0, 0};
int targetBrightness[3] = {0, 0, 0};
int totalBursts = 0; // max is 30                  
unsigned long nextBurst = 0; 

// Score 
int currentScoreDigit = 0;
int digitValues[3] = {0, 0, 0};
unsigned long lastDigitChange = 0;

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
  
  for (int i = 0; i < 3; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    analogWrite(LED_PINS[i], 0);
  }
  
  Serial.begin(9600);
  randomSeed(analogRead(0));
}

void loop() {
  unsigned long currentTime = millis();
  static unsigned long but4time = 0;
  static bool button4Held = false;
  
  bool but4 = digitalRead(BUTTON_PINS[3]) == LOW;
  
  if (but4 && !button4Held) {
    but4time = currentTime;
    button4Held = true;
  } else if (!but4 && button4Held) {
    button4Held = false;
    if (gameState == IDLE && currentTime - but4time < 2000) {
      currentSpeed = (currentSpeed + 1) % 3;
      showSpeedIndicator();
    } else if (currentTime - but4time >= 2000) {
      toggleGame();
    }
  }

  // Game state machine
  switch (gameState) {
    case IDLE:
      setBrightness(0);
      break;
      
    case STARTING:
      if (currentTime - startTime < 3000) {
        if (currentTime - lastFlashTime > 500) {
          toggleLEDs();
          lastFlashTime = currentTime;
        }
      } else {
        gameState = PLAYING;
        score = 0; 
        startNewRound();
      }
      break;
      
    case PLAYING:
      updateLEDs(currentTime);    
      checkButtons(currentTime);   

      if (currentTime >= nextBurst && totalBursts < 30) {
        ledBurst(currentTime);
        totalBursts++;

        int pause = random(100, 451);
        nextBurst = currentTime + pause;
      }

      if (totalBursts >= 30) {
        gameState = ENDING;
        startTime = currentTime;
      }
      break;

    case ENDING:
      if (currentTime - startTime < 3000) {
        if (currentTime - lastFlashTime > 500) {
          toggleLEDs();
          lastFlashTime = currentTime;
        }
      } else {
        prepScore();
        gameState = SHOW_SCORE;
      }
      break;
      
    case SHOW_SCORE:
      displayScore(currentTime);
      break;
  }
}

void toggleGame() {
  if (gameState == IDLE) {
    gameState = STARTING;
    startTime = millis();
  } else if (gameState == PLAYING) {
    gameState = ENDING;
    startTime = millis();
  }
}

void showSpeedIndicator() {
  for (int i = 0; i < 3; i++) {
    for (int brightness = 0; brightness <= 255; brightness += 5) {
      analogWrite(LED_PINS[i], brightness);
      delay(10);
    }
    for (int brightness = 255; brightness >= 0; brightness -= 5) {
      analogWrite(LED_PINS[i], brightness);
      delay(10);
    }
  }
}

void setBrightness(int brightness) {
  for (int i = 0; i < 3; i++) {
    analogWrite(LED_PINS[i], brightness);
  }
}

void toggleLEDs() { 
  for (int i = 0; i < 3; i++) {
    int current = analogRead(LED_PINS[i]) > 128 ? 255 : 0;
    int newVal = current ? 0 : 255;
    analogWrite(LED_PINS[i], newVal);
    ledsActive[i] = (newVal > 0);
  }
}

void ledBurst(unsigned long currentTime) {
  int numLEDs = random(1, 4);  
  bool selected[3] = {false, false, false};

  for (int i = 0; i < numLEDs; ) {
    int led = random(0, 3);
    if (!selected[led]) {
      selected[led] = true;
      ledsActive[led] = true;
      ledStartTimes[led] = currentTime;
      targetBrightness[led] = 255;
      analogWrite(LED_PINS[led], 0);
      i++;
    }
  }
}

void startNewRound() {
  for (int i = 0; i < 3; i++) {
    ledsActive[i] = false;
    analogWrite(LED_PINS[i], 0);
  }
  totalBursts = 0;
  nextBurst = millis();
}

void updateLEDs(unsigned long currentTime) {
  for (int i = 0; i < 3; i++) {
    if (ledsActive[i]) {
      unsigned long elapsed = currentTime - ledStartTimes[i];
      int rampTime = SPEED_DELAYS[currentSpeed];

      if (elapsed < rampTime) {
        int brightness = map(elapsed, 0, rampTime, 0, 255);
        analogWrite(LED_PINS[i], brightness);
      } else if (elapsed < rampTime + 100) {
        analogWrite(LED_PINS[i], 255);
      } else {
        analogWrite(LED_PINS[i], 0);
        ledsActive[i] = false;
      }
    }
  }
}

void checkButtons(unsigned long currentTime) {
  for (int i = 0; i < 3; i++) {
    if (ledsActive[i] && digitalRead(BUTTON_PINS[i]) == LOW) {
      unsigned long elapsed = currentTime - ledStartTimes[i];
      int rampTime = SPEED_DELAYS[currentSpeed];
      int timeDiff = abs(elapsed - rampTime);

      if (timeDiff <= 40) {
        score += 2;
        analogWrite(LED_PINS[i], 0);
        ledsActive[i] = false;
      } else if (timeDiff <= 80) {
        score += 1;
        analogWrite(LED_PINS[i], 0);
        ledsActive[i] = false;
      }
    }
  }
}

void prepScore() {
  digitValues[0] = (score / 100) % 10; 
  digitValues[1] = (score / 10) % 10;  
  digitValues[2] = score % 10;        
  currentScoreDigit = 0;
  lastDigitChange = millis();
}

void displayScore(unsigned long currentTime) {
  if (currentTime - lastDigitChange > 1000) {
    currentScoreDigit = (currentScoreDigit + 1) % 3;
    lastDigitChange = currentTime;
    setBrightness(0);
  }

  if (currentScoreDigit < 3) {
    unsigned long flashTime = (currentTime - lastDigitChange) % 500;
    int flashes = digitValues[currentScoreDigit];

    if (flashes > 0) {
      int flashState = (flashTime / (500 / (flashes * 2))) % 2;
      analogWrite(LED_PINS[currentScoreDigit], flashState ? 255 : 0);
    } else {
      analogWrite(LED_PINS[currentScoreDigit], 0);
    }
  }

  if (currentScoreDigit == 0 && currentTime - lastDigitChange > 1000) {
    gameState = IDLE;
  }
}
