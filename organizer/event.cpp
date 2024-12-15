#include "event.h"

Event::Event(const QString &name, const QDateTime &dateTime, int repeatInterval)
    : m_name(name), m_dateTime(dateTime), m_repeatInterval(repeatInterval)
{
}

QString Event::name() const
{
    return m_name;
}

QDateTime Event::dateTime() const
{
    return m_dateTime;
}

int Event::repeatInterval() const
{
    return m_repeatInterval;
}