#define VERSION "Programme de gestion des LED et d'ouverture tiroir sous Manhattan Version 6.0"

#ifdef UNIT_TEST
  #include "ArduinoFake.h"
#else
  #include "Arduino.h"
#endif

#include "manhattan.h"

int tableauEtaBtn[NB_ELEM];

int tableauOrdDonnes[NB_ELEM];
int nbRead;
int etatAutom, etatAutomPrec;

// position initiale du tiroir
EtatTiroir etatTiroir = TIR_FERME;

/*
  Actions Arduino
 */

void setup() {
  // initialisation du port série pour débogage
  Serial.begin(9600);
  // permet d'attendre que le buffer d'envoi des données soit vide
  Serial.flush();
  // lecture de la présence de la batterie
  pinMode(POS_BATTERIE, INPUT);
  // sortie vers relais ouverture
  pinMode(IMP_OUVERTURE, OUTPUT);
  // sortie vers relais fermeture
  pinMode(IMP_FERMETURE, OUTPUT);
  // ouverture des relais = arrêt moteur
  digitalWrite(IMP_FERMETURE, HIGH);
  // id
  digitalWrite(IMP_OUVERTURE, HIGH);
  Serial.println("Depart");
  // initialisation des tables
  for (int i = 0; i < NB_ELEM; i++) {
    pinMode(tableauPinBtn[i], INPUT_PULLUP);
    pinMode(tableauPinLed[i], OUTPUT);
  }
  Serial.println("Depart2");
  etatAutom = EA_INIT;
}

void ouvertureTiroir() {
  long debut = millis();

  Serial.println("appel de ouvertureTiroir");

  digitalWrite(IMP_OUVERTURE, LOW);
  delay(1000);
  digitalWrite(IMP_OUVERTURE, HIGH);
  Serial.print(debut);
  Serial.print(" ");
  Serial.println(millis() - debut);

  while ((DEPL_TIROIR * 1000) > (millis() - debut)) {
    Serial.println("Ca tourne... ");
  }

  digitalWrite(IMP_OUVERTURE, HIGH);
  // arrêt moteur
  digitalWrite(IMP_FERMETURE, LOW);
  // le tiroir est ouvert
  etatTiroir = TIR_OUVERT;

  Serial.println("retour de ouvertureTiroir");
}

void fermetureTiroir() {
  Serial.println("appel de fermetureTiroir");

  delay(30000);
  digitalWrite(IMP_FERMETURE, HIGH);
  etatTiroir = TIR_REFERME;

  Serial.println("retour de fermetureTiroir");
}

