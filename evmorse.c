#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>

#define MAX_EVENTS 10

  
int send_key(struct libevdev_uinput *uidev, char c) {
  int err = libevdev_uinput_write_event(uidev, EV_KEY, c, 1);
  if (err != 0) {
    fprintf(stderr, "Failed in writing to uinput device (%s)\n", strerror(-err));
    return 1;
  }
  err = libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
  if (err != 0) {
    fprintf(stderr, "Failed in writing to uinput device (%s)\n", strerror(-err));
    return 1;
  }
  err = libevdev_uinput_write_event(uidev, EV_KEY, c, 0);
  if (err != 0) {
    fprintf(stderr, "Failed in writing to uinput device (%s)\n", strerror(-err));
    return 1;
  }
  err = libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
  if (err != 0) {
    fprintf(stderr, "Failed in writing to uinput device (%s)\n", strerror(-err));
    return 1;
  }
  return 0;
}

int main() {
  struct libevdev *dev = NULL;
  int fd;
  int err;

  fd = open("/dev/input/by-id/virtual-kmonad-device", O_RDONLY|O_NONBLOCK);

  err = libevdev_new_from_fd(fd, &dev);
  if (err < 0) {
    fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-err));
    return 1;
  }
  err = libevdev_grab(dev, LIBEVDEV_GRAB);
  if (err < 0) {
    fprintf(stderr, "Failed to grab device (%s)\n", strerror(-err));
    return 1;
  }
  fprintf(stderr, "Binding device '%s'\n", libevdev_get_name(dev));

  printf("Version is %d\n", libevdev_get_id_version(dev));
  printf("Vendor id is %x\n", libevdev_get_id_vendor(dev));

  fprintf(stderr, "Binding device '%s'\n", libevdev_get_name(dev));

  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    fprintf(stderr, "Failed to init epoll\n");
    return 1;
  }

  struct epoll_event epoll_ev;
  epoll_ev.events = EPOLLIN;
  epoll_ev.data.ptr = dev;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_ev) == -1) {
    fprintf(stderr, "Failed to add fd to epoll set\n");
    return 1;
  }

  struct libevdev *newdev = libevdev_new();
  libevdev_set_name(newdev, "evmerge morse device");
  libevdev_set_id_vendor(newdev, 0x0991); // Random apparently unused vendor id.
  libevdev_set_id_product(newdev, 0xaf16); // Random digits
  libevdev_set_id_bustype(newdev, BUS_USB);
  libevdev_set_id_version(newdev, 256);
  libevdev_set_phys(newdev, "evmerge-morse");
  libevdev_enable_event_type(newdev, EV_KEY);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_LEFTCTRL, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_LEFTSHIFT, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_TAB, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_A, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_B, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_C, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_D, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_E, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_F, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_G, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_F, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_H, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_I, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_J, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_K, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_L, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_M, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_N, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_O, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_P, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_Q, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_R, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_S, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_T, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_U, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_V, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_W, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_X, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_Y, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_Z, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_SPACE, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_COMMA, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_DOT, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_DOWN, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_UP, NULL);

  struct libevdev_uinput *uidev;

  int uifd = open("/dev/uinput", O_RDWR);
  if (uifd < 0) {
    fprintf(stderr, "Failed to open /dev/uinput (%m)\n");
    return 1;
  }

  err = libevdev_uinput_create_from_device(newdev, uifd, &uidev);
  if (err != 0) {
    fprintf(stderr, "Failed to create uinput device (%s)\n", strerror(-err));
    return 1;
  }
  fprintf(stderr, "Created device '%s'\n", libevdev_get_name(newdev));

  const char *syspath = libevdev_uinput_get_syspath(uidev);
  const char *devnode = libevdev_uinput_get_devnode(uidev);

  fprintf(stderr, "Path and name %s / %s\n", syspath, devnode);

  int mapping[256];
  memset(mapping, 0, sizeof(mapping));
  mapping[5] = KEY_A;
  mapping[24] = KEY_B;
  mapping[26] = KEY_C;
  mapping[12] = KEY_D;
  mapping[2] = KEY_E;
  mapping[18] = KEY_F;
  mapping[14] = KEY_G;
  mapping[16] = KEY_H;
  mapping[4] = KEY_I;
  mapping[23] = KEY_J;
  mapping[13] = KEY_K;
  mapping[20] = KEY_L;
  mapping[7] = KEY_M;
  mapping[6] = KEY_N;
  mapping[15] = KEY_O;
  mapping[22] = KEY_P;
  mapping[29] = KEY_Q;
  mapping[10] = KEY_R;
  mapping[8] = KEY_S;
  mapping[3] = KEY_T;
  mapping[9] = KEY_U;
  mapping[17] = KEY_V;
  mapping[11] = KEY_W;
  mapping[25] = KEY_X;
  mapping[27] = KEY_Y;
  mapping[28] = KEY_Z;

  int average_length = 100000;
  int average_pause = 200000;
  int pressed = 0;
  int cur_char = 1;
  int wait_time = -1;
  struct timeval lastEvent = {0, 0};

  do {
    struct input_event ev;
    struct epoll_event epoll_events[MAX_EVENTS];
    printf("Waiting for %d\n", wait_time);
    int nfds = epoll_wait(epoll_fd, epoll_events, MAX_EVENTS, wait_time);
    if (nfds == -1) {
      fprintf(stderr, "Failed in epoll_wait\n");
      return 1;
    }

    // timeout
    if (!nfds) {
      if (cur_char == 1) {
        // no keys pressed since last timeout; it's a space.
        err = libevdev_uinput_write_event(uidev, EV_KEY, KEY_SPACE, 1);
        send_key(uidev, KEY_SPACE);
        wait_time = -1;
      } else {
        // send the current letter, and set timeout for a space
        printf("%6d %4d Letter %d\n", wait_time / 1000, average_pause / 1000, cur_char);
        int c = mapping[cur_char];
        if (c > 0) {
          send_key(uidev, c);
        }
        cur_char = 1;
        //wait_time = 4 * average_pause / 1000;
        wait_time = -1;
      }
    }

    //printf("Got events: %d\n", nfds);
    for (int i = 0; i < nfds; i++) {
      struct libevdev *dev = epoll_events[i].data.ptr;

      while(!(err = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev))) {
        /*fprintf(stderr, "Event: %s %s %d\n",
            libevdev_event_type_get_name(ev.type),
            libevdev_event_code_get_name(ev.type, ev.code),
            ev.value);*/
        int keycode = ev.code;
        if(ev.type == EV_KEY && keycode == KEY_F17) {
          int timeDiff = (ev.time.tv_sec - lastEvent.tv_sec) * 1000000 + (ev.time.tv_usec - lastEvent.tv_usec);
          if(ev.value == 1) {
            if (!pressed) {
              /*if (timeDiff > 20 * average_pause | timeDiff < 0) {
                printf("%6d %4d Break\n", timeDiff / 1000, average_pause / 1000);
                cur_char = 1;
              }
              if (timeDiff > 8 * average_pause) {
                printf("%6d %4d Space\n", timeDiff / 1000, average_pause / 1000);
              } else if (timeDiff > average_pause) {
                printf("%6d %4d Letter %d\n", timeDiff / 1000, average_pause / 1000, cur_char);
                int c = mapping[cur_char];
                printf("Mapped %d to %d\n", cur_char, c);
                if (c > 0) {
                  err = libevdev_uinput_write_event(uidev, EV_KEY, c, 1);
                  if (err != 0) {
                    fprintf(stderr, "Failed in writing to uinput device (%s)\n", strerror(-err));
                    return 1;
                  }
                  err = libevdev_uinput_write_event(uidev, EV_KEY, c, 0);
                  if (err != 0) {
                    fprintf(stderr, "Failed in writing to uinput device (%s)\n", strerror(-err));
                    return 1;
                  }
                }
                cur_char = 1;
              } else {
                printf("%6d %4d Gap\n", timeDiff / 1000, average_pause / 1000);
              }*/
              /*if (timeDiff < 20 * average_pause && timeDiff > 0)
                average_pause = average_pause * 0.9 + timeDiff * 0.1;*/
              lastEvent = ev.time;
              pressed = 1;
              wait_time = -1; // No timeout while holding the key
            }
          } else {
            if (pressed) {
              if (timeDiff > 20 * average_length || timeDiff < 0) {
                // cancel existing character on a very long press
                printf("%6d %4d Very Long\n", timeDiff / 1000, average_length / 1000);
                cur_char = 1;
                wait_time = -1;
              } else if (timeDiff > 2 * average_length) {
                printf("%6d %4d Long\n", timeDiff / 1000, average_length / 1000);
                cur_char = cur_char * 2 + 1;
              }
              else {
                printf("%6d %4d Short\n", timeDiff / 1000, average_length / 1000);
                cur_char *= 2;
              }
              if (timeDiff < 20 * average_length && timeDiff > 0) {
                printf("Setting wait_time to %d from %d\n", (int)(1.5 * average_pause), average_pause);
                //average_length = average_length * 0.9 + timeDiff * 0.1;
                wait_time = 1.5 * average_pause / 1000;
              }
              
              lastEvent = ev.time;
              pressed = 0;
            }
          }
        } else {
          err = libevdev_uinput_write_event(uidev, ev.type, keycode, ev.value);
        } 
        if (err != 0) {
          fprintf(stderr, "Failed in writing to uinput device (%s)\n", strerror(-err));
          return 1;
        }
      }
    }
  } while (err == 1 || err == 0 || err == -EAGAIN);


  printf("hi: %d\n", err);
}
