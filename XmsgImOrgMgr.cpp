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

#include "XmsgImOrgMgr.h"
#include "XmsgImOrgSubClientMgr.h"
#include "XmsgImOrgCfg.h"

XmsgImOrgMgr* XmsgImOrgMgr::inst = new XmsgImOrgMgr();

XmsgImOrgMgr::XmsgImOrgMgr()
{
	this->ver4usr = 1ULL;
	this->ver4dept = 1ULL;
	this->ver4deptUsr = 1ULL;
	this->seqVer4usr = 1ULL;
	this->seqVer4dept = 1ULL;
	this->seqVer4deptUsr = 1ULL;
}

XmsgImOrgMgr* XmsgImOrgMgr::instance()
{
	return XmsgImOrgMgr::inst;
}

const SptrOrgUsr XmsgImOrgMgr::addOrgUsr(SptrOrgUsr usr)
{
	string cgt = usr->cgt->toString();
	unique_lock<mutex> lock(this->lock4xx);
	this->usr[cgt] = usr;
	this->usr4dept[cgt] = shared_ptr<map<string , SptrOrgDeptUsr>>(new map<string , SptrOrgDeptUsr>());
	this->addUsrOrderByVer(usr);
	return usr;
}

const SptrOrgDept XmsgImOrgMgr::addOrgDept(SptrOrgDept dept)
{
	string dcgt = dept->cgt->toString();
	unique_lock<mutex> lock(this->lock4xx);
	this->dept[dcgt] = dept;
	this->addDeptOrderByVer(dept);
	this->dept4usr[dcgt] = shared_ptr<map<string , SptrOrgDeptUsr>>(new map<string , SptrOrgDeptUsr>());
	this->dept4dept[dcgt] = shared_ptr<map<string , SptrOrgDept>>(new map<string , SptrOrgDept>());
	if (dept->pcgt == nullptr) 
		return dept;
	string pcgt = dept->pcgt->toString();
	shared_ptr<map<string , SptrOrgDept>> mapx = nullptr;
	auto it = this->dept4dept.find(pcgt);
	if (it != this->dept4dept.end())
		mapx = it->second;
	else
	{
		mapx.reset(new map<string , SptrOrgDept>());
		this->dept4dept[pcgt] = mapx;
	}
	mapx->insert(make_pair<>(dcgt, dept)); 
	return dept;
}

const SptrOrgDeptUsr XmsgImOrgMgr::addOrgDeptUsr(SptrOrgDeptUsr deptUsr)
{
	string ucgt = deptUsr->ucgt->toString();
	string dcgt = deptUsr->dcgt->toString();
	unique_lock<mutex> lock(this->lock4xx);
	shared_ptr<map<string , SptrOrgDeptUsr>> map4usr4dept;
	auto it0 = this->usr4dept.find(ucgt);
	if (it0 != this->usr4dept.end())
		map4usr4dept = it0->second;
	else
	{
		map4usr4dept.reset(new map<string , SptrOrgDeptUsr>());
		this->usr4dept[ucgt] = map4usr4dept;
	}
	map4usr4dept->insert(make_pair<>(dcgt, deptUsr)); 
	shared_ptr<map<string , SptrOrgDeptUsr>> map4dept4usr;
	auto it1 = this->dept4usr.find(dcgt);
	if (it1 != this->dept4usr.end())
		map4dept4usr = it1->second;
	else
	{
		map4dept4usr.reset(new map<string , SptrOrgDeptUsr>());
		this->dept4usr[dcgt] = map4dept4usr;
	}
	this->addDeptUsrOrderByVer(deptUsr);
	map4dept4usr->insert(make_pair<>(ucgt, deptUsr));
	return deptUsr;
}

bool XmsgImOrgMgr::updateOrgUsr(SptrOrgUsr usr )
{
	unique_lock<mutex> lock(this->lock4xx);
	auto it = this->usr.find(usr->cgt->toString());
	if (it == this->usr.end())
	{
		LOG_ERROR("can not found usr: %s", usr->toString().c_str())
		return false;
	}
	it->second = usr;
	this->sortUsrOrderByVer();
	XmsgImOrgSubClientMgr::instance()->pubEventUsr(usr->clone());
	return true;
}

