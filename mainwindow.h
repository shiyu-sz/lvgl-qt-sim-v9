#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QImage>
#include <QThread>
#include "clabel.h"

class LvglThread : public QThread
{
    Q_OBJECT
public:
    explicit LvglThread(QObject *parent = nullptr);

protected:
    void run();

signals:

private slots:

};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QImage display_image;
    ClickableLabel *lb_display;
};
#endif // MAINWINDOW_H
