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
fd_t file_discriptors[MAX_FILE_NUM] = {{.ino = 0, .access = 0, .r_pos = 0, .w_pos = 0}};


int k_fsop(int op){
    if(op == 0){
        return k_mkfs();
    }else{
        if(!check_fs())
        {
            prints("> Error: No file system.\n");
            return -1;
        }
        return k_statfs();
    }
}

int k_mkfs(){
    // read superblock from disk
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);

    // judge if the fs exists. 
    // if(superblock->magic == FS_MAGIC){
    //     prints("The FS has existed!\n");
    //     prints("magic : 0x%x                                      \n",superblock->magic);
    //     prints("start sector : %d, num sector : %d                \n",superblock->fs_start,superblock->fs_size);
    //     prints("block map: start at %d (size %d)                  \n",superblock->block_map_start,superblock->block_map_size);
    //     prints("inode map: start at %d (size %d)                  \n",superblock->inode_map_start,superblock->inode_map_size);
    //     prints("inode map: start at %d (size %d)                  \n",superblock->inode_start,superblock->inode_size);
    //     prints("data offset : %d (%d)                             \n",superblock->datablock_start,superblock->datablock_size);
    //     return 1;
    // }else{
        printk("[FS] Start initialize filesystem!\n\r");

        // init superblock
        printk("[FS] Setting superblock...\n\r");
        clear_block((uintptr_t)superblock);
        superblock->magic = FS_MAGIC;
        superblock->fs_start = FS_START_BLK;
        superblock->fs_size = FS_SIZE_BLK;
        superblock->block_map_start = BLK_MAP_SD_START;
        superblock->block_map_size = BLK_MAP_SIZE;
        superblock->inode_map_start = INO_MAP_SD_START;
        superblock->inode_map_size = INO_MAP_SIZE;
        superblock->inode_start = INO_SD_START;
        superblock->inode_size = INO_SIZE;
        superblock->datablock_start = DATA_SD_START;
        superblock->datablock_size = DATA_BLK_SIZE;

        printk("magic : %d                                      \n\r",superblock->magic);
        printk("start sector : %d, num sector : %d                \n\r",superblock->fs_start,superblock->fs_size);
        printk("block map: start at %d (size %d)                  \n\r",superblock->block_map_start,superblock->block_map_size);
        printk("inode map: start at %d (size %d)                  \n\r",superblock->inode_map_start,superblock->inode_map_size);
        printk("inode map: start at %d (size %d)                  \n\r",superblock->inode_start,superblock->inode_size);
        printk("data offset : %d (%d)                             \n\r",superblock->datablock_start,superblock->datablock_size);

        sbi_sd_blk_write(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK);

        // init block map
        printk("[FS] Setting block-map...\n\r");
        uint8_t *blockmap = (uint8_t *)pa2kva(BLK_MAP_MEM_ADDR);
        int i;
        for(i = 0; i < BLK_MAP_SIZE; i++)
            clear_block((uintptr_t)blockmap + SECTOR_SIZE * i);

        sbi_sd_blk_write(BLK_MAP_MEM_ADDR, BLK_MAP_SIZE, FS_START_BLK + BLK_MAP_SD_START);

        // init inode map
        printk("[FS] Setting inode-map...\n\r");
        uint8_t *inodemap = (uint8_t *)pa2kva(INO_MAP_MEM_ADDR);
        clear_block((uintptr_t)inodemap);
        
        // the first inode has been used as root-dir
        inodemap[0] = 1;

        sbi_sd_blk_write(INO_MAP_MEM_ADDR, INO_MAP_SIZE, FS_START_BLK + INO_MAP_SD_START);

        // init root-dir inode
        printk("[FS] Setting inode...\n\r");
        inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR);
        clear_block((uintptr_t)inode);
        inode[0].ino = 0;
        inode[0].mode = ROOT;
        inode[0].access = READ | WRITE;
        inode[0].link_num = 0;
        inode[0].size = 2;
        inode[0].used_size = 2;
        inode[0].create_time = get_timer();
        inode[0].modified_time = get_timer();
        inode[0].dir_blks[0] = alloc_block();
        inode[0].dir_blks[1] = alloc_block();
        for(i = 2; i < MAX_DIR_BLK; i++)
            inode[0].dir_blks[i] = 0;
        inode[0].indir_blks_offset = 0;
        inode[0].dbindir_blks_offset = 0;
        inode[0].tpindir_blks_offset = 0;

        sbi_sd_blk_write(INO_MEM_ADDR, 1, FS_START_BLK + INO_SD_START);

        printk("[FS] Setting dentry...\n\r");
        memset(pa2kva(DATA_MEM_ADDR), 0, 2 * BLOCK_SIZE);
        dir_entry_t *de = (dir_entry_t *)pa2kva(DATA_MEM_ADDR);
        memcpy((uint8_t *)de->name, "root", 4);
        memcpy((uint8_t *)de->alias, ".",1);
        de->mode = CUR;
        de->ino = 0;
        de = (dir_entry_t *)pa2kva(DATA_MEM_ADDR + BLOCK_SIZE);
        memcpy((uint8_t *)de->name, "root", 4);
        memcpy((uint8_t *)de->alias, "..",2);
        de->mode = PAR;
        de->ino = 0;

        sbi_sd_blk_write(DATA_MEM_ADDR , 1, FS_START_BLK + DATA_SD_START + inode[0].dir_blks[0]);
        sbi_sd_blk_write(DATA_MEM_ADDR + BLOCK_SIZE,1,FS_START_BLK + DATA_SD_START + inode[0].dir_blks[1]);

        //uintptr_t sbi_sd_blk_write(unsigned int mem_address, unsigned int num_of_blocks(512B), unsigned int block_id)
        printk("[FS] Initialize filesystem finished!\n\r");
        // screen_reflush();
        current_inode = pa2kva(INO_MEM_ADDR);
        current_ino = inode[0].ino;
        
    // }
    return 0;
}

