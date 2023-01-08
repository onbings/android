#include "AndroidLvglDrv.h"

#include <unistd.h>

#include <bofstd/bofstd.h>
#include <bof2d/bof2d.h>
#include <lvgl/src/lvgl.h>

#define USER_DEFAULT_SCREEN_DPI 96
/* Scale window by this factor (useful when simulating small screens) */
# define ANDROID_DRV_MONITOR_ZOOM        1
#define MULDIV(a,b,c) (((int64_t)(a) * (int64_t)(b))/(int64_t)(c))

static bool S_lv_android_init(lv_coord_t _Width, lv_coord_t _Height);
static bool S_lv_android_add_all_input_devices_to_group(lv_group_t *_pGroup_X);
static void S_lv_android_display_driver_flush_callback(lv_disp_drv_t *_pDispDrv_X, const lv_area_t *_pArea_X, lv_color_t *_pColor_X);
static void S_lv_android_display_refresh_handler(lv_timer_t *_pTimer_X);
static void S_lv_android_pointer_driver_read_callback(lv_indev_drv_t *_pInDevDrv_X, lv_indev_data_t *_pInDevData_X);
static void S_lv_android_keypad_driver_read_callback(lv_indev_drv_t *_pInDevDrv_X, lv_indev_data_t *_pInDevData_X);
static void S_lv_android_encoder_driver_read_callback(lv_indev_drv_t *_pInDevDrv_X, lv_indev_data_t *_pInDevData_X);

/**********************
 *  GLOBAL VARIABLES
 **********************/

bool GL_lv_android_quit_signal_B = false;
lv_indev_t *GL_pLvPointerDevObj_X = nullptr;
lv_indev_t *GL_pLvKeypadDevObj_X = nullptr;
lv_indev_t *GL_pLvEncoderDevObj_X = nullptr;

/**********************
 *  STATIC VARIABLES
 **********************/

static uint32_t *S_pPixelBuffer_U32 = nullptr;
static BOF2D::BOF_SIZE S_PixelBufferSize_X = {0, 0};

static lv_disp_t *S_pDisplay_X = nullptr;
static bool volatile S_DisplayRefreshing_B = false;

static bool volatile S_MousePressed_B = false;
static BOF2D::BOF_POINT_2D volatile S_MousePos_X = {0, 0};

static bool volatile S_MouseWheelPressed_B = false;
static int16_t volatile S_MouseWheelValue_S16 = 0;

static bool volatile S_KeyboardPressed_B = false;
static uint16_t volatile S_KeyboardValue_U16 = 0;

static int volatile g_dpi_value = USER_DEFAULT_SCREEN_DPI;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
comparer les lv_conf
voire quoi pour le timer qui n est pas redefinit ici
quid pour keyboard'
quid pour wheel

bool AndroidLvlgDrvInit(Renderer *_pRenderer, uint16_t _Width_U16, uint16_t _Height_U16) {
    bool Rts_B = false;
    
    lv_init();
    if (S_lv_android_init(_Width_U16, _Height_U16)) {
        Rts_B = S_lv_android_add_all_input_devices_to_group(nullptr);
    }
    return Rts_B;
}

bool AndroidLvlgDrvRun() {
    bool Rts_B = true;

    lv_task_handler();
    usleep(5 * 1000);
    //lv_tick_inc(5);

    return Rts_B;
}

