
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

#define STRING_SIZE 40

int set_interface_attribs(int fd, int speed, int parity, int databits, int stopbits, int hwcheck, int swcheck, int smcheck);
void set_mincount(int fd, int mcount);
int send_cmd(int fd, char *cmd);
int get_cmd(int fd, char *buff);

#endif
