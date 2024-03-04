#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

int historyIndex[128 + 1];
char historyCmd[8192];
static int doBuiltInCmd(int argc, char**argv); 

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
int _gettoken(char *s, char **p1, char **p2) {
	*p1 = 0;
	*p2 = 0;
	if (s == 0) {
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;
	}
	if (*s == '\"') {
		++s;
		*p1 = s;
		while(*s != '\"' && *s != 0) {
			++s;
		}
		if (*s != '\"') {
			user_panic("\" format wrong,need another \"!\n");
		} 
		*s = 0;
		*p2 = ++s;
		return 'w';	
	}

	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}

	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s)) {
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1) {
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe) {
	int argc = 0;
	while (1) {
		char *t;
		int fd;
		int c = gettoken(0, &t);
		switch (c) {
			case 0:
				return argc;
			case 'w':
				if (argc >= MAXARGS) {
					debugf("too many arguments\n");
					exit();
				}
				argv[argc++] = t;
				break;
				/*case '&':
				  argv[argc++] = argv[0];
				  argv[0] = t;
				  break;*/
			case '&':
				argv[argc] = 0;
				int child = spawn(argv[0],argv);
				//close_all();
				if (child < 0) {
					user_panic("& can;t spwan!\n");
				}
				argc = 0;
				break;
			case '<':
				if (gettoken(0, &t) != 'w') {
					debugf("syntax error: < not followed by word\n");
					exit();
				}
				// Open 't' for reading, dup it onto fd 0, and then close the original fd.
				/* Exercise 6.5: Your code here. (1/3) */
				fd = open(t,O_RDONLY);
				if (fd < 0) {
					user_panic("file can't be opened!\n");
				}
				dup(fd,0);
				close(fd);
				//user_panic("< redirection not implemented");
				break;
			case '>':
				if (gettoken(0, &t) != 'w') {
					debugf("syntax error: > not followed by word\n");
					exit();
				}
				// Open 't' for writing, dup it onto fd 1, and then close the original fd.
				/* Exercise 6.5: Your code here. (2/3) */
				fd = open(t,O_WRONLY);
				if (fd < 0) {
					//	user_panic("file can't be opened!\n");
					fd = open(t,O_RDWR | O_CREAT);
					if (fd < 0) {	
						user_panic("file can't be opened!\n");
					}
				}
				//user_panic("> redirection not implemented");
				dup(fd,1);
				close(fd);
				break;
			case '|':;
					 /*
					  * First, allocate a pipe.
					  * Then fork, set '*rightpipe' to the returned child envid or zero.
					  * The child runs the right side of the pipe:
					  * - dup the read end of the pipe onto 0
					  * - close the read end of the pipe
					  * - close the write end of the pipe
					  * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
					  *   command line.
					  * The parent runs the left side of the pipe:
					  * - dup the write end of the pipe onto 1
					  * - close the write end of the pipe
					  * - close the read end of the pipe
					  * - and 'return argc', to execute the left of the pipeline.
					  */
					 int p[2];
					 /* Exercise 6.5: Your code here. (3/3) */
					 pipe(p);
					 if ((*rightpipe = fork()) == 0) {
						 dup(p[0],0);
						 close(p[0]);
						 close(p[1]);
						 return parsecmd(argv,rightpipe);	
					 } else {
						 dup(p[1],1);
						 close(p[1]);
						 close(p[0]);
						 return argc;
					 }
					 //user_panic("| not implemented");
					 break;
		}
	}

	return argc;
}


