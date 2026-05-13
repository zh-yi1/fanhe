#include "lwip/arch.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "lwip/tcp.h"
#include "lwip/ip.h"
#include "lwip/dhcp.h"
#include "lwip/sockets.h"
#include "lwip/prot/dhcp.h"
#include "lwip/ethip6.h"
#include "lwip/dns.h"
#include "lwip/mem.h"

#include "lwip/apps/sntp.h"
#include "netif/etharp.h"

#include "arch/sys_arch.h"
#include "os_api.h"

#define LWIP_OUTGOING_BUFFER_SIZE (NETIF_MTU + 14)

/* Short name used for netif in lwIP */
#define IFNAME0     'b'
#define IFNAME1     't'

#define log_debug   printf
#define log_info    printf
#define log_error   printf

#define WEAK                    __attribute__((weak))
#define printf(...)     my_printf(__VA_ARGS__)

typedef uint8_t bd_addr_t[6];

#if 1//BT_PANU_EN

static os_mq_t bnep_lwip_outgoing_mbox = NULL;
static struct netif btstack_netif;

// next packet only modified from btstack context
static void *bnep_lwip_outgoing_next_packet;
static u8_t bnep_lwip_outgoing_buf[LWIP_OUTGOING_BUFFER_SIZE];

void netlib_keep_symbols(void);
bool bnep_network_is_ok(void);
int lwip_system_init(void);
void btstack_thread_event_trigger(void);
void bt_thread_check_trigger(void);
char *bd_addr_to_str(bd_addr_t addr);
void bnep_network_send_packet(const uint8_t *packet, uint16_t size);
void tcp_kill_all(void);

WEAK void mbedtls_platform_init(void) {}

static uint16_t
bnep_lwip_pbuf_copy_partial(void *pbuf, void *dest, uint16_t dest_size)
{
    struct pbuf *p   = pbuf;
    uint16_t     len = minu(dest_size, p->tot_len);
    //printf("copy: %d %d %d\n", p->tot_len, dest_size, len);
    pbuf_copy_partial(p, dest, len, 0);
    return len;
}

WEAK void bnep_netif_status_callback(struct netif *netif)
{
    struct dhcp *dhcp = netif_dhcp_data(netif);

    switch (dhcp->state) {
    case DHCP_STATE_OFF:
        printf("DHCP state: off\n");
        // bsp_net_set_state(BSP_NET_OFF);
        break;
    case DHCP_STATE_REQUESTING:
        printf("DHCP state: request\n");
        // bsp_net_set_state(BSP_NET_SEARCHING);
        break;
    case DHCP_STATE_BOUND:
        printf("DHCP state: bound\n");
        printf("IP : %s\n", ipaddr_ntoa(&netif->ip_addr));
        printf("gw : %s\n", ipaddr_ntoa(&netif->gw));
        printf("netmask : %s\n", ipaddr_ntoa(&netif->netmask));
        break;
    // 其他状态可根据需要添加
    default:
        printf("DHCP state: %d\n", dhcp->state);
        break;
    }

    if (bnep_network_is_ok()) {
        ip_addr_t ntp_addr;
        // ipaddr_aton("ntp.aliyun.com", &ntp_addr);
        IP_ADDR4(&ntp_addr, 203, 107, 6, 88); //ntp.aliyun.com

        sntp_setserver(0, &ntp_addr);
        sntp_init();
    }
}

static void bnep_lwip_outgoing_packet_processed(void)
{
    // free pbuf
    pbuf_free(bnep_lwip_outgoing_next_packet);
    // mark as done
    bnep_lwip_outgoing_next_packet = NULL;
}

static int bnep_lwip_outgoing_init_queue(void)
{
    int pool_size = (4 + 4) * (TCP_SND_QUEUELEN + 2);

    bnep_lwip_outgoing_mbox = mem_malloc(sizeof(struct os_messagequeue));
    LWIP_ASSERT("bnep_lwip_outgoing_mbox", bnep_lwip_outgoing_mbox);

    void *msgpool = mem_malloc(pool_size);
    LWIP_ASSERT("bnep_lwip_outgoing_mbox pool", msgpool);

    os_mq_init(bnep_lwip_outgoing_mbox, "lb", msgpool, 4, pool_size, OS_IPC_FLAG_FIFO);
    return 0;
}

static bool bnep_lwip_outgoing_queue_packet(struct pbuf *p)
{
    // printf("%s\n", __func__);
    u32_t addr = (u32_t)p;
    if (os_mq_send(bnep_lwip_outgoing_mbox, &addr, 4) != OS_EOK) {
        printf("outgoing full\n");
        return false;
    }
    return true;
}

