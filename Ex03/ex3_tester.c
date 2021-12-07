#include "message_slot.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <sys/ioctl.h>

static char* DEV0 = "/dev/test0";
static char* DEV1 = "/dev/test1";

void test1();
void test2();
void test3();
void test4();
void test5();
void test6();
void test7();
void test8();
void test9();
void test10();
void test11();
void test12();
void test13();
void test14();
void print_failure(int test_num);
void print_success(int test_num);

int main(void)
{
	printf("RESULTS\n-------\n");
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();
	test8();
	test9();
	test10();
	test11();
	test12();
	test13();
	test14();

	printf("DONE!\n");

	return 0;
}

void test1()
{
	int device0_fd;
	size_t bytes_written;
	size_t bytes_read;
	char msg[128];

	device0_fd = open(DEV0, O_RDWR);
	if (device0_fd < 0)
	{ print_failure(1); exit(0); }

	if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 6) < 0)
	{ print_failure(1); exit(0); }

	bytes_written = write(device0_fd, "Hello World!", 12);
	if (bytes_written < 12)
	{ print_failure(1); exit(0); }

	bytes_read = read(device0_fd, msg, 128);
	if (bytes_read < 12)
	{ print_failure(1); exit(0); }

	msg[12] = '\0';
	if (strcmp(msg, "Hello World!"))
	{ print_failure(1); exit(0); }

	print_success(1);
}

void test2()
{
        int device0_fd;
	int device1_fd;
        size_t bytes_written;
        size_t bytes_read;
        char msg[128];

        device0_fd = open(DEV0, O_RDWR);
        if (device0_fd < 0)
        { print_failure(2); exit(0); }

        device1_fd = open(DEV1, O_RDWR);
        if (device1_fd < 0)
        { print_failure(2); exit(0); }

        if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 99) < 0)
        { print_failure(2); exit(0); }

        if (ioctl(device1_fd, MSG_SLOT_CHANNEL, 99) < 0)
        { print_failure(2); exit(0); }

        bytes_written = write(device0_fd, "dev0", 4);
        if (bytes_written < 4)
        { print_failure(2); exit(0); }

        bytes_written = write(device1_fd, "dev1", 4);
        if (bytes_written < 4)
        { print_failure(2); exit(0); }

        bytes_read = read(device0_fd, msg, 128);
        if (bytes_read < 4)
        { print_failure(2); exit(0); }

        msg[4] = '\0';
        if (strcmp(msg, "dev0"))
        { print_failure(2); exit(0); }

        bytes_read = read(device1_fd, msg, 128);
        if (bytes_read < 4)
        { print_failure(2); exit(0); }

        msg[4] = '\0';
        if (strcmp(msg, "dev1"))
        { print_failure(2); exit(0); }

	close(device0_fd);
	close(device1_fd);

        print_success(2);

}

void test3()
{
        int device0_fd;
        size_t bytes_read;
        char msg[128];

        device0_fd = open(DEV0, O_RDWR);
        if (device0_fd < 0)
        { print_failure(3); exit(0); }

        if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 6) < 0)
        { print_failure(3); exit(0); }

        bytes_read = read(device0_fd, msg, 128);
        if (bytes_read < 12)
        { print_failure(3); exit(0); }

	msg[12] = '\0';
	if (strcmp(msg, "Hello World!"))
	{ print_failure(3); exit(0); }

	close(device0_fd);

	print_success(3);
}

void test4()
{
	int device0_fd;
	int device1_fd;

        device0_fd = open(DEV0, O_RDWR);
        if (device0_fd < 0)
        { print_failure(4); exit(0); }

        device1_fd = open(DEV1, O_RDWR);
        if (device1_fd < 0)
        { print_failure(4); exit(0); }

        if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 6) < 0)
        { print_failure(4); exit(0); }

	if (write(device1_fd, "hey", 3) >= 0)
	{ print_failure(4); exit(0); }

        close(device0_fd);
	close(device1_fd);

        print_success(4);
}

void test5()
{
        int device0_fd;
        int device1_fd;

        device0_fd = open(DEV0, O_RDWR);
        if (device0_fd < 0)
        { print_failure(5); exit(0); }

        if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 6) < 0)
        { print_failure(5); exit(0); }

	device1_fd = open(DEV1, O_RDWR);
	if (device1_fd < 0)
	{ print_failure(5); exit(0); }

        if (write(device1_fd, "hey", 3) >= 0)
        { print_failure(5); exit(0); }

        close(device0_fd);
        close(device1_fd);

	print_success(5);
}