int runcmd(char *s) {
	gettoken(s, 0);

	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
	if (argc == 0) {
		return 1;
	}
	argv[argc] = 0;
	if (doBuiltInCmd(argc,argv) != 0) {
		return 1;
		//exit();
	} 
	int child = spawn(argv[0], argv);
	close_all();

	if (child >= 0) {
		wait(child);

	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	/*if (strcmp(argv[0],"history") == 0 && (history_fd = open(HISTORY_CMD_FILE,O_WRONLY)) < 0 ) {
	  user_panic("close .history file failed!,err : %3d\n",history_fd);
	  }*/
	exit();
}

static void doCd(int argc,char** argv) {
	if (argc != 2) {
		user_panic("cd usage : cd path\n");
	}
	chdir(argv[1]);
}

static void doPwd(int argc,char** argv) {
	char buf[128];
	if (argc != 1) {
		user_panic("pwd usage : pwd\n");
	}
	getcwd(buf,sizeof(buf));
	printf("%s\n",buf);
}

static int doBuiltInCmd(int argc, char**argv) {
	if (strcmp(argv[0],"cd") == 0) {
		doCd(argc,argv);
		return 1;
	} else if (strcmp(argv[0],"pwd") == 0) {
		doPwd(argc,argv);	
		return 1;
	}
	return 0;
}


//temporary static
static void moveCursor(int c) {
	printf("%c%c%c",27,91,c);
}

static void deleteCharFromCmd(char* buf,int *curIndex,int *allIndex) {
	if (*curIndex > 0) {
		for (int i = *curIndex - 1; i < *allIndex - 1 ;++i) {
			buf[i] = buf[i + 1];
		}
		*allIndex -= 1;
		*curIndex -= 1;
		printf("\b");
		write(0,buf + *curIndex,*allIndex - *curIndex);
		printf(" ");
		for (int i = *curIndex; i < *allIndex + 1; ++i) {
			printf("\b");
		}
	}
}

static void AddCharToCmd(char* buf,char ch,int *curIndex,int *allIndex) {
	for (int i = *allIndex; i > *curIndex; --i) {
		buf[i] = buf[i - 1];
	}
	buf[*curIndex] = ch;
	*curIndex += 1;
	*allIndex += 1;
	write(0,buf + *curIndex,*allIndex - *curIndex);
	for (int i = *curIndex; i < *allIndex; ++i) {
		printf("\b");
	}
}

static void getPrevCmd(char*buf,int* curCmdCnt,int* curIndex, int*allIndex) {
	if (*curCmdCnt == 0) {
		return;
	}
	if (historyIndex[2 * historyIndex[0] + 2] != *allIndex && *curCmdCnt == historyIndex[0]) {		
		int formerLength = historyIndex[2 * historyIndex[0] - 1] + historyIndex[2 * historyIndex[0]];
		historyIndex[2 * historyIndex[0] + 1] = formerLength;
		for (int i = 0; i < *allIndex; ++i) {
			historyCmd[formerLength + i] = buf[i];
		}
	}
	for (int i = 0; i < *curIndex; ++i) {
		printf("\b");
	}
	for (int i = 0; i < *allIndex; ++i) {
		printf(" ");
	}
	for (int i = 0; i < *allIndex; ++i) {
		printf("\b");
	}
	*curCmdCnt -= 1;
	for (int i = 0; i < historyIndex[2 * (*curCmdCnt)]; ++i) {
		buf[i] = historyCmd[historyIndex[2 * (*curCmdCnt) - 1] + i];
	}

	write(0,historyCmd + historyIndex[2 * (*curCmdCnt) - 1],historyIndex[2 * (*curCmdCnt)]);
	*curIndex = *allIndex = historyIndex[2 * (*curCmdCnt)];
}

static void getNextCmd(char*buf,int* curCmdCnt,int* curIndex,int*allIndex) {
	if (curCmdCnt == historyIndex[0]) {
		return;
	}
	for (int i = 0; i < *curIndex; ++i){
		printf("\b");
	}
	for (int i = 0; i < *allIndex; ++i) {
		printf(" ");
	}
	for (int i = 0; i < *allIndex; ++i) {
		printf("\b");
	}
	*curCmdCnt += 1;
	for (int i = 0; i < historyIndex[2 * (*curCmdCnt)]; ++i) {
		buf[i] = historyCmd[historyIndex[2 * (*curCmdCnt) - 1] + i];
	}

	write(0,historyCmd + historyIndex[2 * (*curCmdCnt) - 1],historyIndex[2 * (*curCmdCnt)]);
	*curIndex = *allIndex = historyIndex[2 * (*curCmdCnt)];
}

static void doWithDirection(char * buf,int*curCmdCnt,int* curIndex,int* allIndex) {
	switch(*buf) {
		case CURSOR_FORMER_RIGHT:
			if (*curIndex < *allIndex) {
				moveCursor(CURSOR_RIGHT);
				*curIndex += 1;
			}
			break;
		case CURSOR_FORMER_LEFT:
			if (*curIndex > 0) {
				moveCursor(CURSOR_LEFT);
				*curIndex -= 1;
			}
			break;
		case CURSOR_FORMER_UP:
			getPrevCmd(buf,curCmdCnt,curIndex,allIndex);
			break;
		case CURSOR_FORMER_DOWN:
			getNextCmd(buf,curCmdCnt,curIndex,allIndex);
			break;
defualt:
			break;
	}

}

static void storeHistoryCmd(char* buf) {
	int r,history_fd,index_fd;
	if ((history_fd = open(HISTORY_CMD_FILE,O_RDWR)) < 0) {
		user_panic("can't create .history file!\n");
	}
	if ((index_fd = open(HISTORY_CMD_INDEXFILE,O_RDWR)) < 0) {
		user_panic("create .hisIndex file failed!\n");
	}
	int historyCnt = 0;
	if ((r = read(index_fd,&historyCnt,sizeof(int)) != sizeof(int))) {
		user_panic("can't read the history count!\n");		
	}
	int t[2];
	if (historyCnt == 0) {
		t[0] = 0;
		t[1] = strlen(buf);
		historyCnt += 1;
		seek(index_fd,0);
		write(index_fd,&historyCnt,sizeof(int));	
		write(index_fd,t,sizeof(int) * 2);
		write(history_fd,buf,strlen(buf));
	} else {
		seek (index_fd,(2 * (historyCnt - 1) + 1) * sizeof(int));
		if ((r = read(index_fd,t,sizeof(int) * 2) != sizeof(int) * 2)) {
			user_panic("can't read the history last index!\n");		
		}
		//seek(index_fd,2 * historyCnt * sizeof(int));
		t[0] += t[1];
		//printf(";;%d;;%d\n",historyCnt,t[0]);
		t[1] = strlen(buf);
		seek(history_fd,t[0]);
		write(history_fd,buf,strlen(buf));
		write(index_fd,t,sizeof(int) * 2);
		seek(index_fd,0);
		historyCnt += 1;
		write(index_fd,&historyCnt,sizeof(int));
	}
	close(history_fd);
	close(index_fd);
}

static void createHistoryAndIndex() {
	struct Stat st;
	int r,history_fd,index_fd;
	if ((r = stat(HISTORY_CMD_FILE, &st)) < 0) {
		printf("creating history and index\n");
		if ((history_fd = open(HISTORY_CMD_FILE,O_CREAT | O_RDWR)) < 0) {
			user_panic("can't create .history file!\n");
		}
		if ((index_fd = open(HISTORY_CMD_INDEXFILE,O_CREAT | O_RDWR)) < 0) {
			user_panic("create .hisIndex file failed!\n");
		}
		int t = 0;
		write(index_fd,&t,sizeof(int));
		close(index_fd);
		close(history_fd);
	}
}

static void readHistory() {
	int history_fd, index_fd,r,historyCnt,historyLength;
	if ((history_fd = open(HISTORY_CMD_FILE,O_RDONLY)) < 0) {
		user_panic("can't open history file!\n");
	}
	if ((index_fd = open(HISTORY_CMD_INDEXFILE,O_RDONLY)) < 0) {
		user_panic("can't open history index!\n");
	}
	read(index_fd,&historyCnt,sizeof(int));
	historyIndex[0] = historyCnt;
	read(index_fd,historyIndex + 1,2 * historyCnt * sizeof(int));
	historyLength = historyIndex[2 * historyCnt - 1] + historyIndex[2 * historyCnt];
	if ((r = read(history_fd,historyCmd,historyLength)) != historyLength) {
		user_panic("read historyCmd failed!\n");
	}
	close(history_fd);
	close(index_fd);
}

void readline(char *buf, u_int n) {
	int r,curIndex = 0,allIndex = 0,curCmdCnt = 0;
	char ch;
	readHistory();
	curCmdCnt = historyIndex[0];
	for (allIndex = 0; allIndex < n;) {
		if ((r = read(0,&ch,1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n",r);
			}
			exit();
		}
		if (ch < 0) {
			doWithDirection(&ch,&curCmdCnt,&curIndex,&allIndex);
		} else {
			if (ch == '\b' || ch == 0x7f) {
				deleteCharFromCmd(buf,&curIndex,&allIndex);
			}
			else if (ch  == '\r' || ch== '\n') {
				buf[allIndex] = 0;
				storeHistoryCmd(buf);
				return;
			} else {
				AddCharToCmd(buf,ch,&curIndex,&allIndex);
				//buf[allIndex] = ch;
				//allIndex += 1;
				//curIndex += 1;
			}		
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}

char buf[1024];

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
	int echocmds = 0,prepos = 0,pos = 0;
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                     MOS Shell 2023                      ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN {
		case 'i':
			interactive = 1;
			break;
		case 'x':
			echocmds = 1;
			break;
		default:
			usage();
	}
	ARGEND

		if (argc > 1) {
			usage();
		}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}
	createHistoryAndIndex();
	for (;;) {
		if (interactive) {
			printf("\n$ ");
		}
		readline(buf, sizeof buf);
		pos = prepos = 0;
		do {
			prepos = pos;
			for (pos; buf[pos] != 0; ++pos) {
				if (buf[pos] == ';') {
					buf[pos++] = 0;
					break;
				}
			}
			if (buf[0] == '#') {
				continue;
			}
			if (echocmds) {
				printf("# %s\n", buf);
			}
			if ((r = fork()) < 0) {
				user_panic("fork: %d", r);
			}
			if (r == 0) {
				if (runcmd(buf + prepos) == 0) {
					exit();
				}
			} else {
				wait(r);
			}
		} while (buf[pos] != 0);
	}
	return 0;
}
