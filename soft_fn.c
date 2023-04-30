#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/uinput.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SetBit(A, k) (A[(k) / 8] |= (1 << (k) % 8))
#define ClearBit(A, k) (A[(k) / 8] &= ~(1 << (k) % 8))            
#define TestBit(A, k) (A[(k) / 8] & (1 << (k) % 8))

#define KEY_MASK { \
  0xFE, 0xFF, 0xFF, 0xFF, \
  0xFF, 0xFF, 0x7F, 0xFF, \
  0x1F, 0x00, 0x80, 0x00, \
  0xD2, 0xBF, 0x1E, 0x21, \
  0x00, 0x08, 0x00, 0xC0, \
  0x00, 0x20, 0x80, 0x00, \
  0x00, 0x10, 0x00, 0x00, \
  0x03, 0x00, 0x00, 0x00 \
}

#define FN_MAP { \
  KEY_BACK, \
  KEY_REFRESH, \
  KEY_SCALE, \
  KEY_DASHBOARD, \
  KEY_BRIGHTNESSDOWN, \
  KEY_BRIGHTNESSUP, \
  KEY_PLAYPAUSE, \
  KEY_MUTE, \
  KEY_VOLUMEDOWN, \
  KEY_VOLUMEUP \
}

#define KEYBOARD_NAME "Virtual keyboard (soft-fn)"

static unsigned short fx_map[] = FN_MAP;

static int kbin, kbout;

static struct {
  unsigned int alt_down : 1;
  unsigned int meta_down : 1;
  unsigned int backspace_down : 1;
  unsigned int up_down : 1;
  unsigned int left_down : 1;
  unsigned int right_down : 1;
  unsigned int down_down : 1;
  unsigned int hamburger_down : 1;
  unsigned char fx_down[2];
  unsigned int v_fn : 1;
  unsigned int v_meta : 1;
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
    return code;
  }

  switch (code) {
    case KEY_BACKSPACE: return KEY_DELETE;
    case KEY_UP: return KEY_PAGEUP; 
    case KEY_LEFT: return KEY_HOME; 
    case KEY_RIGHT: return KEY_END; 
    case KEY_DOWN: return KEY_PAGEDOWN;
    case KEY_F13: return KEY_F11;
  }

  return -1;
}

static int map(int code) {
  if(code >= KEY_F1 && code <= KEY_F10) {
    return fx_map[code - KEY_F1];
  }
  if(code == KEY_F13) {
    return KEY_MENU;
  }
  return code;
}

static int get_key_fn(int code) {
  if(code >= KEY_F1 && code <= KEY_F10) {
    return TestBit(state.fx_down, code - KEY_F1);
  }

  switch (code) {
    case KEY_BACKSPACE: return state.backspace_down;
    case KEY_UP: return state.up_down; 
    case KEY_LEFT: return state.left_down; 
    case KEY_RIGHT: return state.right_down; 
    case KEY_DOWN: return state.down_down;
    case KEY_F13: return state.hamburger_down; 
  }

  return -1;
}

static void set_key_fn(int code, int value) {
  if(code >= KEY_F1 && code <= KEY_F10) {
    if(value) {
      SetBit(state.fx_down, code - KEY_F1);
    } else {
      ClearBit(state.fx_down, code - KEY_F1);
    }
    return;
  }

  switch (code) {
    case KEY_BACKSPACE: 
      state.backspace_down = value > 0;
      return;
    case KEY_UP: 
      state.up_down = value > 0;
      return; 
    case KEY_LEFT: 
      state.left_down = value > 0;
      return; 
    case KEY_RIGHT: 
      state.right_down = value > 0;
      return; 
    case KEY_DOWN: 
      state.down_down = value > 0;
      return; 
    case KEY_F13:
      state.hamburger_down = value > 0;
      return;
  }
}

static int is_accelerator(int code) {
  switch (code) {
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:
    case KEY_LEFTALT:
    case KEY_RIGHTALT:
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
    case KEY_RIGHTMETA:
      return 1;
  }

  return 0;
}
 
static void write_event(struct input_event* ev) {
  if (ev->type == EV_KEY) {
    fprintf(stderr, "tx: %d [%d]\n", ev->code, ev->value);
  }
  if(write(kbout, ev, sizeof(struct input_event)) < 0) {
    fprintf(stderr, "failed to write event");
    exit(-1);
  }
}

static void read_event(struct input_event* ev) {
  unsigned int err = read(kbin, ev, sizeof(struct input_event));
  if(err != sizeof(struct input_event)) {
      fprintf(stderr, "failed to read event [%d]", err);
      exit(-1);
    }
}

static void insert_meta_event(struct timeval* t, int v) {
  state.v_meta = v;

  meta_ev.time = *t;
  meta_ev.value = v;
  write_event(&meta_ev);
}

static void insert_caps_events(struct timeval* t) {
  caps_ev.time = *t;
  caps_ev.value = 1;
  write_event(&caps_ev);
  caps_ev.value = 0;
  write_event(&caps_ev);
}