bool XmsgImOrgMgr::updateOrgDept(SptrOrgDept dept )
{
	string cgt = dept->cgt->toString();
	unique_lock<mutex> lock(this->lock4xx);
	auto it0 = this->dept.find(cgt);
	if (it0 == this->dept.end())
	{
		LOG_ERROR("can not found department: %s", dept->toString().c_str())
		return false;
	}
	it0->second = dept;
	this->sortDeptOrderByVer();
	if (dept->pcgt != nullptr)
	{
		auto it1 = this->dept4dept.find(dept->pcgt->toString());
		if (it1 == this->dept4dept.end()) 
		{
			LOG_FAULT("can not found parent department for cgt: %s", dept->pcgt->toString().c_str())
		} else
		{
			(*(it1->second))[cgt] = dept;
		}
	}
	XmsgImOrgSubClientMgr::instance()->pubEventDept(dept->clone());
	return true;
}

bool XmsgImOrgMgr::updateOrgDeptUsr(SptrOrgDeptUsr deptUsr )
{
	unique_lock<mutex> lock(this->lock4xx);
	auto dept4usr = this->dept4usr.find(deptUsr->dcgt->toString()); 
	if (dept4usr == this->dept4usr.end())
	{
		LOG_ERROR("can not found department, dept-usr: %s", deptUsr->toString().c_str())
		return false;
	}
	auto it = dept4usr->second->find(deptUsr->ucgt->toString());
	if (it == dept4usr->second->end()) 
	{
		LOG_ERROR("can not found usr in department, dept-usr: %s", deptUsr->toString().c_str())
		return false;
	}
	it->second = deptUsr;
	this->sortDeptUsrOrderByVer();
	XmsgImOrgSubClientMgr::instance()->pubEventDeptUsr(deptUsr->clone());
	return true;
}

bool XmsgImOrgMgr::disableOrgUsr(const string& ucgt)
{
	ullong now = Xsc::clock;
	unique_lock<mutex> lock(this->lock4xx);
	auto usr = this->usr.find(ucgt);
	if (usr == this->usr.end())
	{
		LOG_WARN("can not found usr for ucgt: %s", ucgt.c_str())
		return false;
	}
	auto ver = this->nextVer4usr();
	this->ver4usr = ver;
	usr->second->enable = false;
	usr->second->ver = ver;
	usr->second->uts = now;
	this->sortUsrOrderByVer();
	XmsgImOrgSubClientMgr::instance()->pubEventUsr(usr->second->clone());
	return true;
}

bool XmsgImOrgMgr::disableOrgDept(const string& dcgt)
{
	ullong now = Xsc::clock;
	unique_lock<mutex> lock(this->lock4xx);
	auto dept = this->dept.find(dcgt);
	if (dept == this->dept.end())
	{
		LOG_ERROR("can not found department for cgt: %s", dcgt.c_str())
		return false;
	}
	auto ver = this->nextVer4dept();
	this->ver4dept = ver;
	dept->second->enable = false;
	dept->second->ver = ver;
	dept->second->uts = now;
	this->sortDeptOrderByVer();
	XmsgImOrgSubClientMgr::instance()->pubEventDept(dept->second->clone());
	return true;
}

bool XmsgImOrgMgr::disableOrgDeptUsr(const string& dcgt, const string& ucgt)
{
	ullong now = Xsc::clock;
	unique_lock<mutex> lock(this->lock4xx);
	auto dept4usr = this->dept4usr.find(dcgt);
	if (dept4usr == this->dept4usr.end()) 
	{
		LOG_ERROR("can not found department, dcgt: %s, ucgt: %s", dcgt.c_str(), ucgt.c_str())
		return false;
	}
	auto it = dept4usr->second->find(ucgt);
	if (it == dept4usr->second->end()) 
	{
		LOG_ERROR("can not found usr in department, dcgt: %s, ucgt: %s", dcgt.c_str(), ucgt.c_str())
		return false;
	}
	auto ver = this->nextVer4deptUsr();
	this->ver4deptUsr = ver;
	it->second->enable = false;
	it->second->ver = ver;
	it->second->uts = now;
	this->sortDeptUsrOrderByVer();
	XmsgImOrgSubClientMgr::instance()->pubEventDeptUsr(it->second->clone());
	return true;
}