bool AndroidLvlgDrvShutdown() {
    bool Rts_B = true;

    return Rts_B;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static bool S_lv_android_add_all_input_devices_to_group(lv_group_t *_pGroup_X) {
    bool Rts_B = true;    
    if (!_pGroup_X) {
        LV_LOG_WARN("The _pGroup_X object is nullptr. Get the default _pGroup_X object instead.");

        _pGroup_X = lv_group_get_default();
        if (!_pGroup_X) {
            LV_LOG_WARN(
                    "The default _pGroup_X object is nullptr. Create a new _pGroup_X object "
                    "and set it to default instead.");

            _pGroup_X = lv_group_create();
            if (_pGroup_X) {
                lv_group_set_default(_pGroup_X);
            }
        }
    }

    LV_ASSERT_MSG(_pGroup_X, "Cannot obtain an available _pGroup_X object.");

    lv_indev_set_group(GL_pLvPointerDevObj_X, _pGroup_X);
    lv_indev_set_group(GL_pLvKeypadDevObj_X, _pGroup_X);
    lv_indev_set_group(GL_pLvEncoderDevObj_X, _pGroup_X);
    return Rts_B;
}

bool S_lv_android_init(lv_coord_t _Width, lv_coord_t _Height) {
bool Rts_B = true;

    static lv_disp_draw_buf_t S_DisplayBuffer_X;
#if (LV_COLOR_DEPTH == 32) || \
    (LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0) || \
    (LV_COLOR_DEPTH == 8) || \
    (LV_COLOR_DEPTH == 1)
    lv_disp_draw_buf_init(
        &S_DisplayBuffer_X,
        (lv_color_t*)S_pPixelBuffer_U32,
        nullptr,
        _Width * _Height);
#else
    lv_disp_draw_buf_init(
            &S_DisplayBuffer_X,
            (lv_color_t *) malloc(_Width * _Height * sizeof(lv_color_t)),
            nullptr,
            _Width * _Height);
#endif

    static lv_disp_drv_t S_DisplayDriver_X;
    lv_disp_drv_init(&S_DisplayDriver_X);
    S_DisplayDriver_X.hor_res = _Width;
    S_DisplayDriver_X.ver_res = _Height;
    S_DisplayDriver_X.flush_cb = S_lv_android_display_driver_flush_callback;
    S_DisplayDriver_X.draw_buf = &S_DisplayBuffer_X;
    S_DisplayDriver_X.direct_mode = 1;
    S_pDisplay_X = lv_disp_drv_register(&S_DisplayDriver_X);
    lv_timer_del(S_pDisplay_X->refr_timer);
    S_pDisplay_X->refr_timer = nullptr;
    lv_timer_create(S_lv_android_display_refresh_handler, 0, nullptr);

    static lv_indev_drv_t S_PointerDriver_X;
    lv_indev_drv_init(&S_PointerDriver_X);
    S_PointerDriver_X.type = LV_INDEV_TYPE_POINTER;
    S_PointerDriver_X.read_cb = S_lv_android_pointer_driver_read_callback;
    GL_pLvPointerDevObj_X = lv_indev_drv_register(&S_PointerDriver_X);

    static lv_indev_drv_t S_KeypadDriver_X;
    lv_indev_drv_init(&S_KeypadDriver_X);
    S_KeypadDriver_X.type = LV_INDEV_TYPE_KEYPAD;
    S_KeypadDriver_X.read_cb = S_lv_android_keypad_driver_read_callback;
    GL_pLvKeypadDevObj_X = lv_indev_drv_register(&S_KeypadDriver_X);

    static lv_indev_drv_t S_EncoderDriver_X;
    lv_indev_drv_init(&S_EncoderDriver_X);
    S_EncoderDriver_X.type = LV_INDEV_TYPE_ENCODER;
    S_EncoderDriver_X.read_cb = S_lv_android_encoder_driver_read_callback;
    GL_pLvEncoderDevObj_X = lv_indev_drv_register(&S_EncoderDriver_X);

    return Rts_B;
}

static void S_lv_android_display_driver_flush_callback(lv_disp_drv_t *_pDispDrv_X, const lv_area_t *_pArea_X, lv_color_t *_pColor_X) {
    if (lv_disp_flush_is_last(_pDispDrv_X)) {
#if (LV_COLOR_DEPTH == 32) || \
    (LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0) || \
    (LV_COLOR_DEPTH == 8) || \
    (LV_COLOR_DEPTH == 1)
        //UNREFERENCED_PARAMETER(_pColor_X);
#elif (LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP != 0)
        SIZE_T count = S_PixelBufferSize_X / sizeof(uint16_t);
        PUINT16 source = (PUINT16)_pColor_X;
        PUINT16 destination = (PUINT16)S_pPixelBuffer_U32;
        for (SIZE_T i = 0; i < count; ++i)
        {
            uint16_t current = *source;
            *destination = (LOBYTE(current) << 8) | HIBYTE(current);

            ++source;
            ++destination;
        }
#else
        for (int y = _pArea_X->y1; y <= _pArea_X->y2; ++y) {
            for (int x = _pArea_X->x1; x <= _pArea_X->x2; ++x) {
                S_pPixelBuffer_U32[y * _pDispDrv_X->hor_res + x] =
                        lv_color_to32(*_pColor_X);
                _pColor_X++;
            }
        }
#endif
    }
    lv_disp_flush_ready(_pDispDrv_X);
}

static void S_lv_android_display_refresh_handler(lv_timer_t *_pTimer_X) {
    //UNREFERENCED_PARAMETER(_pTimer_X);

    if (!S_DisplayRefreshing_B) {
        _lv_disp_refr_timer(nullptr);
    }
}


static void S_lv_android_pointer_driver_read_callback(lv_indev_drv_t *_pInDevDrv_X, lv_indev_data_t *_pInDevData_X) {
    //UNREFERENCED_PARAMETER(_pInDevDrv_X);

    _pInDevData_X->state = (lv_indev_state_t)(S_MousePressed_B ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL);

    _pInDevData_X->point.x = MULDIV(S_MousePos_X.x_S32, USER_DEFAULT_SCREEN_DPI, ANDROID_DRV_MONITOR_ZOOM * g_dpi_value);
    _pInDevData_X->point.y = MULDIV(S_MousePos_X.y_S32, USER_DEFAULT_SCREEN_DPI, ANDROID_DRV_MONITOR_ZOOM * g_dpi_value);

    if (_pInDevData_X->point.x < 0) {
        _pInDevData_X->point.x = 0;
    }
    if (_pInDevData_X->point.x > S_pDisplay_X->driver->hor_res - 1) {
        _pInDevData_X->point.x = S_pDisplay_X->driver->hor_res - 1;
    }
    if (_pInDevData_X->point.y < 0) {
        _pInDevData_X->point.y = 0;
    }
    if (_pInDevData_X->point.y > S_pDisplay_X->driver->ver_res - 1) {
        _pInDevData_X->point.y = S_pDisplay_X->driver->ver_res - 1;
    }
}

static void S_lv_android_keypad_driver_read_callback(lv_indev_drv_t *_pInDevDrv_X, lv_indev_data_t *_pInDevData_X) {
    //UNREFERENCED_PARAMETER(_pInDevDrv_X);

    _pInDevData_X->state = (lv_indev_state_t)(
            S_KeyboardPressed_B ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL);

    uint16_t KeyboardValue_U16 = S_KeyboardValue_U16;
//https://developer.android.com/games/agdk/game-activity/use-text-input
#if 0
    switch (KeyboardValue_U16) {
        case VK_UP:
            _pInDevData_X->key = LV_KEY_UP;
            break;
        case VK_DOWN:
            _pInDevData_X->key = LV_KEY_DOWN;
            break;
        case VK_LEFT:
            _pInDevData_X->key = LV_KEY_LEFT;
            break;
        case VK_RIGHT:
            _pInDevData_X->key = LV_KEY_RIGHT;
            break;
        case VK_ESCAPE:
            _pInDevData_X->key = LV_KEY_ESC;
            break;
        case VK_DELETE:
            _pInDevData_X->key = LV_KEY_DEL;
            break;
        case VK_BACK:
            _pInDevData_X->key = LV_KEY_BACKSPACE;
            break;
        case VK_RETURN:
            _pInDevData_X->key = LV_KEY_ENTER;
            break;
        case VK_NEXT:
            _pInDevData_X->key = LV_KEY_NEXT;
            break;
        case VK_PRIOR:
            _pInDevData_X->key = LV_KEY_PREV;
            break;
        case VK_HOME:
            _pInDevData_X->key = LV_KEY_HOME;
            break;
        case VK_END:
            _pInDevData_X->key = LV_KEY_END;
            break;
        default:
            if (KeyboardValue_U16 >= 'A' && KeyboardValue_U16 <= 'Z') {
                KeyboardValue_U16 += 0x20;
            }

            _pInDevData_X->key = (uint32_t) KeyboardValue_U16;

            break;
    }
#endif
}

static void S_lv_android_encoder_driver_read_callback(lv_indev_drv_t *_pInDevDrv_X, lv_indev_data_t *_pInDevData_X) {
    //UNREFERENCED_PARAMETER(_pInDevDrv_X);

    _pInDevData_X->state = (lv_indev_state_t)(S_MouseWheelPressed_B ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL);
    _pInDevData_X->enc_diff = S_MouseWheelValue_S16;
    S_MouseWheelValue_S16 = 0;
}

/*
  = true;

PAINTSTRUCT ps;
HDC hdc = BeginPaint(hWnd, &ps);

if (S_pDisplay_X)
{
SetStretchBltMode(hdc, HALFTONE
);

StretchBlt(
        hdc,
        ps
.rcPaint.left,
ps.rcPaint.top,
ps.rcPaint.right - ps.rcPaint.left,
ps.rcPaint.bottom - ps.rcPaint.top,
g_buffer_dc_handle,
0,
0,
MULDIV(
        ps
                .rcPaint.right - ps.rcPaint.left,
        USER_DEFAULT_SCREEN_DPI,
        ANDROID_DRV_MONITOR_ZOOM * g_dpi_value
),
MULDIV(
        ps
                .rcPaint.bottom - ps.rcPaint.top,
        USER_DEFAULT_SCREEN_DPI,
        ANDROID_DRV_MONITOR_ZOOM * g_dpi_value
),
SRCCOPY);
}

EndPaint(hWnd, &ps
);

S_DisplayRefreshing_B = false;

 */


#if 0

#include <bofstd/bofstd.h>
#include <bof2d/bof2d.h>
#include <lvgl/src/lvgl.h>


#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "stdio.h"


/**
    bugs fix:lxiaogao@163.com
**/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
//#include "lvgl/lvgl.h"

#define EVDEV_NAME "/dev/input/event3"

/**********************
 *  STATIC PROTOTYPES
 **********************/
int map(int x, int in_min, int in_max, int out_min, int out_max);

/**********************
 *  STATIC VARIABLES
 **********************/
int evdev_fd;
int evdev_root_x;
int evdev_root_y;
lv_indev_state_t evdev_button;

int evdev_key_val;

struct touch_mv
{
    int active_w;
    int active_h;
    int active_x;
    int active_y;
    bool is_active;
};
struct touch_mv t_mv
        {
                .active_w = 100,
                .active_h = 100,
                .active_x = 0,
                .active_y = 0,
                .is_active = false
        };

extern void set_surface_position(int x, int y);
/**
 * Initialize the evdev interface
 */
void evdev_init(void)
{
    /*
     To add the android.permission.READ_EXTERNAL_STORAGE permission to your app's manifest file, you will need to edit the AndroidManifest.xml file in your app's project.

First, make sure that the <uses-permission> element for the android.permission.READ_EXTERNAL_STORAGE permission is present in the <manifest> element of the AndroidManifest.xml file. If it is not present, you can add it as follows:

Copy code
<manifest ...>
  <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />
  ...
</manifest>
     */

    return ;

    int e;
    evdev_fd = open(EVDEV_NAME, O_RDWR | O_NOCTTY | O_NDELAY);
    e =errno;
    evdev_fd = open(EVDEV_NAME, O_RDONLY);
    e =errno;

    if (evdev_fd == -1)
    {
        perror("unable open evdev interface:");
        return;
    }

    fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK);

    evdev_root_x = 0;
    evdev_root_y = 0;
    evdev_key_val = 0;
    evdev_button = LV_INDEV_STATE_REL;
}
/**
 * reconfigure the device file for evdev
 * @param dev_name set the evdev device filename
 * @return true: the device file set complete
 *         false: the device file doesn't exist current system
 */
