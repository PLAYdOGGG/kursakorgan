#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>

class Task
{
public:
    enum Priority { Low, Medium, High };

    Task(const QString &name, Priority priority, const QDateTime &deadline);

    QString name() const;
    Priority priority() const;
    QDateTime deadline() const;
    bool isCompleted() const;
    void setCompleted(bool completed);

private:
    QString m_name;
    Priority m_priority;
    QDateTime m_deadline;
    bool m_completed;
};

#endif // TASK_H