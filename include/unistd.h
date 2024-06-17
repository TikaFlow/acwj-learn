//
// Created by tika on 24-6-8.
//

#ifndef ACWJ_LEARN_UNISTD_H
#define ACWJ_LEARN_UNISTD_H

void _exit(int status);
int unlink(char *pathname);

int read(int fd, char *buf, int count);
int write(int fd, void *buf, int count);
int close(int fd);

#endif //ACWJ_LEARN_UNISTD_H
