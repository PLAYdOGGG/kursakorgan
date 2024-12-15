#ifndef COURSE_H
#define COURSE_H

#include <QString>

class Course {
public:
    Course(const QString& n, const QString& c, const QString& i, const QString& s);

    QString name;
    QString code;
    QString instructor;
    QString schedule;
};

#endif // COURSE_H