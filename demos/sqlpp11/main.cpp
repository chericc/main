#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/sqlite3/sqlite3.h"

#include "xlog.h"

namespace sql = sqlpp::sqlite3;

int main()
{
    sql::connection_config config;
    config.path_to_database = "test.db";
    config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    config.debug = true;

    sql::connection db(config);
    db.execute(
R"(
create table student (
id integer primary key,
name text not null,
gender text not null
);
)"
    );

    return 0;
}