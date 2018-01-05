/**
 * @file
 *
 * <HR>
 *
 * Copyright (C) .
 */

#include "mqttDatabase.h"
#include <uuid.h>

//---------------------------------------------------------------------------------------------------
/**
 * Default database location.
 */
//---------------------------------------------------------------------------------------------------
#define DATABASE_DIR "/tmp/"
#define DATABASE_NAME "mqttQueue.db"

//---------------------------------------------------------------------------------------------------
/**
 * Max SQL command size.
 */
//---------------------------------------------------------------------------------------------------
#define SQLCMD_SIZE 8192

//---------------------------------------------------------------------------------------------------
/**
 * SQL command.
 */
//---------------------------------------------------------------------------------------------------
static char sqlcmd[SQLCMD_SIZE];

//---------------------------------------------------------------------------------------------------
/**
 * SQLite database pointer.
 */
//---------------------------------------------------------------------------------------------------
static sqlite3* mqtt_db = NULL;

//---------------------------------------------------------------------------------------------------
/**
 * Generates an uuid.
 *
 * @returns A string corresponding to the uuid result. It is NULL in case of an error.
 */
//---------------------------------------------------------------------------------------------------
char* make_uuid()
{
  uuid_rc_t result;
  uuid_t *uuid;
  char* UUID = NULL;

  result = uuid_create(&uuid);
  if (UUID_RC_OK == result)
  {
    uuid_make(uuid, UUID_MAKE_V1);
    uuid_export(uuid, UUID_FMT_STR, &UUID, 0);
    uuid_destroy(uuid);
  }
  else
  {
    LE_ERROR("uuid_create: %s", uuid_error(result));
  }
  return UUID;
}

//---------------------------------------------------------------------------------------------------
/**
 * Converts a string to sql string.
 *
 * @returns A sql string.
 */
//---------------------------------------------------------------------------------------------------
static char* sqlite_str(char* str)
{
  int size=strlen(str);
  int count=0;
  int i;
  
  for(i=0;i<size;i++) if(str[i]=='\'') count++;
  
  char* result=malloc(size+count+1);
  int j=0;
  
  for (i=0;i<size;i++)
  {
    if (str[i]=='\'') result[j++]='\'';
    result[j++]=str[i];
  }

  result[j]=0;
  return result;
}

//---------------------------------------------------------------------------------------------------
/**
 * Executes query on the database that returns a scalar integer value.
 *
 * @return
 * - LE_OK if execution was successful
 * - LE_FAULT if it failed
 */
//---------------------------------------------------------------------------------------------------
static le_result_t dbQueryExec()
{
  char* sqlError = 0;
  le_result_t result = LE_OK;

  int rc = sqlite3_exec(mqtt_db, sqlcmd, NULL, 0, &sqlError);
  if (rc != SQLITE_OK) 
  {
    LE_ERROR("sql error: %s", sqlError);
    sqlite3_free(sqlError);
    result = LE_FAULT;
  }
  return result;
}

//---------------------------------------------------------------------------------------------------
/**
 * Executes query on the database that returns a scalar integer value.
 *
 * @returns An integer value corresponding to the query execution result as scalar integer value
 */
//---------------------------------------------------------------------------------------------------
static int dbQueryInteger()
{
  sqlite3_stmt* stmt = NULL;

  int rc = sqlite3_prepare_v2(mqtt_db, sqlcmd, -1, &stmt, NULL);

  if(rc != SQLITE_OK) {
    LE_ERROR("sqlerror:%s", sqlite3_errmsg(mqtt_db));
    return -1;
  }

  int value;

  sqlite3_step(stmt);
  value = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);

  return value;
}

//---------------------------------------------------------------------------------------------------
/**
 * Executes query on the database that retuns a scalar string value.
 *
 * @returns A string corresponding to the query execution result. It is NULL in case of an error.
 */
//---------------------------------------------------------------------------------------------------
static char* dbQueryString()
{
  sqlite3_stmt* stmt = NULL;

  int rc = sqlite3_prepare_v2(mqtt_db, sqlcmd, -1, &stmt, NULL);

  if(rc != SQLITE_OK) {
    LE_ERROR("sqlerror:%s", sqlite3_errmsg(mqtt_db));
    return NULL;
  }

  const char* value;
  char* result = NULL;

  sqlite3_step(stmt);
  value = (const char*)sqlite3_column_text(stmt, 0);
  if(value != NULL)
  {
    result = malloc(strlen(value)+1);
    strcpy(result, value);
  }

  sqlite3_finalize(stmt);

  return result;
}

//---------------------------------------------------------------------------------------------------
/**
 * Creates database tables.
 */
 //--------------------------------------------------------------------------------------------------
static le_result_t mqttDatabase_createTables()
{
  char* sqlError = 0;
  char* sql;
  int rc;
  le_result_t result = LE_OK;

  sql = "CREATE TABLE IF NOT EXISTS outgoing("
    "UUID        TEXT    PRIMARY KEY NOT NULL,"
    "TOPIC       TEXT    NOT NULL,"
    "MESSAGE     TEXT    NOT NULL,"
    "MESSAGE_LEN INTEGER NOT NULL);";

  rc = sqlite3_exec(mqtt_db, sql, NULL, 0, &sqlError);
  if (rc != SQLITE_OK) 
  {
    LE_ERROR("Error creating outgoing table: %s", sqlError);
    sqlite3_free(sqlError);
    result = LE_FAULT;
  }

  return result;
}