/* print metadata info of fs */
int k_statfs(){
    // read superblock
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);

    // print info
    if(superblock->magic != FS_MAGIC){
        prints("[ERROR] No File System!\n");
        return -1;
    }
    int i, j;
    uint32_t used_block = 0, used_inodes = 0;
    status = sbi_sd_blk_read(BLK_MAP_MEM_ADDR, BLK_MAP_SIZE, FS_START_BLK + BLK_MAP_SD_START);
    uint8_t *blockmap = (uint8_t *)pa2kva(BLK_MAP_MEM_ADDR);
    for(i = 0; i < (superblock->block_map_size << BLOCK_BYTE_BIOS); i++)
        for(j = 0; j < 8; j++)
            used_block += (blockmap[i] >> j) & 1;
    status = sbi_sd_blk_read(INO_MAP_MEM_ADDR, INO_MAP_SIZE, FS_START_BLK+INO_MAP_SD_START);
    uint8_t *inodemap = (uint8_t *)pa2kva(INO_MAP_MEM_ADDR);
    for(i = 0; i < (superblock->inode_map_size << BLOCK_BYTE_BIOS); i++)
        for(j = 0; j < 8; j++)
            used_inodes += (inodemap[i] >> j) & 1;

    prints("magic : 0x%x\n",superblock->magic);
    prints("used block: %d/%d, start sector: %d\n", used_block, FS_SIZE_BLK, FS_START_BLK);
    prints("block map start at: %d, occupied blocks: %d\n", superblock->block_map_start, superblock->block_map_size);
    prints("inode map start at: %d, occupied blocks: %d, used: %d\n", superblock->inode_map_start, superblock->inode_map_size, used_inodes);
    prints("inode start at: %d, occupied blocks: %d\n", superblock->inode_start, superblock->inode_size);
    prints("data start at: %d, occupied blocks: %d\n", superblock->datablock_start, superblock->datablock_size);
    prints("inode entry size: %dB, dir entry size : %dB\n",  sizeof(inode_t),sizeof(dir_entry_t));
    // screen_reflush();
    return 0;
}

int k_dirop(int op, char *dirname, int option){
    if(!check_fs())
    {
        prints("> Error: No file system.\n");
        return -1;
    }
    if(op == 0){
        return k_mkdir(dirname);
    }
    else if(op == 1){
        return k_rmdir(dirname);
    }
    else if(op == 2){
        return k_cd(dirname);
    }
    else if(op == 3){
        return k_ls(dirname, option);
    }
    else if(op == 4){
        return k_pwd(dirname);
    }
    return -1;
}

int k_mkdir(char * dirname)
{
    if (!strcmp(dirname, ".") || !strcmp(dirname, ".."))
    {
        prints("> Error: cannot use . or .. as filename...\n");
        return -2;
    }
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    // handle parent inode
    // write sons to parent's data blk, link to inode
    status = sbi_sd_blk_read(INO_MEM_ADDR + current_ino, 1, superblock->fs_start + superblock->inode_start + current_ino);
    inode_t *in = (inode_t *)(INO_MEM_ADDR + current_ino);
    in->used_size++;
    in->link_num++;
    int start_raw = superblock->fs_start + superblock->datablock_start;
    int start = 0;
    if(in->used_size < MAX_DIR_BLK - 1){
        in->dir_blks[in->used_size - 1] = alloc_block();
        start += in->dir_blks[(in->used_size - 1)];
    }
    else if(in->used_size == MAX_DIR_BLK - 1) {
        in->indir_blks_offset = alloc_block();
        sbi_sd_blk_read(DATA_MEM_ADDR + in->indir_blks_offset * BLOCK_SIZE, 1, start + in->indir_blks_offset);
        data_blk_list_t *indir_list = (data_blk_list_t *)pa2kva(DATA_MEM_ADDR + in->indir_blks_offset * BLOCK_SIZE);
        indir_list->list_type = DATA;
        indir_list->blks[0] = alloc_block();
        start += indir_list->blks[0];
    }
    else if(in->used_size < MAX_DIR_BLK * 2 - 1){
        sbi_sd_blk_read(DATA_MEM_ADDR + in->indir_blks_offset * BLOCK_SIZE, 1, start + in->indir_blks_offset);
        data_blk_list_t *indir_list = (data_blk_list_t *)pa2kva(DATA_MEM_ADDR + in->indir_blks_offset * BLOCK_SIZE);
        indir_list->blks[in->used_size - MAX_DIR_BLK + 1] = alloc_block();
        start += indir_list->blks[in->used_size - MAX_DIR_BLK + 1];
    }
    else if(in->used_size == MAX_DIR_BLK * 2 - 1){
        in->dbindir_blks_offset = alloc_block();
        sbi_sd_blk_read(DATA_MEM_ADDR + in->dbindir_blks_offset * BLOCK_SIZE, 1, start + in->dbindir_blks_offset);
        data_blk_list_t *dbindir_list = (data_blk_list_t *)pa2kva(DATA_MEM_ADDR + in->dbindir_blks_offset * BLOCK_SIZE);
        dbindir_list->list_type = LIST;
        dbindir_list->blks[0] = alloc_block();
        sbi_sd_blk_read(DATA_MEM_ADDR + dbindir_list->blks[0] * BLOCK_SIZE, 1, start + dbindir_list->blks[0]);
        data_blk_list_t *tpindir_list = (data_blk_list_t *)pa2kva(DATA_MEM_ADDR + dbindir_list->blks[0] * BLOCK_SIZE);
        tpindir_list->list_type = DATA;
        start += tpindir_list->blks[0];
    }
    dir_entry_t *de = (dir_entry_t *)(DATA_MEM_ADDR + start);
    de->mode = DIR;
    de->ino = alloc_inode();
    memcpy((uint8_t *)de->name, dirname, strlen(dirname));
    sbi_sd_blk_write(DATA_MEM_ADDR + start, 1, start + start_raw);
    sbi_sd_blk_write(INO_MEM_ADDR + current_ino, 1, superblock->fs_start + superblock->inode_start + current_ino);
    // handle new inode
    memset(INO_MEM_ADDR + de->ino,0,sizeof(inode_t));
    inode_t *inode = (inode_t *)(INO_MEM_ADDR + de->ino);
    inode->ino = de->ino;
    inode->mode = READ | WRITE; 
    inode->link_num = 0;
    inode->used_size = 2;
    inode->create_time = get_timer();
    inode->modified_time = get_timer();
    inode->dir_blks[0] = alloc_block();
    inode->dir_blks[1] = alloc_block();
    sbi_sd_blk_write(INO_MEM_ADDR + de->ino, 1, superblock->fs_start + superblock->inode_start + inode->ino);
    memcpy((uint8_t *)de->name, dirname, strlen(dirname));
    memcpy((uint8_t *)de->alias, ".", 1);
    de->mode = CUR;
    sbi_sd_blk_write(DATA_MEM_ADDR  + start, 1, superblock->fs_start + superblock->datablock_start + inode->dir_blks[0]);
    status = sbi_sd_blk_read(DATA_MEM_ADDR + in->dir_blks[0] * BLOCK_SIZE, 1, superblock->fs_start + superblock->datablock_start + in->dir_blks[0]);
    dir_entry_t *par_de = (dir_entry_t *)(DATA_MEM_ADDR + in->dir_blks[0] * BLOCK_SIZE);
    memcpy((uint8_t *)de->name, par_de->name, strlen(par_de->name));
    memcpy((uint8_t *)de->alias, "..", 2);
    de->mode = PAR;
    sbi_sd_blk_write(DATA_MEM_ADDR + start, 1, superblock->fs_start + superblock->datablock_start + inode->dir_blks[1]);
    
    return;
}

