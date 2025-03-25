#include "rotarylib4ESP32.h"

void RotaryLib::begin(uint8_t _pin_a, uint8_t _pin_b, uint8_t _pin_sw) {
  pin_a = _pin_a;
  pin_b = _pin_b;
  pin_sw = _pin_sw;
  raw_val = 0;
  raw_val_old = 0;
  rot_val = 0;
  rot_min = 0;
  rot_max = 0;
  buttonShortPressed = false;
  buttonLongPressed = false;
  switch(ROT_SPEEDER) {
    case 0:
      rot_val_tol = 0;
      rot_shift = 0;
      rot_divider = 1;
      break;
    case 1:
      rot_val_tol = 1;
      rot_shift = 1;
      rot_divider = 2;
      break;
    default:
      rot_val_tol = 2;
      rot_shift = 2;
      rot_divider = 4;
      break;
  }
  dir_cnt_l = 0;
  dir_cnt_h = 0;
  read();
  valChanged = false;
  buttonWasDown = false;
}

void RotaryLib::set_val_changed() {
  valChanged = true;
  reset_dir_cnt();
}

void RotaryLib::reset_dir_cnt() {
  dir_cnt_l = 0;
  dir_cnt_h = 0;
}

void RotaryLib::read() {
  unsigned long now = millis();
  pin_a_val = digitalRead(pin_a);
  if (pin_a_val != pin_a_val_old && now - ts_pin_a > BOUNCINGTIME) {
    pin_a_val? raw_a = 0b00000010 : raw_a = 0b00000000;
    ts_pin_a = now;
    pin_a_val_old = pin_a_val;
  }
  pin_b_val = digitalRead(pin_b);
  if (pin_b_val != pin_b_val_old && now - ts_pin_b > BOUNCINGTIME) {
    pin_b_val? raw_b = 0b00000001 : raw_b = 0b00000000;
    ts_pin_b = now;
    pin_b_val_old = pin_b_val;
  }

  raw_val = raw_a | raw_b;
  if ( raw_val != raw_val_old) {
    // Hochdrehen:  00 >> 01 >> 11 >> 10 >> 00
    if (
      (raw_val_old == 0 && raw_val == 1) || 
      (raw_val_old == 1 && raw_val == 3) || 
      (raw_val_old == 3 && raw_val == 2) || 
      (raw_val_old == 2 && raw_val == 0) 
    ) {
      dir_cnt_h++;
      if (rot_val < (rot_max + rot_val_tol)) rot_val++;
      if (((rot_val + rot_val_tol) % rot_divider) == 0 ) {
        if (rot_val != rot_val_old && dir_cnt_h > dir_cnt_l) set_val_changed();
        reset_dir_cnt();
      }
    } 
    // Runterdrehen:  00 >> 10 >> 11 >> 01 >> 00
    else if (
      (raw_val_old == 0 && raw_val == 2) || 
      (raw_val_old == 2 && raw_val == 3) || 
      (raw_val_old == 3 && raw_val == 1) || 
      (raw_val_old == 1 && raw_val == 0)  
    ) {  
      dir_cnt_l++;
      if (rot_val > rot_min) rot_val--;
      if (((rot_val + rot_val_tol) % rot_divider) == 0 ) {
        if (rot_val != rot_val_old && dir_cnt_l > dir_cnt_h) set_val_changed();
        reset_dir_cnt();
      }
    } 
    raw_val_old = raw_val;
    rot_val_old = rot_val;
  }
  /// 1. Read Button
  digitalRead(pin_sw)? raw_sw = 1 : raw_sw = 0;
  /// 2. Button is pressed?
  if (raw_sw == 0) {
    if (!buttonWasDown) {
      //start measuring
      millis_Button_pressed = millis();
    }
    //else we wait since button is still down
    buttonWasDown = true;
  } else {
    //button is up
    if (buttonWasDown) {
      buttonWasDown = false;
      if (millis() - millis_Button_pressed >= LONG_PRESSED_AFTER_MS) {
//      long click
        buttonLongPressed = true;
//        buttonChanged = true;
      } else if (millis() - millis_Button_pressed >= SHORT_PRESSED_AFTER_MS) {
//      short click => Change Level
        buttonShortPressed = true;
//        buttonChanged = true;
      }
    }
  }
}

