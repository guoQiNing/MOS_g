#include <lib.h>

const char file1[100] = "/motd";
const char buf[100] = "I am so stupid to do so in my exam time.!\n";
int main() {
/*	int r = 0;
	char buf1[100];
	int fd = open(file1,O_CREAT | O_RDWR);
	if ((r = write(fd,buf,100)) < 0) {
		user_panic("wrong with write\n");
	}
	close(fd);
	fd = open(file1,O_RDONLY);
	if ((r = read(fd,buf1,100)) < 0) {
		user_panic("wrong with read\n");
	}
	buf1[r] = 0;
	if (strcmp(buf,buf1) != 0) {
		user_panic("content is wrong\n");
	}
	debugf("the read line is ;%s;\n",buf1);
*/
	int r;
	struct Stat st;
	int fd = open("/direct1",O_MKDIR);
	if (fd < 0) {
		user_panic("failed!11\n");
	}
	close(fd);
	fd = open("/direct1/direct2/",O_MKDIR);
	if (fd < 0) {
		user_panic("failed!22\n");
	}
	close(fd);
	if ((r = stat("/direct1/direct2/",&st) < 0)) {
		user_panic("cc\n");
	}
	printf(";%s;\n",st.st_name);
	//printf("long long is '%d'\n",sizeof(long long));
/*	char tt[5];
	tt[4] = 0;
	tt[0] = '0';
	tt[1] = '1';
	tt[2] = '2';
	tt[3] = '\b';
	debugf(";%s;\n",tt);*/
	return 0;

}