static struct pbuf *bnep_lwip_outgoing_pop_packet(void)
{
    if (bnep_lwip_outgoing_mbox == NULL) {
        return NULL;
    }
    u32_t          addr = 0;
    struct pbuf *p    = NULL;
    if (os_mq_recv(bnep_lwip_outgoing_mbox, &addr, 4, 0) != 0) {
        return NULL;
    }
    p = (struct pbuf *)addr;
    return p;
}

static void bnep_lwip_outgoing_reset_queue(void)
{
    if (bnep_lwip_outgoing_mbox) {
        while (bnep_lwip_outgoing_mbox->entry > 0) {
            pbuf_free(bnep_lwip_outgoing_pop_packet());
        }
    }
}

static int bnep_lwip_outgoing_packets_empty(void)
{
    return (bnep_lwip_outgoing_mbox->entry == 0);
}

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    // log_info("low_level_output: packet %p, len %u, total len %u\n",
    //          p,
    //          p->len,
    //          p->tot_len);

    // // bnep up?
    // if (bnep_cid == 0)
    //     return ERR_OK;
    // if (!(netif->flags & NETIF_FLAG_LINK_UP))
    //     return ERR_OK;

    // inc refcount
    pbuf_ref(p);

    LWIP_ASSERT("bnep_lwip_outgoing_mbox != NULL",
                bnep_lwip_outgoing_mbox != NULL);

    // queue up
    if (!bnep_lwip_outgoing_queue_packet(p)) {
        return ERR_MEM;
    }

    btstack_thread_event_trigger();
    bt_thread_check_trigger();
    return ERR_OK;
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
static err_t bnep_lwip_netif_init(struct netif *netif)
{
    // interface short name
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;

    // mtu
    netif->mtu = NETIF_MTU;

    /* device capabilities */
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
#if LWIP_IGMP
    netif->flags |= NETIF_FLAG_IGMP;
#endif

    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...)
     */
    netif->output = etharp_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif
    netif->linkoutput = low_level_output;

    return ERR_OK;
}

static void bnep_lwip_init(void)
{
    int ret = lwip_system_init();
    if (ret > 0) {
        LOCK_TCPIP_CORE();
        netif_set_down(netif_default);
        netif_set_up(netif_default);
        UNLOCK_TCPIP_CORE();
        return;
    } else if (ret < 0) {
        return;
    }
    log_debug("%s\n", __func__);
    bnep_lwip_outgoing_next_packet = NULL;
    netlib_keep_symbols();
    bnep_lwip_outgoing_init_queue();

    // input function differs for sys vs nosys
    netif_input_fn input_function;
    input_function = tcpip_input;

    LOCK_TCPIP_CORE();
    netif_add(&btstack_netif,
              NULL,
              NULL,
              NULL,
              NULL,
              bnep_lwip_netif_init,
              input_function);

    // 启用 IPv6 地址
    // netif_ip6_addr_set_state(&btstack_netif, 0, IP6_ADDR_VALID);

    netif_set_status_callback(&btstack_netif, bnep_netif_status_callback);
    netif_set_default(&btstack_netif);
    netif_set_up(netif_default);
    UNLOCK_TCPIP_CORE();
#ifdef LWIP_HAVE_MBEDTLS
    mbedtls_platform_init();
#endif
}

/**
 * @brief Bring up network interfacd
 * @param network_address
 * @return 0 if ok
 */
static int bnep_lwip_netif_up(bd_addr_t network_address)
{
    log_info("bnep_lwip_netif_up start addr %s\n",
             bd_addr_to_str(network_address));

    LOCK_TCPIP_CORE();
    // set mac address
    btstack_netif.hwaddr_len = 6;
    memcpy(btstack_netif.hwaddr, network_address, 6);

    // link is up
    netif_set_link_up(netif_default);
    dhcp_start(netif_default);

    UNLOCK_TCPIP_CORE();
    return 0;
}

static int bnep_lwip_netif_down(void)
{
    log_info("bnep_lwip_netif_down");

    LOCK_TCPIP_CORE();
    dhcp_stop(netif_default);
    netif_set_link_down(netif_default);
    UNLOCK_TCPIP_CORE();

    return 0;
}

AT(.com_text.stack.run_loop)
static void bnep_lwip_discard_packets(void)
{
    // discard current packet
    if (bnep_lwip_outgoing_next_packet) {
        bnep_lwip_outgoing_packet_processed();
    }

    // reset queue
    bnep_lwip_outgoing_reset_queue();
}

