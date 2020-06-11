#define STUPIDLAYERS_IMPLEMENTATION
#include "stupidlayers.c"

typedef struct {
  stupidlayers_t* sl;
  int virtual_fn_value;
  int physical_leftmeta_value;
} key_state;

struct input_event vev;

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

int is_accelerator(int code) {
  switch (code) {
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:
    case KEY_LEFTALT:
    case KEY_RIGHTALT:
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
      return 1;
  }
  return 0;
}
 
void insert_virtual_event(stupidlayers_t* sl, struct input_event* ev, int v) {
  vev = *ev;
  vev.value = v;
  vev.code = KEY_LEFTMETA;
  //fprintf(stderr, "+ 0x%x = %x\n", vev.code, vev.value);
  sl->keys[KEY_LEFTMETA] = v;
  stupidlayers_send(sl, &vev);
}

static int key_handler(void* data, struct input_event* ev, char* k) {
  key_state* state = data;
  
  if(ev->code == KEY_LEFTMETA) {
    state->physical_leftmeta_value = ev->value;
    if(ev->value == 0) {
      if(state->virtual_fn_value) {
        state->virtual_fn_value = 0;
      } else if(!k[KEY_LEFTMETA]) {
        // tap -> insert down ev.
        insert_virtual_event(state->sl, ev, 1);
      }
      if(k[KEY_LEFTMETA]) {
        return 0;
      }
    }
    // key down -> discard.
    return 1;
  }

  if(is_accelerator(ev->code)) {
    return 0;
  }
  
  if(ev->value != 1) {
    if(k[ev->code]) {
      // key was down.
      return 0;
    }
    ev->code = fn_remap(ev->code);
    if(k[ev->code]) {
      // fn+key was down.
      return 0;
    }
    // unreachable
    return 1;
  }

  if(state->physical_leftmeta_value) {
    int fn_code = fn_remap(ev->code);
    if(fn_code == -1 && !k[KEY_LEFTMETA]) {
      // insert previously suppressed accelerator.
      insert_virtual_event(state->sl, ev, 1);
    } else if (fn_code > 0 && k[KEY_LEFTMETA]) {
      // lift for fn-mapping.
      insert_virtual_event(state->sl, ev, 0);
    }
    
    if(fn_code > 0) {
      state->virtual_fn_value = 1;
      ev->code = fn_code;
    }
  }

  // key down without any fn logic.
  return 0;
}

int main(int argc, char* argv[]) {
  // init state.
  key_state state;
  state.sl = new_stupidlayers(argv[1]);
  state.virtual_fn_value = 0;
  state.physical_leftmeta_value = 0;
  
  stupidlayers_run(state.sl, key_handler, 0, &state);
  return 0;
}