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

void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    bool last = lv_disp_flush_is_last(disp);
    updateDisplay(area, px_map, last);
    lv_disp_flush_ready(disp);         /* Indicate you are ready with the flushing*/
}

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
lv_display_t * hal_init()
{
    lv_display_t * disp;
    lv_group_set_default(lv_group_create());
    disp = lv_display_create(LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_display_set_flush_cb(disp, disp_flush);

    lv_display_set_buffers(disp, buf_1, buf_2, sizeof(buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_default(disp);

    // lv_indev_t * touchscreen = lv_gt911_touchscreen_create();
    // lv_indev_set_display(touchscreen, disp);
    // lv_indev_set_group(touchscreen, lv_group_get_default());

    return disp;
}

void lvgl_app_main(void)
{
    lv_demo_music();
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
    // init_pointer();
    lvgl_app_main();
    while(1) {
        lv_tick_inc(10);
        lv_task_handler();
        QThread::msleep(10);
    }
}

MainWindow::~MainWindow() {}
