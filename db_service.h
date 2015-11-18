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

class db_service_t
{
public:
	db_service_t()
	{
		this->peer = NULL;
	}

	db_service_t( const db_service_t& cr )
	{
		this->name = cr.name;
		this->peer = cr.peer;
	}
	virtual ~db_service_t(){}
	std::string name;
	el::lib_tcp_peer_info_t* peer;	
protected:
	
private:

	db_service_t& operator=(const db_service_t& cr);

};

class service_mgr_t
{
public:
	static service_mgr_t& instance(){
		static service_mgr_t s;
		return s;
	}
	virtual ~service_mgr_t(){}
	int add( const std::string name, el::lib_tcp_peer_info_t* peer )
	{
		auto it = this->service_map.find(peer->get_fd());
		if (this->service_map.end() != it){
			ERROR_LOG("service already exist[fd:%u, name:%s]", peer->get_fd(), name.c_str());
			return -1;
		}

		auto it_name = this->service_name_map.find(name);
		if (this->service_name_map.end() != it_name){
			ERROR_LOG("service already exist[fd:%u, name:%s]", peer->get_fd(), name.c_str());
			return -1;
		}
		db_service_t info;
		std::string s;
		s = name;
		info.name = name;
		info.peer = peer;
		this->service_map.insert(std::make_pair(peer->get_fd(), info));
		this->service_name_map.insert(std::make_pair(name, info));
		return 0;
	}
	int remove( int fd )
	{
		auto it = this->service_map.find(fd);
		if (this->service_map.end() == it){
			ERROR_LOG("service inexist[fd:%u]", fd);
			return -1;
		}
		std::string name = it->second.name;

		this->service_map.erase(fd);
		this->service_name_map.erase(name);

		return 0;
	}
	inline db_service_t* get_service(const std::string& name){
		auto it = this->service_name_map.find(name);
		if (this->service_name_map.end() != it){
			return &it->second;
		}
		return NULL;
	}
	
protected:
	service_mgr_t(){}
private:
	service_mgr_t(const service_mgr_t& cr);
	service_mgr_t& operator=(const service_mgr_t& cr);
	typedef std::map<int, db_service_t> SERVICE_MAP;//key:fd, val:服务信息
	SERVICE_MAP service_map;
	typedef std::map<std::string, db_service_t> SERVICE_NAME_MAP;
	SERVICE_NAME_MAP service_name_map;
};

#define g_service_mgr service_mgr_t::instance()
