/********************************************************************
	platform:	
	author:		kevin
	copyright:	All rights reserved.
	purpose:	
	brief:		¬∑”…√¸¡Ó”Îdb
*********************************************************************/
#pragma once

#include <lib_include.h>
#include <lib_xmlparser.h>
#include <lib_util.h>
#include <lib_log.h>
#include <common.h>
#include "db_service.h"

class route_t
{
public:
	std::vector<service_mgr_t> service_mgr_vec;
	int parser();
	service_mgr_t* find(CMD_ID cmd){
		FOREACH(this->service_mgr_vec, it){
			service_mgr_t& r = *it;
			if (r.start <= cmd && cmd <= r.end){
				return &r;
			}
		}
		ERROR_LOG("not find cmd:0x%#x", cmd);
		return NULL;
	}
private:
	el::lib_xmlparser_t xml;
};

extern route_t g_rotue_t;
