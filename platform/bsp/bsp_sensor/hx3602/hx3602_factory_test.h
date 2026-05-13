#ifndef _hx3602_FACTORY_TEST_H_
#define _hx3602_FACTORY_TEST_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    LEAK_LIGHT_TEST = 1,
	GRAY_CARD_TEST = 2,
    
} TEST_MODE_t;


typedef struct{
	int32_t gr_data_final;
	int32_t ir_data_final;
    
} FT_RESULTS_t;

FT_RESULTS_t hx3602_factroy_test(TEST_MODE_t  test_mode);

#endif // _hx3600_FACTORY_TEST_H_



