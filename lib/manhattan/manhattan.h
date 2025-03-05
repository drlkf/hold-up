#ifndef MANHATTAN_H
#define MANHATTAN_H

// contact d'extraction de la batterie
#define POS_BATTERIE 7

// nombre de couples bouton-led à gérer
#define NB_ELEM 10

// temps en secondes de fonctionnement du moteur pour ouverture du tiroir
#define DEPL_TIROIR 10.5

enum Etat {
  // état initial, raz TableauOrdDonnes, nbRead
  EA_INIT = 5,
  // en attente de lecture des boutons ; tant que nbRead < NB_ELEM - 1
  EA_READBTN = 10,
  // vérification ordre de saisie
  EA_CHECK = 15,
  // ouverture du tiroir
  EA_SUCCESS = 20,
  EA_FAILURE = 25,
  // clignotement des LED tant qu'il reste un bouton enfoncé
  EA_FAILURE2 = 30
};

enum ImpulsionMoteur {
  // relais du moteur sens ouverture
  IMP_OUVERTURE = 6,
  // relais du moteur sens fermeture
  IMP_FERMETURE = 5
};

enum EtatTiroir {
  TIR_FERME,
  TIR_OUVERT,
  TIR_REFERME
};

// le bouton de rang ii gère la led de même rang
// les boutons
const int tableauPinBtn[NB_ELEM] = {30, 32, 34, 36, 38, 40, 42, 44, 46, 48};
// les LED
const int tableauPinLed[NB_ELEM] = {31, 33, 35, 37, 39, 41, 43, 45, 47, 49};

const int tableauOrdRequis[NB_ELEM] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

bool verif_code(const int gauche[NB_ELEM], const int droite[NB_ELEM]);

#endif
