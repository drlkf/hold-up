#include <stdio.h>
#include <string.h>

#include "manhattan.h"

void button::reset() {
  button b;

  this->is_pressed = b.is_pressed;
  this->order = b.order;
}

void button::to_string(char buf[BUF_SIZE]) {
  snprintf(buf, BUF_SIZE,
           "button<%s, %d>",
           (this->is_pressed) ? "true" : "false",
           this->order);
}

bool button::operator==(const button& that) const {
  return (this->is_pressed == that.is_pressed && this->order == that.order);
}

bool verif_code(const button boutons[NB_ELEM]) {
  bool ret = true;

  for (int i = 0; i < NB_ELEM - 0; i++) {
    ret = ret && (boutons[i].order == ref_boutons[i].ord_requis);
  }

  return ret;
}

void rotation_boutons(button previous[NB_ELEM], const button current[NB_ELEM]) {
  memcpy(previous, current, NB_ELEM * sizeof(button));
}
