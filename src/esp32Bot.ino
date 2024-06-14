#include "Arduino.h"

// pinler tanımlandı
#define echoPinR 18 
#define trigPinR 5 

#define echoPinL 22 
#define trigPinL 23 

#define MotorR1 12 
#define MotorR2 14 

#define MotorL1 25 
#define MotorL2 26 

#define MotorA 27 
#define MotorB 13 
#define stby 33

//değişkenler ve sabitler tanımlandı 

#define fullSpeedR 255
#define halfSpeedR 200
#define fullSpeedL 255
#define halfSpeedL 200

#define SOUND_VELOCITY 0.034

long durationR;
long durationL;
float distanceCmR;
float distanceCmL;
bool firstCall = false;

const int pwmaChannel = 0;
const int pwmbChannel = 1;
const int freq = 30000;
const int resolution = 8;

static unsigned long previousMillis = 0;


void setup() {
  //pinlerin atamaları yapıldı
  Serial.begin(115200);

  pinMode(echoPinR, INPUT);
  pinMode(trigPinR, OUTPUT);
  pinMode(echoPinL, INPUT);
  pinMode(trigPinL, OUTPUT);
  pinMode(MotorR1, OUTPUT);
  pinMode(MotorR2, OUTPUT);
  pinMode(MotorA, OUTPUT);
  pinMode(MotorL1, OUTPUT);
  pinMode(MotorL2, OUTPUT);
  pinMode(MotorB, OUTPUT);
  pinMode(stby, OUTPUT);

  ledcSetup(pwmaChannel, freq, resolution);
  ledcSetup(pwmbChannel, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(MotorA, pwmaChannel);
  ledcAttachPin(MotorB, pwmbChannel);
}



// tam hız ileri gidiş fonksiyonu tanımlandı

void tamyolIleri() {
  digitalWrite(stby, HIGH); // Motorları etkinleştir
  ledcWrite(pwmaChannel, fullSpeedR);
  ledcWrite(pwmbChannel, fullSpeedL);
  //analogWrite(MotorA, fullSpeedR);
  //analogWrite(MotorB, fullSpeedL);
  digitalWrite(MotorR1, HIGH); 
  digitalWrite(MotorR2, LOW); 

  digitalWrite(MotorL1, HIGH); 
  digitalWrite(MotorL2, LOW); 

  //Serial.println("TAM HIZ İLERİ GİDİYOR !!!!!");
}

// yarı hız ileri gidiş fonksiyonu tanımlandı

void yarimyolIleri() {
  digitalWrite(stby, HIGH); // Motorları etkinleştir
  ledcWrite(pwmaChannel, halfSpeedR);
  ledcWrite(pwmbChannel, halfSpeedL);
  //analogWrite(MotorA, halfSpeedR);
  //analogWrite(MotorB, halfSpeedL);
  digitalWrite(MotorR1, HIGH); 
  digitalWrite(MotorR2, LOW); 

  digitalWrite(MotorL1, HIGH); 
  digitalWrite(MotorL2, LOW);

  //Serial.println("YARIM HIZ İLERİ GİDİYOR !!!!!");
}

// çalışma döngüsü kuruldu

void loop() {

mesafe();
   //Serial.println("önce !!!!!");
/*
// her 200ms'de bir kere mesafe fonksiyonunu çağırıp sensörlerden mesafe bilgilerini al
 previousMillis = millis();
  while(millis() - previousMillis <= 100) {
    mesafe();
  }
  */
//Serial.println("sonra !!!!!");
 
  if ((distanceCmR >= 50) && (distanceCmL >= 50)) { //  2 si de 50+ ise tam hız ileri git
    // iki motor tam yol ileri
    tamyolIleri();
      mesafe();
  } 

  else if ((20 < distanceCmR && distanceCmR < 50) || (20 < distanceCmL && distanceCmL < 50)) {
    yarimyolIleri();
    // mesafe 20ile 50 arasında ise yarım hız ile ilerle
      mesafe();
  } 

  else if ((5 <= distanceCmL && distanceCmL <= 20)) {
    // sol sensör 20cmden daha yakında nesne algıladıysa sağa dön

    previousMillis = millis();
    while(millis() - previousMillis <= 100){    
      //dönüş işleminde motorların zarar görmemesi için her dönüşte önce motorları durdur
      dur();
    }

    previousMillis = millis();
     while(millis() - previousMillis <= 300) {
      //sağa dön
      sag(); 
    }

    previousMillis = millis();
    while(millis() - previousMillis <= 50) {
      dur();
      // dönüş işlemlerinde bir taraf ileri giderken diğer taraf geri geldiği için yine motorları komple durdur
    }
    yarimyolIleri();
    // dönüş işlemi bittikten sonra yarım hız ile yola devam et
     mesafe();
  }
 
   else if ((5 <= distanceCmR && distanceCmR <= 20)) {
    // sağ sensör 20cmden daha yakında nesne algıladıysa sola dön

    previousMillis = millis();
    while(millis() - previousMillis <= 100) {
      dur();     
    }

    previousMillis = millis();
    while(millis() - previousMillis <= 300) {
      sol();
    }
    previousMillis = millis();
    while(millis() - previousMillis <= 50) {
      dur();
    }
    yarimyolIleri();
  mesafe();
  } 

   //mesafe sağ ve sol 5 cmden az kalırsa önce dur 500 ms bekle sonra geri git
  else if (distanceCmR < 5 || distanceCmL < 5) {
    // ani nesne çıkması durumunda dur-geri git- dur- sağa dön- dur- yarım hız ilerle

    previousMillis = millis();
    while(millis() - previousMillis <= 100) {
      dur();
    }

    previousMillis = millis();
    while (millis() - previousMillis <= 500) {
      geri();
      
    }

    previousMillis = millis();
    while(millis() - previousMillis <= 100) {
      dur();
    }

    previousMillis = millis();
    while(millis() - previousMillis <= 200) {
      sag();
    }

    previousMillis = millis();
    while(millis() - previousMillis <= 100) {
      dur(); 
    }
  yarimyolIleri();
  }

}


void sag() { // Robotun sağa dönme hareketi için fonksiyon tanımlıyoruz.
  //analogWrite(MotorA, halfSpeedR);
  //analogWrite(MotorB, halfSpeedL);
  digitalWrite(stby, HIGH); // Motorları etkinleştir

  ledcWrite(pwmaChannel, halfSpeedR);
  ledcWrite(pwmbChannel, halfSpeedL);
  
  // sağa dönüş için sağ motor geri
  digitalWrite(MotorR1, LOW); 
  digitalWrite(MotorR2, HIGH); 
  // sağa dönüş için sol motor ileri
  digitalWrite(MotorL1, HIGH); 
  digitalWrite(MotorL2, LOW); 

  //Serial.println("SAĞA GİDİYOR !!!!!");
}
void sol() {
  digitalWrite(stby, HIGH); // Motorları etkinleştir

  //analogWrite(MotorA, halfSpeedR);
  //analogWrite(MotorB, halfSpeedL);
  ledcWrite(pwmaChannel, halfSpeedR);
  ledcWrite(pwmbChannel, halfSpeedL);
  
  // sola dönüş için sağ motor ileri
  digitalWrite(MotorR1, HIGH);
  digitalWrite(MotorR2, LOW);
  //sola dönüş için sol motor geri
  digitalWrite(MotorL1, LOW);
  digitalWrite(MotorL2, HIGH);

  //Serial.println("SOLA GİDİYOR !!!!!");
}

void geri() { // Robotun geri yönde hareketi için fonksiyon tanımlıyoruz.
  digitalWrite(stby, HIGH); // Motorları etkinleştir

  ledcWrite(pwmaChannel, halfSpeedR);
  ledcWrite(pwmbChannel, halfSpeedL);

  // iki motor da geri
  digitalWrite(MotorR1, LOW); 
  digitalWrite(MotorR2, HIGH); 

  digitalWrite(MotorL1, LOW); 
  digitalWrite(MotorL2, HIGH); 
  //Serial.println("GERİ GİDİYOR !!!!!");
}

void dur() {
 // digitalWrite(stby, LOW); // Motorları etkinleştir

  // tüm motorlar ve pwm değerleri sıfır olarak değiştirildi.
  //ledcWrite(pwmaChannel, 0);
  //ledcWrite(pwmbChannel, 0);
  //analogWrite(MotorA, 0);
  //analogWrite(MotorB, 0);
  digitalWrite(MotorR1, LOW);
  digitalWrite(MotorR2, LOW);
  digitalWrite(MotorL1, LOW);
  digitalWrite(MotorL2, LOW);
  //Serial.println("DURDU!!!!!");
}
// hc-sr04 okuma rutini
void mesafe() {
  // Clears the trigPin
  digitalWrite(trigPinR, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinR, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinR, LOW);

  durationR = pulseIn(echoPinR, HIGH);
  distanceCmR = durationR * SOUND_VELOCITY / 2;

  digitalWrite(trigPinL, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinL, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinL, LOW);

  durationL = pulseIn(echoPinL, HIGH);
  distanceCmL = durationL * SOUND_VELOCITY / 2;

  //Serial.println("distanceCmR:");
  //Serial.println(distanceCmR);

  //Serial.println("distanceCmL:");
  //Serial.println(distanceCmL);
}