int k_rmdir(char * dirname)
{
    if (!strcmp(dirname, ".") || !strcmp(dirname, ".."))
    {
        prints("> Error: cannot use . or .. as filename...\n");
        return -2;
    }
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    status = sbi_sd_blk_read(INO_MEM_ADDR + current_ino * BLOCK_SIZE, 1, superblock->fs_start + superblock->inode_start + current_ino);
    inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + current_ino * BLOCK_SIZE);
    uint32_t start = superblock->fs_start + superblock->datablock_start;
    int ino_found = -1;
    // Search if directory exists
    for (int i = 0; i < inode->used_size; i++)
    {
        sbi_sd_blk_read(DATA_MEM_ADDR, 1, start + inode->dir_blks[i]);
        dir_entry_t *de = (dir_entry_t *)pa2kva(DATA_MEM_ADDR);
        if (de->mode == DIR && !strcmp(dirname, de->name) && de->ino != 0xff)
        {
            ino_found = de->ino;
            de->ino = 0xff;
            sbi_sd_blk_write(DATA_MEM_ADDR, 1, start + inode->dir_blks[i]);
            break;
        }
    }
    if (ino_found == -1)
    {
        prints("> Error: No such file or directory.\n");
        return -3;
    }
    status = sbi_sd_blk_read(INO_MEM_ADDR + ino_found * BLOCK_SIZE, 1, superblock->fs_start + superblock->inode_start + ino_found);
    inode = (inode_t *)pa2kva(INO_MEM_ADDR + ino_found * BLOCK_SIZE);
    for (int i = 0; i < inode->used_size; i++)
    {
        free_block(inode->dir_blks[i]);
    }
    free_inode(ino_found);
    return 0;
}

dir_entry_t find_dir(uint8_t ino, char * name, int explore_in)
{
    dir_entry_t dir;
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
        if(explore_in){
            uintptr_t status;
            status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK);
            superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
            // find inode
            status = sbi_sd_blk_read(INO_MEM_ADDR + ino * BLOCK_SIZE, SUBLK_SIZE, superblock->fs_start + superblock->inode_start + ino);
            inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + ino * BLOCK_SIZE);
            uint32_t start = superblock->fs_start + superblock->datablock_start;
            for (int i = 0; i < inode->used_size; i++)
            {
                status = sbi_sd_blk_read(DATA_MEM_ADDR + (inode->dir_blks[i] + i) * BLOCK_SIZE, 1, start + inode->dir_blks[i]);
                dir_entry_t *dentry = (dir_entry_t *)pa2kva(DATA_MEM_ADDR + inode->dir_blks[i]);
                if (!strcmp(dentry->name, name) && dentry->ino != 0xff){
                    memcpy((uint8_t *)dir.name, name, strlen(name));
                    dir.ino = dentry->ino;
                    dir.mode = dentry->mode;
                    return dir;
                }
            }
            dir.ino  = 0xff;
            return dir;
        }
        else{{
            memcpy((uint8_t *)dir.name, name, strlen(name));
            dir.ino  = ino;
            return dir;
        }}
    }
    else
    {
        char cur_name[MAX_NAME_LEN] = {0};
        char new_name[MAX_NAME_LEN] = {0};
        memcpy((uint8_t *)cur_name, name, flag);
        memcpy((uint8_t *)new_name, &name[flag + 1], len - flag - 1);
        uintptr_t status;
        status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK);
        superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
        // find inode
        status = sbi_sd_blk_read(INO_MEM_ADDR + ino * BLOCK_SIZE, SUBLK_SIZE, superblock->fs_start + superblock->inode_start + ino);
        inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + ino * BLOCK_SIZE);
        uint32_t start = superblock->fs_start + superblock->datablock_start;
        for (int i = 0; i < inode->used_size; i++)
        {
            status = sbi_sd_blk_read(DATA_MEM_ADDR + inode->dir_blks[i] * BLOCK_SIZE, 1, start + inode->dir_blks[i]);
            dir_entry_t *dentry = (dir_entry_t *)pa2kva(DATA_MEM_ADDR + inode->dir_blks[i] * BLOCK_SIZE);
            if (!strcmp(dentry->name, cur_name) && dentry->ino != 0xff && dentry->mode != FILE && dentry->mode != LINK){
                return find_dir(dentry->ino, new_name,explore_in);
            }
        }
        dir.ino  = 0xff;
        return dir;
    }
}

