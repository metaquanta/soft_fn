#define STUPIDLAYERS_IMPLEMENTATION
#include "stupidlayers.c"

typedef struct {
  stupidlayers_t* sl;
  char virtual_fn_value;
  char virtual_leftmeta_value;
  char leftmeta;
  char fn[10];
  char power;
  char up; 
  char down; 
  char left; 
  char right; 
  char backspace;
} key_state;

static int set_keys(stupidlayers_t* sl) {
  for(int i = 1; i <= 68; ++i) {
    if(i != 55) {
      if (stupidlayers_setkeybit(sl, i) < 0)
        return -1;
    }
  }
  stupidlayers_setkeybit(sl, KEY_F11);
  stupidlayers_setkeybit(sl, KEY_RIGHTCTRL);
  stupidlayers_setkeybit(sl, KEY_RIGHTALT);
  for(int i = 102; i <= 116; ++i) {
    if(i != 110 && i != 112) stupidlayers_setkeybit(sl, i);
  }
  stupidlayers_setkeybit(sl, KEY_SCALE);
  stupidlayers_setkeybit(sl, KEY_LEFTMETA);
  stupidlayers_setkeybit(sl, KEY_BACK);
  stupidlayers_setkeybit(sl, KEY_FORWARD);
  stupidlayers_setkeybit(sl, KEY_REFRESH);
  stupidlayers_setkeybit(sl, KEY_DASHBOARD);
  stupidlayers_setkeybit(sl, KEY_BRIGHTNESSUP);
  stupidlayers_setkeybit(sl, KEY_BRIGHTNESSDOWN);
  return 0;
}

static char get_key_fn(key_state* state, int code) {
  if(code >= KEY_F1 && code <= KEY_F10) {
    return state->fn[code - KEY_F1];
  }
  switch (code) {
    case KEY_UP: return state->up; 
    case KEY_LEFT: return state->left; 
    case KEY_RIGHT: return state->right; 
    case KEY_DOWN: return state->down; 
    case KEY_BACKSPACE: return state->backspace; 
  }
  return 0;
}

static void set_key_fn(key_state* state, int code, char value) {
  if(code >= KEY_F1 && code <= KEY_F10) {
    state->fn[code - KEY_F1] = value;
  }
  switch (code) {
    case KEY_UP: 
      state->up = value;
      break; 
    case KEY_LEFT: 
      state->left = value;
      break; 
    case KEY_RIGHT: 
      state->right = value;
      break; 
    case KEY_DOWN: 
      state->down = value;
      break; 
    case KEY_BACKSPACE: 
      state->backspace = value;
      break; 
  }
  return;
}

static int fn_remap(int code) {
  switch (code) {
    case KEY_F1: return KEY_BACK;
    case KEY_F2: return KEY_FORWARD; 
    case KEY_F3: return KEY_REFRESH; 
    case KEY_F4: return KEY_DASHBOARD;
    case KEY_F5: return KEY_SCALE; 
    case KEY_F6: return KEY_BRIGHTNESSDOWN;
    case KEY_F7: return KEY_BRIGHTNESSUP;
    case KEY_F8: return KEY_MUTE; 
    case KEY_F9: return KEY_VOLUMEDOWN; 
    case KEY_F10: return KEY_VOLUMEUP; 
    case KEY_F11: return KEY_POWER;
    case KEY_UP: return KEY_PAGEUP; 
    case KEY_LEFT: return KEY_HOME; 
    case KEY_RIGHT: return KEY_END; 
    case KEY_DOWN: return KEY_PAGEDOWN; 
    case KEY_BACKSPACE: return KEY_DELETE; 
    default:
      return -1;
  }
}

char is_accelerator(int code) {
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
 
static void insert_virtual_event(stupidlayers_t* sl, key_state* state, struct input_event* ev, int v) {
  static struct input_event vev;
  vev = *ev;
  vev.value = v;
  vev.code = KEY_LEFTMETA;
  //fprintf(stderr, "+ 0x%x = %x\n", vev.code, vev.value);
  state->virtual_leftmeta_value = v;
  stupidlayers_send(sl, &vev);
}

static int key_handler(void* data, struct input_event* ev) {
  key_state* state = data;
  
  if(ev->code == KEY_LEFTMETA) {
    state->leftmeta = ev->value;
    if(ev->value == 0) {
      if(state->virtual_fn_value) {
        state->virtual_fn_value = 0;
      } else if(!state->virtual_leftmeta_value) {
        // tap -> insert down ev.
        insert_virtual_event(state->sl, state, ev, 1);
      }
      if(state->virtual_leftmeta_value) {
        state->virtual_leftmeta_value = 0;
        return 0;
      }
    }
    // key down -> discard.
    return 1;
  }

  if(is_accelerator(ev->code)) {
    return 0;
  }

  if(ev->code == KEY_POWER) {
    ev->code = KEY_F11;
  }
  
  if(ev->value != 1) {
    if(get_key_fn(state, ev->code)) {
      set_key_fn(state, ev->code, ev->value);
      ev->code = fn_remap(ev->code);
    }
    return 0;
  }

  if(state->leftmeta) {
    int fn_code = fn_remap(ev->code);
    if(fn_code == -1 && !state->virtual_leftmeta_value) {
      // insert previously suppressed accelerator.
      insert_virtual_event(state->sl, state, ev, 1);
    } else if (fn_code > 0 && state->virtual_leftmeta_value) {
      // lift for fn-mapping.
      insert_virtual_event(state->sl, state, ev, 0);
    }
    
    if(fn_code > 0) {
      state->virtual_fn_value = 1;
      set_key_fn(state, ev->code, ev->value);
      ev->code = fn_code;
    }
  }

  return 0;
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    fprintf(stderr, "usage: %s /dev/input/eventX\n", argv[0]);
    return -1;
  }

  key_state state;
  state.sl = new_stupidlayers(argv[1], "Chromebook keyboard (sl enhanced)");
  
  if(set_keys(state.sl) < 0) return -1;

  stupidlayers_run(state.sl, key_handler, &state);
  return 0;
}