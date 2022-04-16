#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <toml.h>

#define MAX_EVENTS 10

struct inputDevice {
  const char *name;
  const char *dev;
  int *map;
} *inputDevs;
int nInputDevs = 0;

int tomlSize(toml_table_t *t) {
  int size;
  for (size = 0; ; size++) {
    const char* k = toml_key_in(t, size);
    if (!k)
      break;
  }
  return size;
}

int lookupKey(const char *k) {
  if(!strcmp(k, "1"))
    return KEY_1;
  if(!strcmp(k, "2"))
    return KEY_2;
  if(!strcmp(k, "3"))
    return KEY_3;
  if(!strcmp(k, "4"))
    return KEY_4;
  if(!strcmp(k, "5"))
    return KEY_5;
  if(!strcmp(k, "6"))
    return KEY_6;
  return -1;
}

int readConf(char *filename) {
  FILE *fp = fopen(filename, "r");
  char errbuf[200];
  int j;
  if (!fp) {
    fprintf(stderr, "Unable to open configuration file '%s'\n", filename);
    return -1;
  }

  toml_table_t * conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
  fclose(fp);

  if (!conf) {
    fprintf(stderr, "Unable to parse configuration file '%s': %s\n", filename, errbuf);
    return -1;
  }

  inputDevs = calloc(tomlSize(conf), sizeof(struct inputDevice));

  for (int i = 0; ; i++) {
    const char* key = toml_key_in(conf, i);
    if (!key)
      break;
    toml_table_t *dev = toml_table_in(conf, key);
    toml_datum_t device = toml_string_in(dev, "device");
    if (!device.ok) {
      fprintf(stderr, "Config error: No device field found on dev %s", key);
      exit(1);
    }
    printf("device %i - %s: %s\n", i, key, device.u.s);
    inputDevs[nInputDevs].name = key;
    inputDevs[nInputDevs].dev = device.u.s;
    toml_table_t *remap = toml_table_in(dev, "remap");
    if (remap) {
      int nRemaps;
      for (int nRemaps = 0; ; nRemaps++) {
        const char* from = toml_key_in(remap, nRemaps);
        if (!from)
          break;
      }
      inputDevs[nInputDevs].map = calloc(2*tomlSize(remap) + 1, sizeof(inputDevs[0].map[0]));
      for (j = 0; ; j++) {
        const char* from = toml_key_in(remap, j);
        if (!from)
          break;
        toml_datum_t to = toml_string_in(remap, from);
        if (!to.ok) {
          fprintf(stderr, "Config erorr: Missing or non-string mapping for %s on device %s\n", from, key);
          exit(1);
        }
        inputDevs[nInputDevs].map[2*j] = lookupKey(from);
        inputDevs[nInputDevs].map[2*j+1] = lookupKey(to.u.s);
        printf("remap %s to %s\n", from, to.u.s);
      }
      inputDevs[nInputDevs].map[2*j] = 0;
      free(remap);
    } else {
      inputDevs[nInputDevs].map = NULL;
    }

    //free(dev);
    nInputDevs++;
  }
  free(conf);

  for (int i = 0; i < nInputDevs; i++) {
    printf("Input %d %s: %s\n", i, inputDevs[i].name, inputDevs[i].dev);
    if (inputDevs[i].map) {
      printf("  Remapping:");
      for (int j = 0; inputDevs[i].map[j]; j += 2)
        printf(" %d => %d;", inputDevs[i].map[j], inputDevs[i].map[j+1]);
      printf("\n");
    }
  }


}

int main(int argc, char **argv) {
  struct libevdev **devs;
  int *fds;
  int err;

  readConf("evmerge.toml");
  devs = calloc(nInputDevs, sizeof(struct libevdev *));
  fds = calloc(nInputDevs, sizeof(int));

  for (int i = 0; i < nInputDevs; i++) {
    fds[i] = open(inputDevs[i].dev, O_RDONLY|O_NONBLOCK);
    if (fds[i] < 0) {
      fprintf(stderr, "Failed to open input %s (%s): %s", inputDevs[i].name, inputDevs[i].dev, strerror(-err));
      return 1;
    }

    err = libevdev_new_from_fd(fds[i], &devs[i]);
    if (err < 0) {
      fprintf(stderr, "Failed to init libevdev %s (%s): %s\n", inputDevs[i].name, inputDevs[i].dev, strerror(-err));
      return 1;
    }
    err = libevdev_grab(devs[i], LIBEVDEV_GRAB);
    if (err < 0) {
      fprintf(stderr, "Failed to grab device %s (%s): %s\n", inputDevs[i].name, inputDevs[i].dev, strerror(-err));
      return 1;
    }
    fprintf(stderr, "Binding device %s: (%s) '%s'\n", inputDevs[i].name, inputDevs[i].dev, libevdev_get_name(devs[i]));

    fprintf(stderr, "  Version is %d\n", libevdev_get_id_version(devs[i]));
    fprintf(stderr, "  Vendor id is %x\n", libevdev_get_id_vendor(devs[i]));
  }


  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    fprintf(stderr, "Failed to init epoll\n");
    return 1;
  }

  struct epoll_event epoll_ev;
  for (int i = 0; i < nInputDevs; i++) {
    epoll_ev.events = EPOLLIN;
    epoll_ev.data.ptr = (void *)i;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fds[i], &epoll_ev) == -1) {
      fprintf(stderr, "Failed to add fds[%d] to epoll set\n", i);
      return 1;
    }
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

    fprintf(stderr, "Got events: %d\n", nfds);
    for (int i = 0; i < nfds; i++) {
      int devIndex = (int)epoll_events[i].data.ptr;
      struct libevdev *dev = devs[devIndex];


      while(!(err = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev))) {
        int *map = inputDevs[devIndex].map; 
        if (map) 
          for (int i = 0; map[i]; i += 2)
            if (map[i] == ev.code) {
              ev.code = map[i+1];
              break;
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
}