bool evdev_set_file(char *dev_name)
{
    if (evdev_fd != -1)
    {
        close(evdev_fd);
    }

    evdev_fd = open(dev_name, O_RDWR | O_NOCTTY | O_NDELAY);

    if (evdev_fd == -1)
    {
        perror("unable open evdev interface:");
        return false;
    }

    fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK);

    evdev_root_x = 0;
    evdev_root_y = 0;
    evdev_key_val = 0;
    evdev_button = LV_INDEV_STATE_REL;

    return true;
}

/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool evdev_read_imp(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    struct input_event in;

    while (read(evdev_fd, &in, sizeof(struct input_event)) > 0)
    {
        if (in.type == EV_REL)
        {
            if (in.code == REL_X)
#if EVDEV_SWAP_AXES
                evdev_root_y += in.value;
#else
                evdev_root_x += in.value;
#endif
            else if (in.code == REL_Y)
#if EVDEV_SWAP_AXES
                evdev_root_x += in.value;
#else
                evdev_root_y += in.value;
#endif
        }
        else if (in.type == EV_ABS)
        {
            if (in.code == ABS_X)
#if EVDEV_SWAP_AXES
                evdev_root_y = in.value;
#else
                evdev_root_x = in.value;
#endif
            else if (in.code == ABS_Y)
#if EVDEV_SWAP_AXES
                evdev_root_x = in.value;
#else
                evdev_root_y = in.value;
#endif
            else if (in.code == ABS_MT_POSITION_X)
#if EVDEV_SWAP_AXES
                evdev_root_y = in.value;
#else
                evdev_root_x = in.value;
#endif
            else if (in.code == ABS_MT_POSITION_Y)
#if EVDEV_SWAP_AXES
                evdev_root_x = in.value;
#else
                evdev_root_y = in.value;
#endif
            else if (in.code == ABS_MT_TRACKING_ID)
            {
                if (in.value == -1)
                    evdev_button = LV_INDEV_STATE_REL;
                else if (in.value == 0)
                    evdev_button = LV_INDEV_STATE_PR;
            }
        }
        else if (in.type == EV_KEY)
        {
            if (in.code == BTN_MOUSE || in.code == BTN_TOUCH)
            {
                if (in.value == 0)
                {
                    evdev_button = LV_INDEV_STATE_REL;
                    t_mv.is_active = false;
                }
                else if (in.value == 1)
                {
                    evdev_button = LV_INDEV_STATE_PR;
                    if ((evdev_root_x > t_mv.active_x) && (evdev_root_x < t_mv.active_x + t_mv.active_w) && (evdev_root_y > t_mv.active_y) && (evdev_root_y < t_mv.active_y + t_mv.active_h))
                    {
                        t_mv.is_active = true;
                    }
                }
            }
            else if (drv->type == LV_INDEV_TYPE_KEYPAD)
            {
                data->state = (in.value) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
                switch (in.code)
                {
                    case KEY_BACKSPACE:
                        data->key = LV_KEY_BACKSPACE;
                        break;
                    case KEY_ENTER:
                        data->key = LV_KEY_ENTER;
                        break;
                    case KEY_UP:
                        data->key = LV_KEY_UP;
                        break;
                    case KEY_LEFT:
                        data->key = LV_KEY_PREV;
                        break;
                    case KEY_RIGHT:
                        data->key = LV_KEY_NEXT;
                        break;
                    case KEY_DOWN:
                        data->key = LV_KEY_DOWN;
                        break;
                    default:
                        data->key = 0;
                        break;
                }
                evdev_key_val = data->key;
                evdev_button = data->state;
                return false;
            }
        }
    }

    if (drv->type == LV_INDEV_TYPE_KEYPAD)
    {
        /* No data retrieved */
        data->key = evdev_key_val;
        data->state = evdev_button;
        return false;
    }
    if (drv->type != LV_INDEV_TYPE_POINTER)
        return false;
    /*Store the collected data*/

    if (t_mv.is_active == true)
    {
//BHA        set_surface_position(evdev_root_x, evdev_root_y);
        t_mv.active_x = evdev_root_x;
        t_mv.active_y = evdev_root_y;
        return false;
    }

#if EVDEV_CALIBRATE
    data->point.x = map(evdev_root_x, EVDEV_HOR_MIN, EVDEV_HOR_MAX, 0, drv->disp->driver->hor_res);
    data->point.y = map(evdev_root_y, EVDEV_VER_MIN, EVDEV_VER_MAX, 0, drv->disp->driver->ver_res);
#else
    data->point.x = evdev_root_x - t_mv.active_x;
    data->point.y = evdev_root_y - t_mv.active_y;
#endif

    data->state = evdev_button;

    if (data->point.x < 0)
        data->point.x = 0;
    if (data->point.y < 0)
        data->point.y = 0;
    if (data->point.x >= drv->disp->driver->hor_res)
        data->point.x = drv->disp->driver->hor_res - 1;
    if (data->point.y >= drv->disp->driver->ver_res)
        data->point.y = drv->disp->driver->ver_res - 1;

    return false;
}

