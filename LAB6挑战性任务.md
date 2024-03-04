```
title: BUAAOS LAB6挑战性任务
author: lonelywatch
date: 2023-5-29 00:14 +0800
categories: [BUAA,OS]
tags: [BUAA,OS,shell]
```

## LAB6挑战性任务

### 退格删除

删除主要是通过修改sh.c中readline函数,并且需要删除用户的console.c中输出\b的部分，输出`"\b \b"`  来实现，输入字符如果需要退格并且已经成功输入过字符，则删除，并删去一个输入的字符，如果是回车或者终止字符，则终止输入，其他的字符则正常输入。

```c
void readline(...) {
...
	for (int i = 0; i < n; ) {
		...
		if (buf[i] == '\b' || buf[i] == 0x7f) {
			if (i > 0) {
				printf("\b \b");
				i -= 1;
			}
		} else if (buf[i] == '\r' || buf[i] == '\n') {
			buf[i] = 0;
			return;
		} else {
			i += 1;
		}
	}
}
```

### 实现一行多命令

运行命令由sh.c实现，sh.c通过readline读取一行命令，然后将读取的字符串按照 ； 进行划分，然后一次执行字符串代表的命令。

```c
//sh.c :main
int main(...) {
    ...
    int pos = 0,prepos = 0;
    for (;;) {
        ...
        pos = prepos = 0;
        do {
            prepos = pos;
            for (pos; buf[pos] != 0; ++pos) {
                if (buf[pos] == ';') {
                    buf[pos++] = 0;
                    break;
                }
            }
            ...
            runcmd(buf + prepos); 
        } while (buf[pos] != 0);
    }
}

```

### 省略后缀 .b

这里做的比较简陋，spwan负责打开输入命令中的程序，如果检测程序名中不含有. 后缀，则尝试添加后缀打开。 //错误，直接修改会覆盖后面的数据，这是指针。

```c
int spawn(...) {
    int t = strlen(prog);
    if (strchr(prog,'.') == NULL) {
        prog[t] = '.';
        prog[t + 1] = 'b';
        prog[t + 2] = 0;
    }
    ...
}
```

### 实现引号支持

只需要在解析的时候添加对引号的解析就可以了

### 创建文件与目录

在fs.c中已经有创建文件的函数，在打开时如果带有O_CREAT则尝试创建文件，目录与文件的不同为type的不同，如果时O_MKDIR则将type设置为FTYPE_DIR.

### history历史命令

通过创建.history文件来记录命令，在每次解析后打开该文件并输入，同时需要打开.historypos文件，记录该命令在.history的索引，history命令将.history中的全部内容输出，在输入命令时会打开两个文件，输入上下键，通过.historypos寻找命令，按下回车键，将该命令输入。

### &后台命令

运行该指令不要wait，直接运行。

### 相对路径

 最简单的方法是在env结构体中添加工作路径的属性，然后在shell操作中维护该工作路径。

