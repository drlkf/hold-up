
// #define VERSION "Programme de gestion des LED et d'ouverture tiroir sous Manhattan Version 2.0"
// Cette version utilise un piston linéaire
// Cette version a été validée sur site le 03/07/2024

// #define VERSION "Programme de gestion des LED et d'ouverture tiroir sous Manhattan Version 4.0"
// il faut que les 10 boutons soit enfoncés pour pouvoir faire le check de l'ordre d'enfoncement des boutons
// modif et validation sur site le 2024/08/11

// #define VERSION "Programme de gestion des LED et d'ouverture tiroir sous Manhattan Version 5.0"
/*  changement mode de gestion du case EA_FAILURE ==> EA_FAILURE2
    EA_FAILURE2 : loop
                    eteindre toutes les leds
                    // nombre de boutons encore enfoncés
                    nbButtonOn = 0;
                    pour chaque bouton[ii] ( TblPinBtn[0 à 9] )
                      si bouton[ii] enfoncé
                        allumer la led[ii]
                        nbButtonOn++
                      fsi
                    fpour
                    si nbButtonOn == 0
                      // retour à l'état initial
                      EtatAutom = EA_INIT;
                      // sortir de la loop
                      break
                    fsi
                    delay(500)
                  endLoop
*/

#define VERSION "Programme de gestion des LED et d'ouverture tiroir sous Manhattan Version 6.0"

#ifdef UNIT_TEST
  #include "ArduinoFake.h"
#else
  #include "Arduino.h"
#endif

#include "manhattan.h"

int TblEtaBtn[NB_ELEM];

int TblOrdDonnes[NB_ELEM - 0];
int NbRead;
int EtatAutom, EtatAutomPrec;

