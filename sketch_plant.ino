#include <LiquidCrystal_I2C.h>

#include <Key.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Инициализация экрана

const byte ROWS = 4; //число строк у нашей клавиатуры
const byte COLS = 4; //число столбцов у нашей клавиатуры

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'}, // здесь мы располагаем названия наших клавиш, как на клавиатуре,для удобства пользования
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {5, 4, 3, 2}; //к каким выводам подключаем управление строками

byte colPins[COLS] = {9, 8, 7, 6}; //к каким выводам подключаем управление столбцами

byte passwordLength = 7;
char pass[7];
char inputPass[7];
char timeMass[5];
byte charNumber = 0;
boolean allowType = true;
//boolean allInfoExist = false;
//initialize an instance of class NewKeypad

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), colPins, rowPins,  ROWS, COLS);

//TIMER INIT
byte minuts = 2; // 2 минуты
byte seconds = 0;
long millisStart;
byte errorsCounter = 3;
/*
   0 - только запушена, ожидание ввода нового пароля
   1 - ожидаем ввода времени
   2 - заряжено,
*/
byte idStatus = 0;

void setup() {
  Serial.begin(9600);
  lcd.init(); // Инициализируем экран
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("HELLO!");
  delay(1000);
  newGamePrint();

  customKeypad.addEventListener(keypadEvent); // Добавляем нашей клавиатуре перехватчик событий
  customKeypad.setHoldTime(2000);             // Устанавливаем время зажатия кнопки
  randomSeed(analogRead(A0));
  pinMode(12, OUTPUT);
  millisStart = millis();
}

void loop() {
  char customKey = customKeypad.getKey();

  if (idStatus == 0 && customKey && allowType) {
    if (charNumber != passwordLength) {
      inputNewPassword(customKey);
    } else {
      allowType = false;
      //      allInfoExist = true;
      charNumber = 0;
    }
  }

  if (idStatus == 1 && customKey && allowType) {
    if (charNumber != 5) {
      inputTime(customKey);
    }
    else {

      minuts = (timeMass[0] - '0') * 10 + (timeMass[1] - '0');
      seconds = (timeMass[3] - '0') * 10 + (timeMass[4] - '0');
      Serial.println(minuts);
      Serial.println(seconds);
      //      allInfoExist = true;
      charNumber = 0;
    }
  }

  if (idStatus == 2 && customKey && allowType) {
    checkInput(customKey);
  }
  if (idStatus == 2) {
    startTimer();
  }
}

void newGamePrint() {
  lcd.setCursor(0, 0);
  lcd.print("NEW PASS:");
  lcd.setCursor(0, 1);
  String invisibleStr = "";
  for (int i = 0; i < passwordLength; i++) {
    invisibleStr += '_';
  }
  lcd.print(invisibleStr);
  lcd.setCursor(0, 0);
}

// Обработчик событий для клавиатуры
void keypadEvent(KeypadEvent key) {
  switch (customKeypad.getState()) {
    //    case HOLD:
    //      if (key == '*') {
    //        Serial.println("HOLDED");
    //        printNeedPassText();
    //      }
    //      break;
    case HOLD:
      if (key == '#' && idStatus == 0) {
        Serial.println("pressed");
        tone(12, 200, 50);
        idStatus = 1;
        printNeedInputTime();
        allowType = true;
        break;
      }
      if (key == '#' && idStatus == 1) {
        Serial.println("pressed");
        tone(12, 200, 50);
        idStatus = 2;
        lcd.clear();
        printNeedPassText();
        break;
        charNumber = 0;
      }
      break;
  }
}

void printNeedInputTime() {
  lcd.clear();
  lcd.setCursor(0, 0); //устанавливаем курсор
  lcd.print("INPUT TIME");
  lcd.setCursor(0, 1); //устанавливаем курсор
  lcd.print("__:__");
}
//Печатаем текст о том, что нужно ввести пароль
void printNeedPassText() {
  lcd.clear();
  lcd.setCursor(0, 0); //устанавливаем курсор
  lcd.print("Enter password:");
  lcd.setCursor(0, 1); //устанавливаем курсор
  String invisibleStr = "";
  for (int i = 0; i < passwordLength; i++) {
    invisibleStr += '*';
  }
  lcd.print(invisibleStr);
  charNumber = 0;
}

void inputNewPassword(char key) {

  tone(12, 300, 50);
  lcd.setCursor(charNumber, 1); //устанавливаем курсор
  lcd.print(key);
  pass[charNumber] = key;
  charNumber++;

}

void inputTime(char key) {
  tone(12, 300, 50);
  if (charNumber == 2) {
    lcd.setCursor(charNumber, 1); //устанавливаем курсор
    lcd.print(":");
    charNumber++;
  }
  lcd.setCursor(charNumber, 1); //устанавливаем курсор
  lcd.print(key);
  timeMass[charNumber] = key;
  charNumber++;
}

void checkInput(char key) {
  tone(12, 300, 50);
  lcd.setCursor(charNumber, 1); //устанавливаем курсор
  lcd.print(key);
  inputPass[charNumber] = key;
  charNumber++;
  if (charNumber == passwordLength) {
    int i;
    for (i = 0; i < passwordLength; i++) {
      if (pass[i] != inputPass[i]) {
        lcd.clear();
        lcd.setCursor(0, 0); //устанавливаем курсор
        lcd.print("WRONG!");
        errorsCounter--;
        allowType = false;
        tone(12, 220, 150);
        delay(200);
        tone(12, 220, 150);
        delay(200);
        allowType = true;
        charNumber = 0;
        if (errorsCounter > 0) {
          printNeedPassText();
        } else {
          printBoom();
          reset();
        }
        break;
      }
    }
    if (i == passwordLength) {
      lcd.clear();
      charNumber = 0;
      lcd.setCursor(0, 0); //устанавливаем курсор
      lcd.print("TRUE!");
      tone(12, 400, 600);
      charNumber = 0;
      allowType = false;
      reset();
    }
  }
}

void startTimer() {
  if (minuts == 0 && seconds == 0) {
    printBoom();
  } else {
    if (millis() - millisStart > 1000) {
      if ( seconds == 0) {
        seconds = 59;
        minuts--;
      } else {
        seconds--;
      }

      millisStart = millis();
    }
    printTime(minuts, seconds);
  }


}

void printTime(byte minutsLeft, byte secondsLeft) {
  String outMinuts = String(minutsLeft);
  String outSeconds = String(secondsLeft);

  if (secondsLeft < 10) {
    outSeconds = String(0) + String(secondsLeft);
  }
  if (minutsLeft < 10) {
    outMinuts =  String(0) + String(minutsLeft);
  }
  lcd.setCursor(11, 1);
  lcd.print(outMinuts);
  lcd.print(":");
  lcd.print(outSeconds);
  lcd.setCursor(0, 1);
}

void printBoom() {
  lcd.setCursor(5, 1);
  lcd.clear();
  lcd.print("BOOM!");
  tone(12, 400, 150);
  delay(200);
  tone(12, 350, 150);
  delay(200);
  tone(12, 300, 150);
  delay(200);
  tone(12, 250, 150);
  delay(200);
  reset();
}

void reset() {
  delay(2000);
  idStatus = 0;
  allowType = true;
  newGamePrint();
}