bool XmsgImOrgMgr::delOrgUsr(SptrOrgUsr usr)
{
	LOG_FAULT("it`s a bug, incomplete.")
	return true;
}

bool XmsgImOrgMgr::delOrgDept(SptrOrgDept dept)
{
	LOG_FAULT("it`s a bug, incomplete.")
	return true;
}

bool XmsgImOrgMgr::delOrgDeptUsr(SptrOrgDeptUsr deptUsr)
{
	LOG_FAULT("it`s a bug, incomplete.")
	return true;
}

int XmsgImOrgMgr::isDeptExistInPartent(const string& pcgt , const string& name)
{
	unique_lock<mutex> lock(this->lock4xx);
	auto it = this->dept4dept.find(pcgt);
	if (it == this->dept4dept.end())
		return 2;
	for (auto& iter : *it->second)
	{
		if (name == iter.second->name)
			return 1;
	}
	return 0;
}

int XmsgImOrgMgr::isUsrExistInDept(const string& dcgt, const string& name)
{
	unique_lock<mutex> lock(this->lock4xx);
	auto it = this->dept4usr.find(dcgt);
	if (it == this->dept4usr.end())
		return 2;
	for (auto& iter : *it->second)
	{
		if (name == iter.second->name)
			return 1;
	}
	return 0;
}

void XmsgImOrgMgr::setRoot(SptrOrgDept root)
{
	LOG_INFO("x-msg-im-org root node is: %s", root->toString().c_str())
	this->root = root;
}

const SptrOrgUsr XmsgImOrgMgr::findUsr(const string& cgt)
{
	unique_lock<mutex> lock(this->lock4xx);
	auto it = this->usr.find(cgt);
	return it == this->usr.end() ? nullptr : it->second;
}

const SptrOrgDept XmsgImOrgMgr::findDept(const string& cgt)
{
	unique_lock<mutex> lock(this->lock4xx);
	auto it = this->dept.find(cgt);
	return it == this->dept.end() ? nullptr : it->second;
}

const SptrOrgDeptUsr XmsgImOrgMgr::findDeptUsr(const string& dcgt, const string& ucgt)
{
	unique_lock<mutex> lock(this->lock4xx);
	auto it = this->dept4usr.find(dcgt);
	if (it == this->dept4usr.end())
		return nullptr;
	auto iter = it->second->find(ucgt);
	return iter == it->second->end() ? nullptr : iter->second;
}

void XmsgImOrgMgr::findChild(const string& cgt, list<shared_ptr<XmsgImOrgNodeChild>>& child)
{
	unique_lock<mutex> lock(this->lock4xx);
	auto it0 = this->dept4dept.find(cgt);
	if (it0 == this->dept4dept.end()) 
		return;
	for (auto& it : *it0->second)
	{
		shared_ptr<XmsgImOrgNodeChild> item(new XmsgImOrgNodeChild());
		item->set_cgt(it.second->cgt->toString());
		item->set_name(it.second->name);
		item->set_enable(it.second->enable);
		item->set_type(XmsgImOrgNodeType::X_MSG_IM_ORG_NODE_TYPE_BRANCH);
		for (auto& iter : it.second->info->kv())
			XmsgMisc::insertKv(item->mutable_info()->mutable_kv(), iter.first, iter.second);
		item->set_ver(it.second->ver);
		item->set_gts(it.second->gts);
		item->set_uts(it.second->uts);
		child.push_back(item);
	}
	auto it1 = this->dept4usr.find(cgt);
	if (it1 == this->dept4usr.end())
	{
		LOG_FAULT("it`s a bug, dept: %s", cgt.c_str())
		return;
	}
	for (auto& it : *it1->second)
	{
		shared_ptr<XmsgImOrgNodeChild> item(new XmsgImOrgNodeChild());
		item->set_cgt(it.second->ucgt->toString());
		item->set_name(it.second->name);
		item->set_enable(it.second->enable);
		item->set_type(XmsgImOrgNodeType::X_MSG_IM_ORG_NODE_TYPE_LEAF);
		for (auto& iter : it.second->info->kv())
			XmsgMisc::insertKv(item->mutable_info()->mutable_kv(), iter.first, iter.second);
		item->set_ver(it.second->ver);
		item->set_gts(it.second->gts);
		item->set_uts(it.second->uts);
		child.push_back(item);
	}
}

