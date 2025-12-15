#include "lvgl.h"
#include "lvgl_hal.h"
#include "app_ui.h"

MainWindow * gMainObj;

//----------- C++ functions ---------
static void updateDisplay(const lv_area_t * area, uint8_t * color_p, bool last)
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
        lv_tick_inc(LVGL_TICK_TIME);
        lv_task_handler();
        QThread::msleep(LVGL_TICK_TIME);
    }
}
