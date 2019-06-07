
#include "serial.h"


int set_interface_attribs (int fd, int speed, int parity, int databits, int stopbits, int hwcheck, int swcheck, int smcheck)
{
	struct termios tty;

	if (tcgetattr (fd, &tty) < 0) {
		printf ("Error from tcgetattr: %s\n", strerror (errno));
		return -1;
	}

	cfsetospeed (&tty, (speed_t) speed);
	cfsetispeed (&tty, (speed_t) speed);

	tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
//STOPBITS----------------------------------------------------------------------
	if (stopbits == 1) {
		tty.c_cflag &= ~CSTOPB;
	} else if (stopbits == 2) {
		tty.c_cflag |= CSTOPB;
	}
//------------------------------------------------------------------------------
//DATABITS----------------------------------------------------------------------
	tty.c_cflag &= ~CS5;
	tty.c_cflag &= ~CS6;
	tty.c_cflag &= ~CS7;
	tty.c_cflag &= ~CS8;
	if (databits == 5) {
		tty.c_cflag |= CS5;
	} else if (databits == 6) {
		tty.c_cflag |= CS6;
	} else if (databits == 7) {
		tty.c_cflag |= CS7;
	} else if (databits == 8) {
		tty.c_cflag |= CS8;
	}
//------------------------------------------------------------------------------
//PARITY------------------------------------------------------------------------
	if (parity == 0) { //NONE
		tty.c_cflag &= ~PARENB;     /* no parity bit */
	} else if (parity == 1) { //EVEN
		tty.c_cflag |= PARENB;
		tty.c_cflag &= ~PARODD;
	} else if (parity == 2) { //ODD
		tty.c_cflag |= PARENB;
		tty.c_cflag |= PARODD;
	}
//------------------------------------------------------------------------------
//HANDSHAKE---------------------------------------------------------------------
	if (!swcheck) {
		tty.c_iflag |= IXON | IXOFF;
	} else {
		tty.c_iflag &= ~ (IXON | IXOFF | IXANY);
	}
	if (!hwcheck) {
		tty.c_cflag |= CRTSCTS;
	} else {
		tty.c_cflag &= ~CRTSCTS;
	}
//------------------------------------------------------------------------------
	/* setup for non-canonical mode */
	tty.c_iflag &= ~ (IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
	tty.c_lflag &= ~ (ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tty.c_oflag &= ~OPOST;

	/* fetch bytes as they become available */
//Safemode (timout)-------------------------------------------------------------
		if (!smcheck) {
			tty.c_cc[VMIN] = 0;
		} else {
			tty.c_cc[VMIN] = 1;
		}
	tty.c_cc[VTIME] = 255;

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		printf ("Error from tcsetattr: %s\n", strerror (errno));
		return -1;
	}
	return 0;
}

void set_mincount (int fd, int mcount)
{
	struct termios tty;

	if (tcgetattr (fd, &tty) < 0) {
		printf ("Error tcgetattr: %s\n", strerror (errno));
		return;
	}

	tty.c_cc[VMIN] = mcount ? 1 : 0;
	tty.c_cc[VTIME] = 5;        /* half second timer */

	if (tcsetattr (fd, TCSANOW, &tty) < 0)
		printf ("Error tcsetattr: %s\n", strerror (errno));
}

int send_cmd (int fd, char *cmd)
{
	int wlen = 0;
	/* simple output */
	wlen = write (fd, cmd, STRING_SIZE);
	// wlen=0; //test ERROR
	if (wlen != STRING_SIZE) {
		printf ("Error from write: %d, %d\n", wlen, errno);
		return 1;
	}
	tcdrain (fd);   /* delay for output */
	return 0;
}
int get_cmd (int fd, char *buff)
{
	int n = 0;
	char xbuff = '0';
	int i = 0;
	while ( (xbuff != '\n') && (i < STRING_SIZE)) {
		n = read (fd, &xbuff, 1);
		// n=0; //test ERROR
		if (n < 1) {
			printf ("Error from read: %d, %d\n", n, errno);
			return 1;
		}
		buff[i] = xbuff;
		i++;
	}

	tcdrain (fd);   /* delay for output */
	return 0;
}
