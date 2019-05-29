
#ifndef _serial_lib_
#define _serial_lib_

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "gnome.h"

#define leng 40

int set_interface_attribs(int fd, int speed);
void set_mincount(int fd, int mcount);
void send_cmd(int fd, char *cmd);
void get_cmd(int fd, char *buff);

#endif
