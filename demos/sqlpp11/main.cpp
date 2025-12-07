#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlpp11.h"

#include "xlog.h"
#include "student.h"

namespace sql = sqlpp::sqlite3;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        xlog_err("usage: %s [dbfile]", argv[0]);
        return 1;
    }
    std::string dbfile = argv[1];

    sql::connection_config config;
    config.path_to_database = "main.db";
    config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    config.debug = true;

    sql::connection db(config);

    student::TableStudent stu;
    student::TableStudentClassRel stu_cla_rel;

    db(insert_into(stu).set(stu.number = "0001", stu.name = "zhangsan", stu.gender = "boy"));
    auto id = db.last_insert_id();
    for (auto const& ref : db(select(stu.number, stu.name).from(stu).where(stu.id == id))) {
        xlog_dbg("name=%s, number=%s", ref.name.text, ref.number.text);
    }

    // join
    for (auto const& row : db(select(stu.name, stu_cla_rel.studentId).
        unconditionally().from(stu.left_outer_join(stu_cla_rel).on(stu_cla_rel.studentId == stu.id)))) {
        xlog_dbg("name=%s,studentid.null=%d", row.name.text, (int)row.studentId.is_null());
    }

    return 0;
}