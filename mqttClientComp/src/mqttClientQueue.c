/**
 * This module implements MQTT queue.
 *
 * Copyright (C) .
 *
 */

#include "mqttDatabase.h"

//---------------------------------------------------------------------------------------------------
/**
 * Send all messages into the queue database.
 */
 //--------------------------------------------------------------------------------------------------
void mqttClientQueue_send(mqttClient_t* clientData)
{
  int count = mqttDatabase_countOutgoing();
  if (count == 0)
  {
    LE_DEBUG("no message into the queue");
    return;
  }

  char sql[128];
  sprintf(sql, "select * from outgoing;");

  sqlite3_stmt* stmt = NULL;

  int rc = sqlite3_prepare_v2(mqttDatabase_getDB(), sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) 
  {
    LE_ERROR("sqlerror:%s", sqlite3_errmsg(mqttDatabase_getDB()));
    return;
  }

  int rowCount = 0;
  int colIndex;

  const char* uuid  = NULL;
  const char* topic = NULL;
  const char* msg   = NULL;
  int msg_len = 0;
  
  rc = sqlite3_step(stmt);

  le_result_t res;

  while(rc != SQLITE_DONE && rc != SQLITE_OK)
  {
    rowCount++;
    
    int colCount = sqlite3_column_count(stmt);
  
    for (colIndex = 0; colIndex < colCount; colIndex++) 
    {  
      const char* columnName = sqlite3_column_name(stmt, colIndex);
      
      if (strcmp(columnName, "UUID") == 0) 
      {
        uuid = (const char*)sqlite3_column_text(stmt, colIndex);
      }

      if (strcmp(columnName, "TOPIC") == 0) 
      {
        topic = (const char*)sqlite3_column_text(stmt, colIndex);
      }

      if(strcmp(columnName, "MESSAGE") == 0) 
      {
        msg = (const char*)sqlite3_column_text(stmt, colIndex);
      }

      if(strcmp(columnName, "MESSAGE_LEN") == 0) 
      {
        msg_len = sqlite3_column_int(stmt, colIndex);
      }
    }

    if(uuid != NULL && msg != NULL && topic != NULL)
    {
      LE_INFO("send queued message '%s'", uuid);

      mqttClient_msg_t retryMsg =
      {
          .qos = clientData->session.config.QoS,
          .retained = 0,
          .dup = 0,
          .id = 0,
          .payload = (char*)msg,
          .payloadLen = msg_len,
          .queued = false,
      };
      res = mqttClient_publish(clientData, topic, &retryMsg);
      if (LE_OK == res)
      {
        res = mqttDatabase_deleteOutgoing(uuid);
        LE_ERROR_IF(res, "failed to delete message '%s' from queue", uuid);
      }
    }

    rc = sqlite3_step(stmt);
  }

  rc = sqlite3_finalize(stmt);
}

//---------------------------------------------------------------------------------------------------
/**
 * Initializes mqttClient queue.
 */
 //--------------------------------------------------------------------------------------------------
void mqttClientQueue_init(mqttClient_t* clientData)
{
  LE_ASSERT(clientData);
  mqttDatabase_init();
}

