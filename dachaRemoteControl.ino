#include <SoftwareSerial.h>
SoftwareSerial GSMport(3, 2); // RX, TX
String inputString;
float db;
int relay = 11;
int counter = 0;
int led = 8, b, d2;
int ledG = 7;
String phone, ign, alarm;
String phone1 = "380676560968";
String phone2 = "380970433683";
String phone3 = "380978506740";
long timing = 3600;
int value = 0;
float vout = 0.0;
float vin = 0.0;
float R1 = 100000.0; // сопротивление R1 (100K)
float R2 = 10000.0; // сопротивление R2 (10K)
int powerPin = 0;
int alarmPin = 2;
unsigned long currentMillis, previousMillis = 0;
//long timing = 3600000;

int c, v, p;
boolean timer = false;
void setup() {
  pinMode(led, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(ledG, OUTPUT);
  digitalWrite(relay, HIGH);
  digitalWrite(led, LOW);
  digitalWrite(ledG, LOW);
  Serial.begin(9600);
  Serial.println("Starting device....");
  GSMport.begin(19200);
  modemReset();
  phone = phone1;
  checkBalance();
  sms("Dacha. Pribor vkluchen! Ostatok deneg: " + String (db) + " grn.");
  delay(10000);
}//MODEM:STARTUP


void loop() {
  ReadPort(true); checkRing (); checkPower();
  if (timer == true) {
    delay(1000);
    counter++;
    if (counter == 10)sms("Nasos rabotaet! Ostatok: " + String (db) + " grn.");
    Serial.println(counter);
    if (counter == timing) {
      Serial.println("Relay off");
      digitalWrite(relay, HIGH);  // реле выключено
      timer = false;
      counter = 0;
      sms("Nasos otkluchen! Ostatok: " + String (db) + " grn.");
    }
  }
}
void checkPower() {
  int ignState = analogRead(powerPin);
  float volts = (ignState * 3.46) / 1024.0;
  float ig = volts / (R2 / (R1 + R2));
  if (ig > 3) {
    if (b == 0) {
      Serial.println("Electrichestvo otklucheno! ");
      ign = "1";  b = 1;
      sms("Dacha: Electrichestvo vklucheno! Ostatok: " + String (db) + " grn.");
    }
  }
  else {
    if (b == 1) {
      Serial.println("Electrichestvo vklucheno! ");
      ign = "0"; b = 0;
      sms("Dacha: Electrichestvo otklucheno! Ostatok: " + String (db) + " grn.");
    }
  }
}
void call () {
  GSMport.println("ATD0677990788;");
  waitData("BUSY", 7000);
}
void checkAlarm () {
  int rawReading = analogRead(alarmPin);
  float doorD = (rawReading * 3.46) / 1024.0;
  float al = doorD / (R2 / (R1 + R2));
  if (al > 3) {
    if (d2 == 0) {
      Serial.println("Alarm");
      alarm = "1"; call (); d2 = 1;
    }
  } else {
    if (d2 == 1) {
      Serial.println("Alarm off");
      alarm = "0"; d2 = 0;
    }
  }
}
void checkRing () {
  if (inputString.lastIndexOf("RING") > -1) {  //если звонок обнаружен, то проверяем номер
    Serial.println("--- RING DETECTED ---");
    delay(100);
    ReadPort(true);
    if (inputString.lastIndexOf("0676560968") > -1) { //если номер звонящего наш. Укажите свой номер без "+"
      GSMport.println("ATH0");  //разрываем связь
      phone = phone1;
      digitalWrite(relay, LOW);  // реле включено
      counter = 0;
      timer = true;
      Serial.println(timer);
    }
    if (inputString.lastIndexOf("0970433683") > -1) { //если номер звонящего наш. Укажите свой номер без "+"
      GSMport.println("ATH0");  //разрываем связь
      phone = phone2;
      digitalWrite(relay, LOW);  // реле включено
      counter = 0;
      timer = true;
      Serial.println(timer);
    }
    if (inputString.lastIndexOf("0978506740") > -1) { //если номер звонящего наш. Укажите свой номер без "+"
      GSMport.println("ATH0");  //разрываем связь
      phone = phone3;
      digitalWrite(relay, LOW);  // реле включено
      counter = 0;
      timer = true;
      Serial.println(timer);
    }
  }
}
//void relay_set() {
//  if (counter > 0)
//  {
//    digitalWrite(relay, LOW);   // реле включено
//    digitalWrite(led, HIGH);
//    counter = counter - 1;
//    delay(1000);
//    Serial.print("Min left: ");
//    Serial.println(counter / 60);
//    Serial.println(counter);
//    if (counter == 3590)sms("Nasos rabotaet! Ostatok: " + String (db) + " grn.");
//    if (counter == 1)sms("Nasos otkluchen! Ostatok: " + String (db) + " grn.");
//    { digitalWrite(led, LOW);
//      digitalWrite(relay, HIGH);  // реле выключено
//      counter = 0;
//    }
//  }
//}
void sms(String smsText) {
  GSMport.println("AT+CMGS=\"" + phone + "\""); // даем команду на отправку смс
  delay(100);
  GSMport.print(smsText);  // отправляем текст
  GSMport.print((char)26);          // символ завершающий передачу
  Serial.println("ok");
}
String ReadPort(boolean state) {
  inputString = "";
  while (GSMport.available())
  {
    char inChar = (char)GSMport.read();
    inputString += inChar;
  }
  if (state == true) {
    if (inputString.length() > 0) Serial.print(inputString);
  }
  return inputString;
}
boolean waitData (String str, int count) {
  Serial.flush(); GSMport.flush();
  int  check = -1; int err = 0; boolean status = false; int errc = -1;
  while (check < 0) {
    ReadPort(true);
    check = inputString.lastIndexOf(str);
    errc = inputString.lastIndexOf("ERROR");
    delay(10);
    err++;
    if (errc > 0) err = count;
    if (err == count) {
      check = 0;
      status = false;
      Serial.print("Error get data: ");
      delay(100);
      Serial.println(str);
    } else status = true;
  } return status;
}
void checkBalance() {
  GSMport.println("AT+CUSD=1,\"*111#\"");
  boolean get = waitData("grn", 1000);
  if (get == true) {
    int m = inputString.lastIndexOf("grn.");
    String valueName = inputString.substring(m - 6, m);
    Serial.print("Device balance: ");
    // Serial.println(valueName);
    Serial.println(valueName.toFloat());
    db = valueName.toFloat();
  }
}
void modemReset() {
  Serial.println("Reboot modem complete! Wait modem booting");
  int boot = -1;
  while (boot < 0) {
    GSMport.println("AT+CREG?");
    delay(100);
    ReadPort(false);
    boot = inputString.lastIndexOf("+CREG: 0,1");
  }
  if (boot > 0) {
    Serial.println("Registered in operator network");
    delay(5000);
    GSMport.println("AT+CLIP=1");
    waitData("OK", 100);
    GSMport.println("AT+CMGF=1");  //режим кодировки СМС - обычный (для англ.)
    waitData("OK", 100);
    GSMport.println("AT+CSCS=\"GSM\"");  //режим кодировки текста
    waitData("OK", 100);
    GSMport.println("AT+CMEE=2");
    waitData("OK", 100);
  }
}
