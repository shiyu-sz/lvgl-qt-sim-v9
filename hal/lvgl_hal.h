
#ifndef __LVGL_HAL_H__
#define __LVGL_HAL_H__

#include "mainwindow.h"
#include <QThread>

#define LVGL_TICK_TIME 10 // 10 milliseconds

extern MainWindow * gMainObj;

#ifdef __cplusplus
extern "C" {
#endif

void lv_integr_update_pointer(int x, int y, int state);

#ifdef __cplusplus
};
#endif

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

#endif