// position initiale du tiroir
int etatTiroir = TIR_FERME;

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
  for ( int ii = 0; ii < NB_ELEM; ii++ ) {
    pinMode(TblPinBtn[ii], INPUT_PULLUP);
    pinMode(TblPinLed[ii], OUTPUT);
  }
  Serial.println("Depart2");
  EtatAutom = EA_INIT;
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

  while ((DEPL_TIROIR * 1000) > (millis() - debut )) {
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
  Serial.println("Fermeture tiroir");

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

  switch (EtatAutom) {
    // état initial, raz TblOrdDonnes, NbRead
  case EA_INIT:
    // initialisation des tables, extinction des LED
    for ( int ii = 0; ii <NB_ELEM; ii++ ) {
      Serial.print("EA_INIT     ii = ");
      Serial.println(ii);
      TblOrdDonnes[ii] = -1;
      TblEtaBtn[ii] = 0;
      digitalWrite(TblPinLed[ii],HIGH);
    }
    // RAZ du nombre de boutons appuyés
    NbRead = 0;
    EtatAutom = EA_READBTN;
    // on verifie que tous les boutons sont OFF
    for ( int ii = 0; ii <NB_ELEM; ii++ ) {
      buttonIsOn = !digitalRead(TblPinBtn[ii]);
      if ( buttonIsOn && TblEtaBtn[ii] == 0 ) {
        // force l'utilisateur à remonter tous les boutons
        EtatAutom = EA_FAILURE;
      }
    }
    break;

  // en attente de lecture des boutons ; tant que NbRead < NB_ELEM - 1
  case EA_READBTN:
    for ( int ii = 0; ii <NB_ELEM; ii++ ) {
      buttonIsOn = !digitalRead(TblPinBtn[ii]);
      Serial.println("EA_READBTN");
      Serial.print(buttonIsOn);
      Serial.print(" ");
      Serial.println(TblEtaBtn[ii]);
      if ( buttonIsOn && TblEtaBtn[ii] == 0 ) {
        TblEtaBtn[ii] = 1;
        digitalWrite(TblPinLed[ii], LOW);
        TblOrdDonnes[NbRead] = ii;
        NbRead++;
      }
      if ( !buttonIsOn && TblEtaBtn[ii] == 1 ) {
        int rang = -1;
        TblEtaBtn[ii] = 0;
        digitalWrite(TblPinLed[ii], HIGH);

        // retrouver le rang du bouton ii
        for ( int jj = 0; jj <= NbRead; jj++ )
          if ( TblOrdDonnes[jj] == ii )
            rang = jj;

        // décalage des rangs : pour jj de rang à NbRead :
        // TblOrdDonnes[jj] = TblOrdDonnes[jj+1]
        for ( int jj = rang; jj <= NbRead; jj++ )
          TblOrdDonnes[jj] = TblOrdDonnes[jj+1];
        NbRead--;
      }
    }

    // au moins 9 boutons sur 10 appuyés
    // V4 if ( NbRead == NB_ELEM - 1 ) {
    // 10 boutons sur 10 appuyés
    if ( NbRead == NB_ELEM - 0 ) {
      EtatAutom = EA_CHECK;
    }
    break;

  // vérification du code
  case EA_CHECK:
    EtatAutom = EA_SUCCESS;

    // V4 for ( int ii = 0; ii <NB_ELEM - 1; ii++ ) {
    for ( int ii = 0; ii <NB_ELEM - 0; ii++ ) {
      if ( TblOrdDonnes[ii] != TblOrdRequis[ii] ) {
        EtatAutom = EA_FAILURE;
        break;
      }
    }
    break;

  // le code est bon
  case EA_SUCCESS:
    ouvertureTiroir();
    while (etatTiroir != 2) {
      Serial.print(" for ");
      Serial.println(digitalRead(POS_BATTERIE));
      // farandole de led
      for ( int ii = 0; ii <NB_ELEM; ii++ ) {
        digitalWrite(TblPinLed[ii], LOW);
        delay(50);
        digitalWrite(TblPinLed[ii], HIGH);
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
      for ( int ii = 0; ii <NB_ELEM; ii++ ) {
        digitalWrite(TblPinLed[ii], HIGH);
        delay(5);
      }
      // allumage des LED correspondant aux boutons enfoncés
      for ( int ii = 0; ii <NB_ELEM; ii++ ) {
        if ( TblEtaBtn[ii] == 1 ) {
          digitalWrite(TblPinLed[ii], LOW);
          delay(5);
          nbButtonOn = 0;
        }
      }
      // vérifier si un bouton a changé d'état
      for ( int ii = 0; ii <NB_ELEM; ii++ ) {
        buttonIsOn = !digitalRead(TblPinBtn[ii]);
        if ( !buttonIsOn && TblEtaBtn[ii] == 1 ) {
          TblEtaBtn[ii] = 0;
        }
        if ( buttonIsOn && TblEtaBtn[ii] == 0 ) {
          TblEtaBtn[ii] = 1;
          // mémorise le nombre de boutons enfoncés
          nbButtonOn += TblEtaBtn[ii];
        }
      }
      if ( nbButtonOn == 0 ) {
        // retour à l'état initial
        EtatAutom = EA_INIT;
        break;
      }
    }
    break;

  case EA_FAILURE2:
    // nombre de boutons encore enfoncés
    int nbButtonOn_bis = 0;
    while (1) {
      // eteindre toutes les leds
      for ( int ii = 0; ii <NB_ELEM; ii++ ) {
        digitalWrite(TblPinLed[ii], HIGH);
        delay(5);
      }
    }

    // pour chaque bouton[ii] ( TblPinBtn[0 à 9] )
    for ( int ii = 0; ii <NB_ELEM; ii++ ) {
      buttonIsOn = !digitalRead(TblPinBtn[ii]);
      // si bouton[ii] enfoncé
      if ( buttonIsOn && TblEtaBtn[ii] == 0 ) {
        // allumer la led[ii]
        digitalWrite(TblPinLed[ii], LOW);
        // nbButtonOn++
        nbButtonOn_bis++;
      }
    }

    if ( nbButtonOn_bis == 0 ) {
      // retour à l'état initial
      EtatAutom = EA_INIT;
    }
    delay(500);
    break;
  }
}

