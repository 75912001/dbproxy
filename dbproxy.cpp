#include <lib_proto/lib_msgbuf.h>
#include <lib_log.h>
#include <lib_err_code.h>
#include <lib_platform.h>
#include "dbproxy.h"
#include "route.h"
#include "db_service.h"
#include "wait_db.h"
#include <lib_include.h>

dbproxy_t g_dbproxy;
server_send_data_t g_send_cli_buf;

int g_send_ret(proto_head_t& head, el::lib_tcp_peer_info_t* peer_fd_info, uint32_t ret)
{
    //cmd命令没有定义
	WARN_LOG("[cmd:%#x, ret:%u]", head.cmd, ret);

    proto_head_t err_out = head;

    g_send_cli_buf.set_head(&err_out);

    return el_async::s2peer(peer_fd_info, (void*)g_send_cli_buf.data(), g_send_cli_buf.len());
}	

int dbproxy_t::handle_cli( const void* data, uint32_t len, el::lib_tcp_peer_info_t* peer_fd_info )
{
	/* 返回非零，断开FD的连接 */
    this->head.unpack(data);

//     TRACE_LOG("c=>[body_len:%u, cmd:%#x, seq:%u, ret:%u, uid:%" PRIu64 ", fd:%d, pkglen:%u]",
//             this->head.length, this->head.cmd, this->head.seq,
// 			this->head.result, this->head.id, peer_fd_info->fd, len);

    service_mgr_t* service_mgr = g_rotue_t.find(head.cmd);
	if (NULL == service_mgr){
		return g_send_ret(this->head, peer_fd_info, el::ERR_SYS::UNDEFINED_CMD);
	}
	this->ret = service_mgr->send(peer_fd_info, this->head.id, data, len);
	if (0 != this->ret){
		g_send_ret(head, peer_fd_info, this->ret);
	}
	return 0;
}

int dbproxy_t::handle_srv( const void* data, uint32_t len, el::lib_tcp_peer_info_t* peer_fd_info )
{
    this->head.unpack(data);

//     TRACE_LOG("s=>[body_len:%u, cmd:%#x, seq:%u, ret:%u, uid:%" PRIu64 ", fd:%d, pkglen:%u]",
//             this->head.length, this->head.cmd, this->head.seq, 
// 			this->head.result, this->head.id, peer_fd_info->fd, len);

#ifdef ENABLE_TRACE_LOG
//     std::string outbuf;
//     el::bin2hex(outbuf, (char*)data, len);
//     TRACE_LOG("s=>[len:%u, %s]", len, outbuf.c_str());
#endif

	//处理DB的返回包
	cli_info_t cli_info;
    if (!g_wait_db.get_old_seq(head.seq, cli_info)){
        ERROR_LOG("seq can not find [cmd:%#x, seq:%u]", head.cmd, head.seq);
        return 0;
    } else {
        cli_info.head.seq = EL_BYTE_SWAP(cli_info.head.seq);

        ::memcpy((char*)data + 4, &cli_info.head.seq, sizeof(cli_info.head.seq));
       // TRACE_LOG("=>c[seq:%u]", cli_info.head.seq);
        el_async::s2peer(cli_info.peer_info, data, len);		
    }
	
	return 0;
}

dbproxy_t::dbproxy_t()
{
	this->ret = 0;
}

