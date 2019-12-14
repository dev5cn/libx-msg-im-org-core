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

#include "XmsgImOrgSubClientMgr.h"

XmsgImOrgSubClientMgr* XmsgImOrgSubClientMgr::inst = new XmsgImOrgSubClientMgr();

XmsgImOrgSubClientMgr::XmsgImOrgSubClientMgr()
{

}

XmsgImOrgSubClientMgr* XmsgImOrgSubClientMgr::instance()
{
	return XmsgImOrgSubClientMgr::inst;
}

void XmsgImOrgSubClientMgr::init()
{
	this->abst.reset(new ActorBlockingSingleThread("org-sub"));
}

void XmsgImOrgSubClientMgr::add(SptrClient client)
{
	unique_lock<mutex> lock(this->lock4client);
	this->client[client->apCcid] = client;
}

bool XmsgImOrgSubClientMgr::del(SptrClient client)
{
	unique_lock<mutex> lock(this->lock4client);
	return this->client.erase(client->apCcid) == 1;
}

bool XmsgImOrgSubClientMgr::del(const string& apCcid)
{
	unique_lock<mutex> lock(this->lock4client);
	return this->client.erase(apCcid) == 1;
}

bool XmsgImOrgSubClientMgr::existed(SptrClient client)
{
	unique_lock<mutex> lock(this->lock4client);
	auto it = this->client.find(client->apCcid);
	return it != this->client.end();
}

SptrClient XmsgImOrgSubClientMgr::find(const string& apCcid)
{
	unique_lock<mutex> lock(this->lock4client);
	auto it = this->client.find(apCcid);
	return it == this->client.end() ? nullptr : it->second;
}

void XmsgImOrgSubClientMgr::pubEventUsr(SptrOrgUsr usr)
{
	this->abst->future([usr]
	{
		XmsgImOrgSubClientMgr::instance()->pubEvent(usr->toEvent());
	});
}

void XmsgImOrgSubClientMgr::pubEventDept(SptrOrgDept dept)
{
	this->abst->future([dept]
	{
		XmsgImOrgSubClientMgr::instance()->pubEvent(dept->toEvent());
	});
}

void XmsgImOrgSubClientMgr::pubEventDeptUsr(SptrOrgDeptUsr deptUsr)
{
	this->abst->future([deptUsr]
	{
		XmsgImOrgSubClientMgr::instance()->pubEvent(deptUsr->toEvent());
	});
}

void XmsgImOrgSubClientMgr::pubEvent(shared_ptr<XmsgImOrgEvent> event)
{
	list<SptrClient> lis; 
	unique_lock<mutex> lock(XmsgImOrgSubClientMgr::instance()->lock4client);
	for (auto& it : XmsgImOrgSubClientMgr::instance()->client) 
		lis.push_back(it.second);
	lock.unlock();
	LOG_INFO("have %zu subscribers for org real-time event, event: %s", lis.size(), event->ShortDebugString().c_str())
	shared_ptr<XmsgImOrgChangedNotice> notice(new XmsgImOrgChangedNotice());
	notice->mutable_event()->CopyFrom(*event);
	for (auto& it : lis)
	{
		auto client = it;
		client->future([client, notice]
		{
			client->sendNotice(notice);
		});
	}
}

XmsgImOrgSubClientMgr::~XmsgImOrgSubClientMgr()
{

}

