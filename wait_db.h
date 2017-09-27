/********************************************************************
	platform:	
	author:		kevin
	copyright:	All rights reserved.
	purpose:	
	brief:		等待db的数据
*********************************************************************/

#pragma once

#include <lib_include.h>
#include <lib_util.h>
#include <lib_log.h>
#include <lib_timer.h>
#include <service_if.h>
#include <lib_err_code.h>
#include <lib_proto/lib_msgbuf.h>
#include <proto_header.h>
#include "dbproxy.h"

struct cli_info_t 
{
    proto_head_t head;      //客户端协议包头
	el::lib_tcp_peer_info_t* peer_info; //客户端链接信息
	time_t add_time;        
	cli_info_t(){
		this->peer_info = NULL;
        this->add_time = g_timer->now_sec();
	}
};

class wait_db_t
{
public:
	uint32_t key;
	time_t time_out_sec;
	wait_db_t(){
		this->key = 0;
		this->time_out_sec = 30;
	}

    /**
     * @brief 生成并保存KEY
     *
     * @param proto_head
     * @param peer_info
     *
     * @return 
     */
    uint32_t gen_save_key(proto_head_t& proto_head, el::lib_tcp_peer_info_t* peer_info);
	bool get_old_seq(uint32_t seq, cli_info_t& rci);

	/**
	 * @brief	客户端断开连接时清理对应未返回的等待协议
	 * @return	void
	 */
	void clear_cli_info(int fd);


	/**
	 * @brief	客户端请求超时,返回超时
	 * @return	void
	 */
	void clis_req_time_out();
protected:
	
private:
	
	std::map<uint32_t, cli_info_t> m_proto_head_map;//key:新的序号.val:原seq
};

extern wait_db_t g_wait_db;
