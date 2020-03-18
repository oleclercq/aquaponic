#define PIN_ECLVEG        2
#define PIN_BULLEUR       4
#define PIN_POMPE         6
#define PIN_VANNE         8
#define PIN_FLOTTEUR_BAS  22
#define PIN_FLOTTEUR_HAUT 23


 void test_sorties(int);
 void test_entrees(int);
    
void setup() {
  // put your setup code here, to run once:

  pinMode(PIN_ECLVEG, OUTPUT); // eclairage led
  pinMode(PIN_POMPE, OUTPUT); // pompe a eau
  pinMode(PIN_VANNE, OUTPUT); // vanne de drainnage
  pinMode(PIN_BULLEUR, OUTPUT); // pompe a aire
  pinMode(PIN_FLOTTEUR_BAS, INPUT); // capteur  niveau eau flotteur bas
  pinMode(PIN_FLOTTEUR_HAUT, INPUT); // capteur niveau eau flotteur  haut

  Serial.begin(9600);
  Serial.println("PGM pour tester les pin Out");
}

void loop() {
  test_sorties(1000); // 1s
 //   test_entrees(1000); // 1s
}

void test_entrees(int pause) 
{

  // A voir ici csi t'as pas une inversion de lecture, entre ton code et l'état réel de l'eau
  // car c'est état de floteur quei permettte de démarer ton pgm.
  
    if (digitalRead(PIN_FLOTTEUR_BAS) == HIGH ) 
      {Serial.println("PIN_FLOTTEUR_BAS HIGHT");}
      else
      {Serial.println("PIN_FLOTTEUR_BAS LOW");}

    if (digitalRead(PIN_FLOTTEUR_HAUT) == HIGH ) 
      {Serial.println("PIN_FLOTTEUR_HAUT HIGHT");}
      else
      {Serial.println("PIN_FLOTTEUR_HAUT LOW");}

      delay(pause);
}


void test_sorties(int pause) {
//  regarde bien si les message séries correspondent bien aus pilotages du périphérique. ( pas d'inversion)
  digitalWrite(PIN_ECLVEG,HIGH);     Serial.println("ECLAIRAGE_ON");
  delay(pause); 
  digitalWrite(PIN_ECLVEG,LOW);     Serial.println("ECLAIRAGE_OFF");
  delay(pause);

  digitalWrite(PIN_BULLEUR,HIGH);     Serial.println("BULLEUR_ON");
  delay(pause); 
  digitalWrite(PIN_BULLEUR,LOW);     Serial.println("BULLEUR_OFF");
  delay(pause);

  digitalWrite(PIN_POMPE,HIGH);     Serial.println("POMPE_ON");
  delay(pause); 
  digitalWrite(PIN_POMPE,LOW);     Serial.println("POMPE_OFF");
  delay(pause);

  digitalWrite(PIN_VANNE,HIGH);     Serial.println("VANNE_ON");
  delay(pause); 
  digitalWrite(PIN_VANNE,LOW);     Serial.println("VANNE_OFF");
  delay(pause);
}
