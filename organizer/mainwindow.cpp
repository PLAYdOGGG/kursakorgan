#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QColorDialog>
#include <QStyle>
#include <QMenu>
#include <QFileDialog>
#include <QApplication>
#include "googlecalendarsync.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Student Organizer");
    setMinimumSize(800, 600);

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    createScheduleTab();
    createTaskTab();
    createSettingsTab();

    setupSystemTray();
    loadData();

    reminderTimer = new QTimer(this);
    connect(reminderTimer, &QTimer::timeout, this, &MainWindow::showReminder);
    reminderTimer->start(60000);

    autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::saveData);
    autoSaveTimer->start(300000);

    googleCalendarSync = new GoogleCalendarSync(this);
    connect(googleCalendarSync, &GoogleCalendarSync::syncCompleted, this, &MainWindow::onGoogleCalendarSyncCompleted);
    connect(googleCalendarSync, &GoogleCalendarSync::syncFailed, this, &MainWindow::onGoogleCalendarSyncFailed);
}

MainWindow::~MainWindow()
{
    saveData();
}

void MainWindow::createScheduleTab()
{
    QWidget *scheduleTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(scheduleTab);

    calendar = new QCalendarWidget();
    layout->addWidget(calendar);

    eventList = new QListWidget();
    layout->addWidget(eventList);

    QHBoxLayout *addEventLayout = new QHBoxLayout();
    eventNameEdit = new QLineEdit();
    eventDateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    eventRepeatCombo = new QComboBox();
    eventRepeatCombo->addItems({"No repeat", "Daily", "Weekly", "Monthly"});
    addEventButton = new QPushButton("Add Event");

    addEventLayout->addWidget(new QLabel("Event:"));
    addEventLayout->addWidget(eventNameEdit);
    addEventLayout->addWidget(new QLabel("Date/Time:"));
    addEventLayout->addWidget(eventDateTimeEdit);
    addEventLayout->addWidget(new QLabel("Repeat:"));
    addEventLayout->addWidget(eventRepeatCombo);
    addEventLayout->addWidget(addEventButton);

    layout->addLayout(addEventLayout);

    connect(addEventButton, &QPushButton::clicked, this, &MainWindow::addEvent);
    connect(calendar, &QCalendarWidget::activated, this, &MainWindow::updateEventList);

    tabWidget->addTab(scheduleTab, "Schedule");
}

void MainWindow::createTaskTab()
{
    QWidget *taskTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(taskTab);

    taskList = new QListWidget();
    layout->addWidget(taskList);

    QHBoxLayout *addTaskLayout = new QHBoxLayout();
    taskNameEdit = new QLineEdit();
    taskPriorityCombo = new QComboBox();
    taskPriorityCombo->addItems({"Low", "Medium", "High"});
    taskDeadlineEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    addTaskButton = new QPushButton("Add Task");

    addTaskLayout->addWidget(new QLabel("Task:"));
    addTaskLayout->addWidget(taskNameEdit);
    addTaskLayout->addWidget(new QLabel("Priority:"));
    addTaskLayout->addWidget(taskPriorityCombo);
    addTaskLayout->addWidget(new QLabel("Deadline:"));
    addTaskLayout->addWidget(taskDeadlineEdit);
    addTaskLayout->addWidget(addTaskButton);

    layout->addLayout(addTaskLayout);

    connect(addTaskButton, &QPushButton::clicked, this, &MainWindow::addTask);
    connect(taskList, &QListWidget::itemDoubleClicked, this, &MainWindow::markTaskCompleted);

    tabWidget->addTab(taskTab, "Tasks");
}

void MainWindow::createSettingsTab()
{
    QWidget *settingsTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(settingsTab);

    QHBoxLayout *reminderLayout = new QHBoxLayout();
    reminderTimeSpinBox = new QSpinBox();
    reminderTimeSpinBox->setRange(1, 60);
    reminderUnitCombo = new QComboBox();
    reminderUnitCombo->addItems({"Minutes", "Hours", "Days"});
    reminderLayout->addWidget(new QLabel("Remind me"));
    reminderLayout->addWidget(reminderTimeSpinBox);
    reminderLayout->addWidget(reminderUnitCombo);
    reminderLayout->addWidget(new QLabel("before event/deadline"));
    layout->addLayout(reminderLayout);

    autoSaveCheckBox = new QCheckBox("Auto-save data");
    autoSaveCheckBox->setChecked(true);
    layout->addWidget(autoSaveCheckBox);

    exportButton = new QPushButton("Export Data");
    importButton = new QPushButton("Import Data");
    syncButton = new QPushButton("Sync with Google Calendar");
    colorSchemeButton = new QPushButton("Change Color Scheme");

    layout->addWidget(exportButton);
    layout->addWidget(importButton);
    layout->addWidget(syncButton);
    layout->addWidget(colorSchemeButton);

    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportData);
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importData);
    connect(syncButton, &QPushButton::clicked, this, &MainWindow::syncWithGoogleCalendar);
    connect(colorSchemeButton, &QPushButton::clicked, this, &MainWindow::changeColorScheme);

    tabWidget->addTab(settingsTab, "Settings");
}

