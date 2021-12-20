#include <os/list.h>
#include <os/mm.h>
#include <os/sched.h>
#include <pgtable.h>
#include <os/string.h>
#include <assert.h>
#include <os/smp.h>
#include <os/string.h>
#include <os/stdio.h>
#include <os/fs.h>

uint8_t current_ino = 0;
inode_t *current_inode;

int k_mkfs(){
    // read superblock from disk
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);

    // judge if the fs exists. 
    if(superblock->magic == FS_MAGIC){
        prints("The FS has existed!\n");
        prints("magic : 0x%x                                      \n",superblock->magic);
        prints("start sector : %d, num sector : %d                \n",superblock->fs_start,superblock->fs_size);
        prints("block map: start at %d (size %d)                  \n",superblock->block_map_start,superblock->block_map_size);
        prints("inode map: start at %d (size %d)                  \n",superblock->inode_map_start,superblock->inode_map_size);
        prints("inode map: start at %d (size %d)                  \n",superblock->inode_start,superblock->inode_size);
        prints("data offset : %d (%d)                             \n",superblock->datablock_start,superblock->datablock_size);
        return 1;
    }else{
        prints("[FS] Start initialize filesystem!\n");

        // init superblock
        prints("[FS] Setting superblock...\n");
        clear_block((uintptr_t)superblock);
        superblock->magic = FS_MAGIC;
        superblock->fs_start = FS_START;
        superblock->fs_size = FS_SIZE;
        superblock->block_map_start = BLK_MAP_SD_START;
        superblock->block_map_size = BLK_MAP_SIZE;
        superblock->inode_map_start = INO_MAP_SD_START;
        superblock->inode_map_size = INO_MAP_SIZE;
        superblock->inode_start = INO_SD_START;
        superblock->inode_size = INO_SIZE;
        superblock->datablock_start = DATA_SD_START;
        superblock->datablock_size = DATA_BLK_SIZE;

        prints("magic : 0x%x                                      \n",superblock->magic);
        prints("start sector : %d, num sector : %d                \n",superblock->fs_start,superblock->fs_size);
        prints("block map: start at %d (size %d)                  \n",superblock->block_map_start,superblock->block_map_size);
        prints("inode map: start at %d (size %d)                  \n",superblock->inode_map_start,superblock->inode_map_size);
        prints("inode map: start at %d (size %d)                  \n",superblock->inode_start,superblock->inode_size);
        prints("data offset : %d (%d)                             \n",superblock->datablock_start,superblock->datablock_size);

        // init block map
        prints("[FS] Setting block-map...\n");
        uint8_t *blockmap = (uint8_t *)pa2kva(BLK_MAP_MEM_ADDR);
        int i;
        for(i = 0; i < BLK_MAP_SIZE; i++)
            clear_block((uintptr_t)blockmap + SECTOR_SIZE * i);
        for(i = 0; i < DATA_SD_START; i++){
            blockmap[i/8] = blockmap[i/8] | (1 << (i % 8));
        }

        // init inode map
        prints("[FS] Setting inode-map...\n");
        uint8_t *inodemap = (uint8_t *)pa2kva(INO_MAP_MEM_ADDR);
        clear_block((uintptr_t)inodemap);
        
        // the first inode has been used as root-dir
        inodemap[0] = 1;

        // init root-dir inode
        prints("[FS] Setting inode...\n");
        inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR);
        clear_block((uintptr_t)inode);
        inode[0].ino = 0;
        inode[0].mode = ROOT;
        inode[0].access = READ | WRITE;
        inode[0].link_num = 0;
        inode[0].size = 1;
        inode[0].used_size = 1;
        inode[0].creadte_time = get_timer();
        inode[0].modified_time = get_timer();
        inode[0].dir_blks[0] = DATA_SD_START;
        for(i = 1; i < MAX_DIR_BLK; i++)
            inode[0].dir_blks[i] = 0;
        inode[0].indir_blks_offset = 0;
        inode[0].dbindir_blks_offset = 0;
        inode[0].tpindir_blks_offset = 0;
        

        init_data_entry(inode[0].dir_blks[0], 0, 0);
        
        //uintptr_t sbi_sd_write(unsigned int mem_address, unsigned int num_of_blocks(512B), unsigned int block_id)
        prints("[FS] Initialize filesystem finished!\n");
        screen_reflush();
        current_inode = inode;
    }
}

