#ifndef _hx3605_H_
#define _hx3605_H_
#include <stdint.h>
#include <stdbool.h>

// #define HX3605_IC		HW_I2C0_1

#define HX3605_ADDR_7BIT                    0x44
#define HX3605_WRITE_ADDR(ADDR)     	    ((ADDR) << 1)
#define HX3605_READ_ADDR_UPDATE(ADDR)       ((ADDR) << 1 | 1)
#define HX3605_READ_ADDR(ADDR)      	    ((ADDR) << 1 | 1) << 8 | ((ADDR) << 1)

//******************** alg_swi *********************//
#define HRS_ALG_LIB
#define SPO2_ALG_LIB
//#define HRV_ALG_LIB
//#define CHECK_TOUCH_LIB
//#define CHECK_LIVING_LIB
#define BP_CUSTDOWN_ALG_LIB

//**************** read_data_mode ******************//
#if !HR_INT_MORE_SELECT
#define TIMMER_MODE           //timmer read fifo
#else
#define INT_MODE		      //fifo_all_most_full  3605中断模式不可用
#endif

//#define TYHX_DEMO   // fiso add for drive macro to customer
//****************** gsen_cgf *********************//
//#define GSENSER_DATA
//#define GSEN_40MS_TIMMER
#define NEW_GSEN_SCHME

//****************** other_cgf ********************//
#define DEMO_NEW
//#define HRS_BLE_APP
//#define MALLOC_MEMORY
//#define SPO2_DATA_CALI

//***************** vecter_swi ********************//
//#define SPO2_VECTOR
//#define HR_VECTOR
//#define HRV_TESTVEC

//**************** lab_test_mode ******************//
//#define LAB_TEST
//#define LAB_TEST_AGC

//****************** print_swi ********************//
// #define HRS_DEBUG
// #define AGC_DEBUG

//**************************************************//
#ifdef AGC_DEBUG
//#define  AGC_LOG(...)     SEGGER_RTT_printf(0,__VA_ARGS__)
#define  AGC_LOG(...)     printf(__VA_ARGS__)
#else
#define	 AGC_LOG(...)
#endif

#ifdef HRS_DEBUG
//#define  DEBUG_PRINTF(...)     SEGGER_RTT_printf(0,__VA_ARGS__)
#define  DEBUG_PRINTF(...)     printf(__VA_ARGS__)
#else
#define	 DEBUG_PRINTF(...)
#endif


///* portable 8-bit unsigned integer */
//typedef unsigned char           uint8_t;
///* portable 8-bit signed integer */
//typedef signed char             int8_t;
///* portable 16-bit unsigned integer */
//typedef unsigned short int      uint16_t;
///* portable 16-bit signed integer */
//typedef signed short int        int16_t;
///* portable 32-bit unsigned integer */
//typedef unsigned int            uint32_t;
///* portable 32-bit signed integer */
//typedef signed int              int32_t;

#ifndef bool
#define bool unsigned char
#endif

#ifndef  true
#define  true  1
#endif
#ifndef  false
#define  false  0
#endif

#if defined(DEMO_COMMON)
  #define HRS4100_IIC_CLK  30
  #define HRS4100_IIC_SDA  0
  #define LIS3DH_IIC_CLK   18
  #define LIS3DH_IIC_SDA   16
  #define EXT_INT_PIN      1
  #define GREEN_LED_SLE    1
  #define RED_LED_SLE      4
  #define IR_LED_SLE       2
  #define RED_AGC_OFFSET   51
  #define IR_AGC_OFFSET    51
  #define GREEN_AGC_OFFSET 8
  #define BIG_SCREEN

#elif defined(DEMO_NEW)
  #define HRS4100_IIC_CLK  0
  #define HRS4100_IIC_SDA  30
  #define LIS3DH_IIC_CLK   18
  #define LIS3DH_IIC_SDA   16
  #define EXT_INT_PIN      1
  #define GREEN_LED_SLE    1
  #define RED_LED_SLE      2
  #define IR_LED_SLE       4
  #define RED_AGC_OFFSET   50
  #define IR_AGC_OFFSET    50
  #define GREEN_AGC_OFFSET 4
	#define HR_GREEN_AGC_OFFSET 4
  #define BIG_SCREEN