static void bnep_lwip_netif_process_packet(const uint8_t *packet, uint16_t size)
{
    struct pbuf *p = pbuf_alloc(PBUF_RAW, size, PBUF_RAM);
    // log_debug("%s alloc=%x\n", __func__, p);

    if (!p) {
        printf("%s err\n", __func__);
        return;
    }

    /* store packet in pbuf chain */
    struct pbuf *q = p;
    while (q != NULL && size) {
        memcpy(q->payload, packet, q->len);
        packet += q->len;
        size -= q->len;
        q = q->next;
    }

    if (size != 0) {
        log_error("failed to copy data into pbuf");
        pbuf_free(p);
        return;
    }

    /* pass all packets to ethernet_input, which decides what packets it supports */
    int res = btstack_netif.input(p, netif_default);
    if (res != ERR_OK) {
        log_error("bnep_lwip_netif_process_packet: IP input error %d\n", res);
        pbuf_free(p);
        p = NULL;
    }
}

WEAK void bnep_network_init(void)
{
    bnep_lwip_init();
}

WEAK void bnep_network_up(bd_addr_t addr)
{
    bnep_lwip_discard_packets();
    bnep_lwip_netif_up(addr);
}

WEAK void bnep_network_down(void)
{
    bnep_lwip_discard_packets();
    bnep_lwip_netif_down();
}

WEAK void bnep_network_process_packet(const uint8_t *packet, uint16_t size)
{
    bnep_lwip_netif_process_packet(packet, size);
}

AT(.com_text.stack.run_loop)
WEAK void bnep_network_outgoing_process(void)
{
    uint16_t len;

    // previous packet not sent yet
    if (bnep_lwip_outgoing_next_packet) {
        return;
    }

    if (bnep_lwip_outgoing_packets_empty()) {
        return;
    }

    void *packet = bnep_lwip_outgoing_pop_packet();
    if (!packet) {
        printf("%s empty\n", __func__);
        return;
    }
    bnep_lwip_outgoing_next_packet = packet;

    // flatten into our buffer
    len = bnep_lwip_pbuf_copy_partial(bnep_lwip_outgoing_next_packet,
                                      bnep_lwip_outgoing_buf,
                                      LWIP_OUTGOING_BUFFER_SIZE);

    // request can send now
    bnep_network_send_packet(bnep_lwip_outgoing_buf, len);
    btstack_thread_event_trigger();
    bt_thread_check_trigger();
}

WEAK void bnep_network_packet_sent(uint8_t *buf)
{
    // log_debug("bnep_lwip_packet_sent: %p\n", bnep_lwip_outgoing_next_packet);

    // release current packet
    bnep_lwip_outgoing_packet_processed();

    // more ?
    if (!bnep_lwip_outgoing_packets_empty()) {
        btstack_thread_event_trigger();
        bt_thread_check_trigger();
    }
}

bool bnep_network_is_ok(void)
{
    struct netif *netif = netif_default;

    // 检查接口状态
    if (!netif_is_up(netif) || !netif_is_link_up(netif)) {
        return false;
    }

    // 检查IP地址
    const ip_addr_t* ip_addr = netif_ip_addr4(netif);
    if (ip_addr_isany(ip_addr)) {
        return false;
    }

    // 检查网关
    const ip_addr_t* gw_addr = netif_ip_gw4(netif);
    if (ip_addr_isany(gw_addr)) {
        return false;
    }

    // 检查DNS
    const ip_addr_t* dns_addr = dns_getserver(0);
    if (ip_addr_isany(dns_addr)) {
        return false;
    }

#if LWIP_DHCP
    struct dhcp *dhcp = netif_dhcp_data(netif);
    if (dhcp->state == DHCP_STATE_OFF ||
        dhcp->state == DHCP_STATE_INIT) {
        return false;
    }
#endif

    return true;
}

extern struct tcp_pcb *tcp_tw_pcbs;
extern struct tcp_pcb *tcp_active_pcbs;
void tcp_kill_all(void)
{
    LOCK_TCPIP_CORE();
    bnep_lwip_outgoing_reset_queue();

    struct tcp_pcb *pcb;
    for (pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) {
        LWIP_DEBUGF(TCP_DEBUG, ("tcp_kill_all: killing tw PCB %p\n",
                                (void *)pcb));
        tcp_abort(pcb);
    }
    for (pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
        LWIP_DEBUGF(TCP_DEBUG, ("tcp_kill_all: killing active PCB %p\n",
                                (void *)pcb));
        tcp_abort(pcb);
    }
    UNLOCK_TCPIP_CORE();
}
#endif
