#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "./image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."

#define OS_SIZE_LOC 0x1fc


/* structure to store command line options */
static struct {
    int vm;
    int extended;
} options;

/* prototypes of local functions */
static void create_image(int nfiles, char *files[]);
static void error(char *fmt, ...);
static void read_ehdr(Elf64_Ehdr * ehdr, FILE * fp);
static void read_phdr(Elf64_Phdr * phdr, FILE * fp, int ph,
                      Elf64_Ehdr ehdr);
static void write_segment(Elf64_Ehdr ehdr, Elf64_Phdr phdr, FILE * fp,
                          FILE * img, int *nbytes, int *first);
static void write_os_size(int nbytes, FILE * img);

int main(int argc, char **argv)
{
    char *progname = argv[0];

    /* process command line options */
    options.vm = 0;
    options.extended = 0;
    while ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == '-')) {
        char *option = &argv[1][2];

        if (strcmp(option, "vm") == 0) {
            options.vm = 1;
        } else if (strcmp(option, "extended") == 0) {
            options.extended = 1;
        } else {
            error("%s: invalid option\nusage: %s %s\n", progname,
                  progname, ARGS);
        }
        argc--;
        argv++;
    }
    if (options.vm == 1) {
        error("%s: option --vm not implemented\n", progname);
    }
    if (argc < 3) {
        /* at least 3 args (createimage bootblock kernel) */
        error("usage: %s %s\n", progname, ARGS);
    }
    create_image(argc - 1, argv + 1);
    return 0;
}

static void create_image(int nfiles, char *files[])
{
    int ph, nbytes = 0, first = 1;
    FILE *fp, *img;
    Elf64_Ehdr ehdr;
    Elf64_Phdr phdr;

    /* open the image file */
    img = fopen("image","w+");
    if(!img){
        printf("Failure: cannot open image file!\n");
    }

    /* for each input file */
    while (nfiles-- > 0) {

        /* open input file */
        fp = fopen(files[0],"r+");

        /* read ELF header */
        read_ehdr(&ehdr, fp);
        printf("0x%04lx: %s\n", ehdr.e_entry, *files);

        /* for each program header */
        for (ph = 0; ph < ehdr.e_phnum; ph++) {

            printf("\tsegment %d\n",ph);

            /* read program header */
            read_phdr(&phdr, fp, ph, ehdr);

            /* write segment to the image */
            write_segment(ehdr, phdr, fp, img, &nbytes, &first);
        }
        fclose(fp);
        files++;
    }
    write_os_size(nbytes, img);
    fclose(img);
}

static void read_ehdr(Elf64_Ehdr * ehdr, FILE * fp)
{
    if(!fread(ehdr,sizeof(Elf64_Ehdr),1,fp)){
        error("Warning: the format of input file is not ELF64\n");
    }
}

static void read_phdr(Elf64_Phdr * phdr, FILE * fp, int ph,
                      Elf64_Ehdr ehdr)
{
    fseek(fp,ehdr.e_phoff+ph*ehdr.e_phentsize,SEEK_SET);
    if(!fread(phdr,sizeof(Elf64_Phdr),1,fp)){
        error("Warning: fail to read program header\n");
    }
}

static void write_segment(Elf64_Ehdr ehdr, Elf64_Phdr phdr, FILE * fp,
                          FILE * img, int *nbytes, int *first)
{
    int total_size = (phdr.p_memsz/512+1)*512;
    printf("\t\toffset %x\t\tvaddr %x\n",phdr.p_offset,phdr.p_vaddr);
    printf("\t\tfilesz %x\t\tmemsz %x\n",phdr.p_filesz,phdr.p_memsz);
    // read
    fseek(fp,phdr.p_offset,SEEK_SET);
    char *data=(char *)malloc(total_size*sizeof(char));
    fread(data,phdr.p_filesz,1,fp);
    // write
    fseek(img,(*first-1) * 512,SEEK_SET);
    fwrite(data,total_size,1,img);
    *nbytes += total_size;
    *first += phdr.p_memsz/512+1;
    printf("\t\twriting %x bytes\n",*nbytes);
    printf("\t\tpadding up to %x\n",(*first-1) * 512);
}

static void write_os_size(int nbytes, FILE * img)
{
    int kernel_size = nbytes/512-1;   // -1 excludes bootblock
    fseek(img,OS_SIZE_LOC,SEEK_SET);
    char data[2]={kernel_size & 0xff, (kernel_size>>8) & 0xff};
    fwrite(data,1,2,img);
}

/* print an error message and exit */
static void error(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    if (errno != 0) {
        perror(NULL);
    }
    exit(EXIT_FAILURE);
}
