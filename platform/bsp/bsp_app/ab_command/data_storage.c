#include "include.h"
#include "ab_common.h"

#if (USE_APP_TYPE == APP_BLUE_FIT)

static bool sector_is_null(u8 index)
{
    u8 *ptr = NULL;
    u16 size = 0;

//    printf("%s:index[%d]\n", __func__, index);
    switch (index) {
    case PM_SYNC_SECTOR0:
        ptr = (u8 *)&app_data.sector0;
        size = sizeof(flash_sector0_t);
        break;
    case PM_SYNC_SECTOR1:
        ptr = (u8 *)&app_data.sector1;
        size = sizeof(flash_sector1_t);
        break;
    }

    for (u16 i = 0; i < size; i++) {
        if (*ptr != 0xff) {
//            printf("false ptr[%d] = %x\n", i, *ptr);
            return false;
        }
        ptr++;
    }

    return true;
}

//------------------------------------------------------------------------
//Platform Manager
static void param_pm_clr(u8 index)
{
    switch (index) {
    case 0:
        memset(&app_data.sector0, 0, sizeof(flash_sector0_t));
        break;
    case 1:
        memset(&app_data.sector1, 0, sizeof(flash_sector1_t));
        break;
    default:
        break;
    }
}

static void pm_sync_do(void)
{
    printf("[%s] sector[0:%d][1:%d]\r\n", __FUNCTION__, app_data.pm_swi.sync.sector0, app_data.pm_swi.sync.sector1);

    if (app_data.pm_swi.sync.sector0) {
        os_spiflash_erase(FLASH_SECTOR0_ADDR);
        os_spiflash_program(&app_data.sector0, FLASH_SECTOR0_ADDR, sizeof(flash_sector0_t));
        app_data.pm_swi.sync.sector0 = 0;
    }

    if (app_data.pm_swi.sync.sector1) {
        os_spiflash_erase(FLASH_SECTOR1_ADDR);
        os_spiflash_program(&app_data.sector1, FLASH_SECTOR1_ADDR, sizeof(flash_sector1_t));
        app_data.pm_swi.sync.sector1 = 0;
    }
}

static void pm_sync(void)
{
    if (app_data.status != PM_CHANGE) {
        return;
    }

    pm_sync_do();

    app_data.status = PM_NULL;
}

static void pm_init(void)
{
    if (FLASH_AB_PARAM_ADDR + FLASH_CM_SIZE > FLASH_SIZE) {
        printf("[err]flash overflow\n");
    }

    if (sizeof(flash_sector0_t) > FLASH_SECTOR_SIZE || sizeof(flash_sector1_t) > FLASH_SECTOR_SIZE) {
        printf("[err]sector overflow\n");
    }

    os_spiflash_read(&app_data.sector0, FLASH_SECTOR0_ADDR, sizeof(flash_sector0_t));
    os_spiflash_read(&app_data.sector1, FLASH_SECTOR1_ADDR, sizeof(flash_sector1_t));

    if (sector_is_null(PM_SYNC_SECTOR0)) {
        param_pm_clr(PM_SYNC_SECTOR0);
        sector0_var_init();
        param_pm_write_real(BIT(PM_SYNC_SECTOR0));
    }

    if (sector_is_null(PM_SYNC_SECTOR1)) {
        param_pm_clr(PM_SYNC_SECTOR1);
        sector1_var_init();
        param_pm_write_real(BIT(PM_SYNC_SECTOR1));
    }
}

void param_pm_write(u8 sync_swi)
{
#if PM_WRITE_DELAY
    app_data.status = PM_CHANGE;
    app_data.pm_times = 0;
    app_data.pm_swi.u32_sync |= (1 << sync_swi);
#else
    param_pm_write_real(sync_swi);
#endif // PM_WRITE_DELAY
}

void param_pm_write_real(u32 sync_swi)
{
    app_data.pm_swi.u32_sync |= sync_swi;
    pm_sync_do();
}

//------------------------------------------------------------------------------

void app_ab_data_storage_process(void)
{
    static u32 ticks = 0;
    if (tick_check_expire(ticks, 1000)) {
        ticks = tick_get();
        app_data.sec_cnt++;
        app_data.pm_times++;
    }

#if PM_WRITE_DELAY
    if (app_data.status == PM_CHANGE && app_data.pm_times >= PM_WRITE_DELAY) {
        pm_sync();
    }
#endif // PM_WRITE_DELAY
}

void app_ab_data_storage_init(void)
{
    pm_init();
}
#endif
