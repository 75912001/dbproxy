#include <set>

#include <lib_log.h>

#include "route.h"

route_t g_rotue_t;

namespace {
	const char* ROUTE_XML_PATH = "./route.xml";

	std::set<route_cmd_t> tmp_parser;//临时变量(用于解析 ROUTE_XML_PATH 时使用)

	int parser_cmd(xmlNodePtr cur, el::lib_xmlparser_t& xml, uint32_t db_type){
		tmp_parser.clear();

		cur = cur->xmlChildrenNode;
		while (NULL != cur){
			if (!xmlStrcmp(cur->name,(const xmlChar*)"date")){
				route_cmd_t info;
				info.db_type = db_type;

				std::string str_start;
				std::string str_end;

				xml.get_xml_prop(cur, str_start, "start");
				info.start = ::strtoul(str_start.c_str(), 0, 16);

				xml.get_xml_prop(cur, str_end, "end");
				info.end = ::strtoul(str_end.c_str(), 0, 16);

				if (info.start > info.end){
					assert(0);
					return -1;
				}

				//设置最大//最小命令号
				if (0 == g_rotue_t.get_cmd_min()){
					g_rotue_t.set_cmd_min(info.start);
				}
				if (info.start < g_rotue_t.get_cmd_min()){
					g_rotue_t.set_cmd_min(info.start);
				}

				if (0 == g_rotue_t.get_cmd_max()){
					g_rotue_t.set_cmd_max(info.end);
				}
				if (info.end > g_rotue_t.get_cmd_max()){
					g_rotue_t.set_cmd_max(info.end);
				}			

				DB_SER dbser;
				if (!g_rotue_t.cmd_map.insert(std::make_pair(info, dbser)).second
					|| !tmp_parser.insert(info).second){
					assert(0);
					return -1;
				}
			}
			cur = cur->next;
		}
		return 0;
	}

	int parser_db(xmlNodePtr cur, el::lib_xmlparser_t& xml){
		cur = cur->xmlChildrenNode;
		while (NULL != cur){
			if (!xmlStrcmp(cur->name,(const xmlChar*)"date")){
				route_db_t rd;
				xml.get_xml_prop(cur, rd.name, "name");
				xml.get_xml_prop(cur, rd.ip, "ip");
				xml.get_xml_prop(cur, rd.port, "port");
				xml.get_xml_prop(cur, rd.start, "start");
				xml.get_xml_prop(cur, rd.end, "end");

				FOREACH(g_rotue_t.cmd_map, it)
                {
					DB_SER& dbser = it->second;
					const route_cmd_t& cmdinfo = it->first;
					FOREACH(tmp_parser, tmp_it)
                    {
						const route_cmd_t& tmp_dbser = *tmp_it;
						if (cmdinfo == tmp_dbser)
                        {
							if (!dbser.insert(rd).second)
                            {
								assert(0);
								return -1;
							}
						}
					}
				}
			}
			cur = cur->next;
		}

		tmp_parser.clear();
		return 0;
	}

	int parser_dbser(xmlNodePtr cur, el::lib_xmlparser_t& xml, uint32_t db_type){
		cur = cur->xmlChildrenNode;
		while(NULL != cur){
			if(!xmlStrcmp(cur->name, (const xmlChar*)"cmd")){
				parser_cmd(cur, xml, db_type);
			} else if(!xmlStrcmp(cur->name, (const xmlChar*)"db")){
				parser_db(cur, xml);
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
	while(NULL != this->xml.get_node_ptr()){
		//取出节点中的内容
		if (!xmlStrcmp(this->xml.get_node_ptr()->name, (const xmlChar*)"dbser")){
			xmlNodePtr cur = this->xml.get_node_ptr();

			uint32_t db_type = 0;
			this->xml.get_xml_prop(cur, db_type, "type");
			assert(db_type > 0);

			parser_dbser(cur, this->xml, db_type);
		}
		this->xml.move2next_node();
	}

	FOREACH(this->cmd_map, it){
		const route_cmd_t& cmdinfo = it->first;
		DB_SER& dbser = it->second;
		FOREACH(dbser, it_dbser){
            const route_db_t& rdb = *it_dbser;
            DEBUG_LOG("load conf [cmd:%x-%x db_type:%u ip:%s port:%u]", cmdinfo.start, cmdinfo.end,
                    cmdinfo.db_type, rdb.ip.c_str(), rdb.port);
		}
	}

	return 0;
}
