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

#include <libx-msg-im-org-pb.h>
#include "XmsgImClient.h"
#include "../XmsgImOrgCfg.h"
#include "../coll/XmsgImOrgUsrColl.h"

XmsgImClient::XmsgImClient(shared_ptr<XmsgImOrgUsrColl> usr, const string& plat, const string& did, const string& ccid, shared_ptr<XmsgAp> apLocal) :
		Actor(ActorType::ACTOR_ITC, XmsgImClient::assignXscWorker(usr->cgt))
{
	this->plat = plat;
	this->did = did;
	this->ccid = ccid;
	this->usr = usr;
	this->apLocal = apLocal;
	this->apForeign = nullptr;
	this->apCcid = XmsgMisc::getApCcid(apLocal->usr.lock()->uid, ccid);
	this->syncPubFinished = false;
	this->oob.reset(new list<pair<uchar, string>>());
	this->oob->push_back(make_pair<>(XSC_TAG_UID, this->ccid)); 
}

XmsgImClient::XmsgImClient(shared_ptr<XmsgImOrgUsrColl> usr, const string& plat, const string& did, const string& ccid, SptrCgt apForeign) :
		Actor(ActorType::ACTOR_ITC, XmsgImClient::assignXscWorker(usr->cgt))
{
	this->plat = plat;
	this->did = did;
	this->ccid = ccid;
	this->usr = usr;
	this->apForeign = apForeign;
	this->apLocal = nullptr;
	this->apCcid = XmsgMisc::getApCcid(apForeign->toString(), ccid);
	this->syncPubFinished = false;
	this->oob.reset(new list<pair<uchar, string>>());
	this->oob->push_back(make_pair<>(XSC_TAG_UID, this->ccid)); 
}

void XmsgImClient::finishedSyncPub()
{
	this->syncPubFinished = true;
	while (!this->noticeQueue.empty())
	{
		shared_ptr<XmsgImOrgChangedNotice> notice = this->noticeQueue.front();
		this->noticeQueue.pop();
		if (this->apLocal != nullptr)
		{
			this->apLocal->unidirection(notice, this->oob);
			continue;
		}
	}
}

void XmsgImClient::sendNotice(shared_ptr<XmsgImOrgChangedNotice> notice)
{
	if (!this->syncPubFinished)
	{
		this->noticeQueue.push(notice);
		return;
	}
	this->sendNotice4local(notice);
}

void XmsgImClient::kick(SptrXit trans)
{
	shared_ptr<XmsgApClientKickReq> req(new XmsgApClientKickReq());
	req->set_ccid(this->ccid);
	SptrOob oob(new list<pair<uchar, string>>());
	oob->push_back(make_pair<>(XSC_TAG_INTERCEPT, "enable")); 
	if (this->apLocal != nullptr)
	{
		auto ths = static_pointer_cast<XmsgImClient>(this->shared_from_this());
		XmsgImChannel::cast(this->apLocal)->begin(req, [req, ths](SptrXiti itrans) 
		{
			LOG_DEBUG("got a x-msg-im-client kick response from x-msg-ap, ret: %04X, desc: %s, rsp: %s, this: %s", itrans->ret, itrans->desc.c_str(), (itrans->endMsg == nullptr ? "null" : itrans->endMsg->ShortDebugString().c_str()), ths->toString().c_str())
		}, oob, trans);
		return;
	}
}

void XmsgImClient::sendNotice4local(shared_ptr<XmsgImOrgChangedNotice> notice)
{
	this->apLocal->unidirection(notice, this->oob);
}

void XmsgImClient::begin(shared_ptr<Message> req, function<void(ushort ret, const string& desc, shared_ptr<Message> rsp)> cb, SptrOob oob, SptrXit upstreamTrans)
{
	XmsgImChannel::cast(this->apLocal)->begin(req, [cb](SptrXiti itrans)
	{
		cb(itrans->ret, itrans->desc, itrans->endMsg);
	}, oob, upstreamTrans);
}

bool XmsgImClient::isLocalAttach()
{
	return this->apLocal != nullptr;
}

int XmsgImClient::assignXscWorker(SptrCgt cgt)
{
	return (((int) (cgt->uid.data()[cgt->uid.length() - 1])) & 0x0000FF) % XmsgImOrgCfg::instance()->cfgPb->xsctcpcfg().worker();
}

string XmsgImClient::toString()
{
	string str;
	SPRINTF_STRING(&str, "plat: %s, did: %s, ccid: %s, usr: %s", this->plat.c_str(), this->did.c_str(), this->ccid.c_str(), this->usr->toString().c_str())
	return str;
}

XmsgImClient::~XmsgImClient()
{

}

