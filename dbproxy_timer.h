/********************************************************************
	platform:	
	author:		kevin
	copyright:	All rights reserved.
	purpose:	
	brief:		dbproxy_timer
*********************************************************************/

#pragma once

#include <lib_timer.h>

class dbproxy_timer_t  : public el::lib_timer_base_t
{
public:
	dbproxy_timer_t();

	/**
	 * @brief	定时处理超时协议
	 * @param	void * owner
	 * @param	void * data
	 * @return	int
	 */
	static int clear(void* owner, void* data);
};

extern dbproxy_timer_t* g_dbproxy_timer;
