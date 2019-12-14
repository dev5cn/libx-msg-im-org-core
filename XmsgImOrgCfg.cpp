/*
  Copyright 2019 www.dev5.cn, Inc. dev5@qq.com
 
  This file is part of X-MSG-IM.
 
  X-MSG-IM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  X-MSG-IM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with X-MSG-IM.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <libx-msg-common-dat-struct-cpp.h>
#include "XmsgImOrgCfg.h"

shared_ptr<XmsgImOrgCfg> XmsgImOrgCfg::cfg; 

XmsgImOrgCfg::XmsgImOrgCfg()
{

}

shared_ptr<XmsgImOrgCfg> XmsgImOrgCfg::instance()
{
	return XmsgImOrgCfg::cfg;
}

shared_ptr<XmsgImOrgCfg> XmsgImOrgCfg::load(const char* path)
{
	XMLDocument doc;
	if (doc.LoadFile(path) != 0)
	{
		LOG_ERROR("load config file failed, path: %s", path)
		return nullptr;
	}
	XMLElement* root = doc.RootElement();
	if (root == NULL)
	{
		LOG_ERROR("it a empty xml file? path: %s", path)
		return nullptr;
	}
	shared_ptr<XmsgImOrgCfg> cfg(new XmsgImOrgCfg());
	cfg->cfgPb.reset(new XmsgImOrgCfgPb());
	cfg->cgt = ChannelGlobalTitle::parse(Misc::strAtt(root, "cgt"));
	if (cfg->cgt == nullptr)
	{
		LOG_ERROR("load config file failed, cgt format error: %s", Misc::strAtt(root, "cgt").c_str())
		return nullptr;
	}
	cfg->cfgPb->set_cgt(cfg->cgt->toString());
	Misc::strAtt(root, "cfgType", cfg->cfgPb->mutable_cfgtype());
	if ("mongodb" == cfg->cfgPb->cfgtype())
	{
		XMLElement* dbUri = root->FirstChildElement("mongodb");
		auto mongodb = cfg->cfgPb->mutable_mongodb();
		Misc::strAtt(dbUri, "uri", mongodb->mutable_uri());
		XmsgImOrgCfg::setCfg(cfg); 
		return cfg;
	}
	if ("mysql" == cfg->cfgPb->cfgtype())
	{
		LOG_ERROR("not supported mysql.")
		return nullptr;
	}
	if ("mongodb" == Misc::strAtt(root, "db") && !cfg->loadMongodbCfg(root))
		return nullptr;
	if ("mysql" == Misc::strAtt(root, "db") && !cfg->loadMysqlCfg(root))
		return nullptr;
	if (!cfg->loadLogCfg(root))
		return nullptr;
	if (!cfg->loadXscServerCfg(root))
		return nullptr;
	if (!cfg->loadXmsgNeH2nCfg(root))
		return nullptr;
	if (!cfg->loadXmsgNeN2hCfg(root))
		return nullptr;
	if (!cfg->loadMiscCfg(root))
		return nullptr;
	LOG_INFO("load config file successful, cfg: %s", cfg->toString().c_str())
	XmsgImOrgCfg::setCfg(cfg);
	return cfg;
}

void XmsgImOrgCfg::setCfg(shared_ptr<XmsgImOrgCfg> cfg)
{
	XmsgImOrgCfg::cfg = cfg;
}

shared_ptr<XscTcpCfg> XmsgImOrgCfg::xscServerCfg()
{
	shared_ptr<XscTcpCfg> tcpCfg(new XscTcpCfg());
	tcpCfg->addr = this->cfgPb->xsctcpcfg().addr();
	tcpCfg->worker = this->cfgPb->xsctcpcfg().worker();
	tcpCfg->peerLimit = this->cfgPb->xsctcpcfg().peerlimit();
	tcpCfg->peerMtu = this->cfgPb->xsctcpcfg().peermtu();
	tcpCfg->peerRcvBuf = this->cfgPb->xsctcpcfg().peerrcvbuf();
	tcpCfg->peerSndBuf = this->cfgPb->xsctcpcfg().peersndbuf();
	tcpCfg->lazyClose = this->cfgPb->xsctcpcfg().lazyclose();
	tcpCfg->tracing = this->cfgPb->xsctcpcfg().tracing();
	tcpCfg->heartbeat = this->cfgPb->xsctcpcfg().heartbeat();
	tcpCfg->n2hZombie = this->cfgPb->xsctcpcfg().n2hzombie();
	tcpCfg->n2hTransTimeout = this->cfgPb->xsctcpcfg().n2htranstimeout();
	tcpCfg->n2hTracing = this->cfgPb->xsctcpcfg().n2htracing();
	tcpCfg->h2nReConn = this->cfgPb->xsctcpcfg().h2nreconn();
	tcpCfg->h2nTransTimeout = this->cfgPb->xsctcpcfg().h2ntranstimeout();
	return tcpCfg;
}

bool XmsgImOrgCfg::loadLogCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("log");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <log>");
		return false;
	}
	XmsgImOrgCfgLog* log = this->cfgPb->mutable_log();
	log->set_level(Misc::toUpercase(Misc::strAtt(node, "level")));
	log->set_output(Misc::toUpercase(Misc::strAtt(node, "output")));
	return true;
}

bool XmsgImOrgCfg::loadXscServerCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("xsc-server");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <xsc-server>")
		return false;
	}
	XmsgImOrgCfgXscTcpServer* tcpCfg = this->cfgPb->mutable_xsctcpcfg();
	tcpCfg->set_addr(Misc::strAtt(node, "addr"));
	tcpCfg->set_worker(Misc::hexOrInt(node, "worker"));
	tcpCfg->set_peerlimit(Misc::hexOrInt(node, "peerLimit"));
	tcpCfg->set_peermtu(Misc::hexOrInt(node, "peerMtu"));
	tcpCfg->set_peerrcvbuf(Misc::hexOrInt(node, "peerRcvBuf"));
	tcpCfg->set_peersndbuf(Misc::hexOrInt(node, "peerSndBuf"));
	tcpCfg->set_lazyclose(Misc::hexOrInt(node, "lazyClose"));
	tcpCfg->set_tracing("true" == Misc::strAtt(node, "tracing"));
	tcpCfg->set_heartbeat(Misc::hexOrInt(node, "heartbeat"));
	tcpCfg->set_n2hzombie(Misc::hexOrInt(node, "n2hZombie"));
	tcpCfg->set_n2htranstimeout(Misc::hexOrInt(node, "n2hTransTimeout"));
	tcpCfg->set_n2htracing("true" == Misc::strAtt(node, "n2hTracing"));
	tcpCfg->set_h2nreconn(Misc::hexOrInt(node, "h2nReConn"));
	tcpCfg->set_h2ntranstimeout(Misc::hexOrInt(node, "h2nTransTimeout"));
	return true;
}

bool XmsgImOrgCfg::loadXmsgNeH2nCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("ne-group-h2n");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <ne-group-h2n>")
		return false;
	}
	node = node->FirstChildElement("ne");
	while (node != NULL)
	{
		auto ne = this->cfgPb->add_h2n();
		Misc::strAtt(node, "neg", ne->mutable_neg());
		Misc::strAtt(node, "addr", ne->mutable_addr());
		Misc::strAtt(node, "pwd", ne->mutable_pwd());
		Misc::strAtt(node, "alg", ne->mutable_alg());
		if (ne->neg().empty() || ne->addr().empty() || ne->alg().empty())
		{
			LOG_ERROR("load config failed, node: <ne-group-h2n><ne>, ne: %s", ne->ShortDebugString().c_str())
			return false;
		}
		node = node->NextSiblingElement("ne");
	}
	if (this->cfgPb->h2n().empty())
	{
		LOG_ERROR("load config failed, node: <ne-group-h2n><ne>")
		return false;
	}
	return true;
}

bool XmsgImOrgCfg::loadXmsgNeN2hCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("ne-group-n2h");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: ne-group-n2h")
		return false;
	}
	node = node->FirstChildElement("ne");
	while (node != NULL)
	{
		auto ne = this->cfgPb->add_n2h();
		Misc::strAtt(node, "neg", ne->mutable_neg());
		Misc::strAtt(node, "cgt", ne->mutable_cgt());
		Misc::strAtt(node, "pwd", ne->mutable_pwd());
		Misc::strAtt(node, "addr", ne->mutable_addr());
		SptrCgt cgt = ChannelGlobalTitle::parse(ne->cgt());
		if (ne->neg().empty() || ne->cgt().empty() || ne->pwd().empty() || cgt == nullptr)
		{
			LOG_ERROR("load config failed, node: <ne-group-n2h><ne>, ne: %s", ne->ShortDebugString().c_str())
			return false;
		}
		node = node->NextSiblingElement("ne");
	}
	if (this->cfgPb->n2h().empty())
	{
		LOG_ERROR("load config failed, node: <ne-group-n2h><ne>")
		return false;
	}
	return true;
}

bool XmsgImOrgCfg::loadMysqlCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("mysql");
	if (node == nullptr)
	{
		LOG_ERROR("load config failed, node: <mysql>")
		return false;
	}
	string host;
	auto mysql = this->cfgPb->mutable_mysql();
	Misc::strAtt(node, "host", &host);
	Misc::strAtt(node, "db", mysql->mutable_db());
	Misc::strAtt(node, "usr", mysql->mutable_usr());
	Misc::strAtt(node, "password", mysql->mutable_password());
	mysql->set_poolsize(Misc::hexOrInt(node, "poolSize"));
	int port;
	if (!Net::str2ipAndPort(host.c_str(), mysql->mutable_host(), &port))
	{
		LOG_ERROR("load config failed, node: <mysql>, host format error: %s", host.c_str())
		return false;
	}
	mysql->set_port(port);
	if (mysql->db().empty() || mysql->usr().empty() || mysql->password().empty())
	{
		LOG_ERROR("load config failed, node: <mysql>")
		return false;
	}
	return true;
}

bool XmsgImOrgCfg::loadMongodbCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("mongodb");
	if (node == nullptr)
	{
		LOG_ERROR("load config failed, node: <mongodb>")
		return false;
	}
	auto mongodb = this->cfgPb->mutable_mongodb();
	Misc::strAtt(node, "uri", mongodb->mutable_uri());
	if (mongodb->uri().empty())
	{
		LOG_ERROR("load config failed, node: <mongodb>")
		return false;
	}
	return true;
}

bool XmsgImOrgCfg::loadMiscCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("misc");
	if (node == NULL)
	{
		LOG_ERROR("load  config failed, node: <misc>")
		return false;
	}
	auto misc = this->cfgPb->mutable_misc();
	string hlr = Misc::strAtt(node, "hlr");
	if (hlr.empty())
	{
		LOG_ERROR("load config failed, node: <misc><hlr>")
		return false;
	}
	this->hlr = ChannelGlobalTitle::parse(hlr);
	if (this->hlr == nullptr)
	{
		LOG_ERROR("load config failed, hlr channel global title format error: %s", hlr.c_str())
		return false;
	}
	misc->set_hlr(this->hlr->toString());
	misc->set_orgsyncpagesize(Misc::hexOrInt(node, "orgSyncPageSize"));
	misc->set_orgsyncpagesize(misc->orgsyncpagesize() < 1 ? 1 : misc->orgsyncpagesize());
	return true;
}

string XmsgImOrgCfg::toString()
{
	return this->cfgPb->ShortDebugString();
}

XmsgImOrgCfg::~XmsgImOrgCfg()
{

}

