# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概览

中科蓝讯 AB5790（RV32 单核）智能饭盒固件。主板负责屏幕 UI + 按键 + BLE，
真正的加热执行体是另一颗"加热模块"MCU，两者走 UART1 私有协议；手机 APP 走 BLE，
主板同时充当 **BLE ↔ UART 翻译桥**。

仓库只有一个工程：`projects/watch466`（沿用手表 SDK 的工程名，页面已换成饭盒 UI）。
`platform/` 是芯片原厂 SDK（BSP + GUI + 预编译 `.a`），除非确有必要不要改。
本仓库是 `~/zhou/elunchbox` 的 UI 重建版本，通信栈从那边移植而来，遇到缺件可去那边取。

## 构建 / 烧录

**这个 WSL 检出里没有工具链，无法编译。** 构建只在 Windows 侧做：
Code::Blocks 打开 `projects/watch466/app.cbp`（编译器配置 `riscv32-v3`），Build。

- 编译前 `Output/bin/prebuild.bat`：`riscv32-elf-xmaker` 打包 `ui.xm` / `res.xm` / `xcfg.xm`
  → 生成 `ui.bin`、并把 `ui.h` / `res.h` 拷回工程根目录（页面用的 `UI_BUF_*_BIN` 宏就来自这里）。
- 编译后 `postbuild.bat` → `app.bin` / `app.dcf`；`postbuild_ble_ota.bat` → `app_ble_ota_all.fot`（BLE OTA 包）。
- 烧录用原厂 Downloader（`Output/bin/ota_dog/`）；串口日志抓在仓库根的 `log.txt`，
  排查上板问题先读它（复位原因、sram 占用、各模块 printf 都在）。
- 没有单元测试 / lint。改完代码请用户在 Windows 侧编译验证，不要声称"已编译通过"。

**新增 `.c` 文件必须手工加进 `app.cbp` 的 `<Unit>`**，否则链接到 `func_watch_stubs.c`
里的同名空桩，症状是运行期跑飞/WDT 复位而不是链接错误。
从 `new` / `new_ui` 分支合并后先 `grep functions/comm projects/watch466/app.cbp`
确认 include 目录和 Unit 还在——历史上被合并冲掉过。

## 架构

### 启动与主循环

`main()`（`projects/watch466/main.c`）→ 打印复位原因 → `bsp_sys_init()` →
`lunchbox_uart_init(LB_BAUD)` → `func_run()`（[func.c](projects/watch466/functions/func.c)，永不返回）。

`func_run()` 是 "页面即任务" 的状态机：`func_cb.sta` 决定当前页，循环里按
`tbl_func_entry` 查表调用该页的入口函数；入口函数内部是
`enter() → while (func_cb.sta == 本页) { process(); message(msg_dequeue()); } → exit()`。
**切页 = 直接写 `func_cb.sta`**，本页 while 条件失效即退出，回到 `func_run` 重新查表。

[func_tbl.h](projects/watch466/functions/func_tbl.h) 里有四张表，新增页面四张都要登记：

| 表 | 内容 | 用途 |
|:---|:---|:---|
| `tbl_func_create` | `xxx_form_create` | 切换动画/`func_create_form()` 建窗体 |
| `tbl_func_entry` | `xxx`（页面主循环） | `func_run` 调度 |
| `tbl_func_enter` | `xxx_enter` | 切换动画路径下的进页钩子 |
| `tbl_func_exit` | `xxx_exit` | `func_cur_sta_exit()` |

`FUNC_*` 枚举在 [func.h](projects/watch466/functions/func.h)；枚举值不要重排——
手表遗留页仍占位，实现全在 `func_watch_stubs.c`（进去就弹回 HOME）。

