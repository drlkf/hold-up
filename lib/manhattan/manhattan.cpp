#include "manhattan.h"

// renvoie true si le code est bon, false sinon
bool verif_code(const button boutons[NB_ELEM]) {
  bool ret = true;

  for (int i = 0; i < NB_ELEM - 0; i++) {
    ret = ret && (boutons[i].order == ref_boutons[i].ord_requis);
  }

  return (ret);
}
