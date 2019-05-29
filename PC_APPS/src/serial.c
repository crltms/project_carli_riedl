
#include "serial.h"

int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    //tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void set_mincount(int fd, int mcount)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}

void send_cmd(int fd, char *cmd)
{
  int wlen = 0;
  /* simple output */
  wlen = write(fd, cmd, leng);
  printf("write fd %i %s\n",fd,cmd);

  if (wlen != leng) {
      printf("Error from write: %d, %d\n", wlen, errno);
  }
  tcdrain(fd);    /* delay for output */
}
void get_cmd(int fd, char *buff)
{
  // int wlen = 0;
  // /* simple output */
  // wlen = read(fd, &buff, leng);
  // printf("read fd %i\n",fd);
  //
  // if (wlen != leng) {
  //     printf("Error from read: %d, %d, %s\n", wlen, errno,buff);
  // }

  int n;
  char xbuff='0';
  int i =0;
  while(xbuff != '\n'){
    n = read(fd, &xbuff, 1);
    // printf("xbuff %c n %i\n", xbuff,n);
    buff[i]=xbuff;
    i++;
  }
  // printf("buff %s", buff);
  // n = read(fd, &xbuff[1], 1);
  // printf("buff %c, n %i\n", xbuff[1],n);
  // n = read(fd, &xbuff, 1);
  // printf("buff %c, n %i\n", xbuff,n);
  // n = read(fd, &xbuff, 1);
  // printf("buff %c, n %i\n", xbuff,n);
  // n = read(fd, &xbuff, 1);
  // printf("buff %c, n %i\n", xbuff,n);


  tcdrain(fd);    /* delay for output */
}
