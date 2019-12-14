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

#ifndef XMSGIMORGMGR_H_
#define XMSGIMORGMGR_H_

#include "coll/XmsgImOrgDeptColl.h"
#include "coll/XmsgImOrgDeptUsrColl.h"
#include "coll/XmsgImOrgUsrColl.h"
#include "usr/XmsgImClient.h"

class XmsgImOrgMgr
{
public:
	const SptrOrgUsr addOrgUsr(SptrOrgUsr usr); 
	const SptrOrgDept addOrgDept(SptrOrgDept dept); 
	const SptrOrgDeptUsr addOrgDeptUsr(SptrOrgDeptUsr deptUsr); 
	bool updateOrgUsr(SptrOrgUsr usr ); 
	bool updateOrgDept(SptrOrgDept dept ); 
	bool updateOrgDeptUsr(SptrOrgDeptUsr deptUsr ); 
	bool disableOrgUsr(const string& ucgt); 
	bool disableOrgDept(const string& dcgt); 
	bool disableOrgDeptUsr(const string& dcgt, const string& ucgt); 
	bool delOrgUsr(SptrOrgUsr usr); 
	bool delOrgDept(SptrOrgDept dept); 
	bool delOrgDeptUsr(SptrOrgDeptUsr deptUsr); 
	int isDeptExistInPartent(const string& pcgt , const string& name); 
	int isUsrExistInDept(const string& dcgt, const string& name); 
	const SptrOrgUsr findUsr(const string& cgt); 
	const SptrOrgDept findDept(const string& cgt); 
	const SptrOrgDeptUsr findDeptUsr(const string& dcgt, const string& ucgt); 
	void findChild(const string& cgt, list<shared_ptr<XmsgImOrgNodeChild>>& child); 
	shared_ptr<XmsgImOrgSyncSubRsp> snapShot(SptrClient client); 
	void syncNextPageDept(ullong start, ullong end, list<shared_ptr<XmsgImOrgEvent>>& lis); 
	void syncNextPageDeptUsr(ullong start, ullong end, list<shared_ptr<XmsgImOrgEvent>>& lis); 
	void syncNextPageUsr(ullong start, ullong end, list<shared_ptr<XmsgImOrgEvent>>& lis); 
	size_t size4usr(); 
	size_t size4dept(); 
	void setVer4usr(ullong ver); 
	void setVer4dept(ullong ver); 
	void setVer4deptUsr(ullong ver); 
	ullong getVer4usr(); 
	ullong getVer4dept(); 
	ullong getVer4deptUsr(); 
	ullong nextVer4usr(); 
	ullong nextVer4dept(); 
	ullong nextVer4deptUsr(); 
	SptrOrgDept getRoot(); 
	void updateNodeInfo(const XmsgImOrgNodeInfo* from, shared_ptr<XmsgImOrgNodeInfo> to); 
	void setRoot(SptrOrgDept root); 
	static void loadCb4usr(SptrOrgUsr coll); 
	static void loadCb4dept(SptrOrgDept coll); 
	static void loadCb4deptUsr(SptrOrgDeptUsr coll); 
	static XmsgImOrgMgr* instance();
private:
	SptrOrgDept root; 
	ullong ver4usr; 
	ullong ver4dept; 
	ullong ver4deptUsr; 
	unordered_map<string , SptrOrgUsr> usr; 
	list<SptrOrgUsr> usrOrderByVer; 
	unordered_map<string , shared_ptr<map<string , SptrOrgDeptUsr>>> usr4dept; 
	unordered_map<string , SptrOrgDept> dept; 
	list<SptrOrgDept> deptOrderByVer; 
	unordered_map<string , shared_ptr<map<string , SptrOrgDeptUsr>>> dept4usr; 
	list<SptrOrgDeptUsr> deptUsrOrderByVer; 
	unordered_map<string , shared_ptr<map<string , SptrOrgDept>>> dept4dept; 
	atomic_ullong seqVer4usr; 
	atomic_ullong seqVer4dept; 
	atomic_ullong seqVer4deptUsr; 
	mutex lock4xx; 
	static XmsgImOrgMgr* inst;
	void addDeptOrderByVer(SptrOrgDept dept); 
	void addDeptUsrOrderByVer(SptrOrgDeptUsr depUsr); 
	void addUsrOrderByVer(SptrOrgUsr usr); 
	void sortDeptOrderByVer(); 
	void sortDeptUsrOrderByVer(); 
	void sortUsrOrderByVer(); 
	XmsgImOrgMgr();
	virtual ~XmsgImOrgMgr();
};

#endif 
