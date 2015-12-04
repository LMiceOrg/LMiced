#include "net_manage.h"

#include "eal/lmice_trace.h"
#include "eal/lmice_eal_inc.h"
#include "resource/resource_manage.h"
#include "rtspace.h"

//int net_wsa_close_mc_handle(eal_wsa_handle *hd)
//{

//}

//int net_wsa_mc_init_inet(lm_mc_cfg_t *cfg, eal_wsa_handle* hd, struct addrinfo *bp)
//{
//    int ret;

//    struct ip_mreq req;
//    do {
//        /* 加入组播组 */
//        req.imr_interface.s_addr = ((struct sockaddr_in*)bp->ai_addr)->sin_addr.s_addr;
//        req.imr_multiaddr.s_addr = inet_addr(cfg->remote_addr);
//        ret = setsockopt(hd->nfd , IPPROTO_IP , IP_ADD_MEMBERSHIP ,
//                         (const char *)&req , sizeof(req));
//        if(ret != 0) {
//            ret = WSAGetLastError();
//            lmice_error_print("setsockopt failed. Error[%d]\n", ret);
//            closesocket(hd->nfd);
//            break;
//        }

//        /* 设置TTL */
//        ret = setsockopt(hd->nfd , IPPROTO_IP , IP_ADD_MEMBERSHIP ,
//                         (const char *)&cfg->ttl , sizeof(cfg->ttl));
//        if(ret != 0) {
//            ret = WSAGetLastError();
//            lmice_error_print("setsockopt failed. Error[%d]", ret);
//            closesocket(hd->nfd);
//            break;
//        }
//    } while(0);
//    return ret;
//}

//int net_wsa_mc_init_inet6(lm_mc_cfg_t *cfg, eal_wsa_handle* hd, struct addrinfo *bp)
//{
//    int ret = 0;

////    struct ipv6_mreq req;
////    do {
////        /* 加入组播组 */
////        req.ipv6mr_interface.s_addr = ((struct sockaddr_in*)bp->ai_addr)->sin_addr.s_addr;
////        req.ipv6mr_multiaddr.s_addr = inet_addr(cfg->remote_addr);
////        ret = setsockopt(hd->nfd , IPPROTO_IPV6 , IPV6_ADD_MEMBERSHIP ,
////                         (const char *)&req , sizeof(req));
////        if(ret != 0) {
////            ret = WSAGetLastError();
////            lmice_error_print("setsockopt failed. Error[%d]", ret);
////            closesocket(pm->hd->nfd);
////            break;
////        }

////        /* 设置TTL */
////        ret = setsockopt(hd->nfd , IPPROTO_IPV6 , IPV6_ADD_MEMBERSHIP ,
////                         (const char *)&cfg->ttl , sizeof(cfg->ttl));
////        if(ret != 0) {
////            ret = WSAGetLastError();
////            lmice_error_print("setsockopt failed. Error[%d]", ret);
////            closesocket(pm->hd->nfd);
////            break;
////        }
////    } while(0);
//    return ret;
//}

//int net_wsa_create_mc_handle(lm_mc_cfg_t *cfg, eal_wsa_handle *hd)
//{
//    int ret = 0;
//    struct addrinfo *local = NULL;
//    struct addrinfo *bp = NULL;
//    struct addrinfo hints;


//    memset(&hints, 0, sizeof(struct addrinfo));
//    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
//    hints.ai_socktype = SOCK_DGRAM; /* Stream socket */
//    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
//    hints.ai_protocol = 0;          /* TCP protocol */
//    hints.ai_canonname = NULL;
//    hints.ai_addr = NULL;
//    hints.ai_next = NULL;

//    /* getaddrinfo() returns a list of address structures.
//                 Try each address until we successfully bind(2).
//                 If socket(2) (or bind(2)) fails, we (close the socket
//                 and) try the next address.
//    */
//    ret = getaddrinfo(cfg->local_addr, cfg->local_port, &hints, &local);
//    if (ret != 0) {
//        lmice_error_print("getaddrinfo[%s]: %d\n", cfg->local_addr, ret);
//        return -1;
//    }

//    for (bp = local; bp != NULL; bp = bp->ai_next) {
//        ret = -1;
//        //pm->inet = bp->ai_family;
//        hd->nfd = WSASocket(bp->ai_family,
//                                bp->ai_socktype,
//                                bp->ai_protocol,
//                                NULL,
//                                0,
//                                WSA_FLAG_OVERLAPPED);

//        if (INVALID_SOCKET == hd->nfd)
//            continue;

//        ret = net_wsa_mc_init_inet(cfg, hd, bp);
//        if(ret != 0)
//            continue;


//        /* 更新 id (bind address, multicast group address) */
//        eal_wsa_hash(EAL_WSA_MULTICAST_MODE,
//                     cfg->local_addr, 64,
//                     cfg->local_port, 8,
//                     cfg->remote_addr, 64,
//                     cfg->remote_port, 8,
//                     &(hd->inst_id) );
//        break;
//    }/* end-for: bp */

//    freeaddrinfo(local);
//    return ret;
//}

//int EAL_(HANDLE cp, eal_wsa_handle *hd)
//{
//    HANDLE hdl = NULL;
//    DWORD err = 0;
//    /* 将接受套接字和完成端口关联 */
//    hdl = CreateIoCompletionPort((HANDLE)(hd->nfd),
//                                 cp,
//                                 (ULONG_PTR)hd,
//                                 0);
//    if(hdl == NULL)
//    {
//        err = GetLastError();
//        lmice_error_print("CreateIoCompletionPort failed[%u]\n", err);
//        return -1;
//    }

//    /* 准备数据 */
//    eal_iocp_append_data(pm->ilist, pm->hd->inst_id, &pm->data);

//    /* 接收数据 */
//    WSARecvFrom(pm->hd->nfd,
//                &(pm->data->data),
//                1,
//                &(pm->data->recv_bytes),
//                &(pm->data->flags),
//                (struct sockaddr*)&(pm->hd->addr),
//                &(pm->hd->addrlen),
//                &(pm->data->overlapped),
//                NULL);

//    ret = 0;
//    break;

//}

//int create_network_server(lm_mc_cfg_t *cfg)
//{
//    int ret = 0;
//    ret = eal_wsa_init();
//    if(ret != 0)
//    {
//        lmice_error_print("Call eal_wsa_init failed[%d]", ret);
//        return ret;
//    }
//    eal_wsa_handle handle;
//    eal_wsa_create_mc_handle(cfg, handle);
//}

#include "net_beatheart.h"

/* create beatheart service */
int lmnet_beatheart_create(lm_res_param_t *res_param);
/* destroy beatheart service */
int lmnet_beatheart_destroy(lm_res_param_t *pm);


int create_network_server(lm_res_param_t *pm)
{
    int ret = 0;
    lm_worker_res_t *worker = pm->res_worker;
    eal_inc_param *bhpm = &(pm->bh_param);

    /* init network */
    ret = eal_inc_init();



    /* create beatheart service */
    ret = lmnet_beatheart_create(pm);

    /* create synctime service */
    /* Create pub-sub service(Multicast, Broadcast) */
    /* Create request-reply service(TCP, UDP) */

    return ret;
}

int stop_network_server(lm_res_param_t *pm)
{
    /*destroy beatheart service */
    lmnet_beatheart_destroy(pm);

    /* finit network */
    eal_inc_finit();

    return 0;
}
