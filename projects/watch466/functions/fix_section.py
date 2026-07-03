import sys

with open('func.c', 'r', encoding='utf-8', errors='replace') as f:
    content = f.read()

# Define the old mangled block (lines 306-320)
old_block = '''\t    /*
\t     * 手动关机: GPU 掉电 → BLE/BT 断连 → 省电 → sfunc_sleep 睡眠循环
\t     * 使用芯片标准 bt_sleep_proc() 路径，周期性唤醒轮询 TCH5 长按。
\t     */
\t\t    /*
\t\t     * Manual shutdown: set manual_off flag FIRST to block UI/BLE callbacks
\t\t     * from accessing GPU after it is powered down.
\t\t     */
\t\t    elunchbox_pwr_manual_off = true;
\t\t    gui_sleep(true);
\t\t    elunchbox_pwr_gui_off = true;
\t\t    sys_cb.gui_need_wakeup = 0;
\t    elunchbox_pwr_gui_off = true;
\t    elunchbox_pwr_manual_off = true;
\t    sys_cb.gui_need_wakeup = 0;'''

new_block = '''\t    /*
\t     * 手动关机: 先设 manual_off 标志阻止 UI/BLE 回调访问 GPU,
\t     * 再关 GPU → BLE/BT 断连 → 省电 → MCU 深度睡眠。
\t     * 标志位必须在 gui_sleep 之前设, 否则 ble_disconnect 的回调
\t     * (func_message 等) 会在 GPU 已掉电时访问显存 → ERR:2 非法指令。
\t     */
\t    elunchbox_pwr_manual_off = true;
\t    gui_sleep(true);
\t    elunchbox_pwr_gui_off = true;
\t    sys_cb.gui_need_wakeup = 0;'''

if old_block in content:
    content = content.replace(old_block, new_block, 1)
    print("Part 1: replaced duplicate block")
else:
    print("Part 1: old block NOT FOUND, checking...")
    # Try to find what's there
    idx = content.find('elunchbox_boot_power_sent = false;')
    if idx >= 0:
        print("Found context at offset", idx)
        snippet = content[idx:idx+600]
        for i, line in enumerate(snippet.split('\n'), 1):
            print(f"  {i}: {repr(line[:100])}")

# Fix the second mangled block (GPIO polling section)
old_block2 = '''\t    /*
\t     * 自定义 MCU 睡眠: LPMCON + PE1 端口唤醒 + RTC 1s 周期轮询
\t     * 不用 ROM bt_sleep_proc() — 该芯片的 ROM 睡眠与手表不同，会导致指令访问错误。
\t     * 不用 bt_off() — 保持 BT 初始化，射频在空闲态功耗可接受。
\t     * 参考: sfunc_lowbat_do() 的 LPMCON 睡眠模式。
\t     */

\t\t    /*
\t\t     * TCH5 manual off -> MCU sleep: wait for key release.
\t\t     * Direct GPIO poll instead of pt8028_wait_out_flag_release()
\t\t     * because that function is in .text.pwroff.pwrdwn section and
\t\t     * will crash (ERR:1) without sfunc_power_save_enter() first.
\t\t     */
\t\t    GPIOEDE |= (BIT(0) | BIT(1));
\t\t    GPIOEPU200K |= BIT(1);
\t\t    while (bsp_gpio_get_sta(IO_PE1) == 0) {
\t\t        delay_5ms(1);
\t\t    }
\t\t    pt8028_key_scan();
\t\t    pt8028_release_clear();
\t    port_wakeup_init(PT8028_GPIO_OUT_FLAG, 1, 1);   /* PE1 下降沿唤醒 */'''

new_block2 = '''\t    /*
\t     * 自定义 MCU 睡眠: LPMCON + PE1 端口唤醒 + RTC 1s 周期轮询
\t     * 不用 ROM bt_sleep_proc() — 该芯片 ROM 睡眠导致指令访问错误。
\t     * 参考: sfunc_lowbat_do() 的 LPMCON 睡眠模式。
\t     *
\t     * 等松手: 直接读 PE1 不用 pt8028_wait_out_flag_release (在
\t     * .text.pwroff.pwrdwn 段, 未调 sfunc_power_save_enter 会崩溃)
\t     */
\t    GPIOEDE |= (BIT(0) | BIT(1));                   /* PE0+PE1 数字输入 */
\t    GPIOEPU200K |= BIT(1);                          /* PE1 200K 上拉 */
\t    /* PE1=0 -> OUT_FLAG 有效(有键按下); PT8028_FLAG_ACTIVE_LOW=1 */
\t    while (bsp_gpio_get_sta(IO_PE1) == 0) {
\t        delay_5ms(1);
\t    }
\t    pt8028_key_scan();
\t    pt8028_release_clear();

\t    port_wakeup_init(PT8028_GPIO_OUT_FLAG, 1, 1);   /* PE1 下降沿唤醒 */'''

if old_block2 in content:
    content = content.replace(old_block2, new_block2, 1)
    print("Part 2: replaced GPIO polling block")
else:
    print("Part 2: old block NOT FOUND")
    idx = content.find('自定义 MCU 睡眠')
    if idx >= 0:
        snippet = content[idx:idx+500]
        for i, line in enumerate(snippet.split('\n'), 1):
            print(f"  {i}: {repr(line[:100])}")

with open('func.c', 'w', encoding='utf-8', newline='') as f:
    f.write(content)
print("Done")