void MainWindow::setupSystemTray()
{
    trayIcon = new QSystemTrayIcon(QIcon(":/icons/app_icon.png"), this);
    trayIcon->setToolTip("Student Organizer");
    trayIcon->show();

    QMenu *trayMenu = new QMenu();
    trayMenu->addAction("Open", this, &MainWindow::show);
    trayMenu->addAction("Exit", qApp, &QApplication::quit);
    trayIcon->setContextMenu(trayMenu);

    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::show);
}

void MainWindow::addEvent()
{
    QString name = eventNameEdit->text();
    QDateTime dateTime = eventDateTimeEdit->dateTime();
    int repeatInterval = eventRepeatCombo->currentIndex() * 7;

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Event name cannot be empty.");
        return;
    }

    events.append(Event(name, dateTime, repeatInterval));
    updateEventList();
    eventNameEdit->clear();
}

void MainWindow::addTask()
{
    QString name = taskNameEdit->text();
    Task::Priority priority = static_cast<Task::Priority>(taskPriorityCombo->currentIndex());
    QDateTime deadline = taskDeadlineEdit->dateTime();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Task name cannot be empty.");
        return;
    }

    tasks.append(Task(name, priority, deadline));
    updateTaskList();
    taskNameEdit->clear();
}

void MainWindow::updateEventList()
{
    eventList->clear();
    QDate selectedDate = calendar->selectedDate();

    for (const Event &event : events) {
        if (event.dateTime().date() == selectedDate) {
            QString eventText = QString("%1 - %2")
                .arg(event.dateTime().toString("hh:mm"))
                .arg(event.name());
            eventList->addItem(eventText);
        }
    }
}

void MainWindow::updateTaskList()
{
    taskList->clear();
    for (const Task &task : tasks) {
        QString priorityStr;
        switch (task.priority()) {
            case Task::Low: priorityStr = "Low"; break;
            case Task::Medium: priorityStr = "Medium"; break;
            case Task::High: priorityStr = "High"; break;
        }

        QString taskText = QString("%1 - %2 - Due: %3")
            .arg(task.name())
            .arg(priorityStr)
            .arg(task.deadline().toString("yyyy-MM-dd hh:mm"));

        QListWidgetItem *item = new QListWidgetItem(taskText);
        if (task.isCompleted()) {
            item->setForeground(Qt::gray);
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
        taskList->addItem(item);
    }
}

void MainWindow::markTaskCompleted(QListWidgetItem* item)
{
    int index = taskList->row(item);
    tasks[index].setCompleted(!tasks[index].isCompleted());
    updateTaskList();
}

void MainWindow::showReminder()
{
    QDateTime now = QDateTime::currentDateTime();
    int reminderTime = reminderTimeSpinBox->value();
    int reminderUnit = reminderUnitCombo->currentIndex();

    for (const Event &event : events) {
        QDateTime reminderDateTime = event.dateTime();
        switch (reminderUnit) {
            case 0: reminderDateTime = reminderDateTime.addSecs(-reminderTime * 60); break;
            case 1: reminderDateTime = reminderDateTime.addSecs(-reminderTime * 3600); break;
            case 2: reminderDateTime = reminderDateTime.addDays(-reminderTime); break;
        }

        if (now >= reminderDateTime && now < event.dateTime()) {
            trayIcon->showMessage("Upcoming Event", event.name(), QSystemTrayIcon::Information, 10000);
        }
    }

    for (const Task &task : tasks) {
        if (task.isCompleted()) continue;

        QDateTime reminderDateTime = task.deadline();
        switch (reminderUnit) {
            case 0: reminderDateTime = reminderDateTime.addSecs(-reminderTime * 60); break;
            case 1: reminderDateTime = reminderDateTime.addSecs(-reminderTime * 3600); break;
            case 2: reminderDateTime = reminderDateTime.addDays(-reminderTime); break;
        }

        if (now >= reminderDateTime && now < task.deadline()) {
            trayIcon->showMessage("Upcoming Deadline", task.name(), QSystemTrayIcon::Information, 10000);
        }
    }
}

void MainWindow::exportData()
{
    QJsonObject data;
    QJsonArray eventsArray;
    for (const Event &event : events) {
        QJsonObject eventObj;
        eventObj["name"] = event.name();
        eventObj["dateTime"] = event.dateTime().toString(Qt::ISODate);
        eventObj["repeatInterval"] = event.repeatInterval();
        eventsArray.append(eventObj);
    }
    data["events"] = eventsArray;

    QJsonArray tasksArray;
    for (const Task &task : tasks) {
        QJsonObject taskObj;
        taskObj["name"] = task.name();
        taskObj["priority"] = task.priority();
        taskObj["deadline"] = task.deadline().toString(Qt::ISODate);
        taskObj["completed"] = task.isCompleted();
        tasksArray.append(taskObj);
    }
    data["tasks"] = tasksArray;

    QJsonDocument doc(data);
    QString fileName = QFileDialog::getSaveFileName(this, "Export Data", "", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
            QMessageBox::information(this, "Export Successful", "Data has been exported successfully.");
        } else {
            QMessageBox::critical(this, "Export Failed", "Unable to open file for writing.");
        }
    }
}

