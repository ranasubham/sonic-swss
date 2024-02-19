/*
 * Copyright 2019 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or
 * its subsidiaries.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <set>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <bitset>
#include "orch.h"
#include "dbconnector.h"
#include "netmsg.h"
#include "producerstatetable.h"
#include <stddef.h>
#include <algorithm>

#define CFG_ERPS_GLOBAL_TABLE_NAME        "ERP"
#define STATE_ERPS_TABLE_NAME             "ERP_STATE"

namespace swss {

class ErpsMgr : public Orch
{
public:
    ErpsMgr(DBConnector *cfgDb, DBConnector *appDb, DBConnector *stateDb,const std::vector<std::string> &tableNames);

    using Orch::doTask;

private:
    Table m_cfgErpsGlobalTable;
    Table m_stateErpsTable;

    bool erpsGlobalTask;

    void doTask(Consumer &consumer);
    void doErpsGlobalTask(Consumer &consumer);
};

}
