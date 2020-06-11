#define STUPIDLAYERS_IMPLEMENTATION
#include "stupidlayers.c"

typedef struct {
  stupidlayers_t* sl;
  int matched_hotkey;
  int physical_leftmeta_value;
} phys_state;

struct input_event vev;
void insert_virtual_event(stupidlayers_t* sl, struct input_event* ev, char* k, int value) {
  vev = *ev;
  vev.value = value;
  vev.code = KEY_LEFTMETA;
  //fprintf(stderr, "+ 0x%x = %x\n", vev.code, vev.value);
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
  phys_state* state = data;
  
  if(ev->code == KEY_LEFTMETA) {
    state->physical_leftmeta_value = ev->value;
    if(ev->value == 0) {
      if(state->matched_hotkey) { // used as fn -> consume ev.
        state->matched_hotkey = 0;
      } else if(!k[KEY_LEFTMETA]) {
        // tap -> insert down ev.
        insert_virtual_event(state->sl, ev, k, 1);
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

  if(state->physical_leftmeta_value) {
    int fn_code = fn_remap(ev->code);
    if(fn_code == -1 && !k[KEY_LEFTMETA]) {
      insert_virtual_event(state->sl, ev, k, 1);
    } else if (fn_code > 0 && k[KEY_LEFTMETA]) {
      insert_virtual_event(state->sl, ev, k, 0);
    }
    
    if(fn_code > 0) {
      state->matched_hotkey = 1;
      ev->code = fn_code;
    }
  }
  return 0;
}

int main(int argc, char* argv[]) {
  phys_state state;
  stupidlayers_t* sl;
  state.sl = new_stupidlayers(argv[1]);
  state.matched_hotkey = 0;
  state.physical_leftmeta_value = 0;
  stupidlayers_run(state.sl, pre_handler, 0, &state);
  return 0;
}