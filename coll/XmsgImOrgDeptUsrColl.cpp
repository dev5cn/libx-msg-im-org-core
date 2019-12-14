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

#include "XmsgImOrgDeptUsrColl.h"

XmsgImOrgDeptUsrColl::XmsgImOrgDeptUsrColl()
{
	this->enable = true;
	this->ver = 0;
	this->gts = 0ULL;
	this->uts = 0ULL;
}

shared_ptr<XmsgImOrgDeptUsrColl> XmsgImOrgDeptUsrColl::clone()
{
	SptrOrgDeptUsr deptUsr(new XmsgImOrgDeptUsrColl());
	deptUsr->dcgt = ChannelGlobalTitle::parse(this->dcgt->toString());
	deptUsr->ucgt = ChannelGlobalTitle::parse(this->ucgt->toString());
	deptUsr->name = this->name;
	deptUsr->enable = this->enable;
	deptUsr->info.reset(new XmsgImOrgNodeInfo());
	for (auto& it : this->info->kv())
		XmsgMisc::insertKv(deptUsr->info->mutable_kv(), it.first, it.second);
	deptUsr->ver = this->ver;
	deptUsr->gts = this->gts;
	deptUsr->uts = this->uts;
	return deptUsr;
}

shared_ptr<XmsgImOrgEvent> XmsgImOrgDeptUsrColl::toEvent()
{
	shared_ptr<XmsgImOrgEvent> event(new XmsgImOrgEvent());
	auto d = event->mutable_deptusr();
	d->set_dcgt(this->dcgt->toString());
	d->set_ucgt(this->ucgt->toString());
	d->set_name(this->name);
	d->set_enable(this->enable);
	for (auto& it : this->info->kv())
		XmsgMisc::insertKv(d->mutable_info()->mutable_kv(), it.first, it.second);
	d->set_ver(this->ver);
	d->set_gts(this->gts);
	d->set_uts(this->uts);
	return event;
}

string XmsgImOrgDeptUsrColl::toString()
{
	string str;
	SPRINTF_STRING(&str, "dcgt: %s, ucgt: %s, name: %s, enable: %s, info: %s, ver: %llu, gts: %s, uts: %s", 
			this->dcgt->toString().c_str(),
			this->ucgt->toString().c_str(),
			this->name.c_str(),
			this->enable ? "true" : "false",
			this->info->ShortDebugString().c_str(),
			this->ver,
			DateMisc::to_yyyy_mm_dd_hh_mi_ss_ms(this->gts).c_str(),
			DateMisc::to_yyyy_mm_dd_hh_mi_ss_ms(this->uts).c_str())
	return str;
}

XmsgImOrgDeptUsrColl::~XmsgImOrgDeptUsrColl()
{

}

