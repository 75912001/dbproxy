#include <lib_log.h>

#include "dbproxy_timer.h"
#include "wait_db.h"

dbproxy_timer_t* g_dbproxy_timer;

int dbproxy_timer_t::clear( void* owner, void* data )
{
	//����ʱЭ�鷵��
	g_wait_db.clis_req_time_out();
	g_dbproxy_timer->add_sec_event(dbproxy_timer_t::clear, NULL, el_async::get_now_sec() + 3);
	return 0;
}

dbproxy_timer_t::dbproxy_timer_t() : el::lib_timer_base_t(g_timer)
{

}