/* print metadata info of fs */
int k_statfs(){
    // read superblock
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);

    // print info
    if(superblock->magic != FS_MAGIC){
        prints("[ERROR] No File System!\n");
        return -1;
    }
    int i, j;
    uint32_t used_block = 0, used_inodes = 0;
    status = sbi_sd_blk_read(BLK_MAP_MEM_ADDR, BLK_MAP_SIZE, FS_START + BLK_MAP_SD_START);
    uint8_t *blockmap = (uint8_t *)pa2kva(BLK_MAP_MEM_ADDR);
    for(i = 0; i < (superblock->block_map_size << BLOCK_BYTE_BIOS); i++)
        for(j = 0; j < 8; j++)
            used_block += (blockmap[i] >> j) & 1;
    status = sbi_sd_blk_read(INO_MAP_MEM_ADDR, INO_MAP_SIZE, FS_START+INO_MAP_SD_START);
    uint8_t *inodemap = (uint8_t *)pa2kva(INO_MAP_MEM_ADDR);
    for(i = 0; i < (superblock->inode_map_size << BLOCK_BYTE_BIOS); i++)
        for(j = 0; j < 8; j++)
            used_inodes += (inodemap[i] >> j) & 1;

    prints("magic : 0x%x\n",superblock->magic);
    prints("used block: %d/%d, start sector: %d\n", used_block, FS_SIZE, FS_START);
    prints("block map start at: %d, occupied blocks: %d\n", superblock->block_map_start, superblock->block_map_size);
    prints("inode map start at: %d, occupied blocks: %d, used: %d\n", superblock->inode_map_start, superblock->inode_map_size, used_inodes);
    prints("inode start at: %d, occupied blocks: %d\n", superblock->inode_start, superblock->inode_size);
    prints("data start at: %d, occupied blocks: %d\n", superblock->datablock_start, superblock->datablock_size);
    prints("inode entry size: %dB, dir entry size : %dB\n",  sizeof(inode_t),sizeof(data_entry_t));
    // screen_reflush();
}

data_entry_t find_dir(uint8_t ino, char * name)
{
    data_entry_t dir;
    int len = strlen(name);
    if (len == 0)
    {
        dir.ino = ino;
        return dir;
    }
    int pos = 0, cnt = 0, flag = 0;
    for (; pos < len; pos++){
        if (name[pos] == '/') 
        {
            if (flag == 0)
                flag = pos;
            cnt ++;
        }
    }
    if (cnt == 0)
    {
        uintptr_t status;
        status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START);
        superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
        // find inode
        status = sbi_sd_blk_read(INO_MEM_ADDR + ino, SUBLK_SIZE, superblock->fs_start + superblock->inode_start + ino);
        inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + dir.ino);
        uint32_t start = superblock->fs_start + superblock->datablock_start;
        for (int i = 0; i < inode->used_size; i++)
        {
            status = sbi_sd_blk_read(DATA_MEM_ADDR + inode->dir_blks[i] + i, 1, start + inode->dir_blks[i]);
            data_entry_t *dentry = (data_entry_t *)pa2kva(DATA_MEM_ADDR + inode->dir_blks[i]);
            if (!kstrcmp(dentry->name, name) && dentry->ino != 0xff){
                kmemcpy(dir.name, name, kstrlen(name));
                dir.ino = dentry->ino;
                dir.mode = dentry->mode;
                return dir;
            }
        }
        dir.ino  = 0xff;
        return dir;
    }
    else
    {
        char cur_name[MAX_NAME_LEN] = {0};
        char new_name[MAX_NAME_LEN] = {0};
        kmemcpy(cur_name, name, flag);
        kmemcpy(new_name, &name[flag + 1], len - flag - 1);
        uintptr_t status;
        status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START);
        superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
        // find inode
        status = sbi_sd_blk_read(INO_MEM_ADDR + ino, SUBLK_SIZE, superblock->fs_start + superblock->inode_start + ino);
        inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + dir.ino);
        uint32_t start = superblock->fs_start + superblock->datablock_start;
        for (int i = 0; i < inode->used_size; i++)
        {
            status = sbi_sd_blk_read(DATA_MEM_ADDR + inode->dir_blks[i], 1, start + inode->dir_blks[i]);
            data_entry_t *dentry = (data_entry_t *)pa2kva(DATA_MEM_ADDR + inode->dir_blks[i]);
            if (!kstrcmp(dentry->name, cur_name) && dentry->ino != 0xff && dentry->mode != FILE && dentry->mode != LINK){
                return find_dir(dentry->ino, new_name);
            }
        }
        dir.ino  = 0xff;
        return dir;
    }
}