static int fn_key_handler(struct input_event* ev) {
  state.meta_down = ev->value > 0;
  if(ev->value != 0) {
    // key down -> discard.
    return 0;
  }

  if(state.v_meta) {
    // key up with an earlier key down.
    state.v_meta = 0;
    state.v_fn = 0;
    write_event(ev);
    return 1;
  }
  
  if(state.v_fn) {
    state.v_fn = 0;
    return 0;
  }
  
  // alt+tap
  if(state.alt_down) {
    insert_caps_events(&ev->time);
    return 2;
  }

  // tap
  // insert down discarded earlier
  insert_meta_event(&ev->time, 1);
  state.v_meta = 0;
  write_event(ev);
  return 2;
}

static int acc_key_handler(struct input_event* ev) {
  if(ev->code == KEY_LEFTALT || ev->code == KEY_RIGHTALT) {
    state.alt_down = ev->value > 0;
  }
  write_event(ev);
  return 1;
}

static int key_handler(struct input_event* ev) {
  int ev_count = 0;

  if(ev->code == KEY_LEFTMETA) {
    return fn_key_handler(ev);
  }

  if(is_accelerator(ev->code)) {
    return acc_key_handler(ev);
  }

  // ignore power key. event2 (power key) will handle
  if(ev->code == KEY_POWER) {
    return 0;
  }

  // ordinary, non-accelerator key
  if(ev->value != 1) {
    // key is already down
    if(get_key_fn(ev->code) > 0) {
      set_key_fn(ev->code, ev->value);
      ev->code = fn_map(ev->code);
    } else {
      ev->code = map(ev->code);
    }
  } else if(state.meta_down) {
    // key down and fn-level set
    int fn_code = fn_map(ev->code);
    
    if(fn_code > 0) {
      if(state.v_meta) {
        // set meta up 
        insert_meta_event(&ev->time, 0);
        ev_count++;
      }

      state.v_fn = 1;
      set_key_fn(ev->code, ev->value);
      ev->code = fn_code;
    } else if(!state.v_meta) {
      // insert previously suppressed meta down.
      insert_meta_event(&ev->time, 1);
      ev_count++;
    } 
  } else {
    ev->code = map(ev->code);
  }

  write_event(ev);
  return ++ev_count;
}
  
static int setup(char* kbin_path) {
  struct uinput_user_dev uidev;
  
  // open keyboard
  kbin = open(kbin_path, O_RDONLY);
  if (kbin < 0) {
    fprintf(stderr, "failed to open %s\n", kbin_path);
    return -1;
  }

  // consume all events here
  if (ioctl(kbin, EVIOCGRAB, (void*)1) < 0) {
    fprintf(stderr, "ioctl: failed to grab %s\n", kbin_path);
    return -1;
  }

  // create virtual keyboard
  kbout = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (kbout < 0) {
    fprintf(stderr, "failed to open /dev/uinput\n");
    return -1;
  }

  if (ioctl(kbout, UI_SET_EVBIT, EV_KEY) < 0) {
    fprintf(stderr, "ioctl: failed to set EVBIT\n");
    return -1;
  }

  unsigned char keys[] = KEY_MASK;
  for(int i = 1; i <= KEY_BRIGHTNESSUP; i++) {
    if(TestBit(keys, i)) {
      if(ioctl(kbout, UI_SET_KEYBIT, i) < 0) {
        fprintf(stderr, "ioctl: failed to set KEYBIT [%d]\n", i);
        return -1;
      }
    }
  }

  memset(&uidev, 0, sizeof(uidev));
  strcpy(uidev.name, KEYBOARD_NAME);
  if (write(kbout, &uidev, sizeof(uidev)) < 0) {
    fprintf(stderr, "failed to write to /dev/uinput\n");
    return -1;
  }

  int err;
  err = ioctl(kbout, UI_DEV_CREATE);
  if (err < 0) {
    fprintf(stderr, "ioctl: failed to create device [%d]\n", err);
    return -1;
  }
  
  return 0;
}

static int cruise() {
  struct input_event ev;
  struct input_event syn_ev = {
    .type = EV_SYN, 
    .code = SYN_REPORT
  };
  
  while (1) {
    read_event(&ev);
    if (ev.type == EV_KEY && ev.value != 2) {
      fprintf(stderr, "rx: %d [%d]\n", ev.code, ev.value);
      int n = key_handler(&ev);
      if(n > 0) {
        syn_ev.time = ev.time;
        write_event(&syn_ev);
      }
      if(n < 0) {
        fprintf(stderr, "unknown error handling event\n");
        return -1;
      }
    }
  }
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    fprintf(stderr, "usage: %s /dev/input/eventX\n", argv[0]);
    return -1;
  }

  if(setup(argv[1]) != 0) {
    fprintf(stderr, "failed to initialize devices\n");
    return -1;
  }

  return cruise();
}
