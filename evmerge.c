#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 10

int main(int argc, char **argv) {
  struct libevdev **devs = calloc(3, sizeof(struct libevdev *));
  int *fds = calloc(3, sizeof(int));
  int err;

  printf("argc: %d; argv[1]: %s", argc, argv[1]);
  fds[0] = open("/dev/input/by-id/usb-VEC_VEC_USB_Footpedal-event-if00", O_RDONLY|O_NONBLOCK);
  if (fds[0] < 0) {
    fprintf(stderr, "Failed to open input 0: %s", strerror(-err));
    return 1;
  }

  fds[1] = open("/dev/input/by-id/foot-pedal-back", O_RDONLY|O_NONBLOCK);
  if (fds[1] < 0) {
    fprintf(stderr, "Failed to open input 1: %s", strerror(-err));
    return 1;
  }
  fds[2] = open("/dev/input/by-id/foot-pedal-front", O_RDONLY|O_NONBLOCK);
  if (fds[2] < 0) {
    fprintf(stderr, "Failed to open input 2: %s", strerror(-err));
    return 1;
  }

  err = libevdev_new_from_fd(fds[0], &devs[0]);
  if (err < 0) {
    fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-err));
    return 1;
  }
  err = libevdev_grab(devs[0], LIBEVDEV_GRAB);
  if (err < 0) {
    fprintf(stderr, "Failed to grab device (%s)\n", strerror(-err));
    return 1;
  }
  fprintf(stderr, "Binding device '%s'\n", libevdev_get_name(devs[0]));

  printf("Version is %d\n", libevdev_get_id_version(devs[0]));
  printf("Vendor id is %x\n", libevdev_get_id_vendor(devs[0]));

  err = libevdev_new_from_fd(fds[1], &devs[1]);
  if (err < 0) {
    fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-err));
    return 1;
  }
  err = libevdev_grab(devs[1], LIBEVDEV_GRAB);
  if (err < 0) {
    fprintf(stderr, "Failed to grab device (%s)\n", strerror(-err));
    return 1;
  }

  err = libevdev_new_from_fd(fds[2], &devs[2]);
  if (err < 0) {
    fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-err));
    return 1;
  }
  err = libevdev_grab(devs[2], LIBEVDEV_GRAB);
  if (err < 0) {
    fprintf(stderr, "Failed to grab device (%s)\n", strerror(-err));
    return 1;
  }

  fprintf(stderr, "Binding device '%s'\n", libevdev_get_name(devs[0]));
  fprintf(stderr, "Binding device '%s'\n", libevdev_get_name(devs[1]));
  fprintf(stderr, "Binding device '%s'\n", libevdev_get_name(devs[2]));

  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    fprintf(stderr, "Failed to init epoll\n");
    return 1;
  }

  struct epoll_event epoll_ev;
  epoll_ev.events = EPOLLIN;
  epoll_ev.data.ptr = devs[0];
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fds[0], &epoll_ev) == -1) {
    fprintf(stderr, "Failed to add fds[0] to epoll set\n");
    return 1;
  }

  epoll_ev.events = EPOLLIN;
  epoll_ev.data.ptr = devs[1];
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fds[1], &epoll_ev) == -1) {
    fprintf(stderr, "Failed to add fds[1] to epoll set\n");
    return 1;
  }

  epoll_ev.events = EPOLLIN;
  epoll_ev.data.ptr = devs[2];
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fds[2], &epoll_ev) == -1) {
    fprintf(stderr, "Failed to add fds[2] to epoll set\n");
    return 1;
  }

  struct libevdev *newdev = libevdev_new();
  libevdev_set_name(newdev, "evmerge device");
  libevdev_set_id_vendor(newdev, 0x0991); // Random apparently unused vendor id.
  libevdev_set_id_product(newdev, 0xaf14); // Random digits
  libevdev_set_id_bustype(newdev, BUS_USB);
  libevdev_set_id_version(newdev, 256);
  libevdev_set_phys(newdev, "evmerge");
  libevdev_enable_event_type(newdev, EV_KEY);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_F14, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_F15, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_F16, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_1, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_2, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_3, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_4, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_5, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, KEY_6, NULL);
  libevdev_enable_event_code(newdev, EV_KEY, BTN_LEFT, NULL);

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

  do {
    struct input_event ev;
    struct epoll_event epoll_events[MAX_EVENTS];
    int nfds = epoll_wait(epoll_fd, epoll_events, MAX_EVENTS, -1);
    if (nfds == -1) {
      fprintf(stderr, "Failed in epoll_wait\n");
      return 1;
    }

    printf("Got events: %d\n", nfds);
    for (int i = 0; i < nfds; i++) {
      struct libevdev *dev = epoll_events[i].data.ptr;

      while(!(err = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev))) {
        if(dev == devs[2]) {
          switch(ev.code) {
            case KEY_1:
              ev.code = KEY_4;
              break;
            case KEY_2:
              ev.code = KEY_5;
              break;
            case KEY_3:
              ev.code = KEY_6;
              break;
          }
        }
        fprintf(stderr, "Event: %s %s %d\n",
            libevdev_event_type_get_name(ev.type),
            libevdev_event_code_get_name(ev.type, ev.code),
            ev.value);
        err = libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
        if (err != 0) {
          fprintf(stderr, "Failed in writing to uinput device (%s)\n", strerror(-err));
          return 1;
        }
      }
    }
  } while (err == 1 || err == 0 || err == -EAGAIN);


  printf("hi\n");
}
