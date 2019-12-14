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

#ifndef USR_XMSGIMCLIENT_H_
#define USR_XMSGIMCLIENT_H_

#include <libx-msg-im-org-pb.h>
#include "../ne/XmsgAp.h"

class XmsgImOrgUsrColl;

class XmsgImClient: public Actor
{
public:
	string plat; 
	string did; 
	string apCcid; 
	string ccid; 
	shared_ptr<XmsgAp> apLocal; 
	SptrCgt apForeign; 
	shared_ptr<XmsgImOrgUsrColl> usr; 
	queue<shared_ptr<XmsgImOrgChangedNotice>> noticeQueue; 
	SptrOob oob; 
	bool syncPubFinished; 
public:
	void finishedSyncPub(); 
	void sendNotice(shared_ptr<XmsgImOrgChangedNotice> notice); 
	void kick(SptrXit trans = nullptr); 
	bool isLocalAttach(); 
	string toString();
	XmsgImClient(shared_ptr<XmsgImOrgUsrColl> usr, const string& plat, const string& did, const string& ccid, shared_ptr<XmsgAp> apLocal );
	XmsgImClient(shared_ptr<XmsgImOrgUsrColl> usr, const string& plat, const string& did, const string& ccid, SptrCgt apForeign );
	virtual ~XmsgImClient();
public:
	void begin(shared_ptr<Message> req, function<void(ushort ret, const string& desc, shared_ptr<Message> rsp)> cb, SptrOob oob = nullptr, SptrXit upstreamTrans = nullptr); 
private:
	void sendNotice4local(shared_ptr<XmsgImOrgChangedNotice> notice); 
	static int assignXscWorker(SptrCgt ucgt); 
};

typedef shared_ptr<XmsgImClient> SptrClient;

#endif 