int k_ls(char * dirname, int option)
{
    // read superblock
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    // find inode
    dir_entry_t dir;
    if (!dirname || dirname[0] != '/')
        dir = find_dir(current_ino, dirname,1);
    else
        dir = find_dir(0, &dirname[1],1);
    if (dir.ino == 0xff)
    {
        prints("[> Error]: No such directory.\n");
        return -1;
    }
    status = sbi_sd_blk_read(INO_MEM_ADDR + dir.ino * BLOCK_SIZE, SUBLK_SIZE, superblock->fs_start + superblock->inode_start + dir.ino);
    inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + dir.ino * BLOCK_SIZE);
    uint32_t start = superblock->fs_start + superblock->datablock_start;
    for (int i = 0; i < inode->used_size; i++)
    {
        status = sbi_sd_blk_read(DATA_MEM_ADDR + inode->dir_blks[i] * BLOCK_SIZE, 1, start + inode->dir_blks[i]);
        dir_entry_t *dentry = (dir_entry_t *)pa2kva(DATA_MEM_ADDR + inode->dir_blks[i] * BLOCK_SIZE);
        if (dentry->ino != 0xff){
            prints("%s/%s  ", dentry->name,dentry->alias);
        }
    }
    prints("\n");
    return 0;
}

int k_cd(char * dirname)
{
    // read superblock
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    // find inode
    dir_entry_t dir;
    if (dirname[0] != '/')
        dir = find_dir(current_ino, dirname,0);
    else
        dir = find_dir(0, &dirname[1],0);
    if (dir.ino == 0xff)
    {
        prints("[> Error]: No such directory.\n");
        return -1;
    }
    status = sbi_sd_blk_read(INO_MEM_ADDR + dir.ino * BLOCK_SIZE, SUBLK_SIZE, superblock->fs_start + superblock->inode_start + dir.ino);
    inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + dir.ino * BLOCK_SIZE);
    uint32_t start = superblock->fs_start + superblock->datablock_start;
    for (int i = 1; i < inode->used_size; i++)
    {
        status = sbi_sd_blk_read(DATA_MEM_ADDR + inode->dir_blks[i] * BLOCK_SIZE, 1, start + inode->dir_blks[i]);
        dir_entry_t *dentry = (dir_entry_t *)pa2kva(DATA_MEM_ADDR + inode->dir_blks[i] * BLOCK_SIZE);
        if (dentry->ino != 0xff){
            current_ino = dentry->ino;
            current_inode = inode;
            return 0;
        }
    }
    prints("\n");
    return 0;
}

int k_pwd(char *dirname)
{
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    status = sbi_sd_blk_read(INO_MEM_ADDR + current_ino * BLOCK_SIZE, 1, FS_START_BLK + INO_SD_START + current_ino);
    current_inode = (inode_t *)pa2kva(INO_MEM_ADDR + current_ino);
    status = sbi_sd_blk_read(DATA_MEM_ADDR + current_inode->dir_blks[0] * BLOCK_SIZE, 1, superblock->fs_start + superblock->datablock_start + current_inode->dir_blks[0]);
    dir_entry_t *dentry = (dir_entry_t *)pa2kva(DATA_MEM_ADDR + current_inode->dir_blks[0] * BLOCK_SIZE);
    memcpy((uint8_t *)dirname,dentry->name,MAX_NAME_LEN);
    return 0;
}


int k_fileop(int op, char *mul_char, int mul_int, int size)
{
    if(!check_fs())
    {
        prints("> Error: No file system.\n");
        return -1;
    }
    if(op == 0){
        return k_touch(mul_char);
    }
    else if(op == 1){
        return k_cat(mul_char);
    }
    else if(op == 2){
        return k_fopen(mul_char,mul_int);
    }
    else if(op == 3){
        return k_fread(mul_int,mul_char,size);
    }
    else if(op == 4){
        return k_fwrite(mul_int, mul_char,size);
    }
    else if(op == 5){
        return k_fclose(mul_int);
    }
    else{
        return k_frm(mul_char);
    }
    return -1;
}

