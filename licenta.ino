// Pini L298N
#define IN1 7
#define IN2 8
#define IN3 9
#define IN4 10
#define BUZZER 6

// HC-SR04
#define TRIG 11
#define ECHO 12

#define DIST_OBSTACOL 20   // cm

// ---------- CALIBRARE ----------
#define TIMP_SPATE      400    // ms înapoi după obstacol
#define TIMP_VIRAJ_MIN  300    // ms viraj minim (~50-60°)
#define TIMP_VIRAJ_MAX  900    // ms viraj maxim (~160-180°)

bool ultimaDreapta = true;
int  obstacoleLaRand = 0;
unsigned long ultimulObstacol = 0;

void setup() {
  Serial.begin(9600);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  digitalWrite(BUZZER, LOW);
  opreste();
  randomSeed(analogRead(A0));
  delay(1000);
}

// ---------- Motoare ----------
void stanga(bool fata) {
  digitalWrite(IN1, fata ? HIGH : LOW);
  digitalWrite(IN2, fata ? LOW : HIGH);
}
void dreapta(bool fata) {
  digitalWrite(IN3, fata ? LOW : HIGH);
  digitalWrite(IN4, fata ? HIGH : LOW);
}
void inainte()     { stanga(true);  dreapta(true);  }
void inapoi()      { stanga(false); dreapta(false); }
void viraStanga()  { stanga(false); dreapta(true);  }
void viraDreapta() { stanga(true);  dreapta(false); }
void opreste() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

// ---------- Senzor ----------
long masoaraDistanta() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long durata = pulseIn(ECHO, HIGH, 30000);
  if (durata == 0) return 400;
  return durata * 0.034 / 2;
}

long distantaFiltrata() {
  long suma = 0;
  for (int i = 0; i < 3; i++) {
    suma += masoaraDistanta();
    delay(10);
  }
  return suma / 3;
}

// ---------- Utilitare ----------
void bip(int ori, int durata) {
  for (int i = 0; i < ori; i++) {
    digitalWrite(BUZZER, HIGH); delay(durata);
    digitalWrite(BUZZER, LOW);  delay(100);
  }
}

void vireaza(bool spreDreapta, int ms) {
  if (spreDreapta) viraDreapta();
  else             viraStanga();
  delay(ms);
  opreste();
  delay(150);
}

// ---------- Evitare obstacol ----------
void eviteObstacol() {
  opreste();
  bip(1, 100);

  inapoi();
  delay(TIMP_SPATE);
  opreste();
  delay(150);

  // direcție random, cu tendință să alterneze (acoperire mai uniformă)
  bool dreapta = (random(100) < 65) ? !ultimaDreapta : ultimaDreapta;
  ultimaDreapta = dreapta;

  // unghi random; dacă dă mereu în obstacole (colț), viraj mare
  int ms = random(TIMP_VIRAJ_MIN, TIMP_VIRAJ_MAX);
  if (obstacoleLaRand >= 3) {
    ms = TIMP_VIRAJ_MAX + 200;   // aprox. întoarcere completă
    obstacoleLaRand = 0;
  }
  vireaza(dreapta, ms);

  // dacă încă e obstacol în față, mai virează în pași mici
  int incercari = 0;
  while (distantaFiltrata() < DIST_OBSTACOL && incercari < 10) {
    vireaza(dreapta, 200);
    incercari++;
  }
}

// ---------- Program principal ----------
void loop() {
  long d = distantaFiltrata();
  Serial.println(d);   // DEBUG: șterge după ce merge

  if (d < DIST_OBSTACOL) {
    // obstacole la mai puțin de 3 s unul de altul = probabil colț
    if (millis() - ultimulObstacol < 3000) obstacoleLaRand++;
    else                                   obstacoleLaRand = 0;
    ultimulObstacol = millis();

    eviteObstacol();
  } else {
    inainte();
  }
}
