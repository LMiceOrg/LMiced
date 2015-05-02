#ifndef LMSPI_C_H
#define LMSPI_C_H

#if defined(LMSPI_PROJECT)
#define dllfunc __declspec(dllexport)
#else
#define dllfunc __declspec(dllimport)
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C"  {
#endif

/* C interface type*/
typedef void* lmspi_t;

dllfunc lmspi_t lmspi_create();
dllfunc void lmspi_delete(lmspi_t spi);

// 辅助函数
dllfunc int lmspi_init(lmspi_t spi);
dllfunc int lmspi_commit(lmspi_t spi);

//场景管理
dllfunc int lmspi_join_session(lmspi_t spi, uint64_t session_id);
dllfunc int lmspi_leave_session(lmspi_t spi, uint64_t session_id);
//资源注册
dllfunc int lmspi_register_publish(lmspi_t spi, const char* type, const char* inst, int size, uint64_t *event_id);
dllfunc int lmspi_register_subscribe(lmspi_t spi, const char* type, const char* inst, uint64_t* event_id);
//事件管理
dllfunc int lmspi_register_tick_event(lmspi_t spi, int period, int size, int due, uint64_t* event_id);
dllfunc int lmspi_register_timer_event(lmspi_t spi, int period, int size, int due, uint64_t* event_id);
dllfunc int lmspi_register_custom_event(lmspi_t spi, uint64_t* event_list, size_t count, uint64_t* event_id);

//基于ID的回调函数管理
dllfunc int lmspi_register_callback(lmspi_t spi, uint64_t id, lmice_event_callback *callback);
dllfunc int lmspi_unregister_callback(lmspi_t spi, uint64_t id, lmice_event_callback *callback);

//可信计算,QoS管理
dllfunc int lmspi_set_tc_level(lmspi_t spi, int level);
dllfunc int lmspi_set_qos_level(lmspi_t spi, int level);

//阻塞运行与资源回收
dllfunc int lmspi_join(lmspi_t spi);

#ifdef __cplusplus
}
#endif

#endif /** LMSPI_C_H*/