int k_touch(char * filename)
{
    if (!strcmp(filename, ".") || !strcmp(filename, ".."))
    {
        prints("> Error: cannot use . or .. as filename.\n");
        return;
    }
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK + SUBLK_SD_START);
    superblock_t *sublk = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    // handle parent inode
    status = sbi_sd_blk_read(INO_MEM_ADDR + current_ino * BLOCK_SIZE, 1, sublk->fs_start + sublk->inode_start + current_ino);
    inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + current_ino * BLOCK_SIZE);
    uint32_t start = sublk->fs_start + sublk->datablock_start;
    inode->used_size++;
    inode->link_num++;
    if(inode->used_size < MAX_DIR_BLK - 1){
        inode->dir_blks[inode->used_size - 1] = alloc_block();
    }
    memset(pa2kva(DATA_MEM_ADDR), 0, sizeof(dir_entry_t));
    dir_entry_t *de = (dir_entry_t *)DATA_MEM_ADDR;
    de->mode = FILE;
    de->ino = alloc_inode();
    memcpy((uint8_t *)de->name, filename, strlen(filename));
    sbi_sd_blk_write(DATA_MEM_ADDR, 1, sublk->fs_start + sublk->datablock_start + inode->dir_blks[inode->used_size - 1]);
    sbi_sd_blk_write(INO_MEM_ADDR + current_ino, 1, sublk->fs_start + sublk->inode_start + current_ino);
    // handle new inode
    memset(pa2kva(INO_MEM_ADDR + de->ino), 0, sizeof(inode));
    inode = (inode_t *)pa2kva(INO_MEM_ADDR + de->ino);
    inode->ino = de->ino;
    inode->mode = READ | WRITE; 
    inode->link_num = 0;
    inode->used_size = 1;
    inode->create_time = get_timer();
    inode->modified_time = get_timer();
    inode->dir_blks[0] = alloc_block();
    inode->indir_blks_offset = 0;
    inode->dbindir_blks_offset = 0;
    sbi_sd_blk_write(kva2pa(inode), 1, sublk->fs_start + sublk->inode_start + inode->ino);
    return 0;
}

int k_cat(char * filename)
{
    // show file content
    uint8_t file_ino = find_file_ino(find_dir(current_ino, filename, 0));
    superblock_t *sublk = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    sbi_sd_blk_read(INO_MEM_ADDR + file_ino * BLOCK_SIZE, 1, sublk->fs_start + sublk->inode_start + file_ino);
    inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + file_ino * BLOCK_SIZE);
    uint32_t start = sublk->fs_start + sublk->datablock_start;
    memset(DATA_MEM_ADDR, 0, inode->used_size * SECTOR_SIZE * BLOCK_SECTORS);
    for (int i = 0; i < inode->used_size; i++)
    {
        sbi_sd_blk_read(DATA_MEM_ADDR + inode->dir_blks[i] * BLOCK_SIZE, 1, start + inode->dir_blks[i]);
        prints("%s", (char *)DATA_MEM_ADDR);
    }
    return 0;
}

int k_fopen(char * filename, int access)
{
    uint8_t file_ino = find_file_ino(find_dir(current_ino, filename, 0));
    // alloc a file descriptor
    int id;
    for (id = 0; id < MAX_FILE_NUM; id++)
        if (file_discriptors[id].ino == 0)
            break;
    file_discriptors[id].ino = file_ino;
    file_discriptors[id].access = access;
    file_discriptors[id].r_pos = 0;
    file_discriptors[id].w_pos = 0;
    return id;
}

int k_fread(int fd, char *buff, int size)
{
    uint8_t inode = file_discriptors[fd].ino;
    if(size <= 0){
        return -2;
    }
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK + SUBLK_SD_START);
    superblock_t *sublk = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    status = sbi_sd_blk_read(INO_MEM_ADDR + inode * BLOCK_SIZE, 1, sublk->fs_start + sublk->inode_start + inode);
    inode_t *in = (inode_t *)pa2kva(INO_MEM_ADDR + inode* BLOCK_SIZE);
    int sz = 0;
    uint32_t start = sublk->fs_start + sublk->datablock_start;
    uint8_t buffer_raw[BLOCK_SIZE * BYTE_BIT_BIOS] = {0};
    uint8_t *buffer = buffer_raw;
    // handle unalign
    if (file_discriptors[fd].r_pos % BLOCK_SIZE != 0){
        int tmp_size = BLOCK_SIZE - file_discriptors[fd].r_pos % BLOCK_SIZE;
        sbi_sd_blk_read(kva2pa(buffer), 1, file_discriptors[fd].r_pos / BLOCK_SIZE);
        memcpy((uint8_t *)buff + file_discriptors[fd].r_pos % BLOCK_SIZE, buffer
            ,tmp_size );
        file_discriptors[fd].r_pos += tmp_size;
        sz += tmp_size;
        buff += tmp_size;
    }
    if(sz >= size){
        return sz;
    }
    int through_times = 0;
    while (through_times < MAX_DIR_BLK - 1 && file_discriptors[fd].r_pos < (MAX_DIR_BLK - 1) * BLOCK_SIZE && size > sz){
        int tmp_size = size - sz < BLOCK_SIZE ? size -sz : BLOCK_SIZE;
        sbi_sd_blk_read(kva2pa(buffer), 1, start + in->dir_blks[through_times + 1]);
        memcpy((uint8_t *)buff, buffer, tmp_size); 
        sz += tmp_size;
        buff += tmp_size;
        file_discriptors[fd].r_pos += BLOCK_SIZE;
        through_times++;
    }
    if(sz >= size){
        return sz;
    }
    int total_size = (MAX_DIR_BLK - 1) * BLOCK_SIZE;
    through_times = 0;
    status = sbi_sd_blk_read(DATA_MEM_ADDR + in->indir_blks_offset * BLOCK_SIZE, 1, sublk->fs_start + sublk->datablock_start + in->indir_blks_offset);
    data_blk_list_t *indir_list = (data_blk_list_t *)pa2kva(DATA_MEM_ADDR + in->indir_blks_offset* BLOCK_SIZE);
    while (through_times < MAX_DIR_BLK && file_discriptors[fd].r_pos - total_size < MAX_DIR_BLK * BLOCK_SIZE && size > sz){
        int tmp_size = size - sz < BLOCK_SIZE ? size - sz : BLOCK_SIZE;
        sbi_sd_blk_read(kva2pa(buffer), 1, start + indir_list->blks[through_times]);
        memcpy((uint8_t *)buff, buffer, tmp_size); 
        sz += tmp_size;
        buff += tmp_size;
        file_discriptors[fd].r_pos += BLOCK_SIZE;
        through_times++;
    }
    if(sz >= size){
        return sz;
    }
    total_size += MAX_DIR_BLK * BLOCK_SIZE;
    int tmp_total_size = total_size;
    through_times = 0;
    int through_times_db = 0;
    status = sbi_sd_blk_read(DATA_MEM_ADDR + in->dbindir_blks_offset * BLOCK_SIZE, 1, sublk->fs_start + sublk->datablock_start + in->dbindir_blks_offset);
    indir_list = (data_blk_list_t *)pa2kva(DATA_MEM_ADDR + in->dbindir_blks_offset* BLOCK_SIZE);
    while (through_times < MAX_DIR_BLK && file_discriptors[fd].r_pos - total_size < (MAX_DIR_BLK << 1) * BLOCK_SIZE && size > sz){
        status = sbi_sd_blk_read(DATA_MEM_ADDR + indir_list->blks[through_times] * BLOCK_SIZE, 1, sublk->fs_start + sublk->datablock_start + indir_list->blks[through_times]);
        data_blk_list_t *db_indir_list = (data_blk_list_t *)pa2kva(DATA_MEM_ADDR + indir_list->blks[through_times] * BLOCK_SIZE);
        while (through_times_db < MAX_DIR_BLK && file_discriptors[fd].r_pos - tmp_total_size < MAX_DIR_BLK * BLOCK_SIZE && size > sz){
            int tmp_size = size - sz < BLOCK_SIZE ? size - sz : BLOCK_SIZE;
            sbi_sd_blk_read(kva2pa(buffer), 1, start + db_indir_list->blks[through_times_db]);
            memcpy((uint8_t *)buff, buffer, tmp_size); 
            sz += tmp_size;
            buff += tmp_size;
            file_discriptors[fd].r_pos += BLOCK_SIZE;
            through_times_db++;
        }
        tmp_total_size += MAX_DIR_BLK * BLOCK_SIZE;
        through_times++;
        through_times_db = 0;
    }
    if(sz >= size){
        return sz;
    }
    return size;
}

