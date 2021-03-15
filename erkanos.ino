#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// constant definitions

#define voice 3951
#define duration 250
#define invalid "Yanlis Secim"
#define pushed "[+] Button Pushed : "
#define finished "[+] Process Finished."
#define setting "[*] Setting Everything up.."

// enumeration for switch case

enum mem {
  DAY,
  HOUR,
  MIN,
  SEC
};

// allocating to give input to user

char* text = new char;

// constant variables

const int port = 9600;

// pins

const byte red = 13;
const byte green = 12;
const byte blue = 11;
const byte sound = 10;
const int lamps[] = {A1, A2, A3};
const int termometer = A0;

// keypad and lcd screen setup

const int ROWS = 4;
const int COLS = 4;
const char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(termometer);
DallasTemperature sensors(&oneWire);

// functions

long long prev = 0;
long long current = millis();
int br = 0;

void logger(const char *logs, bool ln = true) {
  // log everything on Serial
  // port had set up early so no need to use begin()

  if (ln) {
    Serial.println(logs);
  } else {
    Serial.print(logs);
  }
}

void logger(char logs, bool ln = true) {
  if (ln) {
    Serial.write(logs);
    Serial.println();
  } else {
    Serial.write(logs);
  }
}

void fading() {
  current = millis();
  const long long ms = 50;

  // fade blue led in 50ms

  if (current - prev >= ms) {
    analogWrite(blue, br);

    if (br > 255) {
      br = 0;
    } else {
      br++;
    }

    prev = current;
  }
}

void printer(LiquidCrystal_I2C mon, const char *text, bool cls = true, bool ln = false) {
  mon.backlight();
  mon.setCursor(0, 0);

  // clear screen

  if (cls) {
    mon.clear();
  }

  // set cursor on second row for new line

  if (ln) {
    mon.setCursor(0, 1);
  }

  // print text on screen

  logger("[*] Writing on lcd : ", false);
  logger(text);
  mon.print(text);
}

void light(short pin, int sleep) {
  // turn on led delay for sleep time and turn it off

  digitalWrite(pin, HIGH);
  delay(sleep);
  digitalWrite(pin, LOW);
}

void wrongInput() {
  // sound and turn on red led for duration time

  tone(sound, voice, duration * 3);
  printer(lcd, invalid, true, false);
  light(red, duration * 3);
}

void ring() {
  // sound and turn on green led for duration time

  tone(sound, voice, duration);
  light(green, duration);
}

void alarm() {
  // sound and turn on red led for duration*3 time

  tone(sound, voice, duration);
  light(red, duration);
}

void mainMenu(LiquidCrystal_I2C mon) {
  mon.begin(16, 2);
  mon.home();

  // begin the lcd and print something on screen

  printer(mon, "Hosgeldiniz");
  printer(mon, "Bir tusa basin..", false, true);
}

void backOff() {
  // delay for duration * 4 time and go back to main menu

  printer(lcd, "Geri Donuluyor..");
  delay(duration * 4);
  mainMenu(lcd);
}

bool checkKey(char k) {
  // check for the key if it's numeric or not (only # char is valid to make action)

  if (isdigit(k)) {
    return true;
  }

  else {
    if (k == '#') {
      return true;
    }

    return false;
  }
}

long col(char c) {
  // convert char to long

  long n;
  n = c - (int)48;
  return n;
}

void showTemperature(Keypad pad) {
  char k;
  float tmp;

  do {
    k = pad.getKey();
    logger(pushed, false);
    logger(k);

    // get temperature.

    sensors.requestTemperatures();
    tmp = sensors.getTempCByIndex(0);

    // check if device and sensors available to get information if it is then print it onto screen until user push any button

    if (tmp != DEVICE_DISCONNECTED_C) {
      delete text;
      sprintf(text, "Sicaklik : %d", static_cast<int>(tmp));
      printer(lcd, text);
      logger(finished);

    } else {
      printer(lcd, "Bilgi Yok", true, false);
      logger("[!] Could not read temperature data!");
    }
  } while (k == '\0');

  logger(pushed, false);
  logger(k);
  ring();
  backOff();
}

