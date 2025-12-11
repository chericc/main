#include <optional>

#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlpp11.h"
#include "sqlpp11/value_or_null.h"
#include "sqlpp11/serializer_context.h"

#include "xlog.h"
#include "student.h"

namespace sql = sqlpp::sqlite3;

template <typename Type>
sqlpp::value_or_null_t<Type> to_sql(std::string name)
{
    if (name.empty()) {
        return sqlpp::value_or_null<Type>(sqlpp::null);
    } else {
        return sqlpp::value_or_null(name);
    }
}


struct _serializer_context_t
{
    ::sqlpp::detail::float_safe_ostringstream _os;

    _serializer_context_t() = default;
    _serializer_context_t(const _serializer_context_t& rhs)
    {
        _os << rhs._os.str();
    }

    std::string str() const
    {
        return _os.str();
    }

    void reset()
    {
        _os.str("");
    }

    template <typename T>
    ::sqlpp::detail::float_safe_ostringstream& operator<<(T t)
    {
        return _os << t;
    }

    static std::string escape(std::string arg)
    {
        return sqlpp::serializer_context_t::escape(arg);
    }
};

int main(int argc, char *argv[])
{
    if (argc < 2) {
        xlog_err("usage: %s [dbfile]", argv[0]);
        return 1;
    }
    std::string dbfile = argv[1];

    sql::connection_config config;
    config.path_to_database = dbfile;
    config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    config.debug = false;

    sql::connection db(config);

    student::TableStudent stu;
    student::TableStudentClassRel stu_cla_rel;

    std::string name;

    db(insert_into(stu).set(
        stu.number = "0001", 
        stu.name = "zhangsan", 
        stu.gender = "boy",
        // stu.age = sqlpp::value_or_null(name)
        stu.age = to_sql<sqlpp::text>(name)
    ));
    auto id = db.last_insert_id();
    for (auto const& ref : db(select(stu.number, stu.name).from(stu).where(stu.id == id))) {
        xlog_dbg("name=%s, number=%s", ref.name.text, ref.number.text);
    }

    _serializer_context_t seri;
    auto str = sqlpp::serialize(select(stu.number), seri);
    xlog_dbg("se: %s", str.str().c_str());

    // join
    for (auto const& row : db(select(stu.name, stu_cla_rel.studentId).
        unconditionally().from(stu.left_outer_join(stu_cla_rel).on(stu_cla_rel.studentId == stu.id)))) {
        xlog_dbg("name=%s,studentid.null=%d", row.name.text, (int)row.studentId.is_null());
    }

    return 0;
}