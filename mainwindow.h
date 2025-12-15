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

private slots:
    void onMousePressed(int x, int y);
    void onMouseReleased(int x, int y);
    void onMouseMoved(int x, int y);

private:
    virtual bool eventFilter(QObject *obj, QEvent *event);

};
#endif // MAINWINDOW_H
