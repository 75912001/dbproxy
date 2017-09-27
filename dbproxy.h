/********************************************************************
	platform:	
	author:		kevin
	copyright:	All rights reserved.
	purpose:	
	brief:		dbproxy处理
*********************************************************************/

#pragma once

#include <lib_include.h>
#include <service_if.h>
#include "proto_header.h"

class dbproxy_t
{
public:
	dbproxy_t();
	int handle_cli(const void* data, uint32_t len, el::lib_tcp_peer_info_t* peer_fd_info);
	int handle_srv(const void* data, uint32_t len, el::lib_tcp_peer_info_t* peer_fd_info);
	proto_head_t head;
	int ret;
};

extern dbproxy_t g_dbproxy;
extern server_send_data_t g_send_cli_buf;



/**
 * @brief	发送错误码给cli
 * @param   proto_head_t & head
 * @param   el::lib_tcp_peer_info_t * peer_fd_info
 * @param	uint32_t ret
 * @return	int
 */
extern int g_send_ret(proto_head_t& head, el::lib_tcp_peer_info_t* peer_fd_info, uint32_t ret);
