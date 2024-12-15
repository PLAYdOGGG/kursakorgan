#include "task.h"

Task::Task(const QString &name, Priority priority, const QDateTime &deadline)
    : m_name(name), m_priority(priority), m_deadline(deadline), m_completed(false)
{
}

QString Task::name() const
{
    return m_name;
}

Task::Priority Task::priority() const
{
    return m_priority;
}

QDateTime Task::deadline() const
{
    return m_deadline;
}

bool Task::isCompleted() const
{
    return m_completed;
}

void Task::setCompleted(bool completed)
{
    m_completed = completed;
}