#elif defined(DEMO_BAND)
  #define HRS4100_IIC_CLK  4
  #define HRS4100_IIC_SDA  3
  #define LIS3DH_IIC_CLK   14
  #define LIS3DH_IIC_SDA   12
  #define EXT_INT_PIN      7
  #define GREEN_LED_SLE    0
  #define RED_LED_SLE      2
  #define IR_LED_SLE       1
  #define RED_AGC_OFFSET   64
  #define IR_AGC_OFFSET    64
  #define GREEN_AGC_OFFSET 8
	#define HR_GREEN_AGC_OFFSET 8

#elif defined(DEMO_GT01)
  #define HRS4100_IIC_CLK  5
  #define HRS4100_IIC_SDA  6
  #define LIS3DH_IIC_CLK   14
  #define LIS3DH_IIC_SDA   12
  #define EXT_INT_PIN      7
  #define GREEN_LED_SLE    1
  #define RED_LED_SLE      4
  #define IR_LED_SLE       2
  #define RED_AGC_OFFSET   32
  #define IR_AGC_OFFSET    32
  #define GREEN_AGC_OFFSET 8
  #define BIG_SCREEN

#elif defined(EVB)
  #define HRS4100_IIC_CLK  9
  #define HRS4100_IIC_SDA  10
  #define LIS3DH_IIC_CLK   13
  #define LIS3DH_IIC_SDA   14
  #define EXT_INT_PIN      11
  #define GREEN_LED_SLE    1
  #define RED_LED_SLE      2
  #define IR_LED_SLE       4
  #define RED_AGC_OFFSET   64
  #define IR_AGC_OFFSET    64
  #define GREEN_AGC_OFFSET 8
	#define HR_GREEN_AGC_OFFSET 10

#else
  #define HRS4100_IIC_CLK  9
  #define HRS4100_IIC_SDA  10
  #define LIS3DH_IIC_CLK   13
  #define LIS3DH_IIC_SDA   14
  #define EXT_INT_PIN      11
  #define GREEN_LED_SLE    1
  #define RED_LED_SLE      4
  #define IR_LED_SLE       2
  #define RED_AGC_OFFSET   64
  #define IR_AGC_OFFSET    64
  #define GREEN_AGC_OFFSET 8

#endif

typedef enum {
	DEFAULT_MOTION = 0,
	STATIC_MOTION = 1,
	WALKING_MOTION = 2,
	RUNING_MOTION = 3,
	CYCLING_MOTION = 4,
	SWINMING_MOTION = 5,
	TENNIS_MOTION = 6,
	HRM_MONITOR_MODE = 7
} MOIION_STATUS;

typedef enum {
	PPG_INIT,
	PPG_OFF,
	PPG_LED_OFF,
	CAL_INIT,
	CAL_OFF,
	RECAL_INIT
} hx3605_mode_t;

typedef enum {
	HX3605_NORMAL_MODE,
	HX3605_RIDE_MODE,
	HX3605_WALK_MODE,
	HX3605_HIGH_SPORTS_MODE
} hx3605_sports_mode_t;

typedef enum {
	SENSOR_OK,
	SENSOR_OP_FAILED,
} SENSOR_ERROR_T;

typedef enum {
	DATA_TYPE_PPG,
	DATA_TYPE_GSEN,
} SENSOR_TYPE_T;

typedef enum {
	HRS_MODE,
	SPO2_MODE,
	WEAR_MODE,
	HRV_MODE,
	LIVING_MODE,
	LAB_TEST_MODE,
	FT_LEAK_LIGHT_MODE,
	FT_GRAY_CARD_MODE,
	FT_INT_TEST_MODE
} WORK_MODE_T;

typedef struct {
	SENSOR_TYPE_T type;
	uint8_t count;
	int32_t red_data[64];
	int32_t green_data[64];
	int32_t ir_data[64];
	int32_t s_buf[64];
	uint8_t agc_green;
	uint8_t agc_red;
	uint8_t agc_ir;
}spo2_sensor_data_t;

typedef struct {
	SENSOR_TYPE_T type;
	uint8_t count;
	int32_t ppg_data[64];
	int32_t ir_data[64];
	int32_t s_buf[64];
	uint32_t als;
	uint32_t agc_green;
}hrs_sensor_data_t;

typedef struct {
	SENSOR_TYPE_T type;
	uint8_t count;
	int32_t ppg_data[64];
	int32_t ir_data[64];
	int32_t s_buf[64];
	uint32_t als;
	uint32_t agc_green;
}hrv_sensor_data_t;

