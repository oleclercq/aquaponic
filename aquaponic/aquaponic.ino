#ifdef LIB_RTC
#include <Wire.h>
#include "RTClib.h" //librairie horloge rtc
RTC_DS1307 rtc; // module rtc  ds1302
#endif
// ================================================================================
// DEFINES
#define PIN_ECLVEG        2
#define PIN_BULLEUR       4
#define PIN_POMPE         6
#define PIN_VANNE         8
#define PIN_FLOTTEUR_BAS  22
#define PIN_FLOTTEUR_HAUT 23

#define   NB_DE_CYCLE_POUR_ATTENDRE_UNE_SECONDE 100 // 100 x valeurTimer = 100x10ms = 1000ms  = 1s; //   peux-tu m'expliquer  brièvement à quoi sert cette ligne de code s'il te plaît??
#define   TEMPO_MARRE_BASSE      10  // exemple: 10 seconce ( 10 x NB_DE_CYCLE_POUR_ATTENDRE_UNE_SECONDE)
#define   TEMPO_MARRE_HAUTE  15 // exemple 15s
#define   VITESSE_RS232 9600
 
// ================================================================================
// TYPES  Ça veut dire quoi type?
typedef  enum { BAC_SE_VIDE,  MAREE_BASSE, BAC_SE_REMPLIE, MAREE_HAUTE, } ENUM_ETAT_BAC ; // etat du bac culture aquaponique
ENUM_ETAT_BAC eEtatBac = BAC_SE_VIDE; // on initialise au vidagen et la fonction vidage est un bon point d'entrée du processus

int duree_marre_haute = TEMPO_MARRE_HAUTE;
int duree_marre_basse = TEMPO_MARRE_BASSE;

bool FLOTTEUR_BAS;
bool FLOTTEUR_HAUT;

 
// ================================================================================
// PROTOTYPAGES DES FONCTIONS
void fc_bac_se_remplie(void);
void fc_maree_basse(void);
void fc_maree_haute(void);
void fc_bac_se_vide(void);

void eclairage(void);
void irrigation(void);
void lectureCapteur(void);

void VANNE_ON()  {  digitalWrite(PIN_VANNE, LOW);   Serial.println("VANNE_ON");}
void VANNE_OFF() {  digitalWrite(PIN_VANNE, HIGH);  Serial.println("VANNE_OFF");}
void POMPE_ON()  {  digitalWrite(PIN_POMPE, LOW);   Serial.println("POMPE_ON");}
void POMPE_OFF() {  digitalWrite(PIN_POMPE, HIGH);  Serial.println("POMPE_OFF");}
void BULLEUR_ON()  {  digitalWrite(PIN_BULLEUR, LOW);   Serial.println("BULLEUR_ON");}
void BULLEUR_OFF() {  digitalWrite(PIN_BULLEUR, HIGH);  Serial.println("BULLEUR_OFF");}
void ECLAIRAGE_ON()  {  digitalWrite(PIN_ECLVEG, LOW);   Serial.println("ECLAIRAGE_ON");}
void ECLAIRAGE_OFF() {  digitalWrite(PIN_ECLVEG, HIGH);  Serial.println("ECLAIRAGE_OFF");}

void setup()
{
  FLOTTEUR_BAS  = false;
  FLOTTEUR_HAUT = false;
  ////////////////////horloge////////////////
  Serial.begin(VITESSE_RS232);
 #ifdef LIB_RTC
  if (! rtc.begin()) //probleme
  {
    Serial.println("Horloge RTC non trouvé");
    while (1);
  }
  if (! rtc.isrunning())
  {
    Serial.println("RTC non initialisé");
    //Paramètre le temps à l'envoi du code
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // rtc.adjust(DateTime(2019, 4, 7, 6, 23, 0));
  }
 #endif
  pinMode( PIN_ECLVEG, OUTPUT); // eclairage led
  pinMode(PIN_POMPE, OUTPUT); // pompe a eau
  pinMode(PIN_VANNE, OUTPUT); // vanne de drainnage
  pinMode(PIN_BULLEUR, OUTPUT); // pompe a aire
  pinMode(PIN_FLOTTEUR_BAS, INPUT); // capteur  niveau eau flotteur bas
  pinMode(PIN_FLOTTEUR_HAUT, INPUT); // capteur niveau eau flotteur  haut

 
//  ================================================================================
//  PARAMETRAGE DU TIMER 1 16BIT     à quoi correspondent tous ces lignes de code est-ce que c'est la durée de la temporisation ? Où je peux rentrer la durée de la temporisation
 
  cli(); // ??
 
  TCCR1A = 0;
  TCCR1B = ( 1<<WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10) ;  // Prescaler / 64 ==> des PAS de 5ms
  TCCR1C = 0;                         // ne pas forcer la comparaison de sortie
  TCNT1 = 0;                          // définir la valeur initiale du compteur de la minuterie(16 bit value)
 
  OCR1A = 2500;                       // Plus proche d'une seconde que la valeur ci-dessus, 2500 pas de 4�s ca fait 10ms
 
  TIMSK1 = 1 << OCIE1A;                     // Activer la minuterie comparer la correspondance avec l'interruption 1A
 
  sei(); //??
}//J'ai traduit en français mais j'ai quand même du mal à comprendre
 
 
 
/* ************************************************************************ */
/* PROGRAMME PRINCIPALE, ON FAIT RIEN, puisque tous est gere par timer
/* ************************************************************************ */
void loop()
{
}
 
