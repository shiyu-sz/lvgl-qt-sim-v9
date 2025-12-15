#include "mainwindow.h"
#include "lvgl.h"
#include "lvgl_hal.h"

extern "C" {
#include "app_ui.h"
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , display_image(LV_HOR_RES_MAX, LV_VER_RES_MAX, QImage::Format_RGB16)
{
    setWindowTitle("LVGL Qt Simulator V9");

    // 创建一个顶层的widget
    QWidget *widget = new QWidget();
    this->setCentralWidget(widget);

    lb_display = new ClickableLabel();
    lb_display->setMaximumWidth(LV_HOR_RES_MAX);
    lb_display->setMaximumHeight(LV_VER_RES_MAX);

    // 垂直布局
    QVBoxLayout *vLayout_1 = new QVBoxLayout();
    vLayout_1->addWidget(lb_display);

    widget->setLayout(vLayout_1);

    gMainObj = this;

    installEventFilter(this);
    connect(lb_display, SIGNAL(mousePressed(int, int)), this, SLOT(onMousePressed(int, int)));
    connect(lb_display, SIGNAL(mouseReleased(int, int)), this, SLOT(onMouseReleased(int, int)));

    LvglThread *lvgl_thread = new LvglThread(this);
    lvgl_thread->start();
}

static int last_state = LV_INDEV_STATE_REL;

void MainWindow::onMousePressed(int x, int y)
{
    qDebug("Mouse pressed %d, %d\n", x, y);
    lv_integr_update_pointer(x, y, LV_INDEV_STATE_PR);
    last_state = LV_INDEV_STATE_PR;
}

void MainWindow::onMouseReleased(int x, int y)
{
    printf("Mouse released %d, %d\n", x, y);
    lv_integr_update_pointer(x, y, LV_INDEV_STATE_REL);
    last_state = LV_INDEV_STATE_REL;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::MouseMove)
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    statusBar()->showMessage(QString("Mouse move (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));
    int x = mouseEvent->pos().x();
    int y = mouseEvent->pos().y();
    printf("Mouse move %d, %d\n", x, y);
    lv_integr_update_pointer(x, y, last_state);
  }
  return false;
}

void MainWindow::onMouseMoved(int x, int y)
{
    printf("Mouse released %d, %d\n", x, y);
    lv_integr_update_pointer(x, y, last_state);
}

MainWindow::~MainWindow() {}
