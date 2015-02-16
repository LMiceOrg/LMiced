#include "lmice_eal_common.h"
#include "resource_manage.h"
#include "resourec_shm.h"

#include <stdio.h>


int lmice_dump_resource_file()
{

    //TODO: server runtime --> instance scenario --> config --> data-path
    const char* data_path = "data";
    int scenario = 0;
    // Dump resource array
    int cnt = 0;
    cnt = lmice_resource_count(scenario);

}

int lmice_load_resource_file();