int k_ls(char * dirname, int option)
{
    if(!check_fs())
    {
        prints("[> Error]: No file system.\n");
        return;
    }
    // read superblock
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    // find inode
    data_entry_t dir;
    if (dirname[0] != '/')
        dir = find_dir(current_ino, dirname);
    else
        dir = find_dir(0, &dirname[1]);
    if (dir.ino == 0xff)
    {
        prints("[> Error]: No such directory.\n");
        return -1;
    }
    status = sbi_sd_blk_read(INO_MEM_ADDR + dir.ino, SUBLK_SIZE, superblock->fs_start + superblock->inode_start + dir.ino);
    inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + dir.ino);
    uint32_t start = superblock->fs_start + superblock->datablock_start;
    for (int i = 1; i < inode->used_size; i++)
    {
        status = sbi_sd_blk_read(DATA_MEM_ADDR + inode->dir_blks[i], 1, start + inode->dir_blks[i]);
        data_entry_t *dentry = (data_entry_t *)pa2kva(DATA_MEM_ADDR + inode->dir_blks[i]);
        if (dentry->ino != 0xff){
            prints("%s  ", dentry->name);
            return 0;
        }
    }
    prints("\n");
    return 0;
}

int k_cd(char * dirname)
{
    if(!check_fs())
    {
        prints("[> Error]: No file system.\n");
        return;
    }
    // read superblock
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    // find inode
    data_entry_t dir;
    if (dirname[0] != '/')
        dir = find_dir(current_ino, dirname);
    else
        dir = find_dir(0, &dirname[1]);
    if (dir.ino == 0xff)
    {
        prints("[> Error]: No such directory.\n");
        return -1;
    }
    status = sbi_sd_blk_read(INO_MEM_ADDR + dir.ino, SUBLK_SIZE, superblock->fs_start + superblock->inode_start + dir.ino);
    inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + dir.ino);
    uint32_t start = superblock->fs_start + superblock->datablock_start;
    for (int i = 1; i < inode->used_size; i++)
    {
        status = sbi_sd_blk_read(DATA_MEM_ADDR + inode->dir_blks[i], 1, start + inode->dir_blks[i]);
        data_entry_t *dentry = (data_entry_t *)pa2kva(DATA_MEM_ADDR + inode->dir_blks[i]);
        if (dentry->ino != 0xff){
            current_ino = dentry->ino;
            current_inode = inode;
            return 0;
        }
    }
    prints("\n");
    return 0;
}

int check_fs()
{
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, (SUBLK_SIZE << 8), FS_START);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
   if (superblock->magic == FS_MAGIC)
       return 1;
   return 0;
}

/* clear one sector in mem */
void clear_sector(uintptr_t mem_addr) {
    memset((void *)mem_addr,0,SECTOR_SIZE);
}

void clear_block(uintptr_t mem_addr){
    for(int i = 0; i < BLOCK_SECTORS; i++){
        clear_sector(mem_addr + i << SECTOR_BYTE_BIOS);
    }
}

/* alloc 4KB for data, return the block_num */
uint32_t Alloc_datablock(){
    sbi_sd_blk_read(BMAP_MEM_ADDR, 32, START_BLKTOR + BMAP_SD_OFFSET);
    uint8_t *blockmap = (uint8_t *)(BMAP_MEM_ADDR+P2V_OFFSET);
    //search blockmap to find a free block
    int i,j;
    uint32_t free_block = 0;
    for(i = 0; i < 16*1024/8; i++){
        for(j = 0; j < 8; j++)
            if(!(blockmap[i] &  (0x80 >>j))){
                free_block= 8*i+j;
                Set_block_map(free_block);
                return free_block;
            }
    }
    return free_block;
}

/* init "."and".." for a dir */
void init_data_entry(uint32_t block_num, uint32_t cur_inum, uint32_t parent_inum)
{
    data_entry_t *data_entry = (data_entry_t *)(DATA_MEM_ADDR+P2V_OFFSET);
    clear_block(data_entry);
    data_entry[0].mode = T_DIR;
    data_entry[0].inum = cur_inum;
    kstrcpy(data_entry[0].name, (char *)".");

    data_entry[1].mode = T_DIR;
    data_entry[1].inum = parent_inum;
    kstrcpy(data_entry[1].name, (char *)"..");

    sbi_sd_write(DATA_MEM_ADDR, 1, START_BLKTOR+block_num*8);        
}