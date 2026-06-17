#ifndef _BSP_H
#define _BSP_H

#include "bsp_halt.h"
#include "bsp_sys.h"
#include "bsp_backtrace.h"
#include "bsp_hw_timer.h"
#include "bsp_pwm.h"
#include "bsp_key.h"
#include "bsp_dac.h"
#include "bsp_param.h"
#include "bsp_audio.h"
#include "bsp_music.h"
#include "bsp_bt.h"
#include "bsp_eq.h"
#include "bsp_piano.h"
#include "bsp_i2c.h"
#include "bsp_ble.h"
#include "bsp_app.h"
#include "bsp_charge.h"
#include "bsp_saradc.h"
#include "bsp_vbat.h"
#include "bsp_rtc.h"
#include "bsp_sensor.h"
#include "bsp_port_int.h"
#include "bsp_sleep.h"
#include "bsp_vusb.h"
#include "bsp_uitool_phrase.h"
#if MODEM_CAT1_EN
#include "bsp_modem.h"
#endif
#include "bsp_opus.h"
#if CALL_MGR_EN
#include "bsp_call_mgr.h"
#endif
#include "bsp_spi.h"
#include "bsp_spi1flash.h"
#include "bsp_uart.h"
#include "bsp_huart.h"
#include "bsp_asr.h"
#include "bsp_auphy.h"
#include "bsp_sensor_hub.h"
#if VIDEO_RECODE_TAKE_PHOTO_EN
#include "bsp_image_sensor.h"
#endif
#if VIDEO_PLAY_EN
#include "bsp_video_play.h"
#endif
#if VIDEO_RECODE_TAKE_PHOTO_EN
#include "bsp_video_recode.h"
#include "bsp_take_photo.h"
#include "bsp_photo_view.h"
#include "bsp_video_watermark.h"
#endif
#include "bsp_gpio.h"
#include "mic_effect.h"
#include "bsp_fmrx.h"
#include "bsp_disk.h"
#include "bsp_mic_record.h"
#include "bsp_record_fs.h"
#include "bsp_gif.h"

#endif
