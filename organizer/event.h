#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QDateTime>

class Event
{
public:
    Event(const QString &name, const QDateTime &dateTime, int repeatInterval = 0);

    QString name() const;
    QDateTime dateTime() const;
    int repeatInterval() const;

private:
    QString m_name;
    QDateTime m_dateTime;
    int m_repeatInterval;
};

#endif // EVENT_H
