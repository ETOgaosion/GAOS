#ifndef _FS_H
#define _FS_H

#include <type.h>

#define SECTOR_SIZE 512
#define SECTOR_BYTE_BIOS 9

#define BLOCK_SECTORS 8
#define BLOCK_SECTOR_BIOS 3
#define BYTE_BITS 8
#define BYTE_BIT_BIOS 3

#define BLOCK_SIZE 4096
#define BLOCK_BYTE_BIOS 12

#define MAX_DIR_BLK         16
#define MAX_NAME_LEN        16
#define MAX_FILE_NUM        16
#define MAX_LINK_NUM        16

#define FS_MAGIC    1453
#define FS_SIZE_BLK     (1 << 18)
#define FS_START_BLK    (1 << 9)

//Map in MEM
#define SUBLK_MEM_ADDR      0x5d000000
#define BLK_MAP_MEM_ADDR    0x5d001000
#define INO_MAP_MEM_ADDR    0x5d009000
#define INO_MEM_ADDR        0x5d00a000
#define DATA_MEM_ADDR       0x5d20a000

//Initial Offset in Disk (block)
#define SUBLK_SD_START      0
#define BLK_MAP_SD_START    1
#define INO_MAP_SD_START    9
#define INO_SD_START        10
#define DATA_SD_START       522

#define SUBLK_SIZE          1
#define BLK_MAP_SIZE        8
#define INO_MAP_SIZE        1
#define INO_SIZE            512
#define DATA_BLK_SD_SIZE    FS_SIZE_BLK - DATA_SD_START - FS_START_BLK
#define DATA_BLK_SIZE       DATA_BLK_SD_SIZE >> 3

#define BLK_MAP_BYTE_SIZE   32768

// mode
#define ROOT    0b000001
#define DIR     0b000010
#define FILE    0b000100
#define LINK    0b001000
#define PAR     0b010000
#define CUR     0b100000

// access
#define READ  0b001
#define WRITE 0b010
#define EXE   0b100

//whence
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

// list type
#define DATA 0
#define LIST 1

typedef struct superblock
{
    uint32_t magic;
    uint32_t fs_start;
    uint32_t fs_size;
    uint32_t blk_sectors;

    uint32_t block_map_start;
    uint32_t block_map_size;

    uint32_t inode_map_start;
    uint32_t inode_map_size;

    uint32_t inode_start;
    uint32_t inode_size;

    uint32_t datablock_start;
    uint32_t datablock_size;
    // uint32_t dir_blk_num;
} superblock_t;

typedef struct inode {
    uint32_t ino;
    uint8_t  mode;
    uint8_t  access;
    uint32_t link_num;

    uint32_t start;
    uint32_t size;
    uint32_t used_size;

    uint64_t create_time;
    uint64_t modified_time;

    uint32_t dir_blks[MAX_DIR_BLK];
    uint32_t indir_blks_offset;
    uint32_t dbindir_blks_offset;
    uint32_t tpindir_blks_offset;
} inode_t;

typedef struct data_blk_list
{
    uint8_t  list_type;
    uint32_t blks[MAX_DIR_BLK];
} data_blk_list_t;


typedef struct dir_entry
{
    uint8_t ino;
    uint8_t access;
    uint8_t mode;
    char name[MAX_NAME_LEN];
    char alias[MAX_NAME_LEN];
    uint8_t par_ino[MAX_LINK_NUM];
} dir_entry_t;

typedef struct fd
{
    uint8_t ino;
    uint8_t access;
    uint32_t r_pos;
    uint32_t w_pos;
} fd_t;

extern fd_t file_discriptors[MAX_FILE_NUM];

// fs
int k_fsop(int op,int option);
int k_mkfs(int option);
int k_statfs();

// dir
int k_dirop(int op, char *dirname, int option);
int k_mkdir(char * dirname);
int k_cd(char * dirname);
int k_rmdir(char * dirname);
int k_ls(char * dirname, int option);
int k_pwd(char *dirname);
// inode_t *k_find(inode_t *dp, char *name, int type);

// file
int k_fileop(int op, char *mul_char, int mul_int, int size);
int k_touch(char * filename);
int k_cat(char * filename);
int k_fopen(char * filename, int access);
int k_fread(int fd, char *buff, int size);
int k_fwrite(int fd, char *buff, int size);
int k_fclose(int fd);
int k_frm(char *filename);

//link
int k_linkop(int option, char *src, char *dst);
int k_links(char *src, char *dst);
// int k_linkh(char *src, char *dst);
int k_lseek(int fd, int offset, int whence, int r_or_w);

// mid
dir_entry_t find_dir(uint8_t ino, char * name, int explore_in);
uint8_t find_file_ino(dir_entry_t dir);
int check_fs();
void clear_sector(uintptr_t mem_addr);
void clear_block(uintptr_t mem_addr);
uint32_t alloc_block();
uint32_t free_block(uint32_t blk_bios);
uint32_t alloc_inode();
int free_inode(uint8_t ino);
#endif