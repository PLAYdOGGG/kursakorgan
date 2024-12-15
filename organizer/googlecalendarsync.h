#ifndef GOOGLECALENDARSYNC_H
#define GOOGLECALENDARSYNC_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>
#include <QDateTime>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDesktopServices>
#include "event.h"

class GoogleCalendarSync : public QObject
{
    Q_OBJECT

public:
    explicit GoogleCalendarSync(QObject *parent = nullptr);
    ~GoogleCalendarSync();
    void authenticate();
    void syncEvents(const QVector<Event>& events);

signals:
    void syncCompleted();
    void syncFailed(const QString& error);

private slots:
    void onAuthenticationSuccess();
    void onAuthenticationFailed(const QString& error);
    void handleNetworkReply(QNetworkReply *reply);

private:
    void setupOAuth2();
    void fetchCalendarEvents();
    void pushEventsToCalendar(const QVector<Event>& events);

    QOAuth2AuthorizationCodeFlow *oauth2;
    QNetworkAccessManager *networkManager;
    QString accessToken;
    QVector<Event> pendingEvents;
};

#endif // GOOGLECALENDARSYNC_H