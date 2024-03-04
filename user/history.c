#include <lib.h>

char buf[8192];

void history() {
	int history_fd,r;
	if ((history_fd = open(HISTORY_CMD_FILE,O_RDONLY)) < 0) {
		user_panic("history can't open the log file!,err is %3d\n",history_fd);
	}
	printf("------------------history command --------------\n");
	while ((r = read(history_fd,buf,sizeof(buf))) > 0 ) {
		write(0,buf,sizeof(buf));
	}
	printf("---------------------------------------------------\n");
	//printf("----------history command-------\n%s\n---------------------\n",buf);
	if ((r = close(history_fd)) < 0) {
		user_panic("history can't close the log file!,err is %3d\n",r);
	}
}

void usage() {
	printf("format wrong!\n usage: history\n");
}

int main(int argc, char** argv) {
	if (argc != 1) {
		usage();	
	} else {
		history();
	}
	return 0;
}
