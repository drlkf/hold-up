#define VERSION "Programme de gestion des LED et d'ouverture tiroir sous Manhattan Version 6.0"

#ifdef UNIT_TEST
  #include "ArduinoFake.h"
#else
  #include "Arduino.h"
#endif

#include "manhattan.h"

button boutons[NB_ELEM];

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
    pinMode(ref_boutons[i].pin_button, INPUT_PULLUP);
    pinMode(ref_boutons[i].pin_led, OUTPUT);
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

// eteindre toutes les leds
void extinctionLeds() {
  for (int i = 0; i < NB_ELEM; i++) {
    digitalWrite(ref_boutons[i].pin_led, HIGH);
    delay(5);
  }
}

void farandoleLed() {
  for (int i = 0; i < NB_ELEM; i++) {
    digitalWrite(ref_boutons[i].pin_led, LOW);
    delay(50);
    digitalWrite(ref_boutons[i].pin_led, HIGH);
    delay(50);
    Serial.print(etatTiroir);
    Serial.print(" attente ");
    Serial.println(digitalRead(POS_BATTERIE));
  }
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
      boutons[i].order = -1;
      boutons[i].is_pressed = false;
      digitalWrite(ref_boutons[i].pin_led, HIGH);
    }
    // RAZ du nombre de boutons appuyés
    nbRead = 0;
    etatAutom = EA_READBTN;
    // on verifie que tous les boutons sont OFF
    for (int i = 0; i < NB_ELEM; i++) {
      buttonIsOn = !digitalRead(ref_boutons[i].pin_button);
      if (buttonIsOn && !boutons[i].is_pressed) {
        // force l'utilisateur à remonter tous les boutons
        etatAutom = EA_FAILURE;
      }
    }
    break;

    // en attente de lecture des boutons ; tant que nbRead < NB_ELEM - 1
  case EA_READBTN:
    Serial.println("EA_READBTN");
    for (int i = 0; i < NB_ELEM; i++) {
      buttonIsOn = !digitalRead(ref_boutons[i].pin_button);
      Serial.print(buttonIsOn);
      Serial.print(" ");
      Serial.println(boutons[i].is_pressed);
      if (buttonIsOn && !boutons[i].is_pressed) {
        boutons[i].is_pressed = true;
        digitalWrite(ref_boutons[i].pin_led, LOW);
        boutons[nbRead].order = i;
        nbRead++;
      }
      if (!buttonIsOn && boutons[i].is_pressed) {
        int rang = -1;
        boutons[i].is_pressed = false;
        digitalWrite(ref_boutons[i].pin_led, HIGH);

        // retrouver le rang du bouton i
        for (int j = 0; j <= nbRead; j++)
          if (boutons[j].order == i)
            rang = j;

        // décalage des rangs : pour j de rang à nbRead :
        // tableauOrdDonnes[j] = tableauOrdDonnes[j+1]
        for (int j = rang; j <= nbRead; j++)
          boutons[j].order = boutons[j+1].order;

        nbRead--;
      }
    }

    // au moins 9 boutons sur 10 appuyés
    // V4 if (nbRead == NB_ELEM - 1) {
    // 10 boutons sur 10 appuyés
    if (nbRead == NB_ELEM) {
      etatAutom = EA_CHECK;
    }
    break;

    // vérification du code
  case EA_CHECK:
    etatAutom = (verif_code(boutons)) ? EA_SUCCESS : EA_FAILURE;
    break;

    // le code est bon
  case EA_SUCCESS:
    ouvertureTiroir();
    // jouer la farandole de led jusqu'au retrait de la batterie
    while (etatTiroir != TIR_REFERME) {
      Serial.print(" for ");
      Serial.println(digitalRead(POS_BATTERIE));
      farandoleLed();

      // la batterie est-elle retirée ?
      if (digitalRead(POS_BATTERIE) != HIGH && etatTiroir != TIR_REFERME)
        fermetureTiroir();
    }
    break;

    // erreur : on force l'utilisateur à remonter tous les boutons
  case EA_FAILURE:
    nbButtonOn = 1;
    // tant qu'il y a au moins un bouton enfoncé
    while (nbButtonOn > 0) {
      nbButtonOn = 0;

      extinctionLeds();
      // allumage des LED correspondant aux boutons enfoncés
      for (int i = 0; i < NB_ELEM; i++) {
        if (boutons[i].is_pressed) {
          digitalWrite(ref_boutons[i].pin_led, LOW);
          delay(5);
        }
      }
      // vérifier si un bouton a changé d'état
      for (int i = 0; i < NB_ELEM; i++) {
        buttonIsOn = !digitalRead(ref_boutons[i].pin_button);

        if (!buttonIsOn && boutons[i].is_pressed) {
          boutons[i].is_pressed = false;
        }

        if (buttonIsOn && !boutons[i].is_pressed) {
          boutons[i].is_pressed = true;
          // mémorise le nombre de boutons enfoncés
          nbButtonOn++;
        }
      }
    }

    etatAutom = EA_INIT;
    break;

  case EA_FAILURE2:
    // nombre de boutons encore enfoncés
    int nbButtonOn_bis = 0;

    extinctionLeds();

    // pour chaque bouton[i] (ref_boutons[0 à 9])
    for (int i = 0; i < NB_ELEM; i++) {
      buttonIsOn = !digitalRead(ref_boutons[i].pin_button);
      // si bouton[i] enfoncé
      if (buttonIsOn && !boutons[i].is_pressed) {
        // allumer la led[i]
        digitalWrite(ref_boutons[i].pin_led, LOW);
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

