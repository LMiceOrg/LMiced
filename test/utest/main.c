#include <stdio.h>
#include "../../eal/lmice_trace.h"
#include "../../eal/lmice_eal_hash.h"
#pragma comment(lib, "D:\\work\\build-src-Desktop_Qt_5_4_0_MSVC2013_OpenGL_64bit\\debug\\src.lib")

#include <stdio.h>
#include <stdint.h>

static __inline
void hash_to_nameA(uint64_t hval, char* name)
{
    const char* hex_list="0123456789ABCDEF";
    for(int i=0; i<8; ++i)
    {
        name[i*2] = hex_list[ *( (uint8_t*)&hval+i) >> 4];
        name[i*2+1] = hex_list[ *( (uint8_t*)&hval+i) & 0xf ];
    }
    name[16]='\0';
}


struct bjson_element {
    FILE* fp;

    void (*write)(struct bjson_element* e, const char* name, int count, int size, int64_t hval);
//    {
//        char buff[32];

//        fwrite("\t{\n", 1, 3, fp);
//        fwrite("\t\tname:", 1, 7, fp);
//        fwrite(name, 1, strlen(name), fp);
//        fwrite("\n\t\tcount:", 1, 9, fp);
//        itoa(count, buff, 10);
//        fwrite(buff, 1, sizeof(buff), fp);
//        fwrite("\n\t\tsize:", 1, 8, fp);
//        itoa(size, buff, 10);
//        fwrite(buff, 1, sizeof(buff), fp);
//        fwrite("\n\t\thash:", 1, 8, fp);
//        hash_to_nameA(hval, buff);
//        fwrite(buff, 1, 16, fp);
//        fwrite("\n\t}\n", 1, 4, fp);

//    }
};


void bjson_element_write(struct bjson_element* e, const char* name, int count, int size, int64_t hval)
{
    char buff[32];
    FILE* fp = e->fp;
    fwrite("\t{\n", 1, 3, fp);
    fwrite("\t\tname:\"", 1, 8, fp);
    fwrite(name, 1, strlen(name), fp);
    fwrite("\"\n\t\tcount:", 1, 10, fp);
    memset(buff, 0, sizeof(buff));
    itoa(count, buff, 10);
    fwrite(buff, 1, sizeof(buff), fp);
    fwrite("\n\t\tsize:", 1, 8, fp);
    memset(buff, 0, sizeof(buff));
    itoa(size, buff, 10);
    fwrite(buff, 1, sizeof(buff), fp);
    fwrite("\n\t\thash:\"", 1, 9, fp);
    hash_to_nameA(hval, buff);
    fwrite(buff, 1, 16, fp);
    fwrite("\"\n\t}\n", 1, 5, fp);
}
//struct bjson_document {
//    bjson_document(const char* name)
//    {
//        strcpy_s(mname, name);
//        msize = 0;
//    }
//    void dump_json()
//    {
//        char buff[4096];
//        FILE* fp = fopen(mname, "w");
//        setvbuf(fp, buff, _IOFBF, sizeof(buff));
//        fwrite("{\n", 1, 2, fp);
//        bjson_element e(fp);
//        e.write("sh600001", 128, 36, 0xffe033ab);
//        fwrite("\n}\n", 1, 3, fp);
//    }

//    void dump_bjson()
//    {
//        char buff[4096];
//        FILE* fp = fopen(mname, "w");
//        setvbuf(fp, buff, _IOFBF, sizeof(buff));
//        fwrite((char*)&msize, 1, 4, fp);

//        fwrite('\0', 1, 1, fp);
//        fflush(fp);
//        fclose(fp);
//    }

//    char mname[256];
//    int msize;
//};



int lmice_dump_resource_file()
{

    //TODO: server runtime --> instance scenario --> config --> data-path
    const char* data_path = "data";

    //Dump scenario list json(id, type, owner)
    const char* scen_temp = "\t{\n\t\tid:%lld\n\t\ttype:%lld\n\t\towner:%lld\n\t}\n";
    char path[256]={0};
    strcat(path, data_path);
    strcat(path, "\\scenlist.log");
    FILE* fp = fopen(path, "w");
    setvbuf(fp, NULL, _IOFBF, 1024);
    fprintf(fp, scen_temp, 0, 1, 1);
    fflush(fp);
    fclose(fp);

    return 0;
}


int main(void)
{
    lmice_debug_print("hello world");
    lmice_error_print("Ahhhhh! Dump core");

    struct bjson_element e;
    e.write = bjson_element_write;
    char buff[4096];
    FILE* fp = fopen("test.txt", "w");
    setvbuf(fp, buff, _IOFBF, sizeof(buff));
    e.fp = fp;
    const char* nlist[6]={
        "sh600001",
        "sh600002",
        "sh600003",
        "sh600004",
        "sh600005",
        "sh600006"
    };
    //printf("%d\n", sizeof(nlist));
    //return 1;
    const char* name=nlist[0];
    for(int i=0; i< 6; ++i)
    {
        name = nlist[i];
        e.write(&e, name, 128, 36, eal_hash64_fnv1a(name, sizeof(name)));
    }
    fflush(fp);
    fclose(fp);

    lmice_dump_resource_file();
    return 0;
}

