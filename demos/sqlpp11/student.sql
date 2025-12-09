
CREATE TABLE table_student (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    number TEXT UNIQUE NOT NULL, -- 学号
    name TEXT NOT NULL,
    gender TEXT NOT NULL CHECK(gender in('boy','girl')),
    age TEXT
) STRICT;

CREATE TABLE table_class (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL
) STRICT;

CREATE TABLE table_student_class_rel (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id INTEGER NOT NULL,
    class_id INTEGER NOT NULL,
    UNIQUE(student_id, class_id) -- 一个学生只能属于一个班级 
) STRICT;