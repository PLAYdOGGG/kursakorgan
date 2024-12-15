#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QCalendarWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QMessageBox>
#include <QMenu>
#include "event.h"
#include "task.h"
#include "googlecalendarsync.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addEvent();
    void addTask();
    void updateEventList();
    void updateTaskList();
    void markTaskCompleted(QListWidgetItem* item);
    void showReminder();
    void exportData();
    void importData();
    void syncWithGoogleCalendar();
    void changeColorScheme();
    void onGoogleCalendarSyncCompleted();
    void onGoogleCalendarSyncFailed(const QString& error);

private:
    void createScheduleTab();
    void createTaskTab();
    void createSettingsTab();
    void setupSystemTray();
    void loadData();
    void saveData();
    void updateCalendarDisplay();

    Ui::MainWindow *ui;
    QTabWidget *tabWidget;
    QCalendarWidget *calendar;
    QListWidget *eventList;
    QListWidget *taskList;
    QLineEdit *eventNameEdit;
    QDateTimeEdit *eventDateTimeEdit;
    QComboBox *eventRepeatCombo;
    QLineEdit *taskNameEdit;
    QComboBox *taskPriorityCombo;
    QDateTimeEdit *taskDeadlineEdit;
    QPushButton *addEventButton;
    QPushButton *addTaskButton;
    QSpinBox *reminderTimeSpinBox;
    QComboBox *reminderUnitCombo;
    QCheckBox *autoSaveCheckBox;
    QPushButton *exportButton;
    QPushButton *importButton;
    QPushButton *syncButton;
    QPushButton *colorSchemeButton;

    QSystemTrayIcon *trayIcon;
    QTimer *reminderTimer;
    QTimer *autoSaveTimer;

    QVector<Event> events;
    QVector<Task> tasks;

    GoogleCalendarSync *googleCalendarSync;
};

#endif // MAINWINDOW_H
