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

#include "exec.h"
#include "erpsmgr.h"
#include "logger.h"
#include "tokenize.h"
#include "warm_restart.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <thread>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>
#include "producerstatetable.h"
#include "macaddress.h"
#include "exec.h"
#include "tokenize.h"
#include "shellcmd.h"
#include "warm_restart.h"
#include <swss/redisutility.h>

using namespace std;
using namespace swss;


ErpsMgr::ErpsMgr(DBConnector *cfgDb, DBConnector *applDb, DBConnector *stateDb,const vector<string> &tableNames) :
    Orch(cfgDb,tableNames),
    m_cfgErpsGlobalTable(cfgDb, CFG_ERPS_GLOBAL_TABLE_NAME),
    m_stateErpsTable(stateDb, STATE_ERPS_TABLE_NAME)
{
    SWSS_LOG_ENTER();

    erpsGlobalTask =  false;

}

void ErpsMgr::doTask(Consumer &consumer)
{
    string table = consumer.getTableName();

    SWSS_LOG_INFO("Get task from table %s", table.c_str());

    if (table == CFG_ERPS_GLOBAL_TABLE_NAME)
    {
        doErpsGlobalTask(consumer);
    }
    //else if (table == STATE_ERPS_TABLE_NAME)
        //doErpsUpdateTask(consumer);
    else
    {
        SWSS_LOG_ERROR("Invalid table %s", table.c_str());
    }
}

void ErpsMgr::doErpsGlobalTask(Consumer &consumer)
{
    SWSS_LOG_ENTER();

    if (erpsGlobalTask == false)
        erpsGlobalTask = true;

    auto it = consumer.m_toSync.begin();
    while (it != consumer.m_toSync.end())
    {
        //ERPS_CONFIG_MSG msg;
        //memset(&msg, 0, sizeof(ERPS_CONFIG_MSG));

        KeyOpFieldsValuesTuple t = it->second;

        string key = kfvKey(t);
        string op = kfvOp(t);

        SWSS_LOG_INFO("STP global key %s op %s", key.c_str(), op.c_str());
        if (op == SET_COMMAND)
        {
            SWSS_LOG_DEBUG("set_command used");
            //msg.opcode = ERPS_SET_COMMAND;
            //for (auto i : kfvFieldsValues(t))
            //{
                //SWSS_LOG_DEBUG("Field: %s Val %s", fvField(i).c_str(), fvValue(i).c_str());
               
            //}

            //memcpy(msg.base_mac_addr, macAddress.getMac(), 6);
        }
        else if (op == DEL_COMMAND)
        {
            SWSS_LOG_DEBUG("Inside op DEL command");
            //msg.opcode = ERPS_DEL_COMMAND;
        }

        it = consumer.m_toSync.erase(it);
    }
}
