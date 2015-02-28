#ifndef RESOURCE_MANAGE_H
#define RESOURCE_MANAGE_H

/** 资源包括 发布订阅的(共享内存)信息, 注册的定时器(周期性,一次性),注册的事件
 *
 *  信息分为元数据和信息内容两部分
 *
 * 运行时包含多个情景,每个情景相互独立
 *
 * 情景列表  默认情景(0) | scenlist.log
**/
//资源描述与资源内容导出到文件
int lmice_dump_resource_file();

//从文件恢复资源描述与资源内容
int lmice_load_resource_file();

#endif // RESOURCE_MANAGE_H