int k_fwrite(int fd, char *buff, int size)
{
    uint8_t inode = file_discriptors[fd].ino;
    if(size <= 0){
        return -2;
    }
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK + SUBLK_SD_START);
    superblock_t *sublk = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    status = sbi_sd_blk_read(INO_MEM_ADDR + inode * BLOCK_SIZE, 1, sublk->fs_start + sublk->inode_start + inode);
    inode_t *in = (inode_t *)pa2kva(INO_MEM_ADDR + inode* BLOCK_SIZE);
    int sz = 0;
    uint32_t start = sublk->fs_start + sublk->datablock_start;
    uint8_t buffer_raw[BLOCK_SIZE * BYTE_BIT_BIOS] = {0};
    uint8_t *buffer = buffer_raw;
    // handle unalign
    if (file_discriptors[fd].w_pos % BLOCK_SIZE != 0){
        int tmp_size = BLOCK_SIZE - file_discriptors[fd].w_pos % BLOCK_SIZE;
        memcpy((uint8_t *)buffer, buff + file_discriptors[fd].w_pos % BLOCK_SIZE, tmp_size );
        sbi_sd_blk_write(kva2pa(buffer), 1, file_discriptors[fd].w_pos / BLOCK_SIZE);
        file_discriptors[fd].w_pos += tmp_size;
        sz += tmp_size;
        buff += tmp_size;
    }
    if(sz >= size){
        return sz;
    }
    // w_pos in dir_blks
    int through_times = 0;
    while (through_times < MAX_DIR_BLK - 1 && file_discriptors[fd].w_pos < (MAX_DIR_BLK - 1) * BLOCK_SIZE && size > sz){
        int tmp_size = size - sz < BLOCK_SIZE ? size -sz : BLOCK_SIZE;
        memcpy((uint8_t *)buffer, buff, tmp_size); 
        if(in->used_size < through_times + 1){
            in->dir_blks[through_times + 1] = alloc_block();
        }
        sbi_sd_blk_write(kva2pa(buffer), 1, start + in->dir_blks[through_times + 1]);
        sz += tmp_size;
        buff += tmp_size;
        file_discriptors[fd].w_pos += BLOCK_SIZE;
        through_times++;
    }
    if(sz >= size){
        return sz;
    }
    int total_size = (MAX_DIR_BLK - 1) * BLOCK_SIZE;
    through_times = 0;
    status = sbi_sd_blk_read(DATA_MEM_ADDR + in->indir_blks_offset * BLOCK_SIZE, 1, sublk->fs_start + sublk->datablock_start + in->indir_blks_offset);
    data_blk_list_t *indir_list = (data_blk_list_t *)pa2kva(DATA_MEM_ADDR + in->indir_blks_offset* BLOCK_SIZE);
    while (through_times < MAX_DIR_BLK && file_discriptors[fd].w_pos - total_size < MAX_DIR_BLK * BLOCK_SIZE && size > sz){
        int tmp_size = size - sz < BLOCK_SIZE ? size - sz : BLOCK_SIZE;
        memcpy((uint8_t *)buffer, buff, tmp_size);
        if(in->used_size - total_size)
        sbi_sd_blk_write(kva2pa(buffer), 1, start + indir_list->blks[through_times]);
        sz += tmp_size;
        buff += tmp_size;
        file_discriptors[fd].w_pos += BLOCK_SIZE;
        through_times++;
    }
    if(sz >= size){
        return sz;
    }
    total_size += MAX_DIR_BLK * BLOCK_SIZE;
    int tmp_total_size = total_size;
    through_times = 0;
    int through_times_db = 0;
    status = sbi_sd_blk_read(DATA_MEM_ADDR + in->dbindir_blks_offset * BLOCK_SIZE, 1, sublk->fs_start + sublk->datablock_start + in->dbindir_blks_offset);
    indir_list = (data_blk_list_t *)pa2kva(DATA_MEM_ADDR + in->dbindir_blks_offset* BLOCK_SIZE);
    while (through_times < MAX_DIR_BLK && file_discriptors[fd].w_pos - total_size < (MAX_DIR_BLK << 1) * BLOCK_SIZE && size > sz){
        status = sbi_sd_blk_read(DATA_MEM_ADDR + indir_list->blks[through_times] * BLOCK_SIZE, 1, sublk->fs_start + sublk->datablock_start + indir_list->blks[through_times]);
        data_blk_list_t *db_indir_list = (data_blk_list_t *)pa2kva(DATA_MEM_ADDR + indir_list->blks[through_times] * BLOCK_SIZE);
        while (through_times_db < MAX_DIR_BLK && file_discriptors[fd].w_pos - tmp_total_size < MAX_DIR_BLK * BLOCK_SIZE && size > sz){
            int tmp_size = size - sz < BLOCK_SIZE ? size - sz : BLOCK_SIZE;
            memcpy((uint8_t *)buffer, buff, tmp_size); 
            sbi_sd_blk_write(kva2pa(buffer), 1, start + db_indir_list->blks[through_times_db]);
            sz += tmp_size;
            buff += tmp_size;
            file_discriptors[fd].w_pos += BLOCK_SIZE;
            through_times_db++;
        }
        tmp_total_size += MAX_DIR_BLK * BLOCK_SIZE;
        through_times++;
        through_times_db = 0;
    }
    if(sz >= size){
        return sz;
    }
    return size;
}

