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

#include "XmsgImOrgTcpLog.h"
#include "XmsgImOrgCfg.h"

XmsgImOrgTcpLog::XmsgImOrgTcpLog() :
		XmsgImTcpLog()
{

}

void XmsgImOrgTcpLog::didaMainThread(ullong now)
{

}

void XmsgImOrgTcpLog::dida(ullong now)
{

}

void XmsgImOrgTcpLog::rx(XscChannel* channel, uchar* dat, int len)
{

}

void XmsgImOrgTcpLog::tx(XscChannel* channel, uchar* dat, int len)
{

}

void XmsgImOrgTcpLog::transInitStart(SptrXiti trans, shared_ptr<XscProtoPdu> pdu )
{

}

void XmsgImOrgTcpLog::transInitFinished(SptrXiti trans, shared_ptr<XscProtoPdu> pdu )
{

}

void XmsgImOrgTcpLog::transPassStart(SptrXitp trans, shared_ptr<XscProtoPdu> pdu )
{

}

void XmsgImOrgTcpLog::transPassFinished(SptrXitp trans, shared_ptr<XscProtoPdu> pdu )
{

}

void XmsgImOrgTcpLog::transInitUni(SptrXitui trans, shared_ptr<XscProtoPdu> pdu )
{

}

void XmsgImOrgTcpLog::transPassUni(SptrXitup trans, shared_ptr<XscProtoPdu> pdu )
{

}

XmsgImOrgTcpLog::~XmsgImOrgTcpLog()
{

}
