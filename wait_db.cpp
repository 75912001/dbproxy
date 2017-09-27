#include "wait_db.h"

uint32_t wait_db_t::gen_save_key( proto_head_t& proto_head, el::lib_tcp_peer_info_t* peer_info )
{
	this->key++;

	cli_info_t ci;
	ci.head = proto_head;
	ci.peer_info = peer_info;

	if(!m_proto_head_map.insert(std::make_pair(this->key, ci)).second)
	{
		ERROR_LOG("[body_len:%u, cmd:%#x, seq:%u, ret:%u, key:%u]",
			proto_head.length, proto_head.cmd, proto_head.seq, 
			proto_head.result, this->key);
		return 0;
	}

	return this->key;
}

bool wait_db_t::get_old_seq( uint32_t seq, cli_info_t& rci )
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

void wait_db_t::clear_cli_info( int fd )
{
	for (std::map<uint32_t, cli_info_t>::iterator it = this->m_proto_head_map.begin();
		this->m_proto_head_map.end() != it;){
		if (it->second.peer_info->fd == fd){
			this->m_proto_head_map.erase(it++);
		} else {
			++it;
		}
	}
}

void wait_db_t::clis_req_time_out()
{
	uint32_t now_sec = g_timer->now_sec();
	/*TRACE_LOG("[now sec:%lu, time_out_sec:%lu]", g_timer->get_now_sec(), this->time_out_sec);*/
	for (std::map<uint32_t, cli_info_t>::iterator it = this->m_proto_head_map.begin();
		this->m_proto_head_map.end() != it;){
		if (now_sec >= it->second.add_time + this->time_out_sec){
			proto_head_t& proto_head = it->second.head;
			ERROR_LOG("[lem:%u, cmd:%#x, seq:%u, ret:%u]",
				proto_head.length, proto_head.cmd, proto_head.seq, 
				proto_head.result);
			proto_head.length = sizeof(proto_head);
			proto_head.result = el::ERR_DBPROXY::TIME_OUT;

			g_send_ret(proto_head, it->second.peer_info, el::ERR_DBPROXY::TIME_OUT);
			this->m_proto_head_map.erase(it++);
		} else {
			++it;
		}
	}
}
