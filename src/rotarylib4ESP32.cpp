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
  rot_val_tol = VAL_DIVIDER / 2;
  dir_cnt_l = 0;
  dir_cnt_h = 0;
  read();
  valChanged = false;
  buttonChanged = false;
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
      if (rot_val < (rot_max + VAL_DIVIDER / 2)) rot_val++;
      if (((rot_val + rot_val_tol) % VAL_DIVIDER) == 0 ) {
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
      if (((rot_val + rot_val_tol) % VAL_DIVIDER) == 0 ) {
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
        sw_val = 2;
        buttonChanged = true;
      } else if (millis() - millis_Button_pressed >= SHORT_PRESSED_AFTER_MS) {
//      short click => Change Level
        sw_val = 1;
        buttonChanged = true;
      }
    }
  }
//  Serial.print("read ");
//  Serial.println(valChanged);
}

uint16_t RotaryLib::val() {
  return (rot_val - rot_val_tol) / VAL_DIVIDER;
}

void RotaryLib::set_max(uint16_t _max) {
  rot_max = _max * VAL_DIVIDER;
}

void RotaryLib::set_min(uint16_t _min) {
  rot_min = _min * VAL_DIVIDER;
}

void RotaryLib::set_val(uint16_t _val) {
  rot_val = _val * VAL_DIVIDER;
}

uint8_t RotaryLib::buttonState() {
  return sw_val;
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
  app_cur = 0;
  lev_cur = 0;
}

void RotaryLibMulti::set_max(uint16_t _max){
  p_lev_current->max = _max;
  RotaryLib::set_max(_max);
}

void RotaryLibMulti::set_min(uint16_t _min){
  p_lev_current->min = _min;
  RotaryLib::set_min(_min);
}

void RotaryLibMulti::set_val(uint16_t _val){
  p_lev_current->val = _val;
  RotaryLib::set_val(_val);
}

uint8_t RotaryLibMulti::add_app(){
  return add_app(0,0,0);
}

uint8_t RotaryLibMulti::add_app(uint16_t _min, uint16_t _max, uint16_t _val){
  app_max++;
  //Add a new record for App
  app_t* p_app_search = p_app_initial;
  while (p_app_search->p_app_next) p_app_search = p_app_search->p_app_next;
  Serial.println(p_app_search->app);
//  app_t* p_app_new = new app_t;
  app_t* p_app_new = (app_t*)malloc(sizeof(app_t));
  p_app_new->p_app_next = NULL;
  p_app_new->app = app_max;
  p_app_new->max_lev = 0;
  p_app_search->p_app_next = p_app_new;
  //Add a new record for Lev
  lev_t* p_lev_search = p_lev_initial;
  while (p_lev_search->p_lev_next) p_lev_search = p_lev_search->p_lev_next;
  Serial.println(p_lev_search->app);
//  lev_t* p_lev_new = new lev_t;
  lev_t* p_lev_new = (lev_t*)malloc(sizeof(lev_t));
  p_lev_new->p_lev_next = NULL;
  p_lev_new->app = app_max;
  p_lev_new->lev = 0;
  p_lev_new->min = _min;
  p_lev_new->max = _max;
  p_lev_new->val = _val;
  p_lev_search->p_lev_next = p_lev_new;
  return app_max;
}

uint8_t RotaryLibMulti::add_lev(uint8_t _app){
  return add_lev(_app,0,0,0);
}

uint8_t RotaryLibMulti::add_lev(uint8_t _app, uint16_t _min, uint16_t _max, uint16_t _val){
  //Find the app record
  app_t* p_app_search = p_app_initial;
  bool found = false;
  while (p_app_search->p_app_next && !found) {
    if (p_app_search->app == _app) found = true;
    p_app_search = p_app_search->p_app_next;
  }
  p_app_search->max_lev = p_app_search->max_lev +1;
  Serial.println(p_app_search->max_lev);
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
  Serial.println(p_app_search->max_lev);
  return p_app_search->max_lev;
}

void RotaryLibMulti::set(uint8_t _app, uint8_t _lev, uint16_t _min, uint16_t _max, uint16_t _val) {
  lev_t* p_lev_search = p_lev_initial;
  while (p_lev_search) {
    if (p_lev_search->app == _app && p_lev_search->lev == _lev) {
      p_lev_search->min = _min;
      p_lev_search->max = _max;
      p_lev_search->val = _val;
    }
    p_lev_search = p_lev_search->p_lev_next;
  }

}

void RotaryLibMulti::set(uint8_t appl, uint8_t level, uint16_t _val) {
}

void RotaryLibMulti::set(uint16_t _val){
}

void RotaryLibMulti::set_min(uint8_t appl, uint8_t level, uint16_t min){
}

void RotaryLibMulti::set_max(uint8_t appl, uint8_t level, uint16_t max){
}

bool RotaryLibMulti::switch_app(uint8_t _app, uint8_t _lev){
  app_t* p_app_current_tmp = p_app_current;
  p_app_current = p_app_initial;
  bool found_app = false;
  while (p_app_current->p_app_next && !found_app) {
    if (p_app_current->app == _app) found_app = true;
    if (!found_app) p_app_current = p_app_current->p_app_next;
  }
  if (!found_app) p_app_current = p_app_current_tmp;
  lev_t* p_lev_current_tmp = p_lev_current;
  p_lev_current = p_lev_initial;
  bool found_lev = false;
  while (p_lev_current->p_lev_next && !found_lev) {
    if (p_lev_current->app == _app && p_lev_current->lev == _lev) found_lev = true;
    if (!found_lev) p_lev_current = p_lev_current->p_lev_next;
  }
  if (!found_lev) p_lev_current = p_lev_current_tmp;
  return found_lev && found_app;
}

bool RotaryLibMulti::switch_lev(uint8_t _lev){
  uint8_t _app = p_lev_current->app;
  lev_t* p_lev_current_tmp = p_lev_current;
  p_lev_current = p_lev_initial;
  bool found = false;
  while (p_lev_current->p_lev_next && !found) {
    if (p_lev_current->app == _app && p_lev_current->lev == _lev) found = true;
    if (!found) p_lev_current = p_lev_current->p_lev_next;
  }
  if (!found) p_lev_current = p_lev_current_tmp;
  return found;
}

uint8_t RotaryLibMulti::lev(){
  return p_lev_current->lev;
}

uint8_t RotaryLibMulti::app(){
  return p_lev_current->app;
}

uint8_t RotaryLibMulti::val(){
  //return p_lev_current->val;
  return RotaryLib::val();
}

uint8_t RotaryLibMulti::max_lev(){
  return p_app_current->max_lev;
}

uint8_t RotaryLibMulti::max_app(){
  return app_max;
}