void loop() {
  // drapeau de vérification de l'état des boutons
  bool buttonIsOn;
  // nombre de boutons encore enfoncés
  int nbButtonOn;

  delay(500);
  // l'énigme est résolue...
  if (etatTiroir == TIR_REFERME) {
    Serial.println("Stop");
    // on boucle indéfiniment
    while (true)
      delay(10000);
  }

  switch (etatAutom) {
    // état initial, raz tableauOrdDonnes, nbRead
  case EA_INIT:
    // initialisation des tables, extinction des LED
    for (int i = 0; i < NB_ELEM; i++) {
      Serial.print("EA_INIT     i = ");
      Serial.println(i);
      tableauOrdDonnes[i] = -1;
      tableauEtaBtn[i] = 0;
      digitalWrite(tableauPinLed[i], HIGH);
    }
    // RAZ du nombre de boutons appuyés
    nbRead = 0;
    etatAutom = EA_READBTN;
    // on verifie que tous les boutons sont OFF
    for (int i = 0; i < NB_ELEM; i++) {
      buttonIsOn = !digitalRead(tableauPinBtn[i]);
      if (buttonIsOn && tableauEtaBtn[i] == 0) {
        // force l'utilisateur à remonter tous les boutons
        etatAutom = EA_FAILURE;
      }
    }
    break;

    // en attente de lecture des boutons ; tant que nbRead < NB_ELEM - 1
  case EA_READBTN:
    Serial.println("EA_READBTN");
    for (int i = 0; i < NB_ELEM; i++) {
      buttonIsOn = !digitalRead(tableauPinBtn[i]);
      Serial.print(buttonIsOn);
      Serial.print(" ");
      Serial.println(tableauEtaBtn[i]);
      if (buttonIsOn && tableauEtaBtn[i] == 0) {
        tableauEtaBtn[i] = 1;
        digitalWrite(tableauPinLed[i], LOW);
        tableauOrdDonnes[nbRead] = i;
        nbRead++;
      }
      if (!buttonIsOn && tableauEtaBtn[i] == 1) {
        int rang = -1;
        tableauEtaBtn[i] = 0;
        digitalWrite(tableauPinLed[i], HIGH);

        // retrouver le rang du bouton i
        for (int j = 0; j <= nbRead; j++)
          if (tableauOrdDonnes[j] == i)
            rang = j;

        // décalage des rangs : pour j de rang à nbRead :
        // tableauOrdDonnes[j] = tableauOrdDonnes[j+1]
        for (int j = rang; j <= nbRead; j++)
          tableauOrdDonnes[j] = tableauOrdDonnes[j+1];
        nbRead--;
      }
    }

    // au moins 9 boutons sur 10 appuyés
    // V4 if (nbRead == NB_ELEM - 1) {
    // 10 boutons sur 10 appuyés
    if (nbRead == NB_ELEM - 0) {
      etatAutom = EA_CHECK;
    }
    break;

    // vérification du code
  case EA_CHECK:
    etatAutom = (verif_code(tableauOrdDonnes, tableauOrdRequis)) ? EA_SUCCESS : EA_FAILURE;
    break;

    // le code est bon
  case EA_SUCCESS:
    ouvertureTiroir();
    while (etatTiroir != TIR_REFERME) {
      Serial.print(" for ");
      Serial.println(digitalRead(POS_BATTERIE));
      // farandole de led
      for (int i = 0; i < NB_ELEM; i++) {
        digitalWrite(tableauPinLed[i], LOW);
        delay(50);
        digitalWrite(tableauPinLed[i], HIGH);
        delay(50);
        Serial.print(etatTiroir);
        Serial.print(" attente ");
        Serial.println(digitalRead(POS_BATTERIE));
      }
      // la batterie est-elle retirée ?
      if (digitalRead(POS_BATTERIE) != HIGH && etatTiroir != TIR_REFERME) {
        fermetureTiroir();
      }
    }
    break;

    // erreur : on force l'utilisateur à remonter tous les boutons
  case EA_FAILURE:
    nbButtonOn = 0;
    // tant qu'il y a au moins un bouton enfoncé
    while (1) {
      // extinction des LED
      for (int i = 0; i < NB_ELEM; i++) {
        digitalWrite(tableauPinLed[i], HIGH);
        delay(5);
      }
      // allumage des LED correspondant aux boutons enfoncés
      for (int i = 0; i < NB_ELEM; i++) {
        if (tableauEtaBtn[i] == 1) {
          digitalWrite(tableauPinLed[i], LOW);
          delay(5);
          nbButtonOn = 0;
        }
      }
      // vérifier si un bouton a changé d'état
      for (int i = 0; i < NB_ELEM; i++) {
        buttonIsOn = !digitalRead(tableauPinBtn[i]);
        if (!buttonIsOn && tableauEtaBtn[i] == 1) {
          tableauEtaBtn[i] = 0;
        }
        if (buttonIsOn && tableauEtaBtn[i] == 0) {
          tableauEtaBtn[i] = 1;
          // mémorise le nombre de boutons enfoncés
          nbButtonOn += tableauEtaBtn[i];
        }
      }
      if (nbButtonOn == 0) {
        // retour à l'état initial
        etatAutom = EA_INIT;
        break;
      }
    }
    break;

  case EA_FAILURE2:
    // nombre de boutons encore enfoncés
    int nbButtonOn_bis = 0;
    while (1) {
      // eteindre toutes les leds
      for (int i = 0; i < NB_ELEM; i++) {
        digitalWrite(tableauPinLed[i], HIGH);
        delay(5);
      }
    }

    // pour chaque bouton[i] (tableauPinBtn[0 à 9])
    for (int i = 0; i < NB_ELEM; i++) {
      buttonIsOn = !digitalRead(tableauPinBtn[i]);
      // si bouton[i] enfoncé
      if (buttonIsOn && tableauEtaBtn[i] == 0) {
        // allumer la led[i]
        digitalWrite(tableauPinLed[i], LOW);
        // nbButtonOn++
        nbButtonOn_bis++;
      }
    }

    if (nbButtonOn_bis == 0) {
      // retour à l'état initial
      etatAutom = EA_INIT;
    }
    delay(500);
    break;
  }
}