uint16_t RotaryLib::val() {
  return rot_val >> rot_shift;
}

uint16_t RotaryLib::min() {
  return rot_min >> rot_shift;
}

uint16_t RotaryLib::max() {
  return rot_max >> rot_shift;
}

void RotaryLib::max_set(uint16_t _max) {
  rot_max = _max << rot_shift;
}

void RotaryLib::min_set(uint16_t _min) {
  rot_min = _min << rot_shift;
}

void RotaryLib::val_set(uint16_t _val) {
  rot_val = _val << rot_shift;
}

/**************************************
 * Multi App and Lev Extention
**************************************/

void RotaryLibMulti::begin(uint8_t _pin_a, uint8_t _pin_b, uint8_t _pin_sw) {
  p_lev_initial = NULL;
  p_app_initial = NULL;
  RotaryLib::begin(_pin_a, _pin_b, _pin_sw);
  p_lev_initial = new lev_t;
  p_lev_initial->app = 0;
  p_lev_initial->lev = 0;
  p_lev_initial->min = 0;
  p_lev_initial->max = 0;
  p_lev_initial->val = 0;
  p_lev_initial->p_lev_next = NULL;
  p_lev_current = p_lev_initial;
  p_app_initial = new app_t;
  p_app_initial->app = 0;
  p_app_initial->max_lev = 0;
  p_app_initial->p_app_next = NULL;
  p_app_current = p_app_initial;
  max_app = 0;
}

uint8_t RotaryLibMulti::app_add(){
  return app_add(0,0,0);
}

uint8_t RotaryLibMulti::app_add(uint16_t _min, uint16_t _max, uint16_t _val){
  max_app++;
  //Add a new record for App
  app_t* p_app_search = p_app_initial;
  while (p_app_search->p_app_next) p_app_search = p_app_search->p_app_next;
  app_t* p_app_new = (app_t*)malloc(sizeof(app_t));
  p_app_new->p_app_next = NULL;
  p_app_new->app = max_app;
  p_app_new->max_lev = 0;
  p_app_search->p_app_next = p_app_new;
  //Add a new record for Lev
  lev_t* p_lev_search = p_lev_initial;
  while (p_lev_search->p_lev_next) p_lev_search = p_lev_search->p_lev_next;
  lev_t* p_lev_new = (lev_t*)malloc(sizeof(lev_t));
  p_lev_new->p_lev_next = NULL;
  p_lev_new->app = max_app;
  p_lev_new->lev = 0;
  p_lev_new->min = _min;
  p_lev_new->max = _max;
  p_lev_new->val = _val;
  p_lev_search->p_lev_next = p_lev_new;
  return max_app;
}

uint8_t RotaryLibMulti::lev_add(uint8_t _app){
  return lev_add(_app,0,0,0);
}

uint8_t RotaryLibMulti::lev_add(uint8_t _app, uint16_t _min, uint16_t _max, uint16_t _val){
  //Find the app record
  app_t* p_app_search = p_app_initial;
  bool found = false;
  while (p_app_search->p_app_next && !found) {
    if (p_app_search->app == _app) found = true;
    p_app_search = p_app_search->p_app_next;
  }
  p_app_search->max_lev = p_app_search->max_lev +1;
  //Add a new record for Lev
  lev_t* p_lev_search = p_lev_initial;
  while (p_lev_search->p_lev_next) p_lev_search = p_lev_search->p_lev_next;
  lev_t* p_lev_new = new lev_t;
  p_lev_new->p_lev_next = NULL;
  p_lev_new->app = _app;
  p_lev_new->lev = p_app_search->max_lev;
  p_lev_new->min = _min;
  p_lev_new->max = _max;
  p_lev_new->val = _val;
  p_lev_search->p_lev_next = p_lev_new;
  return p_app_search->max_lev;
}

