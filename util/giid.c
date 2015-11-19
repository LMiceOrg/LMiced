#include <stdio.h>
#include <string.h>
#include <lmice_eal_hash.h>
enum input_mode {
    hex_input,
    int_input,
    txt_input
};

#define Mhash_to_nameA(hval, name) { \
    int i=0;    \
    const char* hex_list="0123456789ABCDEF"; \
    for(i=0; i<8; ++i) \
    { \
        name[i*2] = hex_list[ *( (uint8_t*)&hval+i) >> 4]; \
        name[i*2+1] = hex_list[ *( (uint8_t*)&hval+i) & 0xf ]; \
    } \
}

#define hex_to_code(htxt, code, len) { \
    int i=0;    \
    for(i=0; i<len/2; ++i) { \
        code[i] = ( (htxt[i*2] - '0')<<4)+ (htxt[i*2+1] - '0'); \
    }   \
}

#define usage_print() \
    printf("GIID(Generator of information identity) usage:\n\tgiid [-hit] <information unique label>\n\nOptions:\n\n\t-h\t Hexadecimal input\n\t-i\t Decimal input\n\t-t\t Text input(default)")

int main(int argc, char* argv[])
{
    const char* code;
    int mode = txt_input;
    int ret = 0;
    uint64_t hval = 0;
    char * ncode;
    size_t len;
    char output[32] = {0};
    if(argc < 2 || argc >3) {
        usage_print();
        ret = 1;
    } else if(argc == 2) {
        code = argv[1];
    } else {
        code = argv[2];
        if( strncmp(argv[1], "-h", 2) == 0) { /* Hex */
            mode = hex_input;
        } else if (strncmp(argv[1], "-i", 2) == 0) { /* Int */
            mode =int_input;
        } else if (strncmp(argv[1], "-t", 2) == 0) { /* Txt */
            mode = txt_input;
        } else {
            usage_print();
            ret = 1;
        }
    }

    if(ret == 0) {
        if(mode == txt_input)
            hval = eal_hash64_fnv1a(code, strlen(code));
        else if(mode == hex_input) {
            len = strlen(code);
            ncode = (char*)malloc( len/2 + 1 );
            memset(ncode, 0, len/2+1);
            hex_to_code(code, ncode, len);
            hval = eal_hash64_fnv1a(ncode, len/2+1);
            free(ncode);
        }
        Mhash_to_nameA(hval, output);
        printf("GIID:%s\n", output);
    }
    return 0;
}

