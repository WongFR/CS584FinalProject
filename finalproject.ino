#include <LedControl.h>

// 8x8 LED matrix pin definitions (DIN, CLK, CS)
LedControl lc = LedControl(10, 12, 11, 1);  // DIN = D10, CLK = D12, CS = D11

// LED matrix pin definitions
const int ledRowPins[2] = {2, 3};     // LED row pins
const int ledColPins[2] = {4, 5};     // LED column pins

// Switch matrix pin definitions
const int switchRowPins[2] = {7, 6};  // Switch row pins
const int switchColPins[2] = {8, 9};  // Switch column pins

// 4x8 matrix digit representations (0-9), each digit uses 4 columns
const byte numbers[10][8] = {
  {B0110, B1001, B1001, B1001, B1001, B1001, B0110, B0000}, // 0
  {B0010, B0110, B0010, B0010, B0010, B0010, B0111, B0000}, // 1
  {B0110, B1001, B0001, B0010, B0100, B1000, B1111, B0000}, // 2
  {B0110, B1001, B0001, B0010, B0001, B1001, B0110, B0000}, // 3
  {B0001, B0011, B0101, B1001, B1111, B0001, B0001, B0000}, // 4
  {B1111, B1000, B1110, B0001, B0001, B1001, B0110, B0000}, // 5
  {B0110, B1000, B1110, B1001, B1001, B1001, B0110, B0000}, // 6
  {B1111, B0001, B0010, B0010, B0100, B0100, B0100, B0000}, // 7
  {B0110, B1001, B1001, B0110, B1001, B1001, B0110, B0000}, // 8
  {B0110, B1001, B1001, B0111, B0001, B0001, B0110, B0000}  // 9
};

int score = 0;
int misses = 0;
const int maxMisses = 3;
int moleDuration = 3000;    // Initial mole display time (milliseconds)
const int minMoleDuration = 500;  // Minimum mole display time (milliseconds)

// Current lit LED position
int currentRow = -1;
int currentCol = -1;

void setup() {
  // Initialize LED row pins as output
  for (int i = 0; i < 2; i++) {
    pinMode(ledRowPins[i], OUTPUT);
    digitalWrite(ledRowPins[i], LOW);
  }

  // Initialize LED column pins as output
  for (int i = 0; i < 2; i++) {
    pinMode(ledColPins[i], OUTPUT);
    digitalWrite(ledColPins[i], HIGH);
  }

  // Initialize switch row pins as output
  for (int i = 0; i < 2; i++) {
    pinMode(switchRowPins[i], OUTPUT);
    digitalWrite(switchRowPins[i], LOW);
  }

  // Initialize switch column pins as input (with pull-up resistors)
  for (int i = 0; i < 2; i++) {
    pinMode(switchColPins[i], INPUT_PULLUP);
  }

  // Initialize 8x8 LED matrix
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  Serial.begin(9600);

  // Initially light up a random LED, waiting for the player to start
  showRandomMole();
}

void loop() {
  waitForStart();
  playGame();
}

// Wait for the player to press a switch to start the game
void waitForStart() {
  Serial.println("Waiting for the player to start...");
  while (true) {
    int pressedRow, pressedCol;
    if (checkSwitchPress(pressedRow, pressedCol)) {
      Serial.println("Game started!");
      score = 0;
      misses = 0;
      moleDuration = 3000;
      lc.clearDisplay(0);
      break;
    }
  }
}

// Main game logic
void playGame() {
  while (misses < maxMisses) {
    showRandomMole();
    delay(500);  // Short delay after each mole display
  }

  // Blink the score for 3 seconds when the game ends
  for (int i = 0; i < 6; i++) {
    lc.clearDisplay(0);
    delay(250);
    displayTwoDigitNumber(score);
    delay(250);
  }
}

// Light up a random LED and wait for the player's response
void showRandomMole() {
  if (currentRow != -1 && currentCol != -1) {
    digitalWrite(ledRowPins[currentRow], LOW);
    digitalWrite(ledColPins[currentCol], HIGH);
  }

  currentRow = random(0, 2);
  currentCol = random(0, 2);

  digitalWrite(ledRowPins[currentRow], HIGH);
  digitalWrite(ledColPins[currentCol], LOW);

  unsigned long startTime = millis();
  bool hit = false;

  while (millis() - startTime < moleDuration) {
    int pressedRow, pressedCol;
    if (checkSwitchPress(pressedRow, pressedCol)) {
      if (pressedRow == currentRow && pressedCol == currentCol) {
        Serial.println("Mole hit!");
        score++;
        hit = true;
        break;
      } else {
        Serial.println("Wrong button!");
        misses++;
        digitalWrite(ledRowPins[currentRow], LOW);
        digitalWrite(ledColPins[currentCol], HIGH);
        return;
      }
    }
  }

  if (!hit) {
    Serial.println("Missed or timed out!");
    misses++;
  }

  displayTwoDigitNumber(score);

  // Decrease mole display time after each hit, minimum 0.5 seconds
  if (moleDuration > minMoleDuration) {
    moleDuration -= 100;
  }
}

// Check if a switch is pressed and return the pressed switch position
bool checkSwitchPress(int &pressedRow, int &pressedCol) {
  for (int row = 0; row < 2; row++) {
    digitalWrite(switchRowPins[row], HIGH);
    for (int col = 0; col < 2; col++) {
      if (digitalRead(switchColPins[col]) == LOW) {
        pressedRow = row;
        pressedCol = col;
        digitalWrite(switchRowPins[row], LOW);
        return true;
      }
    }
    digitalWrite(switchRowPins[row], LOW);
  }
  return false;
}

// Display a two-digit score
void displayTwoDigitNumber(int number) {
  int tens = number / 10;
  int ones = number % 10;

  lc.clearDisplay(0);

  if (number < 10) {
    displayDigit(ones, 4);
  } else {
    displayDigit(tens, 0);
    displayDigit(ones, 4);
  }
}

// Display a single digit
void displayDigit(int digit, int colOffset) {
  for (int row = 0; row < 8; row++) {
    byte pattern = numbers[digit][row];
    for (int col = 0; col < 4; col++) {
      lc.setLed(0, row, colOffset + col, (pattern >> (3 - col)) & 0x01);
    }
  }
}
