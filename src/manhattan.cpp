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
int etatTiroir = TIR_FERME;

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
  for (int ii = 0; ii < NB_ELEM; ii++) {
    pinMode(tableauPinBtn[ii], INPUT_PULLUP);
    pinMode(tableauPinLed[ii], OUTPUT);
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
  etatTiroir = 1;

  Serial.println("retour de ouvertureTiroir");
}

void fermetureTiroir() {
  Serial.println("appel de fermetureTiroir");

  delay(30000);
  digitalWrite(IMP_FERMETURE, HIGH);
  etatTiroir = 2;

  Serial.println("retour de fermetureTiroir");
}

void loop() {
  // drapeau de vérification de l'état des boutons
  bool buttonIsOn;
  // nombre de boutons encore enfoncés
  int nbButtonOn;

  delay(500);
  // l'énigme est résolue...
  if (etatTiroir == 2) {
    Serial.println("Stop");
    // on boucle indéfiniment
    while (true);
  }

  switch (etatAutom) {
    // état initial, raz tableauOrdDonnes, nbRead
  case EA_INIT:
    // initialisation des tables, extinction des LED
    for (int ii = 0; ii <NB_ELEM; ii++) {
      Serial.print("EA_INIT     ii = ");
      Serial.println(ii);
      tableauOrdDonnes[ii] = -1;
      tableauEtaBtn[ii] = 0;
      digitalWrite(tableauPinLed[ii],HIGH);
    }
    // RAZ du nombre de boutons appuyés
    nbRead = 0;
    etatAutom = EA_READBTN;
    // on verifie que tous les boutons sont OFF
    for (int ii = 0; ii <NB_ELEM; ii++) {
      buttonIsOn = !digitalRead(tableauPinBtn[ii]);
      if (buttonIsOn && tableauEtaBtn[ii] == 0) {
        // force l'utilisateur à remonter tous les boutons
        etatAutom = EA_FAILURE;
      }
    }
    break;

    // en attente de lecture des boutons ; tant que nbRead < NB_ELEM - 1
  case EA_READBTN:
    for (int ii = 0; ii <NB_ELEM; ii++) {
      buttonIsOn = !digitalRead(tableauPinBtn[ii]);
      Serial.println("EA_READBTN");
      Serial.print(buttonIsOn);
      Serial.print(" ");
      Serial.println(tableauEtaBtn[ii]);
      if (buttonIsOn && tableauEtaBtn[ii] == 0) {
        tableauEtaBtn[ii] = 1;
        digitalWrite(tableauPinLed[ii], LOW);
        tableauOrdDonnes[nbRead] = ii;
        nbRead++;
      }
      if (!buttonIsOn && tableauEtaBtn[ii] == 1) {
        int rang = -1;
        tableauEtaBtn[ii] = 0;
        digitalWrite(tableauPinLed[ii], HIGH);

        // retrouver le rang du bouton ii
        for (int jj = 0; jj <= nbRead; jj++)
          if (tableauOrdDonnes[jj] == ii)
            rang = jj;

        // décalage des rangs : pour jj de rang à nbRead :
        // tableauOrdDonnes[jj] = tableauOrdDonnes[jj+1]
        for (int jj = rang; jj <= nbRead; jj++)
          tableauOrdDonnes[jj] = tableauOrdDonnes[jj+1];
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
    while (etatTiroir != 2) {
      Serial.print(" for ");
      Serial.println(digitalRead(POS_BATTERIE));
      // farandole de led
      for (int ii = 0; ii <NB_ELEM; ii++) {
        digitalWrite(tableauPinLed[ii], LOW);
        delay(50);
        digitalWrite(tableauPinLed[ii], HIGH);
        delay(50);
        Serial.print(etatTiroir);
        Serial.print(" attente ");
        Serial.println(digitalRead(POS_BATTERIE));
      }
      // la batterie est-elle retirée ?
      if (digitalRead(POS_BATTERIE) != HIGH && etatTiroir != 2) {
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
      for (int ii = 0; ii <NB_ELEM; ii++) {
        digitalWrite(tableauPinLed[ii], HIGH);
        delay(5);
      }
      // allumage des LED correspondant aux boutons enfoncés
      for (int ii = 0; ii <NB_ELEM; ii++) {
        if (tableauEtaBtn[ii] == 1) {
          digitalWrite(tableauPinLed[ii], LOW);
          delay(5);
          nbButtonOn = 0;
        }
      }
      // vérifier si un bouton a changé d'état
      for (int ii = 0; ii <NB_ELEM; ii++) {
        buttonIsOn = !digitalRead(tableauPinBtn[ii]);
        if (!buttonIsOn && tableauEtaBtn[ii] == 1) {
          tableauEtaBtn[ii] = 0;
        }
        if (buttonIsOn && tableauEtaBtn[ii] == 0) {
          tableauEtaBtn[ii] = 1;
          // mémorise le nombre de boutons enfoncés
          nbButtonOn += tableauEtaBtn[ii];
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
      for (int ii = 0; ii <NB_ELEM; ii++) {
        digitalWrite(tableauPinLed[ii], HIGH);
        delay(5);
      }
    }

    // pour chaque bouton[ii] (tableauPinBtn[0 à 9])
    for (int ii = 0; ii <NB_ELEM; ii++) {
      buttonIsOn = !digitalRead(tableauPinBtn[ii]);
      // si bouton[ii] enfoncé
      if (buttonIsOn && tableauEtaBtn[ii] == 0) {
        // allumer la led[ii]
        digitalWrite(tableauPinLed[ii], LOW);
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

