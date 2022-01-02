#ifndef _FS_H_
#define _FS_H_

int mkfs();
int statfs();

// dir
int mkdir(char * dirname);
int cd(char * dirname);
int rmdir(char * dirname);
int ls(char * dirname, int option);
int pwd(char *dirname);
// inode_t *find(inode_t *dp, char *name, int type);

// file
int touch(char * filename);
int cat(char * filename);
int fopen(char * filename, int access);
int fread(int fd, char *buff, int size);
int fwrite(int fd, char *buff, int size);
int fclose(int fd);
int frm(char *filename);

//link
int ln(int op, char *src, char *dst);
// int linkh(char *src, char *dst);
int lseek(int fd, int offset, int whence, int r_or_w);

#endif