void MainWindow::importData()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import Data", "", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray saveData = file.readAll();
            QJsonDocument doc(QJsonDocument::fromJson(saveData));
            QJsonObject data = doc.object();

            events.clear();
            QJsonArray eventsArray = data["events"].toArray();
            for (const QJsonValue &value : eventsArray) {
                QJsonObject eventObj = value.toObject();
                QString name = eventObj["name"].toString();
                QDateTime dateTime = QDateTime::fromString(eventObj["dateTime"].toString(), Qt::ISODate);
                int repeatInterval = eventObj["repeatInterval"].toInt();
                events.append(Event(name, dateTime, repeatInterval));
            }

            tasks.clear();
            QJsonArray tasksArray = data["tasks"].toArray();
            for (const QJsonValue &value : tasksArray) {
                QJsonObject taskObj = value.toObject();
                QString name = taskObj["name"].toString();
                Task::Priority priority = static_cast<Task::Priority>(taskObj["priority"].toInt());
                QDateTime deadline = QDateTime::fromString(taskObj["deadline"].toString(), Qt::ISODate);
                bool completed = taskObj["completed"].toBool();
                Task task(name, priority, deadline);
                task.setCompleted(completed);
                tasks.append(task);
            }

            updateEventList();
            updateTaskList();
            QMessageBox::information(this, "Import Successful", "Data has been imported successfully.");
        } else {
            QMessageBox::critical(this, "Import Failed", "Unable to open file for reading.");
        }
    }
}

void MainWindow::syncWithGoogleCalendar()
{
    googleCalendarSync->syncEvents(events);
}

void MainWindow::onGoogleCalendarSyncCompleted()
{
    QMessageBox::information(this, "Sync Successful", "Events have been synced with Google Calendar.");
}

void MainWindow::onGoogleCalendarSyncFailed(const QString& error)
{
    QMessageBox::critical(this, "Sync Failed", "Failed to sync with Google Calendar: " + error);
}

void MainWindow::changeColorScheme()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color Scheme");
    if (color.isValid()) {
        QPalette palette;
        palette.setColor(QPalette::Window, color);
        palette.setColor(QPalette::WindowText, Qt::black);
        palette.setColor(QPalette::Base, color.lighter(120));
        palette.setColor(QPalette::AlternateBase, color.lighter(130));
        palette.setColor(QPalette::ToolTipBase, Qt::white);
        palette.setColor(QPalette::ToolTipText, Qt::black);
        palette.setColor(QPalette::Text, Qt::black);
        palette.setColor(QPalette::Button, color);
        palette.setColor(QPalette::ButtonText, Qt::black);
        palette.setColor(QPalette::Link, Qt::blue);
        palette.setColor(QPalette::Highlight, Qt::blue);
        palette.setColor(QPalette::HighlightedText, Qt::white);

        qApp->setPalette(palette);
    }
}

void MainWindow::loadData()
{
    QFile file("organizer_data.json");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray saveData = file.readAll();
        QJsonDocument doc(QJsonDocument::fromJson(saveData));
        QJsonObject data = doc.object();

        QJsonArray eventsArray = data["events"].toArray();
        for (const QJsonValue &value : eventsArray) {
            QJsonObject eventObj = value.toObject();
            QString name = eventObj["name"].toString();
            QDateTime dateTime = QDateTime::fromString(eventObj["dateTime"].toString(), Qt::ISODate);
            int repeatInterval = eventObj["repeatInterval"].toInt();
            events.append(Event(name, dateTime, repeatInterval));
        }

        QJsonArray tasksArray = data["tasks"].toArray();
        for (const QJsonValue &value : tasksArray) {
            QJsonObject taskObj = value.toObject();
            QString name = taskObj["name"].toString();
            Task::Priority priority = static_cast<Task::Priority>(taskObj["priority"].toInt());
            QDateTime deadline = QDateTime::fromString(taskObj["deadline"].toString(), Qt::ISODate);
            bool completed = taskObj["completed"].toBool();
            Task task(name, priority, deadline);
            task.setCompleted(completed);
            tasks.append(task);
        }

        updateEventList();
        updateTaskList();
    }
}

void MainWindow::saveData()
{
    QJsonObject data;
    QJsonArray eventsArray;
    for (const Event &event : events) {
        QJsonObject eventObj;
        eventObj["name"] = event.name();
        eventObj["dateTime"] = event.dateTime().toString(Qt::ISODate);
        eventObj["repeatInterval"] = event.repeatInterval();
        eventsArray.append(eventObj);
    }
    data["events"] = eventsArray;

    QJsonArray tasksArray;
    for (const Task &task : tasks) {
        QJsonObject taskObj;
        taskObj["name"] = task.name();
        taskObj["priority"] = task.priority();
        taskObj["deadline"] = task.deadline().toString(Qt::ISODate);
        taskObj["completed"] = task.isCompleted();
        tasksArray.append(taskObj);
    }
    data["tasks"] = tasksArray;

    QJsonDocument doc(data);
    QFile file("organizer_data.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

