/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

#define STUPIDLAYERS_IMPLEMENTATION
#include "stupidlayers.c"

/*
 * motospeed ck62 keybinds
 * - caps lock is esc
 * - esc is `
 * - esc acts as fn key if held down
 * - esc + q is `
 * - esc + shift + q is ~
 * - esc + e is ~
 */

typedef struct {
  stupidlayers_t* sl;
  int matched_hotkey;
  int physical_leftmeta_value;
  int power_time;
} ck62_state_t;

struct input_event vev;

void insert_virtual_event(stupidlayers_t* sl, struct input_event* ev, char* k, int value) {
  vev = *ev;
  vev.value = value;
  vev.code = KEY_LEFTMETA;
  fprintf(stderr, "+ 0x%x = %x\n", vev.code, vev.value);
  k[KEY_LEFTMETA] = value;
  stupidlayers_send(sl, &vev);
}

int fn_remap(int code) {
  switch (code) {
    case KEY_F1: return KEY_BACK;
    case KEY_F2: return KEY_FORWARD; 
    case KEY_F3: return KEY_REFRESH; 
    case KEY_F4: return KEY_FULL_SCREEN;
    case KEY_F5: return KEY_SCALE; 
    case KEY_F6: return KEY_BRIGHTNESSDOWN;
    case KEY_F7: return KEY_BRIGHTNESSUP;
    case KEY_F8: return KEY_MUTE; 
    case KEY_F9: return KEY_VOLUMEDOWN; 
    case KEY_F10: return KEY_VOLUMEUP; 
    case KEY_UP: return KEY_PAGEUP; 
    case KEY_LEFT: return KEY_HOME; 
    case KEY_RIGHT: return KEY_END; 
    case KEY_DOWN: return KEY_PAGEDOWN; 
    case KEY_BACKSPACE: return KEY_DELETE; 
    default:
      return -1;
  }
}
 
static int pre_handler(void* data, struct input_event* ev, char* k) {
  ck62_state_t* ck62 = data;
  stupidlayers_t* sl = ck62->sl;

  if(ev->code == KEY_LEFTMETA) {
    ck62->physical_leftmeta_value = ev->value;
    if(ev->value == 0) {
      if(ck62->matched_hotkey) { // used as fn -> consume ev.
        ck62->matched_hotkey = 0;
      } else if(!k[KEY_LEFTMETA]) {
        // tap -> insert down ev.
        insert_virtual_event(sl, ev, k, 1);
      }
      if(k[KEY_LEFTMETA]) {
        return 0;
      }
    }
    // search key down -> consume ev.
    return 1;
  }
  
  if(ev->value != 1) {
    if(k[ev->code]) {
      return 0;
    }
    ev->code = fn_remap(ev->code);
    if(k[ev->code]) {
      return 0;
    }
    return 1;
  }

  if(ck62->physical_leftmeta_value) {
    int fn_code = fn_remap(ev->code);
    if(fn_code == -1 && !k[KEY_LEFTMETA]) {
      insert_virtual_event(sl, ev, k, 1);
    } else if (fn_code > 0 && k[KEY_LEFTMETA]) {
      insert_virtual_event(sl, ev, k, 0);
    }
    
    if(fn_code > 0) {
      ck62->matched_hotkey = 1;
      ev->code = fn_code;
    }
  }
  return 0;
}

static int post_handler(void* data, struct input_event* ev, char* k) {
  /* ignore esc keydown. we will send it ourselves if no hotkeys match */
  return ev->code == KEY_LEFTMETA && ev->value;
}

#define die(x) { fprintf(stderr, x); exit(1); }

static int run_cli(char* device) {
  ck62_state_t ck62;
  stupidlayers_t* sl;
  ck62.sl = sl = new_stupidlayers(device);
  ck62.matched_hotkey = 0;
  if (!sl) { return 1; }
  if (sl->errstr) die(sl->errstr);
  stupidlayers_run(sl, pre_handler, 0, &ck62);
  if (sl->errstr) die(sl->errstr);
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 1) {
    fprintf(stderr, "usage: %s /dev/input/eventX\n"
      "you can use use evtest to list devices\n", argv[0]);
    return 1;
  }
  return run_cli(argv[1]);
}
