#pragma once

#include <libpq-fe.h>

#include "scheduler.h"

PGconn* database_connect();
void database_disconnect(PGconn* conn);
void database_setup(PGconn* conn);
void database_insert_remind(PGconn* conn, SchedulerMessage message);
