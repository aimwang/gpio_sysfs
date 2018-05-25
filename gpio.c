#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define	MODE_READ 0
#define	MODE_WRITE 1

void syntax(char *command)
{
	printf ("compile time %s\n", __DATE__);
	printf ("%s pin [value]\n", command);
	printf ("\tpin: pin number\n");
	printf ("\tvalue: wrtie value. 0 or 1\n");
	printf ("read example:\n");
	printf ("\t%s 33\n", command);
	printf ("write example:\n");
	printf ("\t%s 33 0\n", command);
}

int gpio_set_direction_out(int pin)
{
	char filename[64], buf[1];
	int fd;

	sprintf(filename, "/sys/class/gpio%d/direction", pin);
	fd = open(filename, O_WRONLY);
	if (fd) {
		write(fd, "out", 3);
		close(fd);
	} else {
		printf("can't open direction file of gpio pin %d\n", pin);
		return -1;
	}
	fd = open(filename, O_RDONLY);
	if (fd) {
		read(fd, buf, 1);
		close(fd);
		if(buf[0] != 'o') {
			return -2;
		}
	} else {
		printf("can't set direction as out for gpio pin %d\n", pin);
		return -1;
	}
	return 0;
}

int gpio_export(int pin)
{
	char gpio[64], pinstr[8];
	int err, fd;
	struct stat s;

	sprintf(gpio, "/sys/class/gpio/gpio%d", pin);
	err = stat(gpio, &s);
	if(-1 == err) {	// not exist
		fd = open("/sys/class/gpio/export", O_WRONLY);
		if (fd) {
			sprintf(pinstr, "%d", pin);
			write(fd, pinstr, strlen(pinstr));
			close(fd);
		}
		err = stat(gpio, &s);
		if(-1 == err) {	// not exist
			printf("can't set as gpio [%d]\n", pin);
			return -1;
		}
	}
}

void gpio_read(int pin)
{
	int fd;
	char filename[64], buf[1];

	sprintf(filename, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(filename, O_RDONLY);
	if (fd) {
		read(fd, buf, 1);
		printf("read gpio%d: %c\n", pin, buf[0]);
		close(fd);
	} else {
		printf("can't read gpio%d\n", pin);
	}
}

void gpio_write(int pin, int value)
{
	int fd;
	char filename[64], buf[1];

	sprintf(filename, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(filename, O_WRONLY);
	if (fd) {
		sprintf(buf, "%d", value);
		write(fd, buf, 1);
		printf("write gpio%d: %c\n", pin, buf[0]);
		close(fd);
	} else {
		printf("can't write gpio%d\n", pin);
	}

}

int main(int argc, char **argv)
{
	int mode, pin, value, ret;

	if (argc == 2) {
		mode = MODE_READ;
	} else if (argc == 3) {
		mode = MODE_WRITE;
	} else {
		syntax(argv[0]);
		return -1;
	}

	// [1] pin
	pin = atoi(argv[1]);
	ret = gpio_export(pin);
	if (ret != 0) {
		return ret;
	}

	if (mode == MODE_READ) {
		gpio_read(pin);
	} else {
		ret = gpio_set_direction_out(pin);
		if(ret) {
			return ret;
		}
		// [2] value
		value = atoi(argv[2]);
		gpio_write(pin, value);
	}
	return 0;
}

