#include <lib.h>

int getcwd(char* buf, size_t size) {
	return syscall_read_workdir(buf,size);
}


int chdir(const char* path) {
	int r,i;
	struct Stat st;
	if (path[0] == '/') {
		if ((r = stat(path,&st) < 0)) {
			printf("chdir: can't open the path!\n");			
			return -1;
		}
		if (st.st_isdir == 0) {
			printf("chdir : the file is not a directory!\n");
			return -1;
		}
		return syscall_write_workdir(path,sizeof(path));
	} else {
		char buf[MAXPATHLEN];
		syscall_read_workdir(buf,sizeof(buf));
		int t = strlen(buf);
		if (buf[t - 1] != '/') {
			buf[t++] = '/';
		}
		for (i = 0; i < path[i] != 0; ++i) {
			buf[t + i] = path[i];
		}		
		buf[t + i] = 0;
		if ((r = stat(buf,&st)) < 0) {
			printf("chdir: can't open the path!\n");
		}
		return syscall_write_workdir(buf,sizeof(buf));
	}
	return 0;
}
