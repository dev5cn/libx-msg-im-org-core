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

#include "XmsgAp.h"
#include "../XmsgImOrgCfg.h"
#include "../XmsgImOrgMgr.h"
#include "../XmsgImOrgSubClientMgr.h"

unordered_map<string, shared_ptr<XmsgImMsgStub>> XmsgAp::__h2nMsgs__;

XmsgAp::XmsgAp(shared_ptr<XscTcpServer> tcpServer, const string &peer, const string& pwd, const string& alg) :
		XmsgNe(tcpServer, peer, X_MSG_AP, pwd, alg)
{
	for (auto& it : XmsgAp::__h2nMsgs__)
		this->msgMgr->msgs[it.first] = it.second;
}

XscMsgItcpRetType XmsgAp::itcp(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
{
	if (pdu->transm.trans->trans == XSC_TAG_TRANS_BEGIN)
		return this->itcp4begin(wk, channel, pdu);
	if (pdu->transm.trans->trans == XSC_TAG_TRANS_UNIDIRECTION)
		return this->itcp4unidirection(wk, channel, pdu);
	return XscMsgItcpRetType::DISABLE;
}

XscMsgItcpRetType XmsgAp::itcp4begin(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
{
	string cgt, plat, did, ccid;
	if (!this->checkRequiredPars(pdu, cgt, plat, did, ccid))
		return XscMsgItcpRetType::EXCEPTION;
	auto ap = static_pointer_cast<XmsgAp>(this->shared_from_this());
	SptrXitp trans(new XmsgImTransPassive(ap, pdu));
	trans->dtid = pdu->transm.trans->stid;
	trans->addOob(XSC_TAG_UID, ccid);
	string itcp;
	if (trans->getOob(XSC_TAG_INTERCEPT, itcp))
		trans->addOob(XSC_TAG_INTERCEPT, itcp);
	shared_ptr<XmsgImMsgStub> stub = this->msgMgr->getMsgStub(pdu->transm.trans->msg);
	if (stub == nullptr)
	{
		LOG_DEBUG("can not found x-msg-im message stub for: %s, this: %s", pdu->transm.trans->msg.c_str(), this->toString().c_str())
		XmsgImLog::getLog(trans->channel.get())->transPassStart(trans, pdu);
		trans->endDesc(RET_FORBIDDEN, "unsupported message: %s", pdu->transm.trans->msg.c_str());
		return XscMsgItcpRetType::FORBIDDEN;
	}
	trans->beginMsg = stub->newBegin(pdu->transm.trans->dat, pdu->transm.trans->dlen);
	if (trans->beginMsg == nullptr)
	{
		LOG_DEBUG("can not reflect a begin message from data, msg: %s, this: %s", pdu->transm.trans->msg.c_str(), this->toString().c_str())
		trans->endDesc(RET_EXCEPTION, "request message format error: %s", pdu->transm.trans->msg.c_str());
		return XscMsgItcpRetType::FORBIDDEN;
	}
	SptrOrgUsr usr = XmsgImOrgMgr::instance()->findUsr(cgt); 
	if (usr == nullptr)
	{
		LOG_DEBUG("can not found org usr data for cgt, plat: %s, did: %s, ccid: %s, cgt: %s, msg: %s", plat.c_str(), did.c_str(), ccid.c_str(), cgt.c_str(), pdu->transm.trans->msg.c_str())
		trans->endDesc(RET_FORBIDDEN, "can not found org usr data for cgt: %s", cgt.c_str());
		return XscMsgItcpRetType::FORBIDDEN;
	}
	if (!usr->enable)
	{
		LOG_DEBUG("usr data disabled, plat: %s, did: %s, ccid: %s, cgt: %s", plat.c_str(), did.c_str(), ccid.c_str(), cgt.c_str())
		trans->endDesc(RET_FORBIDDEN, "disabled");
		return XscMsgItcpRetType::FORBIDDEN;
	}
	auto ne = static_pointer_cast<XmsgNeUsr>(this->usr.lock()); 
	auto client = XmsgImOrgSubClientMgr::instance()->find(XmsgMisc::getApCcid(ne->uid, ccid));
	client = client == nullptr ? SptrClient(new XmsgImClient(usr, plat, did, ccid, ap)) : client;
	client->future([stub, ne, client, trans] 
	{
		((void (*)(shared_ptr<XmsgNeUsr> nu, SptrClient client, SptrXitp trans, shared_ptr<Message> req)) (stub->cb))(ne, client, trans, trans->beginMsg);
	});
	return XscMsgItcpRetType::SUCCESS;
}

XscMsgItcpRetType XmsgAp::itcp4unidirection(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
{
	string cgt, plat, did, ccid;
	if (!this->checkRequiredPars(pdu, cgt, plat, did, ccid))
		return XscMsgItcpRetType::EXCEPTION;
	auto ap = static_pointer_cast<XmsgAp>(this->shared_from_this());
	SptrXitup trans(new XmsgImTransUnidirectionPass(ap, pdu));
	shared_ptr<XmsgImMsgStub> stub = this->msgMgr->getMsgStub(pdu->transm.trans->msg);
	if (stub == nullptr)
	{
		LOG_DEBUG("can not found x-msg-im message stub for: %s, this: %s", pdu->transm.trans->msg.c_str(), this->toString().c_str())
		XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
		return XscMsgItcpRetType::FORBIDDEN;
	}
	trans->uniMsg = stub->newUnidirection(pdu->transm.trans->dat, pdu->transm.trans->dlen);
	if (trans->uniMsg == nullptr)
	{
		LOG_DEBUG("can not reflect a begin message from data, msg: %s, this: %s", pdu->transm.trans->msg.c_str(), this->toString().c_str())
		XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
		return XscMsgItcpRetType::FORBIDDEN;
	}
	SptrOrgUsr usr = XmsgImOrgMgr::instance()->findUsr(cgt); 
	if (usr == nullptr)
	{
		LOG_DEBUG("can not found org usr data for cgt, plat: %s, did: %s, ccid: %s, cgt: %s, msg: %s", plat.c_str(), did.c_str(), ccid.c_str(), cgt.c_str(), pdu->transm.trans->msg.c_str())
		XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
		return XscMsgItcpRetType::FORBIDDEN;
	}
	if (!usr->enable)
	{
		LOG_DEBUG("usr data disabled, plat: %s, did: %s, ccid: %s, cgt: %s", plat.c_str(), did.c_str(), ccid.c_str(), cgt.c_str())
		XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
		return XscMsgItcpRetType::FORBIDDEN;
	}
	XmsgImLog::getLog(trans->channel.get())->transPassUni(trans, pdu);
	auto ne = static_pointer_cast<XmsgNeUsr>(this->usr.lock()); 
	auto client = XmsgImOrgSubClientMgr::instance()->find(XmsgMisc::getApCcid(ne->uid, ccid));
	client = client == nullptr ? SptrClient(new XmsgImClient(usr, plat, did, ccid, ap)) : client;
	client->future([stub, ne, client, trans] 
	{
		((void (*)(shared_ptr<XmsgNeUsr> nu, SptrClient client, SptrXitup trans, shared_ptr<Message> uni)) (stub->cb))(ne, client, trans, trans->uniMsg);
	});
	return XscMsgItcpRetType::SUCCESS;
}

bool XmsgAp::checkRequiredPars(shared_ptr<XscProtoPdu> pdu, string& cgt, string& plat, string& did, string& ccid)
{
	if (!pdu->transm.getOob(XSC_TAG_CGT, cgt)) 
	{
		LOG_FAULT("it`s a bug, must be have x-msg-im-client channel global title, this: %s", this->toString().c_str())
		return false;
	}
	if (!pdu->transm.getOob(XSC_TAG_PLATFORM, plat)) 
	{
		LOG_FAULT("it`s a bug, must be have x-msg-im-client platform information, this: %s", this->toString().c_str())
		return false;
	}
	if (!pdu->transm.getOob(XSC_TAG_DEVICE_ID, did)) 
	{
		LOG_FAULT("it`s a bug, must be have x-msg-im-client device information, this: %s", this->toString().c_str())
		return false;
	}
	if (!pdu->transm.getOob(XSC_TAG_UID, ccid)) 
	{
		LOG_FAULT("it`s a bug, must be have x-msg-im-client channel id, this: %s", this->toString().c_str())
		return false;
	}
	return true;
}

bool XmsgAp::regH2N(const Descriptor* begin, const Descriptor* end, const Descriptor* uni, void* cb, bool auth, ForeignAccessPermission foreign)
{
	shared_ptr<XmsgImMsgStub> stub(new XmsgImMsgStub(begin, end, uni, cb, auth, foreign));
	if (XmsgAp::__h2nMsgs__.find(stub->msg) != XmsgAp::__h2nMsgs__.end())
	{
		LOG_ERROR("duplicate message: %s", stub->msg.c_str())
		return false;
	}
	LOG_TRACE("reg h2n messsage: %s", stub->toString().c_str())
	XmsgAp::__h2nMsgs__[stub->msg] = stub;
	return true;
}

XmsgAp::~XmsgAp()
{

}
