
#include <lib_include.h>
#include <lib_log.h>
#include <lib_proto/lib_msgbuf.h>
#include <lib_err_code.h>
#include <lib_util.h>
#include <lib_timer.h>

#include <bench_conf.h>

#include "route.h"
#include "db_service.h"
#include "dbproxy_timer.h"
#include "wait_db.h"
#include "dbproxy.h"

wait_db_t g_wait_db;

extern "C" int on_init()
{
	if(el_async::is_parent()){
		DEBUG_LOG("======daemon start======");
	}else{
		DEBUG_LOG("======server start======");
		if (0 != g_rotue_t.parser()){
			return -1;
		}
		std::string time_out_sec = g_bench_conf->get_strval("dbser_route", "time_out_sec");
		if (!time_out_sec.empty()){
			uint32_t time_out = 0;
			el::convert_from_string(time_out, time_out_sec);
			g_wait_db.time_out_sec = time_out;
		}
		g_dbproxy_timer = new dbproxy_timer_t;
		g_dbproxy_timer->add_sec_event(dbproxy_timer_t::clear, NULL, el_async::get_now_sec() + 1);
	}
	return 0;
}

extern "C" int on_fini()
{
	if (el_async::is_parent()) {
		DEBUG_LOG("======daemon done======");
	}else{
		DEBUG_LOG("======server done======");
		SAFE_DELETE(g_dbproxy_timer);
	}
	return 0;
}

extern "C" void on_events()
{
	if (el_async::is_parent()){
	}else{
        g_timer->handle_timer();
    }
}

extern "C" int on_get_pkg_len(el::lib_tcp_peer_info_t* peer_fd_info,
        const void* data, uint32_t len)
{
    if (len < proto_head_t::PROTO_HEAD_LEN){
        return 0;
    }

	char* c = (char*)data;
	PROTO_LEN pkg_len = EL_BYTE_SWAP((PROTO_LEN)(*(PROTO_LEN*)c));

    if (pkg_len < proto_head_t::PROTO_HEAD_LEN || pkg_len >= g_bench_conf->page_size_max){
        CRIT_LOG("pkg len error |%u", pkg_len);
        return el::ERR_SYS::DISCONNECT_PEER;
    }

    if (len < pkg_len){
        return 0;
    }

	return pkg_len;
}

extern "C" int on_cli_pkg(const void* data, uint32_t len, el::lib_tcp_peer_info_t* peer_fd_info)
{
	g_dbproxy.handle_cli(data, len, peer_fd_info);
	return 0;
}

extern "C" void on_srv_pkg(const void* data, uint32_t len, el::lib_tcp_peer_info_t* peer_fd_info)
{
//	TRACE_LOG("[fd:%d, ip:%s, port:%u]", peer_fd_info->get_fd(), peer_fd_info->get_ip_str().c_str(), peer_fd_info->get_port());
	g_dbproxy.handle_srv(data, len, peer_fd_info);
}


extern "C" void on_cli_conn(el::lib_tcp_peer_info_t* peer_fd_info)
{
    DEBUG_LOG("[fd:%d]", peer_fd_info->fd);
}


extern "C" void on_cli_conn_closed(int fd)
{
	CRIT_LOG("[fd:%d]", fd);
	g_wait_db.clear_cli_info(fd);
}

extern "C" void on_svr_conn_closed(int fd)
{
	CRIT_LOG("[fd:%d]", fd);
	FOREACH(g_rotue_t.service_mgr_vec, it){
		service_mgr_t& sm = *it;
		FOREACH(sm.service_vec, it_2){
			service_t& s = *it_2;
			if (NULL == s.peer){
				continue;
			}
			
			if (s.peer->fd == fd){
				s.peer = NULL;
				break;
			}
		}
	}
}

extern "C" void	on_mcast_pkg(const void* data, int len)
{
}

extern "C"  void on_addr_mcast_pkg(uint32_t id, const char* name, const char* ip, uint16_t port, const char* data, int flag)
{
    //INFO_LOG("id:%u, name:%s, ip:%s, port:%u, flag:%u", id, name, ip, port, flag);
}

extern "C" void on_udp_pkg(int fd, const void* data, int len ,struct sockaddr_in* from, socklen_t fromlen)
{
}

extern "C" void on_svr_conn(int fd){    
    DEBUG_LOG("[fd:%d]", fd);
}
