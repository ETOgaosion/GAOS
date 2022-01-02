#include <fs.h>
#include <sys/syscall.h>
#include <sys/syscall_number.h>

int mkfs()
{
    return sys_fsop(0);
}

int statfs()
{
    return sys_fsop(1);
}

// dir
int mkdir(char * dirname)
{
    return sys_dirop(0, dirname, 0);
}

int rmdir(char * dirname)
{
    return sys_dirop(1,dirname,0);
}

int cd(char * dirname)
{
    return sys_dirop(2,dirname,0);
}

int ls(char * dirname, int option)
{
    return sys_dirop(3,dirname,option);
}

int pwd(char *dirname)
{
    return sys_dirop(4,dirname,0);
}

// inode_t *find(inode_t *dp, char *name, int type);

// file
int touch(char * filename)
{
    return sys_fileop(0,filename,0,0);
}

int cat(char * filename)
{
    return sys_fileop(1,filename,0,0);
}

int fopen(char * filename, int access){
    return sys_fileop(2,filename,access,0);
}

int fread(int fd, char *buff, int size)
{
    return sys_fileop(3,buff,fd,size);
}

int fwrite(int fd, char *buff, int size)
{
    return sys_fileop(4,buff,fd,size);
}

int fclose(int fd)
{
    return sys_fileop(5,0,fd,0);
}

int frm(char *filename)
{
    return sys_fileop(6,filename,0,0);
}

//link
int ln(int op, char *src, char *dst)
{
    return sys_linkop(op,src,dst);
}

// int linkh(char *src, char *dst);
int lseek(int fd, int offset, int whence, int r_or_w)
{
    return sys_lseek(fd,offset,whence, r_or_w);
}