//---------------------------------------------------------------------------------------------------
/**
 * Initializes database.
 */
 //--------------------------------------------------------------------------------------------------
le_result_t mqttDatabase_init()
{
  char* db_path = malloc(strlen(DATABASE_DIR) + strlen(DATABASE_NAME) + 2);
  strcpy(db_path, DATABASE_DIR);
  strcat(db_path, "/");
  strcat(db_path, DATABASE_NAME);
  int rc = sqlite3_open(db_path, &mqtt_db);
  free(db_path);
  if (rc) 
  {
    LE_ERROR("Can't open database: '%s'", sqlite3_errmsg(mqtt_db));
    return LE_FAULT;
  }
  mqttDatabase_createTables();
  sqlite3_busy_timeout(mqtt_db, 60000);
  return LE_OK;
}

//---------------------------------------------------------------------------------------------------
/**
 * Closes database.
 */
 //--------------------------------------------------------------------------------------------------
void mqttDatabase_close()
{
  if (mqtt_db == NULL)
    return;

  sqlite3_close(mqtt_db);
}

//---------------------------------------------------------------------------------------------------
/**
 * Returns database handler pointer.
 */
 //--------------------------------------------------------------------------------------------------
sqlite3* mqttDatabase_getDB()
{
  return mqtt_db;
}

//--------------------------------------------------------------------------------------------------
/**
 * @Runs an execute(no SELECT) query on the database.
 *
 * @return
 * - LE_OK if exporting was successful
 * - LE_FAULT if it failed
 */
//--------------------------------------------------------------------------------------------------
le_result_t mqttDatabase_queryExec(const char* sql, ...)
{
  va_list v_args;

  va_start(v_args, sql);
  vsnprintf(sqlcmd, SQLCMD_SIZE, sql, v_args);
  va_end(v_args);

  return dbQueryExec();
}

//---------------------------------------------------------------------------------------------------
/**
 * Runs a query on the database that returns a scalar integer value.
 *
 * @returns An integer value corresponding to the query execution result as scalar integer value
 */
//---------------------------------------------------------------------------------------------------
int mqttDatabase_queryInteger(const char* sql, ...)
{
  va_list v_args;

  va_start(v_args, sql);
  vsnprintf(sqlcmd, SQLCMD_SIZE, sql, v_args);
  va_end(v_args);

  return dbQueryInteger();
}

//---------------------------------------------------------------------------------------------------
/**
 * Runs a query on the database that retuns a scalar string value.
 *
 * @returns A string corresponding to the query execution result. It is NULL in case of an error.
 */
//---------------------------------------------------------------------------------------------------
char* mqttDatabase_queryString(const char* sql, ...)
{
  va_list v_args;

  va_start(v_args, sql);
  vsnprintf(sqlcmd, SQLCMD_SIZE, sql, v_args);
  va_end(v_args);

  return dbQueryString();
}

//---------------------------------------------------------------------------------------------------
/**
 * Returns the number of message into outgoing table.
 */
//---------------------------------------------------------------------------------------------------
int mqttDatabase_countOutgoing()
{
  int count = mqttDatabase_queryInteger("SELECT count(*) FROM outgoing;");
  LE_DEBUG("count=%i", count);

  return count;
}

//---------------------------------------------------------------------------------------------------
/**
 * Inserts message into outgoing table.
 *
 * @return
 * - LE_OK if adding was successful
 * - LE_FAULT if it failed
 */
 //--------------------------------------------------------------------------------------------------
le_result_t mqttDatabase_addOutgoing(const char* topic, mqttClient_msg_t* message)
{
  le_result_t result = LE_FAULT;

  char* msg = sqlite_str((char*)message->payload);
  if(msg == NULL) {
    return result;
  }

  char* uuid = make_uuid();
  if(uuid == NULL) {
    LE_ERROR("make_uuid error");
    goto result_exit;
  }

  result = mqttDatabase_queryExec(
    "INSERT INTO outgoing(UUID,TOPIC,MESSAGE,MESSAGE_LEN)"
    " VALUES('%s','%s','%s',%d);",
    uuid, topic, msg, message->payloadLen);

  LE_ERROR_IF(result, "Failed to add outgoing message");

result_exit:
  if (uuid) free(uuid);
  if (msg)  free(msg);
  return result;
}

//---------------------------------------------------------------------------------------------------
/**
 * Deletes a message from the outgoing table.
 *
 * uuid The uuid of the message to be deleted
 *
 * @return
 * - LE_OK if deleting was successful
 * - LE_FAULT if it failed
 */
//---------------------------------------------------------------------------------------------------
le_result_t mqttDatabase_deleteOutgoing(const char* uuid)
{
  return mqttDatabase_queryExec("DELETE FROM outgoing WHERE UUID='%s';", uuid);
}