shared_ptr<XmsgImOrgSyncSubRsp> XmsgImOrgMgr::snapShot(SptrClient client)
{
	shared_ptr<XmsgImOrgSyncSubRsp> rsp(new XmsgImOrgSyncSubRsp());
	unique_lock<mutex> lock(this->lock4xx);
	rsp->set_ver4deptlatest(this->ver4dept);
	rsp->set_ver4deptusrlatest(this->ver4deptUsr);
	rsp->set_ver4usrlatest(this->ver4usr);
	XmsgImOrgSubClientMgr::instance()->add(client);
	return rsp;
}

void XmsgImOrgMgr::syncNextPageDept(ullong start, ullong end, list<shared_ptr<XmsgImOrgEvent>>& lis)
{
	uint count = 0;
	unique_lock<mutex> lock(this->lock4xx);
	for (auto& it : this->deptOrderByVer)
	{
		if (it->pcgt == nullptr) 
			continue;
		if (it->ver <= start || it->ver > end)
			continue;
		lis.push_back(it->toEvent()); 
		++count;
		if (count >= XmsgImOrgCfg::instance()->cfgPb->misc().orgsyncpagesize()) 
			return;
	}
}

void XmsgImOrgMgr::syncNextPageDeptUsr(ullong start, ullong end, list<shared_ptr<XmsgImOrgEvent>>& lis)
{
	uint count = 0;
	unique_lock<mutex> lock(this->lock4xx);
	for (auto& it : this->deptUsrOrderByVer)
	{
		if (it->ver <= start || it->ver > end)
			continue;
		lis.push_back(it->toEvent());
		++count;
		if (count >= XmsgImOrgCfg::instance()->cfgPb->misc().orgsyncpagesize()) 
			return;
	}
}

void XmsgImOrgMgr::syncNextPageUsr(ullong start, ullong end, list<shared_ptr<XmsgImOrgEvent>>& lis)
{
	uint count = 0;
	unique_lock<mutex> lock(this->lock4xx);
	for (auto& it : this->usrOrderByVer)
	{
		if (it->ver <= start || it->ver > end)
			continue;
		lis.push_back(it->toEvent());
		++count;
		if (count >= XmsgImOrgCfg::instance()->cfgPb->misc().orgsyncpagesize()) 
			return;
	}
}

void XmsgImOrgMgr::updateNodeInfo(const XmsgImOrgNodeInfo* from, shared_ptr<XmsgImOrgNodeInfo> to)
{
	for (auto& it : from->kv())
	{
		auto iter = to->mutable_kv()->find(it.first);
		if (iter == to->kv().end())
			XmsgMisc::insertKv(to->mutable_kv(), it.first, it.second); 
		else
			iter->second = it.second; 
	}
}

void XmsgImOrgMgr::loadCb4usr(SptrOrgUsr coll)
{
	if (coll->ver < 1)
	{
		LOG_FAULT("it`s a bug, coll: %s", coll->toString().c_str())
		Misc::lazyExit();
	}
	XmsgImOrgMgr::instance()->addOrgUsr(coll);
	XmsgImOrgMgr::instance()->ver4usr = coll->ver > XmsgImOrgMgr::instance()->ver4usr ? coll->ver : XmsgImOrgMgr::instance()->ver4usr; 
	XmsgImOrgMgr::instance()->seqVer4usr = XmsgImOrgMgr::instance()->ver4usr;
}

void XmsgImOrgMgr::loadCb4dept(SptrOrgDept coll)
{
	if (coll->ver < 1)
	{
		LOG_FAULT("it`s a bug, coll: %s", coll->toString().c_str())
		Misc::lazyExit();
	}
	if (coll->pcgt == nullptr)
		XmsgImOrgMgr::instance()->setRoot(coll);
	XmsgImOrgMgr::instance()->addOrgDept(coll);
	XmsgImOrgMgr::instance()->ver4dept = coll->ver > XmsgImOrgMgr::instance()->ver4dept ? coll->ver : XmsgImOrgMgr::instance()->ver4dept; 
	XmsgImOrgMgr::instance()->seqVer4dept = XmsgImOrgMgr::instance()->ver4dept;
}