typedef enum {
	MSG_HRS_NO_WEAR,
	MSG_HRS_WEAR,
	MSG_HRS_INIT
} hx3605_hrs_wear_msg_code_t;

typedef enum {
	MSG_LIVING_NO_WEAR,
	MSG_LIVING_WEAR
} hx3605_living_wear_msg_code_t;

typedef enum {
	MSG_HRV_NO_WEAR,
	MSG_HRV_WEAR
} hx3605_hrv_wear_msg_code_t;

typedef enum {
	MSG_HRS_ALG_NOT_OPEN,
	MSG_HRS_ALG_OPEN,
	MSG_HRS_ALG_OPEN_DEEP,
	MSG_HRS_READY,
	MSG_HRS_ALG_TIMEOUT,
	MSG_HRS_SETTLE
} hx3605_hrs_alg_msg_code_t;

typedef enum {
	MSG_HRV_ALG_NOT_OPEN,
	MSG_HRV_ALG_OPEN,
	MSG_HRV_ALG_OPEN_DEEP,
	MSG_HRV_READY,
	MSG_HRV_ALG_TIMEOUT,
	MSG_HRV_SETTLE
} hx3605_hrv_alg_msg_code_t;

typedef struct {
	hx3605_hrs_alg_msg_code_t  hrs_alg_status;
	hx3605_hrs_wear_msg_code_t  hrs_wear_status;
	uint32_t           data_cnt;
	uint8_t            hr_result;
	uint16_t           cal_result;
	uint8_t            hr_result_qual;
	bool               object_flg;
} hx3605_results_t;

typedef struct {
	hx3605_hrv_alg_msg_code_t  hrv_alg_status;
	hx3605_hrv_wear_msg_code_t  hrv_wear_status;
	uint32_t           data_cnt;
    uint32_t            hrv_peak; // ericy add20170111
	uint8_t            hrv_result;
	uint8_t            hrv_result_qual;
	bool               object_flg;
} hx3605_hrv_results_t;

typedef struct {
	hx3605_living_wear_msg_code_t  wear_status;
	uint32_t           data_cnt;
	uint8_t            signal_quality;
	uint8_t            motion_status;
} hx3605_living_results_t;

typedef enum {
	MSG_SPO2_NO_WEAR,
	MSG_SPO2_WEAR
} hx3605_spo2_wear_msg_code_t;

typedef enum {
	MSG_SPO2_ALG_NOT_OPEN,
	MSG_SPO2_ALG_OPEN,
	MSG_SPO2_ALG_OPEN_DEEP,
	MSG_SPO2_READY,
	MSG_SPO2_ALG_TIMEOUT,
	MSG_SPO2_SETTLE
} hx3605_spo2_alg_msg_code_t;

typedef struct {
  hx3605_spo2_alg_msg_code_t  spo2_alg_status;
  hx3605_spo2_wear_msg_code_t  spo2_wear_status;
  uint32_t                 data_cnt;
  uint8_t                 spo2_result;
  uint8_t                  hr_result;
  uint8_t                  block_cnt;
	uint8_t                  ir_quality;
	uint16_t                 reliable_score_result;
} hx3605_spo2_results_t;

typedef struct {
  uint8_t                 red_offset_idac;
  uint8_t                 ir_offset_idac;
  uint8_t                 green_offset_idac;
  uint16_t                spo2_start_cnt;
} hx3605_spo2_agcpara_t;

typedef struct {
	uint8_t flag;
	bool work;
	uint8_t int_cnt;
	uint8_t cur255_cnt;
	uint8_t led_idac;  // 6,7,8...
	uint16_t LED;     // phasex led driver config
	uint8_t LEDDAC;  // phasex led offset idac cfg
	uint8_t AMBDAC;  // phasex offset idac cfg
	uint8_t RF;      // phasex tia feed back resister cfg
	uint32_t led_step;
	uint8_t state;
	uint8_t led_max_cur;
} HRS_CAL_SET_T;

typedef struct {
	uint8_t flag;
	bool work;
	uint8_t int_cnt;
	uint8_t cur255_cnt;
	uint8_t led_idac;  // 6,7,8...
	uint16_t LED;     // phasex led driver config
	uint8_t LEDDAC;  // phasex led offset idac cfg
	uint8_t AMBDAC;  // phasex offset idac cfg
	uint8_t RF;      // phasex tia feed back resister cfg
	uint32_t led_step;
	uint8_t state;
	uint8_t led_max_cur;
} HRV_CAL_SET_T;

