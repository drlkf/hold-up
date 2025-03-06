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

// le bouton de rang i gère la led de même rang
struct button_data {
  // les boutons
  int pin_button;
  // les LED
  int pin_led;
  // l'ordre dans lequel le bouton doit etre appuye
  int ord_requis;
};

// structure logique d'un bouton
struct button {
  // est-ce que le bouton est enfonce
  bool is_pressed;
  // ordre dans lequel le bouton a ete appuye
  int order;
};

const button_data ref_boutons[NB_ELEM] = {
  {30, 31, 0},
  {32, 33, 1},
  {34, 35, 2},
  {36, 37, 3},
  {38, 39, 4},
  {40, 41, 5},
  {42, 43, 6},
  {44, 45, 7},
  {46, 47, 8},
  {48, 49, 9},
};

bool verif_code(const button boutons[NB_ELEM]);

#endif