/* ************************************************************************ */
/* ON RENTRE DANS CE TIMER TOUTES LES 10ms   pourquoi tous les  10 micros secondes ??
 
/* ************************************************************************ */
ISR(TIMER1_COMPA_vect) // 16 bit timer 1 compare 1A match
{
  static int cmptOneSeconde = NB_DE_CYCLE_POUR_ATTENDRE_UNE_SECONDE ; // 100x10ms = 1000ms = 1s
  static int etatLed = 0;
  if(cmptOneSeconde-- == 0)
  {
	  
	// on fait clignoter la led  toutes les 2s ( 1s ON 1s OFF)
	if (etatLed == 0)
	{	digitalWrite(LED_BUILTIN,HIGH);	
	}
	else 
	{	digitalWrite(LED_BUILTIN,LOW);	
	}
	etatLed = 1- etatLed;
	  
	  
    // on rentre dans ce if toute les seconde.
    lectureCapteur();
    irrigation();
    eclairage();
    cmptOneSeconde = NB_DE_CYCLE_POUR_ATTENDRE_UNE_SECONDE ; // pour le prochain passage.
  }
}


/* ******************************************************************************** */
// gestion irrigation
/* ******************************************************************************** */
void lectureCapteur(void)
{
  FLOTTEUR_BAS  = !digitalRead(PIN_FLOTTEUR_BAS);
  FLOTTEUR_HAUT = !digitalRead(PIN_FLOTTEUR_HAUT);

// SECURITE
  if( FLOTTEUR_HAUT && FLOTTEUR_BAS)
  {
    Serial.println("ERR APP");
    POMPE_OFF();
    VANNE_ON();
    BULLEUR_OFF();
    // eEtatBac = BAC_SE_VIDE;
  }

if (FLOTTEUR_BAS) { Serial.println("FLOTTEUR_BAS");}
if (FLOTTEUR_HAUT) { Serial.println("FLOTTEUR_HAUT");}
}

 
/* ******************************************************************************** */
// gestion irrigation
/* ******************************************************************************** */
void irrigation() //À quoi sert cette fonction ? c'est pas doublon par rapport au ligne ci-dessus ?
{
 
  switch (eEtatBac)
  {
    case  BAC_SE_VIDE:         fc_bac_se_vide();       break;
    case  MAREE_BASSE :        fc_maree_basse();       break;
    case  BAC_SE_REMPLIE:      fc_bac_se_remplie();    break;
    case  MAREE_HAUTE:         fc_maree_haute();       break;
    default:  Serial.println("ERR APP"); // break;              // Cas qui ne doit jammais arrivé, faire un RESET !
  }
}

 
/* ******************************************************************************** */
// le bac de culture est a moitier vide
/* ******************************************************************************** */
void fc_bac_se_vide()
{
      Serial.println("fc_bac_se_vide");
      if (FLOTTEUR_BAS)  {            
        POMPE_OFF(); 
        VANNE_OFF(); 
        eEtatBac = MAREE_BASSE; // une fois  que le bac redevient vide et que flotteur bas et le flotteur haut son declancher on passe a l'etat bac de repos
        
        Serial.print("fc_maree_basse");
        duree_marre_basse = TEMPO_MARRE_BASSE;
      } else {
        VANNE_ON(); 
        POMPE_OFF(); 
      }
}

 
/* ******************************************************************************** */
/* La marré est basse pendnat un certain temsp */
/* ******************************************************************************** */
void fc_maree_basse()
{
      Serial.print("   duree_marre_basse=");Serial.println(duree_marre_basse);
      if (duree_marre_basse-- ==0)
      {
        // Fin de la marée basse
        VANNE_OFF();
        POMPE_ON();
        eEtatBac = BAC_SE_REMPLIE;
      }
      else  {
          // Toujours en marrée base. // Tempo en cours...
      }
}
 
/* ******************************************************************************** */
// le bac de culture est a moitier remplis D'eau
/* ******************************************************************************** */
void fc_bac_se_remplie()
{
      Serial.println("fc_bac_se_remplie");
      if (FLOTTEUR_HAUT)    // on est arrivé en haut, (pas la peine de vérifier le capteur du bas)
      {  
        POMPE_OFF();  
        BULLEUR_ON();
        eEtatBac = MAREE_HAUTE; // une fois que le flotteur bas et haut sont eclancher on passe a l'etat maree haut
        Serial.println("fc_maree_haute()");  
         duree_marre_haute = TEMPO_MARRE_HAUTE;
      }
      else
      {
        //VANNE_OFF(); Pas util car déja fait dans fc_maree_basse()
        //POMPE_ON();
      }
}    
/* ******************************************************************************** */
/* La marré est haute pendnat un certain temsp                                      */
/* ******************************************************************************** */
void fc_maree_haute()
{
      Serial.print("   duree_marre_haute = "); Serial.println(duree_marre_haute);
      // Pas de condition puisqu'ici on est déja a marré haute
      if (duree_marre_haute-- ==0)
      {
          BULLEUR_OFF();
          VANNE_ON();
          eEtatBac = BAC_SE_VIDE;
      }
      else
      {
        // toujours la tempo pour la maree haute
      }
     
}
 
/* ******************************************************************************** */
// gestion_eclairage
/* ******************************************************************************** */
void eclairage(void)
{
#ifdef LIB_RTC
  DateTime now = rtc.now();
  if ((now.hour() >= 21) && (now.hour() <= 15)) //programmateur horaire pour elairage led  PROBLEME "NOW" NON RECONUE PAR LE COMPILATEUR
  {
    ECLAIRAGE_ON(); 
  } else {
    ECLAIRAGE_OFF();
  }
#endif
}
