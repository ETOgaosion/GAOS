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
#define FS_SIZE     (1 << 21)
#define FS_START    (1 << 11)

//Map in MEM
#define SUBLK_MEM_ADDR      0x5d000000
#define BLK_MAP_MEM_ADDR    0x5d000200
#define INO_MAP_MEM_ADDR    0x5d004200
#define INO_MEM_ADDR        0x5d004400
#define DATA_MEM_ADDR       0x5d00c400

//Initial Offset in Disk (block)
#define SUBLK_SD_START      0
#define BLK_MAP_SD_START    1
#define INO_MAP_SD_START    33
#define INO_SD_START        34
#define DATA_SD_START       98

#define SUBLK_SIZE          1
#define BLK_MAP_SIZE        64
#define INO_MAP_SIZE        1
#define INO_SIZE            64
#define DATA_BLK_SD_SIZE    FS_SIZE - DATA_SD_START
#define DATA_BLK_SIZE       DATA_BLK_SD_SIZE >> 3

// mode
#define ROOT 0x0001
#define DIR  0x0010
#define FILE 0x0100
#define LINK 0x1000

// access
#define READ  0x001
#define WRITE 0x010
#define EXE   0x100

//whence
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct superblock
{
    uint32_t magic;
    uint32_t fs_start;
    uint32_t fs_size;

    uint32_t block_map_start;
    uint32_t block_map_size;

    uint32_t inode_map_start;
    uint32_t inode_map_size;

    uint32_t inode_start;
    uint32_t inode_size;

    uint32_t datablock_start;
    uint32_t datablock_size;
} superblock_t;

typedef struct inode {
    uint32_t ino;
    uint8_t  mode;
    uint8_t  access;
    uint32_t link_num;

    uint32_t start;
    uint32_t size;
    uint32_t used_size;

    uint64_t creadte_time;
    uint64_t modified_time;

    uint32_t dir_blks[MAX_DIR_BLK];
    uint32_t indir_blks_offset;
    uint32_t dbindir_blks_offset;
    uint32_t tpindir_blks_offset;
} inode_t;

typedef struct data_blk_list
{
    uint32_t dir_blks[MAX_DIR_BLK];
    uint32_t indir_blks_offset;
    uint32_t dbindir_blks_offset;
} data_blk_list_t;


typedef struct data_entry
{
    uint8_t ino;
    uint8_t access;
    uint8_t mode;
    char name[MAX_NAME_LEN];
    char alias[MAX_NAME_LEN];
    uint8_t par_ino[MAX_LINK_NUM];
} data_entry_t;

typedef struct fd
{
    uint8_t ino;
    uint8_t access;
    uint32_t r_pos;
    uint32_t w_pos;
} fd_t;

extern fd_t file_discriptor[MAX_FILE_NUM];

// fs
int k_mkfs();
int k_statfs();
// dir
int k_cd(char * dirname);
int k_mkdir(char * dirname);
int k_rmdir(char * dirname);
int k_ls(char * dirname, int option);
inode_t *k_find(inode_t *dp, char *name, int type);

// file
int k_touch(char * filename);
uint64_t k_cat(char * filename);
int k_fopen(char * filename, int access);
int k_fread(int fd, char *buff, int size);
int k_fwrite(int fd, char *buff, int size);
int k_fclose(int fd);
int k_frm(char *name);
//link
int k_links(char *src, char *dst);
int k_linkh(char *src, char *dst);
int k_lseek(int fd, int offset, int whencce);

// mid
data_entry_t find_dir(uint8_t ino, char * name);
int check_fs();
void clear_sector(uintptr_t mem_addr);
void clear_block(uintptr_t mem_addr);
void set_block_map(uint32_t block_id);
void set_inode_map(uint32_t inode_id);
void clear_block_map(uint32_t block_id);
void clear_inode_map(uint32_t inode_id);
#endif