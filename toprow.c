/*
 * toprow.c - translate a keyboard's function-row-physmap.
 *
 * function-row-physmap is an ordered list of SCANCODES (left to right) for the
 * top row. A scancode only has meaning relative to a specific device's keymap,
 * so for each scancode we ask the evdev device what KEYCODE it maps to using the
 * EVIOCGKEYCODE ioctl and print the resulting keycodes as hex pairs.
 *
 *   physmap value  --EVIOCGKEYCODE-->  keycode (KEY_*)
 *
 * Build:  cc -O2 -Wall -o toprow toprow.c
 * Run:    sudo ./toprow /dev/input/event0 /sys/.../function_row_physmap
 *
 * Needs read access to the evdev node (root or the 'input' group), because
 * EVIOCGKEYCODE is issued on the open device fd.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#define FUNCTION_ROW_SIZE 13

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s /dev/input/eventX /sys/.../function_row_physmap\n", argv[0]);
        return 1;
    }

    const char *evdev = argv[1];
    const char *physpath = argv[2];

    int fd = open(evdev, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open %s: %s (try sudo)\n", evdev, strerror(errno));
        return 1;
    }

    FILE *f = fopen(physpath, "r");
    if (!f) {
        fprintf(stderr, "open %s: %s\n", physpath, strerror(errno));
        close(fd);
        return 1;
    }

    unsigned int scancode;
    unsigned int keycodes[FUNCTION_ROW_SIZE];
    int pos = 0;

    while (fscanf(f, "%x", &scancode) == 1) {          /* hex, space-separated */
        unsigned int kc[2] = { scancode, 0 };          /* [0]=scancode in */

        if (pos >= FUNCTION_ROW_SIZE) {
            fprintf(stderr, "%s contains more than thirteen entries\n", physpath);
            fclose(f);
            close(fd);
            return 1;
        }

        if (ioctl(fd, EVIOCGKEYCODE, kc) != 0) {        /* kc[1]=keycode out */
            fprintf(stderr, "EVIOCGKEYCODE 0x%x: %s\n", scancode, strerror(errno));
            fclose(f);
            close(fd);
            return 1;
        }

        keycodes[pos++] = kc[1];
    }

    if (pos != FUNCTION_ROW_SIZE) {
        fprintf(stderr, "%s contains %d entries, expected thirteen\n", physpath, pos);
        fclose(f);
        close(fd);
        return 1;
    }

    for (int i = 0; i < FUNCTION_ROW_SIZE; i++) {
        printf("%s%02x", i == 0 ? "" : " ", keycodes[i]);
    }
    printf("\n");

    fclose(f);
    close(fd);
    return 0;
}
