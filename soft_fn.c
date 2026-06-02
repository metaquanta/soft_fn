#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define KEYBOARD_NAME "Virtual keyboard (soft-fn)"
#define FUNCTION_ROW_SIZE 13

static int kbin, kbout;
static int function_row_enabled;
static int function_row[FUNCTION_ROW_SIZE];

static const int function_keys[FUNCTION_ROW_SIZE] = {
  KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7,
  KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_F13
};

static int get_function_row_index(int code);
static int function_row_map(int code);

static struct {
  unsigned int alt_down : 1;
  unsigned int meta_down : 1;
  unsigned int backspace_down : 1;
  unsigned int up_down : 1;
  unsigned int left_down : 1;
  unsigned int right_down : 1;
  unsigned int down_down : 1;
  unsigned int v_fn : 1;
  unsigned int v_meta : 1;
  unsigned int function_row_down[FUNCTION_ROW_SIZE];
} state;

static struct input_event meta_ev = {
  .type = EV_KEY,
  .code = KEY_LEFTMETA
};

static struct input_event caps_ev = {
  .type = EV_KEY,
  .code = KEY_CAPSLOCK
};

static int get_function_row_index(int code) {
  if(!function_row_enabled) {
    return -1;
  }

  for(int i = 0; i < FUNCTION_ROW_SIZE; i++) {
    if(function_row[i] == code) {
      return i;
    }
  }

  return -1;
}

static int function_row_map(int code) {
  int function_row_index = get_function_row_index(code);

  if(function_row_index < 0) {
    return -1;
  }

  return function_keys[function_row_index];
}

static int fn_map(int code) {
  int mapped_code = function_row_map(code);

  if(mapped_code > 0) {
    return mapped_code;
  }

  switch (code) {
    case KEY_BACKSPACE: return KEY_DELETE;
    case KEY_UP: return KEY_PAGEUP;
    case KEY_LEFT: return KEY_HOME;
    case KEY_RIGHT: return KEY_END;
    case KEY_DOWN: return KEY_PAGEDOWN;
  }

  return -1;
}

static int get_key_fn(int code) {
  int function_row_index = get_function_row_index(code);

  if(function_row_index >= 0) {
    return state.function_row_down[function_row_index];
  }

  switch (code) {
    case KEY_BACKSPACE: return state.backspace_down;
    case KEY_UP: return state.up_down;
    case KEY_LEFT: return state.left_down;
    case KEY_RIGHT: return state.right_down;
    case KEY_DOWN: return state.down_down;
  }

  return -1;
}

