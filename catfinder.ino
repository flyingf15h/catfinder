#include "Keyboard.h"

const int LED1 = 26;  // dot
const int LED2 = 27;  // dash
const int BUT1 = 0;
const int BUT2 = 1;
const int BUT3 = 2;
const int BUT4 = 3;

bool LED1locked = false;
bool LED2locked = false;
bool LED1TempState = false;
bool LED2TempState = false;

const unsigned long dD = 20;
const unsigned long kbCd = 600;

unsigned long lastchange1 = 0;
unsigned long lastchange2 = 0;
unsigned long lastchange3 = 0;
unsigned long lastchange4 = 0;
unsigned long BUT1cd = 0;
unsigned long BUT2cd = 0;
unsigned long BUT4cd = 0;

bool laststate1 = HIGH;
bool laststate2 = HIGH;
bool laststate3 = HIGH;
bool laststate4 = HIGH;

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUT1, INPUT_PULLUP);
  pinMode(BUT2, INPUT_PULLUP);
  pinMode(BUT3, INPUT_PULLUP);
  pinMode(BUT4, INPUT_PULLUP);

  Keyboard.begin();
}

void loop() {
  unsigned long currTime = millis();

  bool sw1 = digitalRead(BUT1) == LOW;
  bool sw2 = digitalRead(BUT2) == LOW;
  bool sw3 = digitalRead(BUT3) == LOW;
  bool sw4 = digitalRead(BUT4) == LOW;

  // Button 1 
  if (sw1 != laststate1) {
    lastchange1 = currTime;
  }
  if ((currTime - lastchange1) > dD) {
    if (sw1 != laststate1) {
      laststate1 = sw1;
      if (sw1 && (currTime - BUT1cd >= kbCd)) { 
        BUT1cd = currTime;
        if (!LED1locked) {
          LED1TempState = true;
        }
        Keyboard.print("(ﾉ◕ヮ◕)ﾉ*:･ﾟ✧");
      }
    }
  }
  if (!sw1) {
    LED1TempState = false;
  }

  // Button 2  
  if (sw2 != laststate2) {
    lastchange2 = currTime;
  }
  if ((currTime - lastchange2) > dD) {
    if (sw2 != laststate2) {
      laststate2 = sw2;
      if (sw2 && (currTime - BUT2cd >= kbCd)) {
        BUT2cd = currTime;
        if (!LED2locked) {
          LED2TempState = true;
        }
        Keyboard.print("(^◕.◕^)");
      }
    }
  }
  if (!sw2) {
    LED2TempState = false;
  }

  // Button 3 
  if (sw3 != laststate3) {
    lastchange3 = currTime;
  }
  if ((currTime - lastchange3) > dD) {
    if (sw3 != laststate3) {
      laststate3 = sw3;
      if (sw3) {  
        LED1locked = !LED1locked;
        LED2locked = !LED2locked;
      }
    }
  }

  // Button 4 
  if (sw4 != laststate4) {
    lastchange4 = currTime;
  }
  if ((currTime - lastchange4) > dD) {
    if (sw4 != laststate4) {
      laststate4 = sw4;
      if (sw4 && (currTime - BUT4cd >= kbCd)) {
        BUT4cd = currTime;
        Keyboard.press(KEY_BACKSPACE);
        Keyboard.release(KEY_BACKSPACE);
        playMorse("..-...-......");
      }
    }
  }

  digitalWrite(LED1, LED1locked || LED1TempState);
  digitalWrite(LED2, LED2locked || LED2TempState);
}

void playMorse(const char* morse) {
  for (int i = 0; morse[i] != '\0'; i++) {
    char c = morse[i];

    if (c == '.') {
      digitalWrite(LED1, HIGH);
      delay(200);
      digitalWrite(LED1, LOW);
    } else if (c == '-') {
      digitalWrite(LED2, HIGH);
      delay(400);
      digitalWrite(LED2, LOW);
    }

    delay(200);
    if (digitalRead(BUT1) == LOW || digitalRead(BUT2) == LOW || 
        digitalRead(BUT3) == LOW || digitalRead(BUT4) == LOW) {
      return;
    }

    if (i == 3 || i == 5 || i == 8) {
      delay(400);
    }
  }
}