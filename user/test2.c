#include <lib.h>

int main() {
	char c;
	while ((c = syscall_cgetc()) != '\n') {
		debugf(";;%d;;\n",c);
	}
	return 0;
}