`func_process()`（[func.c:400](projects/watch466/functions/func.c#L400)）是每页 `process()`
都必须调用的公共帧：喂狗、PT8028 扫描、串口/BLE 数据泵、`compo_update()`+`gui_process()`、
`sleep_process()`、充电检测。**页面自己不要另起这些**。

### 通信栈 `functions/comm/`（全部 `lb_` 前缀，`FUNC_LUNCHBOX_UART_EN` 开关）

分层（下→上）：

- `lb_uart_link` — UART1 硬件 + RX 环形缓冲（TX=PB8 / RX=PB9，115200 8N1）
- `lb_proto` — `0x55AA` 帧编解码，纯函数，BLE 和 UART 共用同一帧格式
- `lb_uart_app` — 取字节→拼帧→`lb_uart_on_frame` 分发；还驱动转发队列重试、
  预约列表同步、**开关机时序状态机**、加热模块 OTA
- `lb_ble_app` — BLE 收包（`app_blue_fit.c` 4 槽环形缓冲，UUID ae30/ae03/ae04）+ 应答/主动上报
- `lb_bridge` — BLE↔UART 命令字与数据翻译层 + 时间服务 `lb_time_*`
- `lb_heat_cmd` — 主机主动发给加热模块的命令封装（start/stop/power/schedule…）
- `lb_ui_state` — 设备状态镜像 / 预约列表镜像 / 自动跳页路由（UI 的唯一数据源）
- `lb_ota`（主 MCU） / `lb_uart_heat`（加热模块 OTA，BLE 收→SPI Flash 暂存→UART 推送）

`msg_flag` 号段约定：`0x00~0x7F` = APP 经 BLE 发起、MCU 原样转发；
`0x80~0xFF` = MCU 自己发起（`lb_heat_cmd` 自增后 `|= 0x80`）。

### UI 数据面（单向）

```
加热模块 DP 上报 → lb_ui_state_feed_dp()  [唯一写入口]
                 → lb_ui_state_get()      [镜像, 带 seq 版本号]
                 → lb_ui_sync_pull()      [func_process 每轮, general_ui.c 末尾]
                 → g_ui_sys (ui_sys_t)    [页面只读, 不要写]
```

页面判断"要不要刷"靠自己记 `last_seq` 比对 `st->seq`。
预约列表另有一套：`lb_ui_schedules_get()`，判据必须是 `complete && seq 变化`
（多帧传输中 seq 会连跳）。

自动跳页：`lb_ui_route_poll()` 产生的边沿**只能被消费一次**，
唯一调用点是 `func.c` 的 `lb_ui_route_apply()`（在 `func_process` 里），别在别处调。

`functions/new_ui/app_ui.h` 由 UI 端整份重发覆盖，**只放 UI 自己的结构体**；
移植层给页面的声明一律写在 `general_ui.h`。

### 页面写法（`functions/new_ui/`）

私有状态放 `func_cb.f_cb`（`enter()` 里 `func_zalloc(sizeof(f_xxx_t))`），
控件在 `xxx_form_create()` 里建。顶部状态栏用 `general_status_bar_create/attach/tick/detach`
四步（见 [general_ui.h](projects/watch466/functions/new_ui/general_ui.h)）。
文案走 `i18n[STR_XXX]`（`i18n/lang*.c`），图片走 `UI_BUF_*_BIN`。

### 按键

PT8028S 触摸面板，8 个物理键 TCH0~7，由 `functions/key/func_key.c` 统一成
逻辑键（UP/DOWN/CONFIRM/BACK/HEAT/MODE/RESERVATION/POWER/LOCK）。
页面里 `func_key_poll()` → `while (func_key_get_event(&evt))` →
`func_key_map_logical(evt.tch)`。童锁在 `func_key_lock.c`（过滤在 key 模块，UI 由页面画）。

### 电源 / 低功耗

`functions/lowpower/elunchbox_lp.c` + `functions/common/func_lowpwr.c`。

- 关机时序（状态机在 `lb_uart_app.c`）：`heat_off(DP10=0)` → `power_off(DP1=0)` →
  `OFF_DONE` → `func.c` 的 `lb_shutdown_seq_apply()` 取边沿落地。
  三个入口都汇到 `lunchbox_shutdown_start()`：APP 下发 DP1=0 / 长按 TCH5 3s / 5 分钟无操作。
- 落地分流：未充电 → **manual_off 超低功耗深睡**（唤醒源只剩 PE1 和 PB9，唤醒不重启，RAM 原地复活）；
  充电中 → **黑屏充电页 `FUNC_BLACK_SCREEN`**（不深睡，仍收串口驱动充电动画）。
- **不要走 `FUNC_PWROFF`**：充电时 `func_pwroff()` 会 return 不断电，导致反复进关机页。
- 加热/保温/OTA 期间禁止息屏（`elunchbox_heating_blocks_idle()`）。

## 配置开关

`projects/watch466/config.h`（7 万字，基本是 SDK 原配置 + 饭盒裁剪）。关键几个：

- `FUNC_LUNCHBOX_UART_EN` = 1 — 整个 `lb_*` 通信栈的总开关
- `ELUNCHBOX_PANEL_EN` = 1 — 饭盒面板形态；大量 `#if !ELUNCHBOX_PANEL_EN` 段是被关掉的手表功能
- `USER_PT8028_KEY` / `USER_PANEL_LED` / `FUNC_RESERVATION_UI_EN` / `CHARGE_EN`

## 必读文档 `projects/watch466/docs/`

- **`通信移植遗留事项.md`** — 活文档，改通信/电源/UI 对接前先读。
  记着已实测的假设、UI 对接接口表、开关机时序、已知 bug、联调踩坑记录。改动后顺手更新它。
- `MCU通信协议.md`（主板↔加热模块 UART）、`蓝牙通讯协议1.0.10.md`（APP↔主板 BLE）—— 协议以文档为准
- `指令解析手册.md` / `校验和计算说明.md` / `通信联调测试手册.md`

## 约定

- 注释、文档、commit message 一律中文；commit 前缀 `fix:` / `feat:`，一句话说清现象+原因。
- 当前开发分支 `debug`，PR 目标 `new_ui`。
- 资源生成物 `ui.h` / `res.h` 是 `xmaker` 产物（已在版本库里，各人本地会被 prebuild 覆盖）；
  `.gitignore` 有 `**/ui.h`，**新建的 `*ui.h` 头文件会被静默忽略**——历史上 `new_ui/ui.h`
  就因此进不了仓库，才改名 `app_ui.h`。
- `tools/gen_*.py`（PNG→GPU bin）是旧 UI 时期的脚本，路径指向已不存在的 `ui/home/`，
  新 UI 资源直接放 `Output/bin/ui/new_ui/` 后跑 `prebuild.bat`。
