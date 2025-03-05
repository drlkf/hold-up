
// #define VERSION "Programme de gestion des LED et d'ouverture tiroir sous Manhattan Version 2.0"
// Cette version utilise un piston linéaire
// Cette version a été validée sur site le 03/07/2024

#define VERSION "Programme de gestion des LED et d'ouverture tiroir sous Manhattan Version 4.0"
//  il faut que les 10 boutons soit enfoncés pour pouvoir faire le check de l'ordre d'enfoncement des boutons
//  modif et validation sur site le 2024/08/11

#define VERSION "Programme de gestion des LED et d'ouverture tiroir sous Manhattan Version 5.0"
/*  changement mode de gestion du case EA_FAILURE ==> EA_FAILURE2
    EA_FAILURE2 : loop 
                    eteindre toutes les leds
                    nbButtonOn = 0;                                                   // nombre de boutons encore enfoncés
                    pour chaque bouton[ii] ( TblPinBtn[0 à 9] )
                      si bouton[ii] enfoncé
                        allumer la led[ii]
                        nbButtonOn++
                      fsi
                    fpour
                    si nbButtonOn == 0
                      EtatAutom = EA_INIT;                                              //  retour à l'état initial
                      break                                                             //  sortir de la loop
                    fsi
                    delay(500)
                  endLoop
*/
const int positionBatterie = 7;                                             // contact d'extraction de la batterie
const int impulsOuverture = 6;                                              // relais du moteur sens ouverture
const int impulsFermeture = 5;                                              // relais du moteur sens fermeture

const int NB_ELEM    = 10;                                                  // nombre de couples bouton-led à gérer

const int EA_INIT    =  5;                                                  // état initial, raz TblOrdDonnes, NbRead
const int EA_READBTN = 10;                                                  // en attente de lecture des boutons ; tant que NbRead < NB_ELEM - 1
const int EA_CHECK   = 15;                                                  // vérification ordre de saisie
const int EA_SUCCESS = 20;                                                  // ouverture du tiroir
const int EA_FAILURE = 25;   
const int EA_FAILURE2= 30;                                                 // clignotement des LED tant qu'il reste un bouton enfoncé 

const int deplacementTiroir = 10.5;                                           // temps en secondes de fonctionnement du moteur pour ouverture du tiroir

// le bouton de rang ii gère la led de même rang
int TblPinBtn[NB_ELEM] = {30, 32, 34, 36, 38, 40, 42, 44, 46, 48};          // les boutons
int TblPinLed[NB_ELEM] = {31, 33, 35, 37, 39, 41, 43, 45, 47, 49};          // les LED
int TblEtaBtn[NB_ELEM];

// V4 int TblOrdRequis[NB_ELEM - 1] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
int TblOrdRequis[NB_ELEM - 0] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

int TblOrdDonnes[NB_ELEM - 0];
int NbRead;
int EtatAutom, EtatAutomPrec;
int etatTiroir = 0;                                                         // position initiale du tiroir

bool buttonIsOn;                                                            // drapeau de vérification de l'état des boutons


void setup() {
  Serial.begin(9600);                                                       // initialisation du port série pour débogage
  Serial.flush();                                                           // permet d'attendre que le buffer d'envoi des données soit vide
  pinMode(positionBatterie, INPUT);                                         // lecture de la présence de la batterie
  pinMode(impulsOuverture, OUTPUT);                                         // sortie vers relais ouverture
  pinMode(impulsFermeture, OUTPUT);                                         // sortie vers relais fermeture
  digitalWrite(impulsFermeture, HIGH);                                      // ouverture des relais = arrêt moteur
  digitalWrite(impulsOuverture, HIGH);                                      // id
  Serial.println("Depart");
  for ( int ii = 0; ii < NB_ELEM; ii++ ) {                                  // initialisation des tables
    pinMode(TblPinBtn[ii], INPUT_PULLUP);
    pinMode(TblPinLed[ii], OUTPUT);
  }
  Serial.println("Depart2");
  EtatAutom = EA_INIT;
}