int k_fclose(int fd)
{
    file_discriptors[fd].ino = 0;
    file_discriptors[fd].r_pos = 0;
    file_discriptors[fd].w_pos = 0;
    file_discriptors[fd].access = 0;
    return 0;
}

int k_frm(char *filename)
{
    return k_rmdir(filename);
}

int k_linkop(int option, char *src, char *dst)
{
    if(option == 0){
        return k_links(src,dst);
    }
    else{
        // return k_linkh(src,dst);
        return -1;
    }
}

int k_links(char *src, char *dst)
{
    dir_entry_t dir_src, dir_dst;
    int spash = strlen(src);
    while(spash--)
    {
        if (src[spash] == '/')
            break;
    }
    spash ++;
    if (src[0] != '/')
        dir_src = find_dir(current_ino, src,0);
    else
        dir_src = find_dir(0, &src[1], 0);
    if (!strcmp(src,"/"))
        dir_src.ino = 0;
        
    if (dir_src.ino == 0xff)
    {
        prints("> No such file or directory.\n");
        return -2;
    }
    if (dst[0] != '/')
        dir_dst = find_dir(current_ino, dst, 1);
    else
        dir_dst = find_dir(0, &dst[1], 1);
    if (!strcmp(src,"/"))
        dir_dst.ino = 0;
    // Link
    uint8_t file_ino = find_file_ino(dir_src);
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK + SUBLK_SD_START);
    superblock_t *sublk = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    sbi_sd_blk_read(INO_MEM_ADDR + dir_dst.ino * BLOCK_SIZE, 1, sublk->fs_start + sublk->inode_start + dir_dst.ino);
    inode_t *in = (inode_t *)pa2kva(INO_MEM_ADDR + dir_dst.ino * BLOCK_SIZE);
    in->used_size++;
    in->link_num++;
    in->dir_blks[in->used_size - 1] = alloc_block();
    sbi_sd_blk_write(DATA_MEM_ADDR + dir_dst.ino * BLOCK_SIZE, 1, sublk->fs_start + sublk->datablock_start + in->dir_blks[in->used_size]);
    sbi_sd_blk_write(kva2pa(in), 1, sublk->fs_start + sublk->inode_start + dir_dst.ino);
    return 0;
}

int k_lseek(int fd, int offset, int whence, int r_or_w)
{
    if(file_discriptors[fd].ino == 0){
        return -1;
    }
    if(whence == SEEK_SET){
        if(!r_or_w){
            file_discriptors[fd].r_pos = offset;
        }
        else{
            file_discriptors[fd].w_pos = offset;
        }
    }
    else if(whence == SEEK_CUR){
        if(!r_or_w){
            file_discriptors[fd].r_pos += offset;
        }
        else{
            file_discriptors[fd].w_pos += offset;
        }
    }
    else if(whence == SEEK_END){
        if(r_or_w){
            uintptr_t status;
            status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK + SUBLK_SD_START);
            superblock_t *sublk = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
            status = sbi_sd_blk_read(INO_MEM_ADDR + file_discriptors[fd].ino * BLOCK_SIZE, 1, sublk->fs_start + sublk->inode_start + file_discriptors[fd].ino);
            inode_t *in = (inode_t *)pa2kva(INO_MEM_ADDR + file_discriptors[fd].ino * BLOCK_SIZE);
            file_discriptors[fd].w_pos =  in->used_size + offset;
            in->used_size += offset;
            in->size += offset;
        }
    }
    return 0;
}

