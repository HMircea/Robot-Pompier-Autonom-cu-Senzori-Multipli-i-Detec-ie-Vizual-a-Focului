// Pini L298N
#define IN1 7   // motor stânga
#define IN2 8
#define IN3 9   // motor dreapta
#define IN4 10
#define BUZZER 6

// Pini senzor HC-SR04
#define TRIG 11
#define ECHO 12

#define DIST_OBSTACOL 20

// ---------- CALIBRARE (ajustează pe robot) ----------
#define TIMP_SPATE   350   // ms cât dă înapoi după obstacol
#define TIMP_90      450   // ms pentru viraj de 90° pe loc
#define TIMP_PAS     400   // ms înainte ≈ 5 cm (pune lățimea robotului dacă vrei fără suprapunere)

bool urmatorulDreapta = true;   // true = următorul viraj e dreapta, false = stânga

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  opreste();
  delay(1000);
}

// ---------- Motoare individuale ----------
void stanga(bool fata) {
  digitalWrite(IN1, fata ? HIGH : LOW);
  digitalWrite(IN2, fata ? LOW : HIGH);
}

void dreapta(bool fata) {
  digitalWrite(IN3, fata ? LOW : HIGH);
  digitalWrite(IN4, fata ? HIGH : LOW);
}

// ---------- Mișcări ----------
void inainte()     { stanga(true);  dreapta(true);  }
void inapoi()      { stanga(false); dreapta(false); }
void viraStanga()  { stanga(false); dreapta(true);  }
void viraDreapta() { stanga(true);  dreapta(false); }

void opreste() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
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
    digitalWrite(BUZZER, HIGH);
    delay(durata);
    digitalWrite(BUZZER, LOW);
    delay(100);
  }
}

void viraj90(bool spreDreapta) {
  if (spreDreapta) viraDreapta();
  else             viraStanga();
  delay(TIMP_90);
  opreste();
  delay(200);
}

// Camera acoperită: oprește definitiv
void gata() {
  opreste();
  bip(5, 400);
  while (true) { }
}

// ---------- Schimbare culoar (serpentină) ----------
void schimbaCuloar() {
  opreste();
  bip(1, 150);

  // 1. dă puțin înapoi
  inapoi();
  delay(TIMP_SPATE);
  opreste();
  delay(200);

  // 2. primul viraj de 90°
  viraj90(urmatorulDreapta);

  // 3. dacă imediat în lateral e perete => am ajuns în colțul opus, gata
  if (distantaFiltrata() < DIST_OBSTACOL) {
    gata();
  }

  // 4. pas lateral ≈ 5 cm
  inainte();
  delay(TIMP_PAS);
  opreste();
  delay(200);

  // 5. al doilea viraj de 90° în aceeași parte => acum merge în sens opus
  viraj90(urmatorulDreapta);

  // 6. data viitoare virează în partea cealaltă
  urmatorulDreapta = !urmatorulDreapta;
}

// ---------- Program principal ----------
void loop() {
  long d = distantaFiltrata();
  Serial.println(d);   // DEBUG: șterge după ce merge

  if (d < DIST_OBSTACOL) {
    schimbaCuloar();
  } else {
    inainte();
  }
}