void lightsOn(Keypad pad) {
  const long long ms = 150;
  char k;
  printer(lcd, "C=Cik");
  prev = 0;

block:
  k = pad.getKey();

  for (auto l : lamps) {
    light(l, ms);
    k = pad.getKey();
  }

  k = pad.getKey();

  if (k != '\0' && k == 'C') {
    for (auto l : lamps) {
      analogWrite(l, 0);
    }

    ring();
    backOff();
  }

  else {
    goto block;
  }

  return;
}

// classes for each option

class Calc {
  private:
    long x;
    long y;
    long res;

    // setters & getters

    void setX(long x) {
      this->x = x;
    }

    long getX() {
      return this->x;
    }

    void setY(long y) {
      this->y = y;
    }

    long getY() {
      return this->y;
    }

    void sum() {
      res = getX() + getY();
      delete text;
      sprintf(text, "%ld", res);
      printer(lcd, text, true, false);
      delay(duration * 4);
    }

    void sub() {
      res = getX() - getY();
      delete text;
      sprintf(text, "%ld", res);
      printer(lcd, text, true, false);
      delay(duration * 4);
    }

    void mlt() {
      res = getX() * getY();
      delete text;
      sprintf(text, "%ld", res);
      printer(lcd, text, true, false);
      delay(duration * 4);
    }

    void dv() {
      if (getY() == 0) {
        res = 0;
      } else {
        res = getX() / getY();
      }

      delete text;
      sprintf(text, "%ld", res);
      printer(lcd, text, true, false);
      delay(duration * 4);
    }

    void makeClean() {
      setX(0);
      setY(0);
      res = 0;
      delete text;
      lcd.clear();
      lcd.backlight();
    }

    void getNumbers(Keypad pad) {
      long first = 0;
      long sec = 0;
      int n = 0;
      char k;

      for (int cur = 0; ;) {
        k = pad.getKey();

        if (k != '\0') {
          logger(pushed, false);
          logger(k);

          if (checkKey(k)) {
            if (k == '#') {
              // set x when first numbers entered

              ring();
              setX(first);
              break;
            }

            else {
              if (cur != 4) {
                // print pushed numbers

                ring();
                lcd.backlight();
                lcd.setCursor(cur, 0);
                lcd.write(k);
                n = col(k);
                first *= 10;
                first += n;
                cur++;
              }
            }
          } else {
            wrongInput();
          }
        }
      }

      lcd.clear();
      lcd.backlight();
      lcd.setCursor(0, 0);
      n = 0;

      for (int cur = 0; ;) {
        k = pad.getKey();

        if (k != '\0') {
          logger(pushed, false);
          logger(k);

          if (checkKey(k)) {
            if (k == '#') {
              // set y when first numbers entered

              ring();
              setY(sec);
              break;
            }

            else {
              if (cur != 4) {
                // print pushed numbers

                ring();
                lcd.backlight();
                lcd.setCursor(cur, 0);
                lcd.write(k);
                n = col(k);
                sec *= 10;
                sec += n;
                cur++;
              }
            }
          } else {
            wrongInput();
          }
        }
      }
    }

  public:
    Calc(Keypad pad) {
      const char* math[] = {"1=TOPLAMA", "2=CIKARMA", "3=CARPMA", "4=BOLME", "5=ANA MENU"};
      char key;

      while (true) {
        for (int i = 0; i < 5; ) {
          // print possible choices

          current = millis();

          if ((current - prev) >= (duration * 3)) {
            printer(lcd, math[i], true, false);
            i++;
            prev = current;
          }

          key = pad.getKey();

          if (key != '\0') {
            logger(pushed, false);
            logger(key);

            // clean the memory

            makeClean();

            // calculate total of numbers and print it onto screen

            switch (key) {
              case '1':
                ring();
                getNumbers(pad);
                sum();
                break;

              case '2':
                ring();
                getNumbers(pad);
                sub();
                break;

              case '3':
                ring();
                getNumbers(pad);
                mlt();
                break;

              case '4':
                ring();
                getNumbers(pad);
                dv();
                break;

              case '5':
                ring();
                backOff();
                return;

              default:
                wrongInput();
                break;
            }

            // clean the memory

            makeClean();
          }
        }
      }
    }
};

