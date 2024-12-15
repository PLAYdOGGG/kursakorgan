#include "googlecalendarsync.h"
#include <QUrlQuery>
#include <QTimeZone>
#include <QOAuth2AuthorizationCodeFlow>

GoogleCalendarSync::GoogleCalendarSync(QObject *parent)
    : QObject(parent)
    , oauth2(new QOAuth2AuthorizationCodeFlow(this))
    , networkManager(new QNetworkAccessManager(this))
{
    setupOAuth2();
    connect(networkManager, &QNetworkAccessManager::finished, this, &GoogleCalendarSync::handleNetworkReply);
}

GoogleCalendarSync::~GoogleCalendarSync()
{
    delete oauth2;
    delete networkManager;
}

void GoogleCalendarSync::setupOAuth2()
{
    const QString clientId = "YOUR_CLIENT_ID";
    const QString clientSecret = "YOUR_CLIENT_SECRET";

    oauth2->setScope("https://www.googleapis.com/auth/calendar");
    oauth2->setAuthorizationUrl(QUrl("https://accounts.google.com/o/oauth2/auth"));
    oauth2->setAccessTokenUrl(QUrl("https://oauth2.googleapis.com/token"));
    oauth2->setClientIdentifier(clientId);
    oauth2->setClientIdentifierSharedKey(clientSecret);

    connect(oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            [](const QUrl &url) { QDesktopServices::openUrl(url); });
    connect(oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, &GoogleCalendarSync::onAuthenticationSuccess);
}

void GoogleCalendarSync::authenticate()
{
    oauth2->grant();
}

void GoogleCalendarSync::onAuthenticationSuccess()
{
    accessToken = oauth2->token();
    fetchCalendarEvents();
}

void GoogleCalendarSync::onAuthenticationFailed(const QString& error)
{
    emit syncFailed("Authentication failed: " + error);
}

void GoogleCalendarSync::handleNetworkReply(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit syncFailed(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull()) {
        emit syncFailed("Invalid JSON response");
        return;
    }

    QString requestPath = reply->url().path();
    if (requestPath.contains("/calendars/primary/events")) {
        if (!pendingEvents.isEmpty()) {
            pushEventsToCalendar(pendingEvents);
            pendingEvents.clear();
        } else {
            emit syncCompleted();
        }
    }
}

void GoogleCalendarSync::syncEvents(const QVector<Event>& events)
{
    if (accessToken.isEmpty()) {
        pendingEvents = events;
        authenticate();
        return;
    }
    pushEventsToCalendar(events);
}

void GoogleCalendarSync::fetchCalendarEvents()
{
    QUrl url("https://www.googleapis.com/calendar/v3/calendars/primary/events");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    networkManager->get(request);
}

void GoogleCalendarSync::pushEventsToCalendar(const QVector<Event>& events)
{
    for (const Event& event : events) {
        QJsonObject eventObject;
        eventObject["summary"] = event.name();

        QJsonObject start;
        start["dateTime"] = event.dateTime().toString(Qt::ISODate);
        start["timeZone"] = QString::fromUtf8(QTimeZone::systemTimeZone().id());
        eventObject["start"] = start;

        QJsonObject end;
        end["dateTime"] = event.dateTime().addSecs(3600).toString(Qt::ISODate);
        end["timeZone"] = QString::fromUtf8(QTimeZone::systemTimeZone().id());
        eventObject["end"] = end;

        QJsonDocument doc(eventObject);
        QByteArray data = doc.toJson();

        QUrl url("https://www.googleapis.com/calendar/v3/calendars/primary/events");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());

        networkManager->post(request, data);
    }
}
