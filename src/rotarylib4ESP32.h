#ifndef _ROTARYMODUL_H_
#define _ROTARYMODUL_H_
/***************************************************************************************
 ***************************************************************************************/


#include "rotarylib4ESP32.cfg"
#include "Arduino.h"

class RotaryLib {

public:
    /// @brief Die Initialisierung des Rotary Modules
    /// @brief Alle Werte werden innerhalb des Modules gesetzt.
    void begin(uint8_t _pin_a, uint8_t _pin_b, uint8_t _pin_sw);
    void read();
    bool valChanged = false;
    uint16_t max();
    uint16_t min();
    uint16_t val();
    void max_set(uint16_t _max);
    void min_set(uint16_t _min);
    void val_set(uint16_t _val);
    bool buttonShortPressed = false;
    bool buttonLongPressed = false;

private:
/// @brief Zeitmessung Rotary Schalter
    unsigned long millis_Button_pressed = 0;
/// @brief Letzter Zustand Rotary Schalter
    bool buttonWasDown = false;
    uint8_t  pin_a;
    uint8_t  pin_b;
    bool     pin_a_val;
    bool     pin_b_val;
    bool     pin_a_val_old;
    bool     pin_b_val_old;
    uint8_t  pin_sw;
    uint8_t  raw_a;
    uint8_t  raw_b;
    uint8_t  raw_sw;
    uint8_t  raw_val;
    uint8_t  raw_val_old;
    uint8_t  dir_cnt_h;
    uint8_t  dir_cnt_l;
    uint16_t rot_val;
    uint16_t rot_val_tol;
    uint16_t rot_shift;
    uint16_t rot_divider;
    uint16_t rot_val_old;
    uint16_t rot_max;
    uint16_t rot_min;
    unsigned long ts_pin_a;
    unsigned long ts_pin_b;
    void set_val_changed();
    void reset_dir_cnt();
};

class RotaryLibMulti : public RotaryLib {

public:
    void begin(uint8_t pin_a, uint8_t pin_b, uint8_t pin_sw);
    void add(uint8_t app, uint8_t lev, uint16_t min, uint16_t max, uint16_t val);
    uint8_t app_add();
    uint8_t app_add(uint16_t min, uint16_t max, uint16_t val);
    uint8_t lev_add(uint8_t app);
    uint8_t lev_add(uint8_t app, uint16_t min, uint16_t max, uint16_t val);
    bool set(uint8_t app, uint8_t lev, uint16_t min, uint16_t max, uint16_t val);
    bool app_set(uint8_t app, uint8_t lev);
    bool lev_set(uint8_t lev);
    uint8_t lev_up();
    uint8_t app_up();
    uint8_t app();
    uint8_t lev();
    uint8_t app_max();
    uint8_t lev_max();
    void max_set(uint16_t _max);
    void min_set(uint16_t _min);
    void val_set(uint16_t _val);

private:
    void set_val_changed();
    void reset_dir_cnt();

    struct app_t {
        uint8_t  app;
        uint8_t  max_lev;
        app_t* p_app_next;
    };
    app_t * p_app_initial;
    app_t * p_app_current;
    struct lev_t {
        uint8_t  app;
        uint8_t  lev;
        uint16_t min;
        uint16_t max;
        uint16_t val;
        lev_t* p_lev_next;
    };
    lev_t * p_lev_initial;
    lev_t * p_lev_current;
    uint8_t max_app;
};



#endif