bool RotaryLibMulti::set(uint8_t _app, uint8_t _lev, uint16_t _min, uint16_t _max, uint16_t _val) {
  lev_t* p_lev_search = p_lev_initial;
  bool found = false;
  while (p_lev_search) {
    if (p_lev_search->app == _app && p_lev_search->lev == _lev) {
      p_lev_search->min = _min;
      p_lev_search->max = _max;
      p_lev_search->val = _val;
      found = true;
    }
    p_lev_search = p_lev_search->p_lev_next;
  }
  if (_app == p_lev_current->app && _lev == p_lev_current->lev && found) {
    RotaryLib::max_set(_max);
    RotaryLib::min_set(_min);
    RotaryLib::val_set(_val);
  }
  return found;
}

bool RotaryLibMulti::app_set(uint8_t _app, uint8_t _lev){
  bool found_app = false;
  bool found_lev = false;
  app_t* p_app_search = p_app_initial;
  p_lev_current->min = min();
  p_lev_current->max = max();
  p_lev_current->val = val();
  p_app_current = p_app_initial;
  while (p_app_search && !found_app) {
    if (p_app_search->app == _app) {
      found_app = true;
    } else {
      p_app_search = p_app_search->p_app_next;
    }
  }
  lev_t* p_lev_search = p_lev_initial;
  while (p_lev_search && !found_lev) {
    if (p_lev_search->app == _app && p_lev_search->lev == _lev) {
      found_lev = true;
    } else {
      p_lev_search = p_lev_search->p_lev_next;
    }
  }
  if (found_app && found_lev) {
    p_app_current = p_app_search;
    p_lev_current = p_lev_search;
    RotaryLib::min_set(p_lev_current->min);
    RotaryLib::max_set(p_lev_current->max);
    RotaryLib::val_set(p_lev_current->val);
    return true;
  } else {
    return false;
  }
}

bool RotaryLibMulti::lev_set(uint8_t _lev){
  bool found = false;
  uint8_t _app = p_lev_current->app;
  lev_t* p_lev_current_tmp = p_lev_current;
  p_lev_current->min = min();
  p_lev_current->max = max();
  p_lev_current->val = val();
  p_lev_current = p_lev_initial;
  while (p_lev_current->p_lev_next && !found) {
    if (p_lev_current->app == _app && p_lev_current->lev == _lev) found = true;
    if (!found) p_lev_current = p_lev_current->p_lev_next;
  }
  if (found) {
    RotaryLib::min_set(p_lev_current->min);
    RotaryLib::max_set(p_lev_current->max);
    RotaryLib::val_set(p_lev_current->val);
  } else {
    p_lev_current = p_lev_current_tmp;
  }
  return found;
}

void RotaryLibMulti::max_set(uint16_t _max) {
  p_lev_current->max = _max;
  RotaryLib::max_set(p_lev_current->max);
}

void RotaryLibMulti::min_set(uint16_t _min) {
  p_lev_current->min = _min;
  RotaryLib::min_set(p_lev_current->min);
}

void RotaryLibMulti::val_set(uint16_t _val) {
  p_lev_current->val = _val;
  RotaryLib::val_set(p_lev_current->val);
}

uint8_t RotaryLibMulti::lev_up(){
  bool success = false;
  if (lev() < lev_max()) {
    if (lev_set(lev()+1)) success = true;;
  } else {
    if (lev_set(0)) success = true;
  }
  if (success) {
    return lev();
  } else {
    return 255;
  }
}

uint8_t RotaryLibMulti::app_up(){
  bool success = false;
  lev_set(0);
  if (app() < app_max()) {
    if (app_set(app()+1,0)) success = true;
  } else {
    if (app_set(0,0)) success = true;
  }
  if (success) {
    return app();
  } else {
    return 255;
  }
}

uint8_t RotaryLibMulti::lev(){
  return p_lev_current->lev;
}

uint8_t RotaryLibMulti::app(){
  return p_lev_current->app;
}

uint8_t RotaryLibMulti::lev_max(){
  return p_app_current->max_lev;
}

uint8_t RotaryLibMulti::app_max(){
  return max_app;
}
