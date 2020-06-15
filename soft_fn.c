#define STUPIDLAYERS_IMPLEMENTATION
#include "stupidlayers.c"
 
#define SetBit(A, k) (A[(k) / 8] |= (1 << (k) % 8))
#define ClearBit(A, k) (A[(k) / 8] &= ~(1 << (k) % 8))            
#define TestBit(A, k) (A[(k) / 8] & (1 << (k) % 8))

static int fx_map[] = {
  KEY_BACK, 
  KEY_FORWARD,
  KEY_REFRESH,
  KEY_DASHBOARD,
  KEY_SCALE,
  KEY_BRIGHTNESSDOWN,
  KEY_BRIGHTNESSUP,
  KEY_MUTE,
  KEY_VOLUMEDOWN,
  KEY_VOLUMEUP
};

static stupidlayers_t* sl;
  
static struct {
  unsigned int alt_down : 1;
  unsigned int meta_down : 1;
  unsigned int backspace_down : 1;
  unsigned int f11_down : 1;
  unsigned int up_down : 1;
  unsigned int left_down : 1;
  unsigned int right_down : 1;
  unsigned int down_down : 1;
  char fx_down[2];
  unsigned int v_fn_value : 1;
  unsigned int v_meta_value : 1;
} state;

static struct input_event meta_ev = {
  .type = EV_KEY,
  .code = KEY_LEFTMETA
};

static struct input_event caps_ev = {
  .type = EV_KEY,
  .code = KEY_CAPSLOCK
}; 

static int fn_map(int code) {
  if(code >= KEY_F1 && code <= KEY_F10) {
    return fx_map[code - KEY_F1];
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

static int set_key_bits() {
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

static char get_key_fn(int code) {
  if(code >= KEY_F1 && code <= KEY_F10) {
    return TestBit(state.fx_down, code - KEY_F1);
  }

  switch (code) {
    case KEY_BACKSPACE: return state.backspace_down; 
    case KEY_F11: return state.f11_down;
    case KEY_UP: return state.up_down; 
    case KEY_LEFT: return state.left_down; 
    case KEY_RIGHT: return state.right_down; 
    case KEY_DOWN: return state.down_down; 
  }

  return -1;
}

static void set_key_fn(int code, char value) {
  int b = value > 0;

  if(code >= KEY_F1 && code <= KEY_F10) {
    if(b) {
      SetBit(state.fx_down, code - KEY_F1);
    } else {
      ClearBit(state.fx_down, code - KEY_F1);
    }
    return;
  }

  switch (code) {
    case KEY_BACKSPACE: 
      state.backspace_down = b;
      return;
    case KEY_F11:
      state.f11_down = b; 
      return;
    case KEY_UP: 
      state.up_down = b;
      return; 
    case KEY_LEFT: 
      state.left_down = b;
      return; 
    case KEY_RIGHT: 
      state.right_down = b;
      return; 
    case KEY_DOWN: 
      state.down_down = b;
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
 
static void insert_meta_event(struct timeval* t, int v) {
  state.v_meta_value = v > 0;

  meta_ev.time = *t;
  meta_ev.value = v;
  stupidlayers_send(sl, &meta_ev);
}

static void insert_caps_events(struct timeval* t) {
  caps_ev.time = *t;
  caps_ev.value = 1;
  stupidlayers_send(sl, &caps_ev);
  caps_ev.value = 0;
  stupidlayers_send(sl, &caps_ev);
}

static int fn_key_handler(struct input_event* ev) {
  state.meta_down = ev->value > 0;
  
  if(ev->value != 0) {
    // key down -> discard.
    return 1;
  }
  
  // key up
  if(state.v_fn_value) {
    // not a tap
    state.v_fn_value = 0;
    return 1;
  } 
  
  if(!state.v_meta_value) {
    // tap
    if(state.alt_down) {
      insert_caps_events(&ev->time);
      return 1;
    }
  
    // insert down discarded earlier
    insert_meta_event(&ev->time, 1);
  }
  
  state.v_meta_value = 0;
  return 0;
}

static int acc_key_handler(struct input_event* ev) {
  if(ev->value == 2) {
    // don't repeat accelerators
    return 1;
  }

  if(ev->code == KEY_LEFTALT || ev->code == KEY_RIGHTALT) {
    state.alt_down = ev->value > 0;
  }

  return 0;
}

static int key_handler(void* data, struct input_event* ev) {
  if(ev->code == KEY_LEFTMETA) {
    return fn_key_handler(ev);
  }

  if(is_accelerator(ev->code)) {
    return acc_key_handler(ev);
  }

  if(ev->code == KEY_POWER) {
    ev->code = KEY_F11;
  }
  
  // ordinary, non-accelerator key
  if(ev->value != 1) {
    // key is already down
    if(get_key_fn(ev->code) > 0) {
      set_key_fn(ev->code, ev->value);
      ev->code = fn_map(ev->code);
    }
  } else if(state.meta_down) {
    // key up and fn-level set
    int fn_code = fn_map(ev->code);
    
    if(fn_code > 0) {
      if(state.v_meta_value) {
        // set meta up 
        insert_meta_event(&ev->time, 0);
      }

      state.v_fn_value = 1;
      set_key_fn(ev->code, ev->value);
      ev->code = fn_code;
    } else if(!state.v_meta_value) {
      // insert previously suppressed meta down.
      insert_meta_event(&ev->time, 1);
    } 
  }

  return 0;
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    fprintf(stderr, "usage: %s /dev/input/eventX\n", argv[0]);
    return -1;
  }

  sl = new_stupidlayers(argv[1], "Chromebook keyboard (sl enhanced)");
  
  if(set_key_bits() < 0) return -1;

  stupidlayers_run(sl, key_handler, &state);
  return 0;
}