typedef struct {
	uint8_t flag;
	bool work;
	uint8_t int_cnt;
	uint8_t cur255_cnt;
	uint8_t green_idac;
	uint8_t red_idac;
	uint8_t ir_idac;
	uint16_t G_LED;
	uint16_t R_LED;
	uint16_t IR_LED;
	uint8_t G_LEDDAC;
	uint8_t R_LEDDAC;
	uint8_t IR_LEDDAC;
	uint8_t AMBDAC;
	uint8_t G_RF;
	uint8_t R_RF;
	uint8_t IR_RF;
	uint32_t G_led_step;
	uint32_t R_led_step;
	uint32_t IR_led_step;
	uint8_t state;
	uint8_t green_max_cur;
	uint8_t red_max_cur;
	uint8_t ir_max_cur;
} SPO2_CAL_SET_T;

typedef struct {
	uint32_t data_cnt;
	uint8_t  motion_status;
	uint32_t gsen_pow;
	int8_t   up_clip;
	int8_t   dn_clip;
	uint8_t  runrun_cnt;
} HRS_ALG_INFO_T;

#ifdef BP_CUSTDOWN_ALG_LIB
typedef enum {
	MSG_BP_ALG_NOT_OPEN = 0x01,
	MSG_BP_NO_TOUCH = 0x02,
	MSG_BP_PPG_LEN_TOO_SHORT = 0x03,
	MSG_BP_READY = 0x04,
	MSG_BP_ALG_TIMEOUT = 0x05,
	MSG_BP_SETTLE = 0x06
} hx3605_bp_msg_code_t;

typedef struct {
	uint8_t age;
	uint8_t height;
	uint8_t weight;
	uint8_t gender;
	uint8_t ref_sbp;
	uint8_t ref_dbp;
}bp_exinf_t;

typedef struct {
	hx3605_bp_msg_code_t bp_alg_status;
	uint8_t            sbp;
	uint8_t            dbp;
	uint32_t           data_cnt;
	uint8_t            hr_result; //20170614 ericy
	bool               object_flg;
} hx3605_bp_results_t;
#endif

#ifdef HRS_BLE_APP
typedef enum {
  START_VECTOR_FLAG = 0,
	HRS_VECTOR_FLAG = 0x55,
	SPO2_VECTOR_FLAG = 0x66
} vector_flag_t;

typedef struct {
	vector_flag_t vector_flag;
  uint32_t data_cnt;
	uint8_t  hr_result;
	int32_t red_raw_data; //hrs:green,spo2:red
	int32_t ir_raw_data;  //hrs:no use,spo2:ir
	int16_t gsensor_x;
	int16_t gsensor_y;
	int16_t gsensor_z;
	uint8_t red_cur;       //hrs:green cur drv,spo2:red cur drv
	uint8_t ir_cur;        //hrs:no use,spo2:ir cur drv
}rawdata_vector_t ;
#endif

#if defined(MALLOC_MEMORY)
extern uint8_t alg_ram[2*1024];
#else
extern uint8_t alg_ram[11*1024];
#endif

uint16_t hx3605_read_fifo_data(int32_t *buf, uint8_t phase_num, uint8_t sig);
void read_data_packet(int32_t *ps_data);
extern WORK_MODE_T work_mode_flag;
void hx3605_delay_us(uint32_t us);
void hx3605_delay(uint32_t ms);
bool hx3605_write_reg(uint8_t addr, uint8_t data);
bool hx3605_write_reg(uint8_t addr, uint8_t data);
uint8_t hx3605_read_reg(uint8_t addr);
bool hx3605_brust_read_reg(uint8_t addr , uint8_t *buf, uint8_t length);
bool hx3605_chip_check(void);
uint8_t hx3605_read_fifo_size(void);
void hx3605_ppg_off(void);
void hx3605_ppg_on(void);
void hx3605_320ms_timer_cfg(bool en);
void hx3605_40ms_timer_cfg(bool en);
void hx3605_gpioint_cfg(bool en);
bool hx3605_init(WORK_MODE_T mode);
void hx3605_agc_Int_handle(void);
void hx3605_gesensor_Int_handle(void);
void hx3605_spo2_ppg_init(void);
void hx3605_spo2_ppg_Int_handle(void);
void hx3605_wear_ppg_Int_handle(void);
void hx3605_ft_hrs_Int_handle(void);
void hx3605_ft_spo2_Int_handle(void);
void hx3605_hrs_ppg_init(void);
void hx3605_hrs_ppg_Int_handle(void);
void hx3605_hrv_ppg_Int_handle(void);
void hx3605_living_Int_handle(void);
void display_refresh(void);
void ble_rawdata_clear(void);

