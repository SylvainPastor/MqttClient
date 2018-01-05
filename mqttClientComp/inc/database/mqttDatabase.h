/**
 * @file
 *
 * <HR>
 *
 * Copyright (C) .
 */

#ifndef __MQTT_DATABASE_H_
#define __MQTT_DATABASE_H_

#include "legato.h"
#include "interfaces.h"
#include "mqttMain.h"

#include <sqlite3.h>

le_result_t mqttDatabase_init();
void mqttDatabase_close();
sqlite3* mqttDatabase_getDB();

le_result_t mqttDatabase_queryExec(const char* sql, ...);
int mqttDatabase_queryInteger(const char* sql, ...);
char* mqttDatabase_queryString(const char* sql, ...);

int mqttDatabase_countOutgoing();
le_result_t mqttDatabase_addOutgoing(const char* topic, mqttClient_msg_t* message);
le_result_t mqttDatabase_deleteOutgoing(const char* uuid);

#endif // __MQTT_DATABASE_H_