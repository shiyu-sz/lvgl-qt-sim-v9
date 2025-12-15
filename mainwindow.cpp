#include "mainwindow.h"
#include "lvgl.h"
#include "demos/lv_demos.h"

MainWindow * gMainObj;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , display_image(LV_HOR_RES_MAX, LV_VER_RES_MAX, QImage::Format_RGB16)
{
    setWindowTitle("LVGL Qt Simulator");

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

//----------- C++ functions ---------
static void updateDisplay (const lv_area_t * area, uint8_t * color_p, bool last)
{
    int32_t x, y;
    uint16_t *color_p_16 = (uint16_t *)color_p;
    lv_color16_t pixel;

    QRgb pixel_output;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            memcpy(&pixel, color_p_16, 2);
            pixel_output = pixel.red << (16 + 3);
            pixel_output |= pixel.green << (8 + 2);
            pixel_output |= pixel.blue << 3;

            gMainObj->display_image.setPixelColor(x,y, pixel_output);
            color_p_16++;
        }
    }
    if (last) {
        gMainObj->lb_display->setPixmap(QPixmap::fromImage(gMainObj->display_image));
    }
}

#ifdef __cplusplus
extern "C" {
#endif

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */
#define BUFF_SIZE (LV_HOR_RES_MAX * 10 * BYTE_PER_PIXEL)
uint8_t buf_1[BUFF_SIZE];
uint8_t buf_2[BUFF_SIZE];

static int touchpad_x = 0, touchpad_y = 0;
static lv_indev_state_t touchpad_state = LV_INDEV_STATE_REL;
static lv_indev_state_t touchpad_old_state = LV_INDEV_STATE_REL;

void lv_integr_update_pointer(int x, int y, int state)
{
    touchpad_x = x;
    touchpad_y = y;
    touchpad_state = (lv_indev_state_t)state;
}

void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    bool last = lv_disp_flush_is_last(disp);
    updateDisplay(area, px_map, last);
    lv_disp_flush_ready(disp);         /* Indicate you are ready with the flushing*/
}

void touchpad_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    Q_UNUSED(indev);
    data->point.x = touchpad_x;
    data->point.y = touchpad_y;
    data->state = touchpad_state; //LV_INDEV_STATE_REL; //LV_INDEV_STATE_PR or LV_INDEV_STATE_REL;
    if ( touchpad_state != touchpad_old_state) {
        touchpad_old_state = touchpad_state;
        qDebug("mouse down: x=%d y=%d", touchpad_x, touchpad_y);
    }   /*No buffering now so no more data read*/
}

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
void hal_init()
{
    lv_display_t * disp;
    lv_group_set_default(lv_group_create());
    disp = lv_display_create(LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_display_set_flush_cb(disp, disp_flush);

    lv_display_set_buffers(disp, buf_1, buf_2, sizeof(buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_default(disp);

    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchpad_read);
    lv_indev_set_mode(indev, LV_INDEV_MODE_TIMER);
    lv_indev_set_display(indev, disp);
    lv_indev_set_group(indev, lv_group_get_default());
}

void lv_example_btn_1(void)
{
    lv_obj_t * label;

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "Toggle");
    lv_obj_center(label);
}

void lvgl_app_main(void)
{
    // lv_demo_music();
    lv_example_btn_1();
}

#ifdef __cplusplus
};
#endif

LvglThread::LvglThread(QObject *parent) : QThread(parent)
{

}

void LvglThread::run()
{
    lv_init();
    hal_init();

    lvgl_app_main();
    while(1) {
        lv_tick_inc(10);
        lv_task_handler();
        QThread::msleep(10);
    }
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