#ifdef HRS_BLE_APP
void ble_rawdata_vector_push(rawdata_vector_t rawdata);
uint32_t ble_hrs_heart_rate_send_ext(uint8_t flag,uint8_t data_cnt,uint8_t result,\
                              int32_t red_rawdata,int32_t ir_rawdata,\
                              int16_t gsensor_x, int16_t gsensor_y, int16_t gsensor_z,\
                              uint8_t red_cur, uint8_t ir_cur);

#endif

uint32_t ble_rawdata_send_handler(void);
extern void hx3605_ppg_Int_handle(void);
extern uint32_t hx3605_timers_start(void);
extern uint32_t hx3605_timers_stop(void);
extern uint32_t hx3605_gpioint_init(void);

//extern uint32_t hx3605_gpioint_enable(void);
//extern uint32_t hx3605_gpioint_disable(void);
extern uint32_t gsen_read_timers_start(void);
extern uint32_t gsen_read_timers_stop(void);

extern hx3605_results_t hx3605_alg_get_results(void);
extern hx3605_spo2_results_t hx3605_spo2_alg_get_results(void);

extern HRS_CAL_SET_T PPG_hrs_agc(void);
extern HRV_CAL_SET_T PPG_hrv_agc(void);
extern SPO2_CAL_SET_T PPG_spo2_agc(void);
void hx3605_vin_check(uint16_t led_vin);
void hx3605_demo_power_swi(void);

#ifdef HRS_ALG_LIB
void hx3605_hrs_cal_init(void);
void hx3605_hrs_cal_off(uint8_t enable_50_hz);
uint8_t hx3605_read_fifo_size(void);
void read_hrs_data_packet(int32_t *buf);
void read_hrs_ir_packet(int32_t *buf);
void hx3605_hrs_low_power(void);
void hx3605_hrs_normal_power(void);
void hx3605_hrs_updata_reg(void);
void hx3605_hrs_set_mode(uint8_t mode_cmd);
SENSOR_ERROR_T hx3605_hrs_enable(void);
void hx3605_hrs_disable(void);
hx3605_hrs_wear_msg_code_t hx3605_hrs_get_wear_status(void);
uint8_t hx3605_hrs_read(hrs_sensor_data_t * s_dat);
bool hx3605_hrs_init(void);
hx3605_hrs_wear_msg_code_t hx3605_hrs_wear_mode_check(WORK_MODE_T mode,int32_t infrared_data);
HRS_ALG_INFO_T hx3605_get_hrs_alg_info(void);

void hx3605_hrv_alg_close(void);
bool hx3605_alg_open(void);
bool hx3605_alg_open_deep(void);
void hx3605_alg_close(void);
bool hx3605_alg_send_data(int32_t *new_raw_data, uint8_t dat_len, uint32_t green_data_als, int16_t *gsen_data_x, int16_t *gsen_data_y, int16_t *gsen_data_z);
HRS_CAL_SET_T get_hrs_agc_status(void);
void hx3605_set_sports_mode(hx3605_sports_mode_t sports_mode);
void hx3605_hrs_data_reset(void);
void heart_rate_meas_timeout_handler(void * p_context);
#endif

#ifdef SPO2_ALG_LIB
void hx3605_spo2_cal_init(void);
void hx3605_spo2_cal_off(void);
void read_spo2_data_packet(int32_t *buf);
void read_spo2_ir_packet(int32_t *buf);
void hx3605_spo2_low_power(void);
void hx3605_spo2_normal_power(void);
void hx3605_spo2_updata_reg(void);
void hx3605_spo2_set_mode(uint8_t mode_cmd);
SENSOR_ERROR_T hx3605_spo2_enable(void);
void hx3605_spo2_disable(void);
hx3605_spo2_wear_msg_code_t hx3605_spo2_get_wear_status(void);
uint8_t hx3605_spo2_read(spo2_sensor_data_t * s_dat);
bool hx3605_spo2_check_unwear(WORK_MODE_T mode,int32_t infrared_data);
bool hx3605_spo2_check_wear(WORK_MODE_T mode,int32_t infrared_data);

