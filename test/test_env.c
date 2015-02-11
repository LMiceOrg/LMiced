#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../src/lmice_core.h"

int main()
{
    
    int endian = eal_get_endian();
    int ver = eal_version(endian);
    
    int issame = eal_same_endian(0x2200);
    printf("%ld\t endian is %d\t version is %d, is same endian %d\n", sizeof(lm_net_head_t), endian, ver, issame);
	char* ldpath = getenv("LD_LIBRARY_PATH");
	if(!ldpath)
	{
		ldpath=getenv("PWD");
		printf("don't find LD_LIBRARY_PATH\n");
	}
	else
	{
		printf("LD_LIBRARY_PATH is %s\n", ldpath);
		return 0;
	}
	if(!ldpath)
		printf("don't find pwd\n");
	else
		printf("PWD is %s\n", ldpath);

	return 0;

}