void evdev_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    evdev_read_imp(drv, data);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
int map(int x, int in_min, int in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void surface_init(void) {
}

void wait_surface_flush(struct _lv_disp_drv_t * disp_drv) {
    //lv_disp_draw_buf_t * draw_buf = lv_disp_get_draw_buf(disp_refr);
    aout << " flushing " << disp_drv->draw_buf->flushing << std::endl;
    disp_drv->draw_buf->flushing = 0;
}

void surface_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p) {
    aout << "drv " << drv << ": area " << area->x1 << "," << area->y1 << " - " << area->x2 << "," << area->y2  << " color " << color_p->full << std::endl;
}
/*
void evdev_init(void) {
}
void evdev_read(lv_indev_drv_t * drv, lv_indev_data_t * data) {
}
 */
lv_obj_t * led1 ;

#define DISP_BUF_SIZE (128 * 1024)


static void btn_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_user_data(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        const char * state = lv_obj_get_state(obj) & LV_STATE_CHECKED ? "Checked" : "Unchecked";
        LV_LOG_USER("Clicked %s",state);
        lv_obj_set_size(obj,300,200);
        lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_set_pos(obj,100,100);
        if(lv_obj_get_state(obj) & LV_STATE_CHECKED)
            lv_led_on(led1);
        else
            lv_led_off(led1);
    }
}


