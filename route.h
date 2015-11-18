/********************************************************************
	platform:	
	author:		kevin
	copyright:	All rights reserved.
	purpose:	
	brief:		路由命令与db
*********************************************************************/
#pragma once

#include <lib_include.h>
#include <lib_xmlparser.h>
#include <lib_util.h>
#include <lib_log.h>
#include <common.h>

#pragma pack(1)

struct route_cmd_t
{
	uint32_t start;
	uint32_t end;
	uint32_t db_type;
	route_cmd_t(){
		this->start = 0;
		this->end = 0;
		this->db_type = 0;
	}
	bool operator < (const route_cmd_t& r) const{
		return this->end < r.start;
	}
 	bool operator == (const route_cmd_t& r) const{
 		return r.start == this->start && r.end == this->end && r.db_type == this->db_type;
 	}
};

struct route_db_t
{
	uint32_t start;
	uint32_t end;
	std::string name;
	std::string ip;
	uint16_t port;
	route_db_t(){
		this->name.clear();
		this->ip.clear();
		this->start = 0;
		this->end = 0;
		this->port = 0;
	}
	bool operator < (const route_db_t& r) const{
		return this->end < r.start;
	}
	bool operator == (const route_db_t& r) const{
		return r.start == this->start && r.end == this->end;
	}
};

#pragma pack()

enum e_db_type{
	E_DB_TYPE_1 = 1,
	E_DB_TYPE_100 = 100,
};

typedef std::set<route_db_t> DB_SER;
typedef std::map<route_cmd_t, DB_SER> CMD_MAP;//key:cmd范围, val:dbser列表

class route_t
{
	PRIVATE(el::lib_xmlparser_t, xml);
	PRIVATE_RW(uint32_t, cmd_min);
	PRIVATE_RW(uint32_t, cmd_max);
public:

	CMD_MAP cmd_map;
public:
	route_t(){
		this->cmd_min = 0;
		this->cmd_max = 0;
	}
	virtual ~route_t(){}
	int parser();
	const route_db_t* find_dbser(uint32_t cmd, USER_ID id, uint32_t& db_type){
		const DB_SER* p_db_ser = this->find_cmd(cmd, db_type);
		if (NULL != p_db_ser){
			return this->find_route_db(p_db_ser, id, db_type);
			//if (E_DB_TYPE_100 == db_type){
			//	const route_db_t* p = this->find_route_db_100(p_db_ser, id);
			//	//TRACE_LOG("xxx cmd:%#X", cmd);
			//	return p;
			//} else if (E_DB_TYPE_1 == db_type){
			//	const route_db_t* p = this->find_route_db_1(p_db_ser, id);
			//	//TRACE_LOG("xxx cmd:%#X", cmd);
			//	return p;
			//} else {
			//	ERROR_LOG("can not find [id:%u, db_type:%u]", id, db_type);
			//}
		} else {
			ERROR_LOG("can not find [id:%u, cmd:%#x]", id, cmd);
		}
		return NULL;
	}
protected:
	
private:
    /*const route_db_t* find_route_db_100(const DB_SER* db_ser, uint32_t id){*/
    /*return this->find_route_db(db_ser, id, E_DB_TYPE_100);*/
    /*}*/
    /*const route_db_t* find_route_db_1(const DB_SER* db_ser, uint32_t id){*/
    /*return this->find_route_db(db_ser, id, E_DB_TYPE_1);*/
    /*}*/
	const route_db_t* find_route_db(const DB_SER* db_ser, USER_ID id, uint32_t db_type){
		uint32_t idx = id % db_type;
		FOREACH(*db_ser, it){
			const route_db_t& rdb = *it;
			if (rdb.start <= idx && idx <= rdb.end){
				return &rdb;
			}
		}
		ERROR_LOG("can not find [idx:%u, id:%u, db_type:%u]", idx, id, db_type);
		return NULL;
	}
	const DB_SER* find_cmd(uint32_t cmd, uint32_t& db_type){
		FOREACH(this->cmd_map, it){
			const route_cmd_t& rct = it->first;
			if (rct.start <= cmd && cmd <= rct.end){
				db_type = rct.db_type;
                DEBUG_LOG("cmd:%#x, type:%u", cmd, rct.db_type);
				return &(it->second);
			}
		}
		ERROR_LOG("can not find [cmd:%#x]", cmd);
		return NULL;
	}

	route_t(const route_t& cr);
	route_t& operator=(const route_t& cr);
};

extern route_t g_rotue_t;