static void set_key_fn(int code, unsigned char value) {
  int b = value > 0;
  int function_row_index = get_function_row_index(code);

  if(function_row_index >= 0) {
    state.function_row_down[function_row_index] = b;
    return;
  }

  switch (code) {
    case KEY_BACKSPACE:
      state.backspace_down = b;
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

static int is_accelerator(int code) {
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

static void write_event(struct input_event* ev) {
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
  int function_row_index;

  if(ev->code == KEY_LEFTMETA) {
    return fn_key_handler(ev);
  }

  if(is_accelerator(ev->code)) {
    return acc_key_handler(ev);
  }

  function_row_index = get_function_row_index(ev->code);

  // ordinary, non-accelerator key
  if(ev->value != 1) {
    // key is already down
    if(get_key_fn(ev->code) > 0) {
      set_key_fn(ev->code, ev->value);
      ev->code = fn_map(ev->code);
    }
  } else if(function_row_index >= 0 && !state.meta_down) {
    set_key_fn(ev->code, ev->value);
    ev->code = function_keys[function_row_index];
  } else if(function_row_index >= 0 && state.meta_down) {
    if(state.v_meta) {
      // set meta up
      insert_meta_event(&ev->time, 0);
      ev_count++;
    }

    state.v_fn = 1;
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
  }

  write_event(ev);
  return ++ev_count;
}

static int setup(char* kbin_path) {
  struct uinput_user_dev uidev;

  // open keyboard
  kbin = open(kbin_path, O_RDONLY);
  if (kbin < 0) {
    fprintf(stderr, "failed to open %s: %s\n", kbin_path, strerror(errno));
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
    fprintf(stderr, "failed to open /dev/uinput: %s\n", strerror(errno));
    return -1;
  }

  if (ioctl(kbout, UI_SET_EVBIT, EV_KEY) < 0) {
    fprintf(stderr, "ioctl: failed to set EVBIT\n");
    return -1;
  }

  for(int i = 1; i <= KEY_MAX; i++) {
    if(ioctl(kbout, UI_SET_KEYBIT, i) < 0) {
      fprintf(stderr, "ioctl: failed to set KEYBIT [%d]\n", i);
      return -1;
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

static void print_usage(char* program_name) {
  fprintf(stderr, "usage: %s [--function_row='xx xx xx xx xx xx xx xx xx xx xx xx xx'] /dev/input/eventX\n", program_name);
}

static int parse_hex_keycode(char* value) {
  char* end;
  long code;

  if(*value == '\0') {
    return -1;
  }

  for(char* p = value; *p != '\0'; p++) {
    if(!isxdigit((unsigned char)*p)) {
      return -1;
    }
  }

  code = strtol(value, &end, 16);
  if(*end != '\0' || code < 0 || code > KEY_MAX) {
    return -1;
  }

  return (int)code;
}

static int parse_function_row(char* value) {
  char* token;

  for(int i = 0; i < FUNCTION_ROW_SIZE; i++) {
    token = strtok(i == 0 ? value : NULL, " \t");
    if(token == NULL) {
      fprintf(stderr, "--function_row requires thirteen hex keycodes\n");
      return -1;
    }

    function_row[i] = parse_hex_keycode(token);
    if(function_row[i] < 0) {
      fprintf(stderr, "invalid --function_row value: %s\n", token);
      return -1;
    }

    for(int j = 0; j < i; j++) {
      if(function_row[j] == function_row[i]) {
        fprintf(stderr, "duplicate --function_row value: %s\n", token);
        return -1;
      }
    }
  }

  if(strtok(NULL, " \t") != NULL) {
    fprintf(stderr, "--function_row requires thirteen hex keycodes\n");
    return -1;
  }

  function_row_enabled = 1;
  return 0;
}

static int parse_args(int argc, char* argv[], char** kbin_path) {
  *kbin_path = NULL;

  for(int i = 1; i < argc; i++) {
    if(strncmp(argv[i], "--function_row=", 15) == 0) {
      if(function_row_enabled) {
        fprintf(stderr, "--function_row specified more than once\n");
        return -1;
      }

      if(parse_function_row(argv[i] + 15) != 0) {
        return -1;
      }
    } else if(strcmp(argv[i], "--function_row") == 0) {
      if(function_row_enabled) {
        fprintf(stderr, "--function_row specified more than once\n");
        return -1;
      }

      if(++i >= argc) {
        fprintf(stderr, "--function_row requires a value\n");
        return -1;
      }

      if(parse_function_row(argv[i]) != 0) {
        return -1;
      }
    } else if(*kbin_path == NULL) {
      *kbin_path = argv[i];
    } else {
      fprintf(stderr, "unexpected argument: %s\n", argv[i]);
      return -1;
    }
  }

  if(*kbin_path == NULL) {
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
      switch (key_handler(&ev)) {
        case 0:
          break;
        case 1:
        case 2:
        case 3:
          syn_ev.time = ev.time;
          write_event(&syn_ev);
          break;
        default:
          fprintf(stderr, "unknown error handling event\n");
          return -1;
      }
    }
  }
}

int main(int argc, char* argv[]) {
  char* kbin_path;

  if(parse_args(argc, argv, &kbin_path) != 0) {
    print_usage(argv[0]);
    return -1;
  }

  if(setup(kbin_path) != 0) {
    fprintf(stderr, "failed to initialize devices\n");
    return -1;
  }
  return cruise();
}