void demo_run(void)
{

    static lv_style_t style_main;
    lv_style_init(&style_main);
    // lv_style_set_bg_color(&style_main, lv_palette_lighten(LV_PALETTE_GREY, 2));
    lv_style_set_bg_opa(&style_main,LV_OPA_TRANSP); // set bg alpha to hide bg color
    lv_obj_add_style(lv_scr_act(),&style_main, 0);


    lv_obj_t * label;

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
    lv_obj_add_flag(btn1, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(btn1, btn_event_handler, LV_EVENT_ALL, btn1);
    lv_obj_align(btn1, LV_ALIGN_TOP_MID, 0, 0);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    static lv_style_t style_btn;
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_palette_lighten(LV_PALETTE_LIGHT_GREEN, 2));
    lv_style_set_border_color(&style_btn, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_text_color(&style_btn, lv_palette_main(LV_PALETTE_RED));



    lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
    lv_obj_align_to(btn2, btn1,LV_ALIGN_TOP_LEFT, 200, 100);
    label = lv_label_create(btn2);
    lv_label_set_text(label, "Button2");
    lv_obj_center(label);
    lv_obj_add_style(btn2, &style_btn, 0);

    /*Create a LED and switch it OFF*/
    led1  = lv_led_create(lv_scr_act());
    lv_obj_align(led1, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_led_off(led1);

}
/*A small buffer for LittlevGL to draw the screen's content*/
static lv_color_t buf[DISP_BUF_SIZE];
static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static     lv_indev_drv_t indev_drv_1;
static     lv_indev_drv_t indev_drv_2;
//https://docs.lvgl.io/latest/en/html/porting/indev.html
//bool evdev_read_imp(lv_indev_drv_t *drv, lv_indev_data_t *data)
void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data)
{
    data->point.x = 100;
    data->point.y = 200;
    data->state = LV_INDEV_STATE_PR;    // or LV_INDEV_STATE_REL;
    //return false; /*No buffering now so no more data read*/
}
void keyboard_read(lv_indev_drv_t * drv, lv_indev_data_t*data)
{
    data->key = 'A';    //last_key();            /*Get the last pressed or released key*/

    if(0 /*key_pressed()*/) data->state = LV_INDEV_STATE_PR;
    else data->state = LV_INDEV_STATE_REL;

 //   return false; /*No buffering now so no more data read*/
}
int init_lvgl_under_android(void)
{
    /*LittlevGL init*/
    lv_init();
    /*Linux frame buffer device init*/
    surface_init();

    /*Initialize a descriptor for the buffer*/
    lv_disp_draw_buf_init(&disp_buf, buf, nullptr, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = surface_flush;
    disp_drv.wait_cb   = wait_surface_flush;
    disp_drv.hor_res    = 800;
    disp_drv.ver_res    = 800;
    lv_disp_drv_register(&disp_drv);

    evdev_init();
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;
    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb = my_input_read;    //evdev_read;
    lv_indev_drv_register(&indev_drv_1);

    lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
    indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_2.read_cb = keyboard_read;    //evdev_read;
    lv_indev_drv_register(&indev_drv_2);
    /*Create a Demo*/
#if 0   //BHA 1
    lv_demo_music();
#else
    demo_run();
#endif
    /*Handle LitlevGL tasks (tickless mode)*/
#if 0 //done later
    while(1) {
        lv_task_handler();
        usleep(5*1000);
        //lv_tick_inc(5);
    }
#endif
    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, nullptr);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, nullptr);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}


#endif