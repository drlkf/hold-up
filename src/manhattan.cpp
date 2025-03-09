#define VERSION "Programme de gestion des LED et d'ouverture tiroir sous Manhattan Version 6.0"

#ifdef UNIT_TEST
  #include "ArduinoFake.h"
#else
  #include "Arduino.h"
#endif

#include "manhattan.h"

int nbRead;
int etatAutom, etatAutomPrec;

// position initiale du tiroir
etat_tiroir etat_tiroir = TIR_FERME;

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

void ouverture_tiroir() {
  long debut = millis();

  Serial.println("appel de ouverture_tiroir");

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
  etat_tiroir = TIR_OUVERT;

  Serial.println("retour de ouverture_tiroir");
}

void fermeture_tiroir() {
  Serial.println("appel de fermeture_tiroir");

  delay(30000);
  digitalWrite(IMP_FERMETURE, HIGH);
  etat_tiroir = TIR_REFERME;

  Serial.println("retour de fermeture_tiroir");
}
}

// eteindre toutes les leds
void extinction_leds() {
  for (int i = 0; i < NB_ELEM; i++) {
    digitalWrite(ref_boutons[i].pin_led, HIGH);
    delay(5);
  }
}

void farandole_led() {
  for (int i = 0; i < NB_ELEM; i++) {
    digitalWrite(ref_boutons[i].pin_led, LOW);
    delay(50);
    digitalWrite(ref_boutons[i].pin_led, HIGH);
    delay(50);
    Serial.print(etat_tiroir);
    Serial.print(" attente ");
    Serial.println(digitalRead(POS_BATTERIE));
  }
}

/*
  boucle principale
 */

void loop() {
  // drapeau de vérification de l'état des boutons
  bool buttonIsOn;
  // nombre de boutons encore enfoncés
  int nbButtonOn;
  // etat des boutons poussoir
  button boutons[NB_ELEM];

  delay(500);
  // l'énigme est résolue...
  if (etat_tiroir == TIR_REFERME) {
    Serial.println("Stop");
    // on boucle indéfiniment
    while (true)
      delay(10000);
  }

  switch (etatAutom) {
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
        // FIXME est-ce que c'est ce qu'on veut? retrouver l'ordre si un bouton
        // est desactive, au lieu de demander un raz?
        for (int j = rang; j <= nbRead; j++)
          boutons[j].order = boutons[j+1].order;

        nbRead--;
      }
    }

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
    ouverture_tiroir();
    // jouer la farandole de led jusqu'au retrait de la batterie
    while (etat_tiroir != TIR_REFERME) {
      Serial.print(" for ");
      Serial.println(digitalRead(POS_BATTERIE));
      farandole_led();

      // la batterie est-elle retirée ?
      if (digitalRead(POS_BATTERIE) != HIGH && etat_tiroir != TIR_REFERME)
        fermeture_tiroir();
    }
    break;

    // erreur : on force l'utilisateur à remonter tous les boutons
  case EA_FAILURE:
    nbButtonOn = 1;
    // tant qu'il y a au moins un bouton enfoncé
    while (nbButtonOn > 0) {
      nbButtonOn = 0;

      extinction_leds();
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

    extinction_leds();

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

