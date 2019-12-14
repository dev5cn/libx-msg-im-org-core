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

#include "XmsgImOrgUsrColl.h"
#include "../XmsgImOrgCfg.h"

XmsgImOrgUsrColl::XmsgImOrgUsrColl()
{
	this->cgt = nullptr;
	this->enable = true;
	this->ver = 0ULL;
	this->gts = 0ULL;
	this->uts = 0ULL;
}

SptrOrgUsr XmsgImOrgUsrColl::clone()
{
	SptrOrgUsr usr(new XmsgImOrgUsrColl());
	usr->cgt = ChannelGlobalTitle::parse(this->cgt->toString());
	usr->name = this->name;
	usr->enable = this->enable;
	usr->info.reset(new XmsgImOrgNodeInfo());
	for (auto& it : this->info->kv())
		XmsgMisc::insertKv(usr->info->mutable_kv(), it.first, it.second);
	usr->ver = this->ver;
	usr->gts = this->gts;
	usr->uts = this->uts;
	return usr;
}

shared_ptr<XmsgImOrgEvent> XmsgImOrgUsrColl::toEvent()
{
	shared_ptr<XmsgImOrgEvent> event(new XmsgImOrgEvent());
	auto u = event->mutable_usr();
	u->set_cgt(this->cgt->toString());
	u->set_name(this->name);
	u->set_enable(this->enable);
	for (auto& it : this->info->kv())
		XmsgMisc::insertKv(u->mutable_info()->mutable_kv(), it.first, it.second);
	u->set_ver(this->ver);
	u->set_gts(this->gts);
	u->set_uts(this->uts);
	return event;
}

string XmsgImOrgUsrColl::toString()
{
	string str;
	SPRINTF_STRING(&str, "cgt: %s, usr: %s, name: %s, enable: %s, info: %s, ver: %llu, gts: %s, uts: %s", 
			this->cgt->toString().c_str(),
			this->usr.c_str(),
			this->name.c_str(),
			this->enable ? "true" : "false",
			this->info->ShortDebugString().c_str(),
			this->ver,
			DateMisc::to_yyyy_mm_dd_hh_mi_ss_ms(this->gts).c_str(),
			DateMisc::to_yyyy_mm_dd_hh_mi_ss_ms(this->uts).c_str())
	return str;
}

XmsgImOrgUsrColl::~XmsgImOrgUsrColl()
{

}