class Chron {
  private:
    long long secs;
    long long before;
    int ms;
    int s;
    int m;
    int h;

    // setters & getters

    void setMs(int ms) {
      this->ms = ms;
    }

    int getMs() {
      return this->ms;
    }

    void setS(int s) {
      this->s = s;
    }

    int getS() {
      return this->s;
    }

    void setM(int m) {
      this->m = m;
    }

    int getM() {
      return this->m;
    }

    void setH(int h) {
      this->h = h;
    }

    int getH() {
      return this->h;
    }

    void setTimer() {
      delete text;
      sprintf(text, "%d:%d:%d:%d", getH(), getM(), getS(), getMs());
    }

    void showChoices() {
      lcd.begin(16, 2);
      lcd.home();
      lcd.setCursor(0, 0);
      lcd.backlight();
      lcd.print("B=Baslat C=Geri");
      lcd.setCursor(0, 1);
      lcd.print("D=Durdur");
    }

    void stopChron() {
      delay(duration * 4);
      secs = 0;
      before = 0;
      setH(0);
      setM(0);
      setS(0);
      setMs(0);
    }

    void startChron(Keypad pad) {
      before = millis();
      lcd.clear();

      while (true) {
        secs = millis();
        secs -= before;

        // calculate seconds, minutes, hours, ...

        setMs((secs / 10) % 100);
        setS((secs / 1000) % 60);
        setM(((secs / 1000) / 60) % 60);
        setH((((secs / 1000) / 60) / 60) % 60);
        setTimer();

        if (getS() == 0) {
          lcd.clear();
        }

        printer(lcd, text, false, false);
        char k = pad.getKey();

        if (k != '\0') {
          logger(pushed, false);
          logger(k);

          if (k == 'D') {
            ring();
            stopChron();
            backOff();
            break;
          }
        }
      }
    }

  public:
    Chron(Keypad pad) {
      showChoices();
      char k;

      while (true) {
        k = pad.getKey();

        if (k != '\0') {
          logger(pushed, false);
          logger(k);

          if (k == 'B') {
            // start it

            ring();
            startChron(pad);
            return;
          }

          else if (k == 'C') {
            // backoff

            ring();
            backOff();
            return;
          }

          else {
            alarm();
          }
        }
      }
    }
};

class Timer {
  private:
    long d;
    long h;
    long m;
    long s;
    long DAY;
    long HOUR;
    long MIN;
    long SEC;

    void setD(long d) {
      this->d = d;
    }

    long getD() {
      return d;
    }

    void setH(long h) {
      this->h = h;
    }

    long getH() {
      return h;
    }

    void setM(long m) {
      this->m = m;
    }

    long getM() {
      return m;
    }

    void setS(long s) {
      this->s = s;
    }

    long getS() {
      return s;
    }

    void getTimes(Keypad pad, int which) {
      char k;
      long n = 0;
      long num = 0;

      lcd.clear();
      lcd.backlight();
      lcd.setCursor(0, 0);

      for (int cur = 0; ;) {
        k = pad.getKey();

        if (k != '\0') {
          logger(pushed, false);
          logger(k);

          if (checkKey(k)) {
            if (k == '#') {
              // set x when first numbers entered
              ring();
              break;
            }

            else {
              if (cur != 2) {
                // print pushed numbers

                ring();
                lcd.backlight();
                lcd.setCursor(cur, 0);
                lcd.write(k);
                n = col(k);
                num *= 10;
                num += n;
                cur++;
              }
            }
          } else {
            wrongInput();
          }
        }
      }


      switch (which) {
        case mem::DAY:
          setD(num);
          break;

        case mem::HOUR:
          setH(num);
          break;

        case mem::MIN:
          setM(num);
          break;

        case mem::SEC:
          setS(num);
          break;
      }
    }

