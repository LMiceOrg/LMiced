#include <stdio.h>

int main(void)
{
    printf("Hello World!\n");
    return 0;
}

/**
 string:  int32  ctx \0
 cstring: ctx \0
 int[8,16,32,64]
 file:      d_list  文档列表
 d_list:    document d_list|""
 document:  int32 e_list \0 文档长度 元素列表 \0
 e_list:    element e_list|"" 第一个元素 元素列表或空
 element:   type e_name ctx
 type:      \x01    double
            \x02    utf-8 string
            \x05    binary
            \x08    boolean  \x00 false \x01 true
            \x09    utc datetime(int64)
            \x0a    null
            \x10    32bit integer
            \x11    timestamp(int64)
            \x12    64bit integer
  extern:
            \x80    float
            \x81    int8
            \x82    int16
            \x83    uuid(16 bytes)
  e_name:   cstring
  binary: int32 subtype ctx

*/
