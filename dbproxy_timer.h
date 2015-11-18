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
	virtual ~dbproxy_timer_t();

	/**
	 * @brief	定时处理超时协议
	 * @param	void * owner
	 * @param	void * data
	 * @return	int
	 */
	static int clear(void* owner, void* data);
protected:
private:

	dbproxy_timer_t(const dbproxy_timer_t& cr);
	dbproxy_timer_t& operator=(const dbproxy_timer_t& cr);
};

extern dbproxy_timer_t* g_dbproxy_timer;
/*extern el::lib_timer_t g_timer;*/
