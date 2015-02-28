#include "lmice_eal_common.h"
#include "resource_manage.h"
#include "resourec_shm.h"

#include <stdio.h>
#include <errno.h>

#include <jansson.h>

const char* data_path = "data";

int lmice_dump_resource_file()
{

    //TODO: server runtime --> instance scenario --> config --> data-path

    //Dump scenario list json(id, type, owner)
    const char* scen_temp = "\t{\n\t\tid:%lld\n\t\ttype:%lld\n\t\towner:%lld\n\t}\n";
    char path[256]={0};
    strcat(path, data_path);
    strcat(path, "\\scenlist.log");
    FILE* fp = fopen(path, "w");
    if(!fp)
        return errno;
    setvbuf(fp, NULL, _IOFBF, 1024);
    fwrite("[\n", 1, 2, fp);
    fprintf(fp, scen_temp, 0, 1, 1);
    fwrite("]\n", 1, 2, fp);
    fflush(fp);
    fclose(fp);

    return 0;
}

int lmice_load_resource_file()
{
    char path[256]={0};
    strcat(path, data_path);
    strcat(path, "\\scenlist.log");

    json_error_t err;
    json_t *root;
    FILE* fp = fopen(path, "r");
    if(!fp)
        return errno;

    root = json_loadf(fp, 0, &err);

    fclose(fp);

}