void XmsgImOrgMgr::loadCb4deptUsr(SptrOrgDeptUsr coll)
{
	if (coll->ver < 1)
	{
		LOG_FAULT("it`s a bug, coll: %s", coll->toString().c_str())
		Misc::lazyExit();
	}
	XmsgImOrgMgr::instance()->addOrgDeptUsr(coll);
	XmsgImOrgMgr::instance()->ver4deptUsr = coll->ver > XmsgImOrgMgr::instance()->ver4deptUsr ? coll->ver : XmsgImOrgMgr::instance()->ver4deptUsr; 
	XmsgImOrgMgr::instance()->seqVer4deptUsr = XmsgImOrgMgr::instance()->ver4deptUsr;
}

size_t XmsgImOrgMgr::size4usr()
{
	unique_lock<mutex> lock(this->lock4xx);
	return this->usr.size();
}

size_t XmsgImOrgMgr::size4dept()
{
	unique_lock<mutex> lock(this->lock4xx);
	return this->dept.size();
}

void XmsgImOrgMgr::setVer4usr(ullong ver)
{
	this->ver4usr = ver;
	this->seqVer4usr = ver;
}

void XmsgImOrgMgr::setVer4dept(ullong ver)
{
	this->ver4dept = ver;
	this->seqVer4dept = ver;
}

void XmsgImOrgMgr::setVer4deptUsr(ullong ver)
{
	this->ver4deptUsr = ver;
	this->seqVer4deptUsr = ver;
}

ullong XmsgImOrgMgr::getVer4usr()
{
	return this->ver4usr;
}

ullong XmsgImOrgMgr::getVer4dept()
{
	return this->ver4dept;
}

ullong XmsgImOrgMgr::getVer4deptUsr()
{
	return this->ver4deptUsr;
}

ullong XmsgImOrgMgr::nextVer4usr()
{
	return ++(this->seqVer4usr);
}

ullong XmsgImOrgMgr::nextVer4dept()
{
	return ++(this->seqVer4dept);
}

ullong XmsgImOrgMgr::nextVer4deptUsr()
{
	return ++(this->seqVer4deptUsr);
}

SptrOrgDept XmsgImOrgMgr::getRoot()
{
	return this->root;
}

void XmsgImOrgMgr::addDeptOrderByVer(SptrOrgDept dept)
{
	if (this->deptOrderByVer.empty())
	{
		this->deptOrderByVer.push_back(dept);
		return;
	}
	if (dept->ver < (*(this->deptOrderByVer.rbegin()))->ver) 
	{
		this->deptOrderByVer.push_back(dept);
		this->sortDeptOrderByVer();
		return;
	}
	this->deptOrderByVer.push_back(dept);
}

void XmsgImOrgMgr::addDeptUsrOrderByVer(SptrOrgDeptUsr depUsr)
{
	if (this->deptUsrOrderByVer.empty())
	{
		this->deptUsrOrderByVer.push_back(depUsr);
		return;
	}
	if (depUsr->ver < (*(this->deptUsrOrderByVer.rbegin()))->ver) 
	{
		this->deptUsrOrderByVer.push_back(depUsr);
		this->sortDeptUsrOrderByVer();
		return;
	}
	this->deptUsrOrderByVer.push_back(depUsr);
}

void XmsgImOrgMgr::addUsrOrderByVer(SptrOrgUsr usr)
{
	if (this->usrOrderByVer.empty())
	{
		this->usrOrderByVer.push_back(usr);
		return;
	}
	if (usr->ver < (*(this->usrOrderByVer.rbegin()))->ver) 
	{
		this->usrOrderByVer.push_back(usr);
		this->sortUsrOrderByVer();
		return;
	}
	this->usrOrderByVer.push_back(usr);
}

void XmsgImOrgMgr::sortDeptOrderByVer()
{
	this->deptOrderByVer.sort([](auto a, auto b)
	{
		return a->ver < b->ver;
	});
}

void XmsgImOrgMgr::sortDeptUsrOrderByVer()
{
	this->deptUsrOrderByVer.sort([](auto a, auto b)
	{
		return a->ver < b->ver;
	});
}

void XmsgImOrgMgr::sortUsrOrderByVer()
{
	this->usrOrderByVer.sort([](auto a, auto b)
	{
		return a->ver < b->ver;
	});
}

XmsgImOrgMgr::~XmsgImOrgMgr()
{

}

