#include "include.h"
#include "api.h"
#include "api_modem.h"
#include "modem_custom.h"

#if MODEM_CAT1_EN
static int  custom_recv_items(void *parser);
static bool custom_recv_header(void *parser);

const struct at_cmd_ctrl at_cmd_custom_ctrl = { custom_recv_header };

// 在这边添加新的命令
static const struct at_cmd_recv at_cmd_custom_recv[] = {
    {"CUSTOM0", MODEM_CUSTOM0},
};

void modem_custom_notice(uint evt, void *param)
{
    switch (evt) {
    case MODEM_CUSTOM0:
        break;
    }
}

#define custom_recv_header_do(parser, cmd_recv)                                \
    if (modem_at_recv_header(parser, cmd_recv, custom_recv_items)) {           \
        return true;                                                           \
    }

static bool custom_recv_header(void *parser)
{
    for (u8 i = 0; i < ARRAY_SIZE(at_cmd_custom_recv); i++) {
        custom_recv_header_do(parser, &at_cmd_custom_recv[i]);
    }
    return false;
}

static int custom_recv_items(void *parser)
{
    switch (modem_parser_get_msg(parser)) {
    case MODEM_CUSTOM0:
    {
        struct custom1_notice_info *notice =
            (struct custom1_notice_info *)modem_parser_get_arg(parser);

        // 处理逗号分割的参数
        // e.g. \r\nAT+CUSTOM0\r\n\r\n+CUSTOM0: 1,2\r\n
        switch (modem_parser_get_item(parser)) {
        case 0:
            notice->arg0 = atoi(modem_parser_get_line(parser));
            break;
        case 1:
            notice->arg1 = atoi(modem_parser_get_line(parser));
            break;
        }
        break;
    }

    default:
        break;
    }

    if (modem_parser_get_eol(parser)) {
        modem_custom_notice(modem_parser_get_msg(parser),
                            modem_parser_get_arg(parser));
    }

    modem_parser_item_inc(parser);
    return 0;
}

void modem_custom_cmd0(void)
{
    // wait 50ms
    if (modem_check_can_send(10)) {
        const char *cmd0 = "AT+CUSTOM0\r\n";
        modem_pre_send_cmd();
        modem_raw_write(cmd0, strlen(cmd0));
        modem_aft_send_cmd();
    }
}
#endif
