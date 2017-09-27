#include <set>

#include <lib_log.h>

#include "route.h"

route_t g_rotue_t;

namespace {
	const char* ROUTE_XML_PATH = "./route.xml";

	int parser_ser(xmlNodePtr cur, el::lib_xmlparser_t& xml, service_mgr_t* service_mgr){
		cur = cur->xmlChildrenNode;
		while(NULL != cur){
			if(!xmlStrcmp(cur->name, (const xmlChar*)"date")){
				service_t service;
				xml.get_xml_prop(cur, service.ip, "ip");
				xml.get_xml_prop(cur, service.port, "port");
				service_mgr->service_vec.push_back(service);
			}
			cur = cur->next;
		}
		return 0;
	}
}

int route_t::parser()
{
	int ret = this->xml.open(ROUTE_XML_PATH);
	if (0 != ret){
		return ret;
	}

	this->xml.move2children_node();
	while(NULL != this->xml.node_ptr){
		//取出节点中的内容
		if (!xmlStrcmp(this->xml.node_ptr->name, (const xmlChar*)"ser")){
			service_mgr_t service_mgr;
			xmlNodePtr cur = this->xml.node_ptr;

			std::string str_start;
			std::string str_end;

			xml.get_xml_prop(cur, str_start, "start");
			service_mgr.start = ::strtoul(str_start.c_str(), 0, 16);

			xml.get_xml_prop(cur, str_end, "end");
			service_mgr.end = ::strtoul(str_end.c_str(), 0, 16);

			if (service_mgr.start > service_mgr.end){
				assert(0);
				return -1;
			}

			parser_ser(cur, this->xml, &service_mgr);
			g_rotue_t.service_mgr_vec.push_back(service_mgr);
		}
		this->xml.move2next_node();
	}

	return 0;
}
