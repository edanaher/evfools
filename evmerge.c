#include <libevdev/libevdev.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10

int main() {
  struct libevdev *dev1 = NULL;
  struct libevdev *dev2 = NULL;
  int fd1, fd2;
  int err;

  fd1 = open("/dev/input/by-id/usb-VEC_VEC_USB_Footpedal-event-if00", O_RDONLY|O_NONBLOCK);
  fd2 = open("/dev/input/by-id/usb-Keyboardio_Atreus_CatreusE-if03-event-kbd", O_RDONLY|O_NONBLOCK);

  err = libevdev_new_from_fd(fd1, &dev1);
  if (err < 0) {
    fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-err));
    return 1;
  }
  err = libevdev_grab(dev1, LIBEVDEV_GRAB);
  if (err < 0) {
    fprintf(stderr, "Failed to grab device (%s)\n", strerror(-err));
    return 1;
  }
  fprintf(stderr, "Binding device '%s'\n", libevdev_get_name(dev1));

  err = libevdev_new_from_fd(fd2, &dev2);
  if (err < 0) {
    fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-err));
    return 1;
  }
  err = libevdev_grab(dev2, LIBEVDEV_GRAB);
  if (err < 0) {
    fprintf(stderr, "Failed to grab device (%s)\n", strerror(-err));
    return 1;
  }
  fprintf(stderr, "Binding device '%s'\n", libevdev_get_name(dev1));
  fprintf(stderr, "Binding device '%s'\n", libevdev_get_name(dev2));

  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    fprintf(stderr, "Failed to init epoll\n");
    return 1;
  }

  struct epoll_event epoll_ev1;
  epoll_ev1.events = EPOLLIN;
  epoll_ev1.data.ptr = dev1;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd1, &epoll_ev1) == -1) {
    fprintf(stderr, "Failed to add fd1 to epoll set\n");
    return 1;
  }

  struct epoll_event epoll_ev2;
  epoll_ev2.events = EPOLLIN;
  epoll_ev2.data.ptr = dev2;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd2, &epoll_ev2) == -1) {
    fprintf(stderr, "Failed to add fd2 to epoll set\n");
    return 1;
  }

  int uifd = open("/dev/uinput", O_RDWR);
  if (uifd < 0) {
    fprintf(stderr, "Failed to open /dev/uinput (%m)\n");
    return 1;
  }

  struct libevdev_uinput *uidev;
  err = libevdev_uinput_create_from_device(dev1, uifd, &uidev);
  if (err != 0) {
    fprintf(stderr, "Failed to create uinput device (%s)\n", strerror(-err));
    return 1;
  }
  libevdev_set_name(uidev, "evmerge device");
  fprintf(stderr, "Created device '%s'\n", libevdev_get_name(uidev));


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

      err = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
      if (err == 0) {
        fprintf(stderr, "Event: %s %s %d\n",
            libevdev_event_type_get_name(ev.type),
            libevdev_event_code_get_name(ev.type, ev.code),
            ev.value);
        //err = libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
        if (err != 0) {
          fprintf(stderr, "Failed in writing to uinput device (%s)\n", strerror(-err));
          return 1;
        }
      }
    }
  } while (err == 1 || err == 0 || err == -EAGAIN);


  printf("hi\n");
}
