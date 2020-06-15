#define STUPIDLAYERS_IMPLEMENTATION
#include "stupidlayers.c"

typedef struct {
  stupidlayers_t* sl;
  char virtual_fn_value;
  char virtual_meta_value;
  char alt_value;
  char leftmeta;
  char backspace;
  char fn[10];
  char f11;
  char up;
  char left;
  char right;
  char down; 
  int fn_map[10];
} key_state;

void init_key_state(key_state* state) {
  state->virtual_fn_value = 0;
  state->virtual_meta_value = 0;
  state->alt_value = 0;
  state->leftmeta = 0;
  state->fn_map[0] = KEY_BACK;
  state->fn_map[1] = KEY_FORWARD;
  state->fn_map[2] = KEY_REFRESH;
  state->fn_map[3] = KEY_DASHBOARD;
  state->fn_map[4] = KEY_SCALE;
  state->fn_map[5] = KEY_BRIGHTNESSDOWN;
  state->fn_map[6] = KEY_BRIGHTNESSUP;
  state->fn_map[7] = KEY_MUTE;
  state->fn_map[8] = KEY_VOLUMEDOWN;
  state->fn_map[9] = KEY_VOLUMEUP;
}

static int fn_remap(key_state* state, int code) {
  if(code >= KEY_F1 && code <= KEY_F10) {
    return state->fn_map[code - KEY_F1];
  }

  switch (code) {
    case KEY_BACKSPACE: return KEY_DELETE; 
    case KEY_F11: return KEY_POWER;
    case KEY_UP: return KEY_PAGEUP; 
    case KEY_LEFT: return KEY_HOME; 
    case KEY_RIGHT: return KEY_END; 
    case KEY_DOWN: return KEY_PAGEDOWN;
  }

  return -1;
}

static int set_keys(stupidlayers_t* sl) {
  for(int i = 1; i <= 68; ++i) {
    if(i != 55) {
      if (stupidlayers_setkeybit(sl, i) < 0) {
        return -1;
      }
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
    case KEY_BACKSPACE: return state->backspace; 
    case KEY_F11: return state->f11;
    case KEY_UP: return state->up; 
    case KEY_LEFT: return state->left; 
    case KEY_RIGHT: return state->right; 
    case KEY_DOWN: return state->down; 
  }

  return -1;
}

static void set_key_fn(key_state* state, int code, char value) {
  if(code >= KEY_F1 && code <= KEY_F10) {
    state->fn[code - KEY_F1] = value;
    return;
  }

  switch (code) {
    case KEY_BACKSPACE: 
      state->backspace = value;
      return;
    case KEY_F11:
      state->f11 = value; 
      return;
    case KEY_UP: 
      state->up = value;
      return; 
    case KEY_LEFT: 
      state->left = value;
      return; 
    case KEY_RIGHT: 
      state->right = value;
      return; 
    case KEY_DOWN: 
      state->down = value;
      return; 
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
 
static void insert_meta_event(stupidlayers_t* sl, key_state* state, struct timeval* t, int v) {
  static struct input_event vev;
  vev.type = EV_KEY;
  vev.code = KEY_LEFTMETA;
  vev.time = *t;
  vev.value = v;
  
  state->virtual_meta_value = v;
  stupidlayers_send(sl, &vev);
}

static void insert_caps_events(stupidlayers_t* sl, struct timeval* t) {
  static struct input_event vev;
  vev.type = EV_KEY;
  vev.code = KEY_CAPSLOCK;
  vev.time = *t;

  vev.value = 1;
  stupidlayers_send(sl, &vev);
  vev.value = 0;
  stupidlayers_send(sl, &vev);
}

static int fn_key_handler(key_state* state, struct input_event* ev) {
  state->leftmeta = ev->value;
  
  if(ev->value != 0) {
    // key down -> discard.
    return 1;
  }
  
  // key up
  if(state->virtual_fn_value) {
    // not a tap
    state->virtual_fn_value = 0;
    return 1;
  } 
  
  if(!state->virtual_meta_value) {
    // tap
    if(state->alt_value) {
      insert_caps_events(state->sl, &ev->time);
      return 1;
    }
  
    // insert down discarded earlier
    insert_meta_event(state->sl, state, &ev->time, 1);
  }
  
  state->virtual_meta_value = 0;
  return 0;
}

static int acc_key_handler(key_state* state, struct input_event* ev) {
  if(ev->value == 2) {
    // don't repeat accelerators
    return 1;
  }

  if(ev->code == KEY_LEFTALT || ev->code == KEY_RIGHTALT) {
    state->alt_value = ev->value;
  }

  return 0;
}

static int key_handler(void* data, struct input_event* ev) {
  key_state* state = data;
  
  if(ev->code == KEY_LEFTMETA) {
    return fn_key_handler(state, ev);
  }

  if(is_accelerator(ev->code)) {
    return acc_key_handler(state, ev);
  }

  if(ev->code == KEY_POWER) {
    ev->code = KEY_F11;
  }
  
  // ordinary, non-accelerator key
  if(ev->value != 1) {
    // key is already down
    if(get_key_fn(state, ev->code) > 0) {
      set_key_fn(state, ev->code, ev->value);
      ev->code = fn_remap(state, ev->code);
    }
  } else if(state->leftmeta) {
    // key up and fn-level set
    int fn_code = fn_remap(state, ev->code);
    
    if(fn_code > 0) {
      if(state->virtual_meta_value) {
        // set meta up 
        insert_meta_event(state->sl, state, &ev->time, 0);
      }

      state->virtual_fn_value = 1;
      set_key_fn(state, ev->code, ev->value);
      ev->code = fn_code;
    } else if(!state->virtual_meta_value) {
      // insert previously suppressed meta down.
      insert_meta_event(state->sl, state, &ev->time, 1);
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
  init_key_state(&state);

  state.sl = new_stupidlayers(argv[1], "Chromebook keyboard (sl enhanced)");
  
  if(set_keys(state.sl) < 0) return -1;

  stupidlayers_run(state.sl, key_handler, &state);
  return 0;
}