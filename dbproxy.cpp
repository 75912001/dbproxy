#include <lib_proto/lib_msgbuf.h>
#include <lib_log.h>
#include <lib_err_code.h>

#include "dbproxy.h"
#include "route.h"
#include "db_service.h"
#include "wait_db.h"

dbproxy_t g_dbproxy;
server_send_data_t send_cli_buf;

int send_ret(proto_head_t& head, el::lib_tcp_peer_info_t* peer_fd_info, uint32_t ret)
{
    //cmd命令没有定义
    ERROR_LOG("[cmd:%#x, id:%u, ret:%u]", head.cmd, head.id, ret);
    proto_head_t err_out = head;

    send_cli_buf.set_head(&err_out);

    return el_async::s2peer(peer_fd_info, (void*)send_cli_buf.data(), send_cli_buf.len());
}	


int dbproxy_t::handle_cli( const void* data, uint32_t len, el::lib_tcp_peer_info_t* peer_fd_info )
{
	/* 返回非零，断开FD的连接 */
    proto_head_t head;
    head.unpack(data);


    TRACE_LOG("c=>[body_len:%u, cmd:%#x, seq:%u, ret:%u, uid:%u, fd:%d, pkglen:%u]",
            head.length, head.cmd, head.seq, head.result, head.id, peer_fd_info->get_fd(), len);

    uint32_t db_type = 0;
    const route_db_t* route_db = g_rotue_t.find_dbser(head.cmd, head.id, db_type);
    if (NULL != route_db)
    {
        db_service_t* db_service = g_service_mgr.get_service(route_db->name);
        if (NULL == db_service)
        {
            //连接
            el::lib_tcp_peer_info_t* peer = el_async::connect(route_db->ip, route_db->port);
            if (NULL != peer)
            {
                if (0 != g_service_mgr.add(route_db->name, peer))
                {
                    return send_ret(head, peer_fd_info, el::ERR_DBPROXY::DB_CONNECT_ADD);
                }
            } else {
                return send_ret(head, peer_fd_info, el::ERR_DBPROXY::DB_DISCONNECT);
            }
        }
        db_service = g_service_mgr.get_service(route_db->name);
        if (NULL != db_service){
            //创建新KEY&&保存
            uint32_t key = g_wait_db.gen_save_key(head, peer_fd_info);
            if (0 == key){
                return send_ret(head, peer_fd_info, el::ERR_DBPROXY::GEN_PROTO_KEY);
            }

            //修改头&&发送
            key = EL_BYTE_SWAP(key);

            ::memcpy((char*)data + 4, &key, sizeof(key));
            TRACE_LOG("=>s[seq:%u]", key);
            el_async::s2peer(db_service->peer, data, len);
        }
    } else {
        return send_ret(head, peer_fd_info, el::ERR_SYS::UNDEFINED_CMD);
    }

	return 0;
}

int dbproxy_t::handle_srv( const void* data, uint32_t len, el::lib_tcp_peer_info_t* peer_fd_info )
{
    proto_head_t head;
    head.unpack(data);


    TRACE_LOG("s=>[body_len:%u, cmd:%#x, seq:%u, ret:%u, uid:%u, fd:%d, pkglen:%u]",
            head.length, head.cmd, head.seq, head.result, head.id, peer_fd_info->get_fd(), len);

#ifdef ENABLE_TRACE_LOG
    std::string outbuf;
    el::bin2hex(outbuf, (char*)data, len);
    TRACE_LOG("s=>[len:%u, %s]", len, outbuf.c_str());
#endif

	//处理DB的返回包
    cli_info_t ci;
    if (!g_wait_db.get_old_seq(head.seq, ci))
    {
        ERROR_LOG("seq can not find [cmd:%#x, id:%u, seq:%u]", head.cmd, head.id, head.seq);
        return 0;
    } 
    else 
    {
        ci.head.seq = EL_BYTE_SWAP(ci.head.seq);

        ::memcpy((char*)data + 4, &ci.head.seq, sizeof(ci.head.seq));
        TRACE_LOG("=>c[seq:%u]", ci.head.seq);
        el_async::s2peer(ci.peer_info, data, len);		
    }
	
	return 0;
}