    long calcSeconds() {
      long secs = (getD() * 24 * 60 * 60) + (getH() * 60 * 60) + (getM() * 60) + getS();
      return secs;
    }

    void setTimes(long secs) {
      DAY = (((secs / 24) / 60) / 60) % 60;
      HOUR = ((secs / 60) / 60) % 24;
      MIN = (secs / 60) % 60;
      SEC = secs % 60;

      delete text;
      sprintf(text, "%ld:%ld:%ld:%ld", DAY, HOUR, MIN, SEC);
      printer(lcd, text, true, false);
    }


  public:
    Timer(Keypad pad) {
      printer(lcd, "Gun:Saat:Dk:Sn");
      delay(duration * 4);

      getTimes(pad, mem::DAY);
      getTimes(pad, mem::HOUR);
      getTimes(pad, mem::MIN);
      getTimes(pad, mem::SEC);
      long secs = calcSeconds();
      prev = 0;

      while (true) {
        current = millis();

        if (current - prev >= duration * 4) {
          if (secs <= 0) {
            printer(lcd, "ALARM..!");
            alarm();

            char k = pad.getKey();

            if (k != '\0') {
              logger(pushed, false);
              logger(k);
              break;
            }
          }

          else {
            setTimes(secs);
            secs -= 1;
          }

          prev = current;
        }
      }

      ring();
      backOff();
    }
};

void menus(Keypad pad) {
  const char* options[] = {"1=HESAP MAKINESI", "2=KRONOMETRE", "3=ALARM", "4=TERMOMETRE", "5=GECE LAMBASI", "6=ANA MENU"};
  char k;

  while (true) {
    for (int i = 0; i < 6; ) {
      // print possible choices

      current = millis();

      if ((current - prev) >= (duration * 3)) {
        printer(lcd, options[i]);
        i++;
        prev = current;
      }

      k = pad.getKey();

      if (k != '\0') {
        logger(pushed, false);
        logger(k);

        switch (k) {
          case '1': {
              ring();
              Calc calc(pad);
              return;
            }

          case '2': {
              ring();
              Chron chron(pad);
              return;
            }

          case '3': {
              ring();
              Timer timer(pad);
              return;
            }

          case '4': {
              ring();
              showTemperature(pad);
              return;
            }

          case '5': {
              ring();
              lightsOn(pad);
              return;
            }

          case '6':
            ring();
            backOff();
            return;

          default:
            wrongInput();
            break;
        }
      }
    }
  }
}

void anyButton(Keypad pad) {
  char key = pad.getKey();

  // when user push any button then redirect user to menus

  if (key != '\0') {
    ring();
    logger(pushed, false);
    logger(key);
    menus(pad);
  }
}

void setup() {
  // set everything up and log it through Serial unit

  for (auto l : lamps) {
    pinMode(l, OUTPUT);
  }

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(sound, OUTPUT);
  sensors.begin();
  Serial.begin(9600);
  noTone(sound);
  logger(setting);
  logger("[*] lamps are setting..");
  logger("[+] lamps are set.");
  logger("[*] Red pin is setting..");
  logger("[+] Red pin is set.");
  logger("[*] Green pin is setting..");
  logger("[+] Green pin is set.");
  logger("[*] Blue pin is setting..");
  logger("[+] Blue pin is set.");
  logger("[*] Sound pin is setting..");
  logger("[+] Sound pin is set.");
  logger("[*] Termometer is setting..");
  logger("[+] Termometer is set.");
  logger(finished);

  mainMenu(lcd);
}

void loop() {
  // fade in & out blue led till user push any button

  fading();
  anyButton(customKeypad);
}
