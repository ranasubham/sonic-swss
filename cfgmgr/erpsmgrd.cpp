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

#include <fstream>
#include "erpsmgr.h"
#include "netdispatcher.h"
#include "netlink.h"
#include "select.h"
#include "warm_restart.h"
#include <unistd.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <mutex>
#include <algorithm>
#include "dbconnector.h"
#include "exec.h"
#include "schema.h"
#include "producerstatetable.h"
#include "shellcmd.h"

using namespace std;
using namespace swss;

int gBatchSize = 0;
bool gSwssRecord = false;
bool gLogRotate = false;
ofstream gRecordOfs;
string gRecordFile;
bool gResponsePublisherRecord = false;
bool gResponsePublisherLogRotate = false;
ofstream gResponsePublisherRecordOfs;
string gResponsePublisherRecordFile;
/* Global database mutex */
mutex gDbMutex;

#define SELECT_TIMEOUT 1000

int main(int argc, char **argv)
{
    Logger::linkToDbNative("erpsmgrd");
    SWSS_LOG_ENTER();

    SWSS_LOG_NOTICE("--- Starting erpsmgrd ---");

    if (fopen("/erpsmgrd_dbg_reload", "r"))
    {
        Logger::setMinPrio(Logger::SWSS_DEBUG);
    }

    try
    {
        DBConnector cfgDb("CONFIG_DB", 0);
        DBConnector appDb("APPL_DB", 0);
        DBConnector stateDb("STATE_DB", 0);

        WarmStart::initialize("erpsmgrd", "swss");
        WarmStart::checkWarmStart("erpsmgrd", "swss");

        // Config DB Tables
        TableConnector conf_erps_global_table(&cfgDb, CFG_ERPS_GLOBAL_TABLE_NAME);

        // VLAN DB Tables
        TableConnector state_erps_table(&stateDb, STATE_ERPS_TABLE_NAME);

        vector<string> tableNames = {
            CFG_ERPS_GLOBAL_TABLE_NAME,
            STATE_ERPS_TABLE_NAME
        };

        ErpsMgr erpsmgr(&cfgDb, &appDb, &stateDb, tableNames);
        vector<Orch *> cfgOrchList = {&erpsmgr};

        swss::Select s;
        for (Orch *o : cfgOrchList)
        {
            s.addSelectables(o->getSelectables());
        }

        SWSS_LOG_NOTICE("starting main loop");   
        while (true)
        {
            Selectable *sel;
            int ret;

            ret = s.select(&sel, SELECT_TIMEOUT);
            if (ret == Select::ERROR)
            {
                SWSS_LOG_NOTICE("Error: %s!", strerror(errno));
                continue;
            }
            if (ret == Select::TIMEOUT)
            {
                erpsmgr.doTask();
                continue;
            }

            auto *c = (Executor *)sel;
            c->execute();
        }
    }
    catch(const std::exception &e)
    {
        SWSS_LOG_ERROR("Runtime error: %s", e.what());
    }
    return -1;
}
