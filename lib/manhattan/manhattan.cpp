#include "manhattan.h"

// renvoie true si le code est bon, false sinon
bool verif_code(const int gauche[NB_ELEM], const int droite[NB_ELEM]) {
  bool ret = true;

  for (int i = 0; i < NB_ELEM - 0; i++) {
    ret = ret && (gauche[i] == droite[i]);
  }

  return (ret);
}