/*
 * tools
 */

int check_fs()
{
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, (SUBLK_SIZE << 8), FS_START_BLK);
    superblock_t *superblock = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    if (superblock->magic == FS_MAGIC)
        return 1;
    return 0;
}

uint8_t find_file_ino(dir_entry_t dir)
{
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK + SUBLK_SD_START);
    superblock_t *sublk = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    // recursive
    if (dir.ino == 0xff)
    {
        prints("> Error: No such file.\n");
        return -2;
    }
    // find inode
    status = sbi_sd_blk_read(INO_MEM_ADDR + dir.ino * BLOCK_SIZE, 1, sublk->fs_start + sublk->inode_start + current_ino);
    inode_t *inode = (inode_t *)pa2kva(INO_MEM_ADDR + dir.ino * BLOCK_SIZE);
    uint32_t start = sublk->fs_start + sublk->datablock_start;
    uint8_t file_ino = 0xff;
    for (int i = 0; i < inode->used_size; i++)
    {
        sbi_sd_blk_read(DATA_MEM_ADDR  + inode->dir_blks[i] * BLOCK_SIZE, 1, start + inode->dir_blks[i]);
        dir_entry_t *de = (dir_entry_t *)(DATA_MEM_ADDR + inode->dir_blks[i] * BLOCK_SIZE);
        if (!strcmp(de->name, dir.name) && de->ino != 0xff && de->mode == FILE){
            file_ino = de->ino;
            break;
        }
    }
    if (file_ino == 0xff)
    {
        prints("> Error: No such file.\n");
        return -3;
    }
    return file_ino;
}

/* clear one sector in mem */
void clear_sector(uintptr_t mem_addr) {
    memset((void *)mem_addr,0,SECTOR_SIZE);
}

void clear_block(uintptr_t mem_addr){
    for(int i = 0; i < BLOCK_SECTORS; i++){
        for(int j = 0; j < BLOCK_SECTORS; j++)
            clear_sector(mem_addr + j * SECTOR_SIZE);
    }
}

/* alloc 4KB for data, return the block_num */
uint32_t alloc_block()
{
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK + SUBLK_SD_START);
    superblock_t *sublk = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    if(sublk->magic == 0){
        return -1;
    }
    status = sbi_sd_blk_read(BLK_MAP_MEM_ADDR, sublk->block_map_size, sublk->fs_start + sublk->block_map_start);
    uint8_t *blkmap = (uint8_t *)pa2kva(BLK_MAP_MEM_ADDR);
    int free_blk = -1;
    for(int i = 0; i < BLK_MAP_BYTE_SIZE; i++){
        for(int j = 0; j < 8; j++){
            if((*(blkmap + i) & (0x1 << j)) == 0){
                free_blk = (i << BYTE_BIT_BIOS) + j;
                *(blkmap + i) |= (0x1 << j);
                break;
            }
        }
        if(free_blk != -1){
            break;
        }
    }
    if(free_blk == -1){
        return -2;
    }
    sbi_sd_blk_write(BLK_MAP_MEM_ADDR, sublk->block_map_size, sublk->fs_start + sublk->block_map_start);
    return free_blk;
}

/* alloc 4KB for data, return the block_num */
uint32_t free_block(uint32_t blk_bios)
{
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK + SUBLK_SD_START);
    superblock_t *sublk = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    if(sublk->magic == 0){
        return -1;
    }
    status = sbi_sd_blk_read(BLK_MAP_MEM_ADDR, sublk->block_map_size, sublk->fs_start + sublk->block_map_start);
    uint8_t *blkmap = (uint8_t *)pa2kva(BLK_MAP_MEM_ADDR);
    *(blkmap + blk_bios) = 0;
    sbi_sd_blk_write(BLK_MAP_MEM_ADDR + blk_bios % 512, 1, sublk->fs_start + sublk->block_map_start + blk_bios / 512);
    return blk_bios;
}

uint32_t alloc_inode()
{
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK + SUBLK_SD_START);
    superblock_t *sublk = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    sbi_sd_blk_read(INO_MAP_MEM_ADDR, sublk->inode_map_size, sublk->fs_start + sublk->inode_map_start);
    uint8_t *ino_map = (uint8_t *)INO_MAP_MEM_ADDR;
    int free_inode = -1;
    for(int i = 0; i < BLK_MAP_BYTE_SIZE; i++){
        for(int j = 0; j < 8; j++){
            if((*(ino_map + i) & (0x1 << j)) == 0){
                free_inode = i << BYTE_BIT_BIOS + j;
            }
        }
    }
    if(free_inode == -1){
        return -2;
    }
    sbi_sd_blk_write(INO_MAP_MEM_ADDR, sublk->block_map_size, sublk->fs_start + sublk->block_map_start);
    return free_inode;
}

int free_inode(uint8_t ino)
{
    uintptr_t status;
    status = sbi_sd_blk_read(SUBLK_MEM_ADDR, SUBLK_SIZE, FS_START_BLK + SUBLK_SD_START);
    superblock_t *sublk = (superblock_t *)pa2kva(SUBLK_MEM_ADDR);
    if(sublk->magic == 0){
        return -1;
    }
    sbi_sd_blk_read(INO_MAP_MEM_ADDR, sublk->inode_map_size, sublk->fs_start + sublk->inode_map_start);
    uint8_t *ino_map = (uint8_t *)INO_MAP_MEM_ADDR;
    *(ino_map + ino) = 0;
    sbi_sd_blk_write(BLK_MAP_MEM_ADDR, 1, sublk->fs_start + sublk->block_map_start);
    return ino;
}