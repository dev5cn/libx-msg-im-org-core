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

#include "XmsgImOrgDeptColl.h"
#include "../XmsgImOrgMgr.h"

XmsgImOrgDeptColl::XmsgImOrgDeptColl()
{
	this->enable = true;
	this->ver = 0;
	this->gts = 0ULL;
	this->uts = 0ULL;
}

shared_ptr<XmsgImOrgDeptColl> XmsgImOrgDeptColl::clone()
{
	SptrOrgDept dept(new XmsgImOrgDeptColl());
	dept->cgt = ChannelGlobalTitle::parse(this->cgt->toString());
	dept->pcgt = dept->pcgt == nullptr ? nullptr : ChannelGlobalTitle::parse(this->pcgt->toString());
	dept->name = this->name;
	dept->enable = this->enable;
	dept->info.reset(new XmsgImOrgNodeInfo());
	for (auto& it : this->info->kv())
		XmsgMisc::insertKv(dept->info->mutable_kv(), it.first, it.second);
	dept->ver = this->ver;
	dept->gts = this->gts;
	dept->uts = this->uts;
	return dept;
}

shared_ptr<XmsgImOrgEvent> XmsgImOrgDeptColl::toEvent()
{
	shared_ptr<XmsgImOrgEvent> event(new XmsgImOrgEvent());
	auto d = event->mutable_dept();
	d->set_cgt(this->cgt->toString());
	if (this->pcgt != nullptr && !this->pcgt->isSame(XmsgImOrgMgr::instance()->getRoot()->cgt) )
		d->set_pcgt(this->pcgt->toString());
	d->set_name(this->name);
	d->set_enable(this->enable);
	for (auto& it : this->info->kv())
		XmsgMisc::insertKv(d->mutable_info()->mutable_kv(), it.first, it.second);
	d->set_ver(this->ver);
	d->set_gts(this->gts);
	d->set_uts(this->uts);
	return event;
}

string XmsgImOrgDeptColl::toString()
{
	string str;
	SPRINTF_STRING(&str, "cgt: %s, pcgt: %s, name: %s, enable: %s, info: %s, ver: %llu, gts: %s, uts: %s", 
			this->cgt->toString().c_str(),
			this->pcgt == nullptr ? "null" : this->pcgt->toString().c_str(),
			this->name.c_str(),
			this->enable ? "true" : "false",
			this->info->ShortDebugString().c_str(),
			this->ver,
			DateMisc::to_yyyy_mm_dd_hh_mi_ss_ms(this->gts).c_str(),
			DateMisc::to_yyyy_mm_dd_hh_mi_ss_ms(this->uts).c_str())
	return str;
}

XmsgImOrgDeptColl::~XmsgImOrgDeptColl()
{

}

