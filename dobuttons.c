#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 10

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
  libevdev_set_name(newdev, "evmerge dobuttons device");
  libevdev_set_id_vendor(newdev, 0x0991); // Random apparently unused vendor id.
  libevdev_set_id_product(newdev, 0xaf15); // Random digits
  libevdev_set_id_bustype(newdev, BUS_USB);
  libevdev_set_id_version(newdev, 256);
  libevdev_set_phys(newdev, "evmerge-dobuttons");
  libevdev_enable_event_type(newdev, EV_KEY);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_LEFTCTRL, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_LEFTSHIFT, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_TAB, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_W, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_TAB, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_J, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_K, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_SPACE, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_COMMA, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_DOT, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_DOWN, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_UP, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, BTN_LEFT, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, BTN_MIDDLE, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, BTN_RIGHT, NULL);

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
  mapping[KEY_F14] = BTN_LEFT;
  mapping[KEY_F15] = BTN_MIDDLE;
  mapping[KEY_F16] = BTN_RIGHT;

  mapping[KEY_F17] = -5;
  mapping[KEY_F18] = -20;
  mapping[KEY_F19] = -40;

  int wait_time = -1;
  int button_active = 0;

  do {
    struct input_event ev;
    struct epoll_event epoll_events[MAX_EVENTS];
    int nfds = epoll_wait(epoll_fd, epoll_events, MAX_EVENTS, wait_time);
    if (nfds == -1) {
      fprintf(stderr, "Failed in epoll_wait\n");
      return 1;
    }

    //printf("Got events: %d\n", nfds);
    for (int i = 0; i < nfds; i++) {
      struct libevdev *dev = epoll_events[i].data.ptr;

      while(!(err = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev))) {
        fprintf(stderr, "Event: %s %s %d\n",
            libevdev_event_type_get_name(ev.type),
            libevdev_event_code_get_name(ev.type, ev.code),
            ev.value);
        int keycode = ev.code;
        if(ev.type == EV_KEY && mapping[keycode])
          keycode = mapping[keycode];
        if(ev.type == EV_KEY && keycode < 0) {
          if(ev.value == 1) {
            printf("Repeating button!\n");
            wait_time = -1000 / keycode - 10;
            button_active = BTN_LEFT;
          } else {
            printf("Stopping repeating button!\n");
            wait_time = -1;
            button_active = 0;
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

    if(button_active) {
      err = libevdev_uinput_write_event(uidev, EV_KEY, button_active, 1);
      err = libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
      usleep(10000);
      err = libevdev_uinput_write_event(uidev, EV_KEY, button_active, 0);
      err = libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    }
  } while (err == 1 || err == 0 || err == -EAGAIN);


  printf("hi: %d\n", err);
}
