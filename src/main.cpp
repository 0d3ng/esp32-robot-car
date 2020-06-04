#include <Arduino.h>
#include <BluetoothSerial.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial serial;

// Motor kiri
#define KIRI_en 25
#define KIRI_in1 32
#define KIRI_in2 33
// Motor kanan
#define KANAN_en 14
#define KANAN_in1 26
#define KANAN_in2 27

#define LED_FRONT_1 13
#define LED_BACK_1 12
#define LED_HORN 15

int kecepatan = 100;
int command;
int speed_Coeff = 4;
boolean lightFront = false;
boolean lightBack = false;
boolean horn = false;

// Setting PWM properties
const int freq = 30000, freqBuzzer = 2000;
const int pwmChannel0 = 0;
const int pwmChannel1 = 1;
const int pwmChannel2 = 2;
const int resolution = 8;

TaskHandle_t blink;

/*
Semua roda maju ketika maju, semua diberikan HIGH
Semua roda mundur ketika mundur, semua diberikan LOW
*/
void arah(bool maju)
{
  Serial.print("Maju ");
  Serial.println(maju);
  // motor kiri
  digitalWrite(KIRI_in1, maju ? LOW : HIGH);
  digitalWrite(KIRI_in2, !maju ? LOW : HIGH);

  // motor kanan
  digitalWrite(KANAN_in1, maju ? LOW : HIGH);
  digitalWrite(KANAN_in2, !maju ? LOW : HIGH);
  ledcWrite(pwmChannel0, kecepatan);
  ledcWrite(pwmChannel1, kecepatan);
}

/*
Ketika belok kanan
Roda depan belakang sebelah kanan diam, sedangkan roda depan belakang sebelah kiri maju

Ketika belok kiri
Roda depan belakang sebelah kiri diam, sedangkan roda depan belakang sebelah kanan maju
*/
void belok(bool kanan)
{
  Serial.print("Belok ");
  Serial.println(kanan);
  digitalWrite(KIRI_in1, kanan ? LOW : LOW);
  digitalWrite(KIRI_in2, !kanan ? LOW : HIGH);

  digitalWrite(KANAN_in1, kanan ? LOW : LOW);
  digitalWrite(KANAN_in2, !kanan ? HIGH : LOW);
  ledcWrite(pwmChannel0, kecepatan);
  ledcWrite(pwmChannel1, kecepatan);
}

void belok_maju(bool kanan)
{
  Serial.print("Kanan ");
  Serial.println(kanan);
  digitalWrite(KIRI_in1, kanan ? LOW : HIGH);
  digitalWrite(KIRI_in2, !kanan ? LOW : HIGH);

  digitalWrite(KANAN_in1, kanan ? HIGH : LOW);
  digitalWrite(KANAN_in2, !kanan ? HIGH : LOW);
  ledcWrite(pwmChannel0, kanan ? (kecepatan) : (kecepatan / speed_Coeff));
  ledcWrite(pwmChannel1, !kanan ? (kecepatan) : (kecepatan / speed_Coeff));
}

void belok_mundur(bool kanan)
{
  Serial.print("Kanan ");
  Serial.println(kanan);
  digitalWrite(KIRI_in1, kanan ? HIGH : LOW);
  digitalWrite(KIRI_in2, !kanan ? HIGH : LOW);
  digitalWrite(KANAN_in1, kanan ? LOW : HIGH);
  digitalWrite(KANAN_in2, !kanan ? LOW : HIGH);
  ledcWrite(pwmChannel0, kanan ? (kecepatan) : (kecepatan / speed_Coeff));
  ledcWrite(pwmChannel1, !kanan ? (kecepatan) : (kecepatan / speed_Coeff));
}

void berhenti()
{
  Serial.println("Berhenti");
  digitalWrite(KIRI_in1, LOW);
  digitalWrite(KIRI_in2, LOW);
  digitalWrite(KANAN_in1, LOW);
  digitalWrite(KANAN_in2, LOW);
  ledcWrite(pwmChannel0, kecepatan);
  ledcWrite(pwmChannel1, kecepatan);
}

//Task1code: blinks an LED every 1000 ms
void Task1code(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

void setup()
{
  Serial.begin(115200);
  serial.begin("robot-car");

  pinMode(KIRI_en, OUTPUT);
  pinMode(KIRI_in1, OUTPUT);
  pinMode(KIRI_in2, OUTPUT);

  pinMode(KANAN_en, OUTPUT);
  pinMode(KANAN_in1, OUTPUT);
  pinMode(KANAN_in2, OUTPUT);

  pinMode(LED_FRONT_1, OUTPUT);
  pinMode(LED_BACK_1, OUTPUT);
  pinMode(LED_HORN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  ledcSetup(pwmChannel0, freq, resolution);
  ledcSetup(pwmChannel1, freq, resolution);
  ledcSetup(pwmChannel2, freqBuzzer, resolution);
  ledcAttachPin(KIRI_en, pwmChannel0);
  ledcAttachPin(KANAN_en, pwmChannel1);
  ledcAttachPin(LED_HORN, pwmChannel2);

  xTaskCreatePinnedToCore(Task1code, "Blink", 10000, NULL, 1, &blink, 0);
  delay(500);
  Serial.println("Robot car 4WD ready...");
}

void loop()
{
  if (serial.available())
  {
    command = serial.read();
    berhenti();
    Serial.write(command);

    if (lightFront)
    {
      Serial.println("Light front");
      digitalWrite(LED_FRONT_1, HIGH);
    }
    if (!lightFront)
    {
      digitalWrite(LED_FRONT_1, LOW);
    }

    if (lightBack)
    {
      Serial.println("Light back");
      digitalWrite(LED_BACK_1, HIGH);
    }
    if (!lightBack)
    {
      digitalWrite(LED_BACK_1, LOW);
    }

    if (horn)
    {
      Serial.println("Horn");
      ledcWriteTone(pwmChannel2, freqBuzzer);
    }
    if (!horn)
    {
      ledcWriteTone(pwmChannel2, LOW);
    }

    switch (command)
    {
    case 'F':
      arah(true);
      break;
    case 'B':
      arah(false);
      break;
    case 'R':
      belok(true);
      break;
    case 'L':
      belok(false);
      break;
    case 'I':
      belok_maju(true);
      break;
    case 'G':
      belok_maju(false);
      break;
    case 'J':
      belok_mundur(true);
      break;
    case 'H':
      belok_mundur(false);
      break;
    case '0':
      kecepatan = 100;
      break;
    case '1':
      kecepatan = 115;
      break;
    case '2':
      kecepatan = 130;
      break;
    case '3':
      kecepatan = 145;
      break;
    case '4':
      kecepatan = 160;
      break;
    case '5':
      kecepatan = 175;
      break;
    case '6':
      kecepatan = 190;
      break;
    case '7':
      kecepatan = 205;
      break;
    case '8':
      kecepatan = 220;
      break;
    case '9':
      kecepatan = 235;
      break;
    case 'q':
      kecepatan = 255;
      break;
    case 'W':
      lightFront = true;
      break;
    case 'w':
      lightFront = false;
      break;
    case 'U':
      lightBack = true;
      break;
    case 'u':
      lightBack = false;
      break;
    case 'V':
      horn = true;
      break;
    case 'v':
      horn = false;
      break;
    }
  }
}