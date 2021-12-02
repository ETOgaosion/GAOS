#include <stdio.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <time.h>
#include <stdlib.h>
#include <test.h>

int main()
{
	srand(clock());
	long mem2 = 0;
	uintptr_t mem1 = 0;
	uint64_t i, words = 0, lines = 0;
	sys_move_cursor(1, 1);
	for (i = 0x20000; i < ((uint64_t)0x1 << 36) ; i += 0x1000)
	{
        if(lines >= 10){
            words = 0;
            lines = 0;
            i = 0x20000;
            sys_move_cursor(1,1);
        }
        else{
            if(words >= 3){
                words = 0;
                lines++;
                sys_move_cursor(1, lines);
            }
            else{
                words++;
            }
        }
		mem1 = i;
		mem2 = rand();
		*(long*)mem1 = mem2;
		printf("0x%lx, %ld; ", mem1, mem2);
		if (*(long*)mem1 != mem2) {
			printf("Error!\n");
		}
	}
	//Only input address.
	//Achieving input r/w command is recommended but not required.
	printf("Success!\n");
	while(1);
	return 0;
}
