/********************************************************************
	platform:	
	author:		kevin
	copyright:	All rights reserved.
	purpose:	
	brief:		服务
*********************************************************************/

#pragma once

#include <service_if.h>
#include <lib_log.h>
#include "route.h"
#include "dbproxy.h"
#include "wait_db.h"

//服务数据	
struct service_t
{
	std::string ip;
	uint16_t port;
	el::lib_tcp_peer_info_t* peer;
	service_t(){
		this->init();
	}
	void init(){
		this->peer = NULL;
		this->ip.clear();
		this->port = 0;
	}
	int send(el::lib_tcp_peer_info_t* back_peer, const void* data, uint32_t len){
		if (NULL == this->peer){
			//连接
			this->peer = el_async::connect(this->ip, this->port);
			if (NULL == this->peer){
				return el::ERR_DBPROXY::DB_DISCONNECT;
			}
		}
		if (1 == g_dbproxy.head.cmd%2){
			//创建新KEY&&保存
			uint32_t key = g_wait_db.gen_save_key(g_dbproxy.head, back_peer);

			//修改头&&发送
			key = EL_BYTE_SWAP(key);
			::memcpy((char*)data + 4, &key, sizeof(key));
			//TRACE_LOG("=>s[seq:%u]", key);
		}
		el_async::s2peer(this->peer, data, len);

		return 0;
	}
};

//服务数据管理器	
class service_mgr_t
{
public:
	service_mgr_t(){
		this->start = 0;
		this->end = 0;
	}
	uint32_t start;
	uint32_t end;
	int send(el::lib_tcp_peer_info_t* back_peer, USER_ID route_id, const void* data, uint32_t len){
		uint32_t s_cnt = this->service_vec.size();
		uint32_t idx = route_id%s_cnt;
		return this->service_vec[idx].send(back_peer, data, len);
	}
	std::vector<service_t> service_vec;
};