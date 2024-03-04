#include <lib.h>

void tree(char *path, int tabsize);

void addPrefix(char* prefix,char*name,char*doneStr) {
	int i,j;
	for (i = 0; prefix[i] != 0; ++i) {
		doneStr[i] = prefix[i];
	}
	if (doneStr[i] != '/') {
		doneStr[i++] = '/';
	}
	for (j = 0; name[j] != 0; ++j) {
		doneStr[i + j] = name[j];
	}
	doneStr[i + j] = 0;

}

void tree(char *path, int tabsize) {
	int fd, n;
	struct File f;
	char buf[1024];
	if ((fd = open(path, O_RDONLY)) < 0) {
		user_panic("open %s: %d", path, fd);
	}
	while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
		if (f.f_name[0]) {
			for (int i = 0; i < tabsize * 4; ++i) {
				printf(" ");
			}
			printf("%s\n",f.f_name);

			if (f.f_type == FTYPE_DIR) {
				addPrefix(path,f.f_name,buf);
				tree(buf,tabsize + 1);		
			}
		}
	}
	if (n > 0) {
		user_panic("short read in directory %s", path);
	}
	if (n < 0) {
		user_panic("error reading directory %s: %d", path, n);
	}
}


void usage() {
	printf("wrong format!\n usage: tree [file1] [file2] ...\n");
}


int main(int argc,char** argv) {
	if (argc != 1) {
		usage();
	} else {
		if (argc == 1) {
			char buf[1024];
			syscall_read_workdir(buf,1024);
			tree(buf,0);
		} else {
			for (int i = 1; i < argc; ++i) {
				tree(argv[i],0);
			}
		}
	}

	return 0;
}














