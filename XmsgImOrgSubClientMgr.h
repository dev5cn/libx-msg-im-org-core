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

#ifndef XMSGIMORGSUBCLIENTMGR_H_
#define XMSGIMORGSUBCLIENTMGR_H_

#include "XmsgImOrgMgr.h"
#include "ne/XmsgAp.h"
#include "usr/XmsgImClient.h"

class XmsgImOrgSubClientMgr
{
public:
	void add(SptrClient client); 
	bool del(SptrClient client); 
	bool del(const string& apCcid); 
	bool existed(SptrClient client); 
	SptrClient find(const string& apCcid); 
	void pubEventUsr(SptrOrgUsr usr); 
	void pubEventDept(SptrOrgDept dept); 
	void pubEventDeptUsr(SptrOrgDeptUsr deptUsr); 
	void init(); 
	static XmsgImOrgSubClientMgr* instance();
private:
	unordered_map<string , SptrClient> client; 
	mutex lock4client; 
	shared_ptr<ActorBlockingSingleThread> abst;
	static XmsgImOrgSubClientMgr* inst;
	void pubEvent(shared_ptr<XmsgImOrgEvent> event); 
	XmsgImOrgSubClientMgr();
	virtual ~XmsgImOrgSubClientMgr();
};

#endif 