void loop() {
  delay(500);
  if (etatTiroir == 2){                                                     // l'énigme est résolue... 
    Serial.println("Stop");
    while (true);                                                           // on boucle indéfiniment
  }

  switch (EtatAutom) 
  {  case EA_INIT:                                                           // état initial, raz TblOrdDonnes, NbRead
      for ( int ii = 0; ii <NB_ELEM; ii++ ){                                // initialisation des tables, extinction des LED
        Serial.print("EA_INIT     ii = ");Serial.println(ii);
        TblOrdDonnes[ii] = -1;
        TblEtaBtn[ii] = 0;
        digitalWrite(TblPinLed[ii],HIGH);
      }
      NbRead = 0;                                                           // RAZ du nombre de boutons appuyés
      EtatAutom = EA_READBTN;
      for ( int ii = 0; ii <NB_ELEM; ii++ ){                                // on verifie que tous les boutons sont OFF
        buttonIsOn = !digitalRead(TblPinBtn[ii]);
        if ( buttonIsOn && TblEtaBtn[ii] == 0 ){
          EtatAutom = EA_FAILURE;                                           // force l'utilisateur à remonter tous les boutons
        }
      }
      break;

    case EA_READBTN:                                                        // en attente de lecture des boutons ; tant que NbRead < NB_ELEM - 1
      for ( int ii = 0; ii <NB_ELEM; ii++ ){
        buttonIsOn = !digitalRead(TblPinBtn[ii]);
        Serial.println("EA_READBTN");
        Serial.print(buttonIsOn);
        Serial.print(" ");
        Serial.println(TblEtaBtn[ii]);
        if ( buttonIsOn && TblEtaBtn[ii] == 0 ){
          TblEtaBtn[ii] = 1;
          digitalWrite(TblPinLed[ii], LOW);
          TblOrdDonnes[NbRead] = ii;
          NbRead++;
        }
        if ( !buttonIsOn && TblEtaBtn[ii] == 1 ){
          int rang = -1;
          TblEtaBtn[ii] = 0;
          digitalWrite(TblPinLed[ii], HIGH);
          for ( int jj = 0; jj <= NbRead; jj++ )                            // retrouver le rang du bouton ii
            if ( TblOrdDonnes[jj] == ii )
              rang = jj;
          for ( int jj = rang; jj <= NbRead; jj++ )                         // décalage des rangs : pour jj de rang à NbRead : TblOrdDonnes[jj] = TblOrdDonnes[jj+1]
            TblOrdDonnes[jj] = TblOrdDonnes[jj+1];
          NbRead--;
        }
      }

// V4 if ( NbRead == NB_ELEM - 1 ){                                         // au moins 9 boutons sur 10 appuyés
      if ( NbRead == NB_ELEM - 0 ){                                         //         10 boutons sur 10 appuyés

        EtatAutom = EA_CHECK;
      }
      break;

    case EA_CHECK:                                                          // vérification du code
      EtatAutom = EA_SUCCESS;

// V4 for ( int ii = 0; ii <NB_ELEM - 1; ii++ ){
      for ( int ii = 0; ii <NB_ELEM - 0; ii++ ){

        if ( TblOrdDonnes[ii] != TblOrdRequis[ii] ){
          EtatAutom = EA_FAILURE;
          break;
        }
      }
      break;

    case EA_SUCCESS:                                                        // le code est bon
      Serial.println("appel de ouvertureTiroir");
      ouvertureTiroir();                                                    // ouverture du tiroir
      Serial.println("retour de ouvertureTiroir");
      while (etatTiroir != 2){
        Serial.print(" for ");
        Serial.println(digitalRead(positionBatterie));
        for ( int ii = 0; ii <NB_ELEM; ii++ ){                              // farandole de led
          digitalWrite(TblPinLed[ii], LOW);
          delay(50);
          digitalWrite(TblPinLed[ii], HIGH);
          delay(50);
          Serial.print(etatTiroir);
          Serial.print(" attente ");
          Serial.println(digitalRead(positionBatterie));
        }
        if (digitalRead(positionBatterie) != HIGH && etatTiroir != 2){      // la batterie est-elle retirée ?
            Serial.println("Fermé1");
            fermetureTiroir();
            Serial.println("Fermé2"); 
        }
      }
      break;

    case EA_FAILURE:                                                        // erreur : on force l'utilisateur à remonter tous les boutons
      int nbButtonOn = 0;                                                   // nombre de boutons encore enfoncés
      while (1){                                                            // tant qu'il y a au moins un bouton enfoncé
        for ( int ii = 0; ii <NB_ELEM; ii++ ){                              // extinction des LED
          digitalWrite(TblPinLed[ii], HIGH);
          delay(5);
        }
        for ( int ii = 0; ii <NB_ELEM; ii++ ){                              // allumage des LED correspondant aux boutons enfoncés
          if ( TblEtaBtn[ii] == 1 ){
            digitalWrite(TblPinLed[ii], LOW);
            delay(5);
            nbButtonOn = 0;
          }
        }
        for ( int ii = 0; ii <NB_ELEM; ii++ ){                              // vérifier si un bouton a changé d'état
          buttonIsOn = !digitalRead(TblPinBtn[ii]);
          if ( !buttonIsOn && TblEtaBtn[ii] == 1 ){
            TblEtaBtn[ii] = 0;
          }
          if ( buttonIsOn && TblEtaBtn[ii] == 0 ){
            TblEtaBtn[ii] = 1;
            nbButtonOn += TblEtaBtn[ii];                                    // mémorise le nombre de boutons enfoncés
          }
        }
        if ( nbButtonOn == 0 ){
          EtatAutom = EA_INIT;                                              //  retour à l'état initial
          break;
        }
      }       // end while
      break;  // end case

    case EA_FAILURE2 : 
      int nbButtonOn_bis = 0;                                                   // nombre de boutons encore enfoncés
      while (1) {
        for ( int ii = 0; ii <NB_ELEM; ii++ ){                              // eteindre toutes les leds
          digitalWrite(TblPinLed[ii], HIGH);
          delay(5);
        }

      } 

      for ( int ii = 0; ii <NB_ELEM; ii++ )                         //  pour chaque bouton[ii] ( TblPinBtn[0 à 9] )
      {  buttonIsOn = !digitalRead(TblPinBtn[ii]);
        if ( buttonIsOn && TblEtaBtn[ii] == 0 )                     //  si bouton[ii] enfoncé
        { digitalWrite(TblPinLed[ii], LOW);                         //    allumer la led[ii]
          nbButtonOn_bis++;                                             //    nbButtonOn++
        }
      }

      if ( nbButtonOn_bis == 0 ){
        EtatAutom = EA_INIT;                                              //  retour à l'état initial
      }
      delay(500);
      break;

  }           // end switch                                
} // end loop


void ouvertureTiroir(){
  long debut = millis();
  digitalWrite(impulsOuverture, LOW);
  delay(1000);
  digitalWrite(impulsOuverture, HIGH);
  Serial.print(debut);
  Serial.print(" ");
  Serial.println(millis() - debut);
  while ((deplacementTiroir * 1000) > (millis() - debut )){
    Serial.println("Ca tourne... ");
  }
  digitalWrite(impulsOuverture, HIGH);
  digitalWrite(impulsFermeture, LOW);                                       // arrêt moteur
  etatTiroir = 1;                                                           // le tiroir est ouvert
}


void fermetureTiroir(){
  delay(30000);              
  digitalWrite(impulsFermeture, HIGH);
  etatTiroir = 2;
}