bool hx3605_spo2_alg_send_data(int32_t *red_new_raw_data,int32_t *ir_new_raw_data, uint8_t dat_len,volatile int16_t *gsen_data_x,volatile int16_t *gsen_data_y,volatile int16_t *gsen_data_z);
void hx3605_spo2_alg_close(void);
bool hx3605_spo2_alg_open(void);
bool hx3605_spo2_alg_open_deep(void);
SPO2_CAL_SET_T get_spo2_agc_status(void);
SPO2_CAL_SET_T PPG_spo2_agc(void);
bool hx3605_spo2_alg_send_data(int32_t *red_new_raw_data,int32_t *ir_new_raw_data, uint8_t dat_len,volatile int16_t *gsen_data_x,volatile int16_t *gsen_data_y,volatile int16_t *gsen_data_z);
void heart_rate_meas_timeout_handler(void * p_context);
void hx3605_spo2_data_reset(void);
#endif

#ifdef BP_CUSTDOWN_ALG_LIB
void hx3605_bp_age_fn(uint8_t hr_result , bp_exinf_t body_s);
hx3605_bp_results_t hx3605_alg_get_bp_results(void);
#endif

#ifdef HRV_ALG_LIB
//alg_results = hx3605_alg_get_results();
HRV_CAL_SET_T get_hrv_agc_status(void);
void hx3605_hrv_cal_init(void);
void hx3605_hrv_cal_off(void);
uint8_t hx3605_read_fifo_size(void);
void read_hrv_data_packet(int32_t *buf);
void read_hrv_ir_packet(int32_t *buf);
void hx3605_hrv_low_power(void);
void hx3605_hrv_normal_power(void);
void hx3605_hrv_updata_reg(void);
void hx3605_hrv_set_mode(uint8_t mode_cmd);
SENSOR_ERROR_T hx3605_hrv_enable(void);
void hx3605_hrv_disable(void);
hx3605_hrv_wear_msg_code_t hx3605_hrv_get_wear_status(void);
hx3605_hrv_wear_msg_code_t hx3605_hrv_get_wear_status_pre(void);
uint8_t hx3605_hrv_read(hrv_sensor_data_t * s_dat);
bool hx3605_hrv_init(void);
hx3605_hrv_wear_msg_code_t hx3605_hrv_wear_mode_check(WORK_MODE_T mode,int32_t infrared_data);

void hx3605_hrv_ppg_init(void);
uint16_t hx3605_hrv_alg(int32_t new_raw_data);
bool  hx3605_hrv_alg_open(void);
bool hx3605_hrv_alg_open_deep(void);
void hx3605_hrv_set_mode(uint8_t mode_cmd);
void kfft(double *pr,double *pi,int n,int k,double *fr,double *fi);
//hx3605_msg_code_t hx3605_get_touch_status(void);
hx3605_hrv_results_t hx3605_hrv_alg_send_data(int32_t new_raw_data, int32_t green_data_als, int32_t infrared_data);
hx3605_hrv_results_t hx3605_hrv_alg_send_bufdata(int32_t *new_raw_data, int32_t green_data_als, int32_t infrared_data);
void hx3605_hrv_data_reset(void);
#endif

#ifdef CHECK_TOUCH_LIB
SENSOR_ERROR_T hx3605_check_touch_enable(void);
hx3605_hrs_wear_msg_code_t hx3605_check_touch_send_data(int32_t *ir_data, uint8_t count);
uint8_t hx3605_check_touch_read_fifo(int32_t *ir_buf);
void hx3605_check_touch_init(void);
void hx3605_lab_test_Int_handle(void);
#endif

#ifdef LAB_TEST
void hx3605_lab_test_init(void);
void hx3605_test_alg_cfg(void);
SENSOR_ERROR_T hx3605_lab_test_enable(void);
void hx3605_lab_test_Int_handle(void);
void hx3605_lab_test_read_packet(uint32_t *data_buf);
#endif

bool hx3605_living_send_data(int32_t *new_raw_data, uint8_t dat_len, uint32_t green_data_als, int16_t *gsen_data_x, int16_t *gsen_data_y, int16_t *gsen_data_z);
hx3605_living_results_t hx3605_living_get_results(void);
#endif
