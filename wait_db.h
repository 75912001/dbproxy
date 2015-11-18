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

#pragma pack(1)

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

#pragma pack()

class wait_db_t
{
	PROTECTED_RW(uint32_t, key);
	PROTECTED_RW(time_t, time_out_sec);
public:
	wait_db_t(){
		this->key = 0;
		this->time_out_sec = 30;
	}
	virtual ~wait_db_t(){}

    /**
     * @brief 生成并保存KEY
     *
     * @param proto_head
     * @param peer_info
     *
     * @return 
     */
    uint32_t gen_save_key(proto_head_t& proto_head, el::lib_tcp_peer_info_t* peer_info)
    {
        this->key++;

        if (0 == this->key)
        {
            this->key++;
        }

        cli_info_t ci;
        ci.head = proto_head;
        ci.peer_info = peer_info;

        if(!m_proto_head_map.insert(std::make_pair(this->key, ci)).second)
        {
            ERROR_LOG("[body_len:%u, cmd:%#x, id:%u, seq:%u, ret:%u, key:%u]",
                    proto_head.length, proto_head.cmd, proto_head.id, proto_head.seq, 
                    proto_head.result, this->key);
            return 0;
        }

        return this->key;
    }
	bool get_old_seq(uint32_t seq, cli_info_t& rci)
    {
		std::map<uint32_t, cli_info_t>::iterator it = m_proto_head_map.find(seq);
		if(m_proto_head_map.end() == it){
			ERROR_LOG("can not find [seq:%u]", seq);
			return false;
		}
		rci = it->second;
		m_proto_head_map.erase(seq);
		return true;
	}

	/**
	 * @brief	客户端断开连接时清理对应未返回的等待协议
	 * @return	void
	 */
	void clear_cli_info(int fd)
    {
		for (std::map<uint32_t, cli_info_t>::iterator it = this->m_proto_head_map.begin();
			this->m_proto_head_map.end() != it;)
        {
			if (it->second.peer_info->get_fd() == fd)
            {
				this->m_proto_head_map.erase(it++);
			} 
            else 
            {
				++it;
			}
		}
	}


	/**
	 * @brief	客户端请求超时,返回超时
	 * @return	void
	 */
	void clis_req_time_out()
    {
        /*TRACE_LOG("[now sec:%lu, time_out_sec:%lu]", g_timer->get_now_sec(), this->time_out_sec);*/
		for (std::map<uint32_t, cli_info_t>::iterator it = this->m_proto_head_map.begin();
			this->m_proto_head_map.end() != it;)
        {
            if (g_timer->now_sec() >= it->second.add_time + this->time_out_sec)
            {
                proto_head_t& proto_head = it->second.head;
                ERROR_LOG("[lem:%u, cmd:%#x, id:%u, seq:%u, ret:%u]",
                        proto_head.length, proto_head.cmd, proto_head.id, proto_head.seq, 
                        proto_head.result);
                proto_head.length = sizeof(proto_head);
                proto_head.result = el::ERR_DBPROXY::TIME_OUT;


                send_ret(proto_head, it->second.peer_info, el::ERR_DBPROXY::TIME_OUT);
                this->m_proto_head_map.erase(it++);
            } 
            else 
            {
                ++it;
            }
        }
	}
protected:
	
private:
	
	std::map<uint32_t, cli_info_t> m_proto_head_map;//key:新的序号.val:原seq
	wait_db_t(const wait_db_t& cr);
	wait_db_t& operator=(const wait_db_t& cr);
};

extern wait_db_t g_wait_db;