void test6()
{
	int device0_fd;
	char msg[128];
	int bytes_written;
	int bytes_read;

	device0_fd = open(DEV0, O_RDWR);
	if (device0_fd < 0)
	{ print_failure(6); exit(0); }

        if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 1024) < 0)
        { print_failure(6); exit(0); }

	bytes_written = write(device0_fd, "old", 3);
	if (bytes_written < 3)
	{ print_failure(6); exit(0); }

	bytes_written = write(device0_fd, "new", 3);
	if (bytes_written < 3)
	{ print_failure(6); exit(0); }

	bytes_read = read(device0_fd, msg, 128);
	if (bytes_read < 3)
	{ print_failure(6); exit(0); }

	msg[3] = '\0';
	if (strcmp(msg, "new"))
	{ print_failure(6); exit(0); }

	bytes_read = read(device0_fd, msg, 128);
	if (bytes_read < 3)
	{ print_failure(6); exit(0); }

	msg[3] = '\0';
	if (strcmp(msg, "new"))
	{ print_failure(6); exit(0); }

	close(device0_fd);

	print_success(6);
}

struct junk{
int var1;
char var2;
char *var3;
};

void test7()
{
	struct junk *junk;
	int device0_fd;
	int bytes_written;
	int bytes_read;

	junk = (struct junk *) malloc(sizeof(struct junk));
	if (junk == NULL)
	{ print_failure(7); exit(0); }

	device0_fd = open(DEV0, O_RDWR);
	if (device0_fd < 0)
	{ print_failure(7); exit(0); }

	if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 161616) < 0)
	{ print_failure(7); exit(0); }

	bytes_written = write(device0_fd, junk, sizeof(struct junk));
	if (bytes_written < sizeof(struct junk))
	{ print_failure(7); exit(0); }

	bytes_read = read(device0_fd, junk, sizeof(struct junk));
	if (bytes_read < sizeof(struct junk))
	{ print_failure(7); exit(0); }

	close(device0_fd);

	print_success(7);
}

void test8()
{
        int device0_fd;
	char msg[128];

        device0_fd = open(DEV0, O_RDWR);
        if (device0_fd < 0)
        { print_failure(8); exit(0); }

        if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 199) < 0)
        { print_failure(8); exit(0); }

	if (read(device0_fd, msg, 128) != -1 || errno != EWOULDBLOCK)
	{ print_failure(8); exit(0); }

        close(device0_fd);

        print_success(8);
}

void test9()
{
	int device0_fd;
	size_t bytes_written;

	device0_fd = open(DEV0, O_RDWR);
	if (device0_fd < 0)
	{ print_failure(9); exit(0); }

	if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 54) < 0)
	{ print_failure(9); exit(0); }

	bytes_written = write(device0_fd, "", 0);
	if (bytes_written != -1 || errno != EMSGSIZE)
	{ print_failure(9); exit(0); }

	close(device0_fd);

	print_success(9);
}

void test10()
{
        int device0_fd;
        size_t bytes_written;
	char *msg = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

        device0_fd = open(DEV0, O_RDWR);
        if (device0_fd < 0)
        { print_failure(10); exit(0); }

        if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 2049) < 0)
        { print_failure(10); exit(0); }

        bytes_written = write(device0_fd, msg, 129);
        if (bytes_written != -1 || errno != EMSGSIZE)
        { print_failure(10); exit(0); }

        close(device0_fd);

        print_success(10);
}

void test11()
{
	int device0_fd;
	size_t bytes_written;

	device0_fd = open(DEV0, O_RDWR);
	if (device0_fd < 0)
	{ print_failure(11); exit(0); }

	bytes_written = write(device0_fd, "abcd", 4);
	if (bytes_written != -1 || errno != EINVAL)
	{ print_failure(11); exit(0); }

	close(device0_fd);

	print_success(11);
}

void test12()
{
        int device0_fd;
        size_t bytes_read;

        device0_fd = open(DEV0, O_RDWR);
        if (device0_fd < 0)
        { print_failure(12); exit(0); }

        bytes_read = read(device0_fd, "abcd", 4);
        if (bytes_read != -1 || errno != EINVAL)
        { print_failure(12); exit(0); }

        close(device0_fd);

        print_success(12);
}

void test13()
{
	int device0_fd;
	char msg[4];
	int bytes_written;
	int bytes_read;

	device0_fd = open(DEV0, O_RDWR);
	if (device0_fd < 0)
	{ print_failure(13); exit(0); }

	if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 999) < 0)
	{ print_failure(13); exit(0); }

	bytes_written = write(device0_fd, "testtest", 8);
	if (bytes_written < 8)
	{ print_failure(13); exit(0); }

	bytes_read = read(device0_fd, msg, 4);
	if (bytes_read != -1 || errno != ENOSPC)
	{ print_failure(13); exit(0); }

	close(device0_fd);

	print_success(13);
}

void test14()
{
	int device0_fd;

	device0_fd = open(DEV0, O_RDWR);
	if (device0_fd < 0)
	{ print_failure(14); exit(0); }

	if (ioctl(device0_fd, MSG_SLOT_CHANNEL, 1048578) < 0)
	{ print_failure(14); exit(0); }

	close(device0_fd);

	print_success(14);
}

void print_success(int test_num)
{
	printf("TEST %d: Success\n", test_num);
}

void print_failure(int test_num)
{
	printf("TEST %d: Failure\n", test_num);
}
