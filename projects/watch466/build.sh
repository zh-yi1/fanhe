#!/bin/bash
#=============================================================================
# build.sh — VS Code 编译脚本 (watch466 / RISC-V V3)
# 用法:
#   ./build.sh          # 完整构建 (预编译 + 编译 + 链接 + 后处理)
#   ./build.sh clean    # 清理编译产物
#   ./build.sh rebuild  # 清理后重新构建
#=============================================================================
set -e

# ---- 工具链路径 ----------------------------------------------------------
TOOLCHAIN_DIR="/d/Program Files (x86)/RV32-Toolchain/RV32-V3/bin"
CC="${TOOLCHAIN_DIR}/riscv32-elf-gcc.exe"
OBJCOPY="${TOOLCHAIN_DIR}/riscv32-elf-objcopy.exe"
XMAKER="${TOOLCHAIN_DIR}/riscv32-elf-xmaker.exe"

# ---- 路径定义 ------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

OBJ_DIR="Output/obj"
BIN_DIR="Output/bin"
PLATFORM_DIR="../../platform"

# ---- 编译选项 (来自 app.cbp) ---------------------------------------------
CFLAGS=(
    -Os -Wall
    -march=rv32imc_zba_zbb_zbc_zbs_zca_zcb_zcmp_zfinx_xbs1
    --param=min-pagesize=0
    -Wno-address-of-packed-member
    -ffunction-sections
    -mjump-tables-in-text
    -c
)

# 包含路径 (来自 app.cbp Compiler 段)
INCLUDES=(
    -I.
    -Ifunctions
    -Iport
    -Ii18n
    -Iplugin
    -I"${PLATFORM_DIR}/header"
    -I"${PLATFORM_DIR}/libs"
    -I"${PLATFORM_DIR}/bsp"
    -I"${PLATFORM_DIR}/bsp/bsp_app"
    -I"${PLATFORM_DIR}/bsp/bsp_bt"
    -I"${PLATFORM_DIR}/bsp/bsp_ble"
    -I"${PLATFORM_DIR}/bsp/bsp_sensor"
    -I"${PLATFORM_DIR}/bsp/bsp_music"
    -I"${PLATFORM_DIR}/bsp/bsp_effect"
    -I"${PLATFORM_DIR}/gui"
    -I"${PLATFORM_DIR}/gui/rotate_disp"
    -I"${PLATFORM_DIR}/gui/components"
    -I"${PLATFORM_DIR}/gui/tft"
    -I"${PLATFORM_DIR}/gui/ctp"
    -I"${PLATFORM_DIR}/gui/ctp/axs5106"
    -Ifunctions/common
    -I"${PLATFORM_DIR}/libs/libfixmath"
    -I"${PLATFORM_DIR}/libs/net/include"
    -I"${PLATFORM_DIR}/libs/net/include/lwip"
    -I"${PLATFORM_DIR}/libs/net/include/lwip/ports"
    -I"${PLATFORM_DIR}/bsp/bsp_fs"
    -I"${PLATFORM_DIR}/bsp/bsp_record"
)

# ---- 链接选项 ------------------------------------------------------------
LDFLAGS=(
    "-Wl,-T,${OBJ_DIR}/ram.o"
    "-Wl,--gc-sections"
    "-Wl,--no-warn-rwx-segments"
    "-Wl,-Map=${BIN_DIR}/map.txt"
)

LIB_PATHS=(
    "-L${PLATFORM_DIR}/libs"
    "-L${PLATFORM_DIR}/libs/net"
)

LIBS=(
    -lplatform -lbtstack -ldrivers_v2 -lgui
    "${PLATFORM_DIR}/bsp/bsp_sensor/hx3605/libhx3605.a"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hrs3300/libhrs3300.a"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hrs3300/libhrs3300_s.a"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hrs3300/libhrs3300_test.a"
    "${PLATFORM_DIR}/bsp/bsp_sensor/sc7a20/libsc7a20.a"
    "${PLATFORM_DIR}/libs/libm.a"
    "${PLATFORM_DIR}/libs/libgcc.a"
    "${PLATFORM_DIR}/libs/libc.a"
    "${PLATFORM_DIR}/bsp/bsp_sensor/qmc6309/lib_qst_ical.a"
)

# ---- 源文件列表 (来自 app.cbp) -------------------------------------------
SOURCES=(
    # project root
    main.c config.c
    # functions
    functions/func.c functions/func_activity.c functions/func_ble_gatts.c
    functions/func_key_lock.c functions/home_ui_lock_overlay.c functions/home_ui_lowbat_overlay.c
    functions/func_bt.c functions/func_charge.c functions/func_compo_select.c
    functions/func_compo_select_sub.c functions/func_debug_info.c functions/func_heat.c functions/func_heat_panel.c
    functions/heat_display_reg.c functions/func_home.c functions/func_new_home.c
    functions/new_home_top_time.c functions/home_ui_shared.c functions/home_tab_label.c
    functions/home_top_time.c functions/home_top_time_txt.c functions/home_ui_ram.c functions/home_ui_gpu_detach.c
    functions/func_lunchbox_uart.c functions/func_lunchbox_lcd.c
    functions/func_lunchbox_ble.c functions/func_lunchbox_bridge.c
    functions/func_lunchbox_ota.c functions/func_lunchbox_uart_heat.c
    functions/func_mode.c functions/func_reservation.c
    functions/func_setup.c functions/func_timeing.c functions/func_languageing.c
    functions/func_verinfo.c
    functions/func_new_heat.c functions/func_new_warm.c functions/func_new_mode.c
    functions/func_new_reservation.c functions/func_new_setup.c functions/func_new_timeing.c
    functions/func_new_language.c functions/func_new_verinfo.c
    functions/func_setting.c
    functions/common/func_bt_update.c functions/common/func_idle.c
    functions/common/func_lowpwr.c functions/common/func_manage.c
    functions/common/func_ota_ui.c functions/common/func_switching.c
    functions/common/func_switching3d.c functions/common/func_update.c
    functions/common/listbox.c functions/common/msgbox.c
    functions/common/rotary.c functions/common/sfunc_bt_ota.c
    # i18n
    i18n/lang.c i18n/lang_en.c i18n/lang_zh.c
    # plugin
    plugin/bt_call.c plugin/eq_table.c plugin/multi_lang.c plugin/plugin.c
    # port
    port/port_key.c port/port_pt8028_key.c port/port_panel_led.c
    port/port_mute.c port/port_sd.c port/port_update.c port/stubs_elunchbox.c
    # platform/bsp
    "${PLATFORM_DIR}/bsp/bsp_asr.c"
    "${PLATFORM_DIR}/bsp/bsp_audio.c"
    "${PLATFORM_DIR}/bsp/bsp_auphy.c"
    "${PLATFORM_DIR}/bsp/bsp_backtrace.c"
    "${PLATFORM_DIR}/bsp/bsp_charge.c"
    "${PLATFORM_DIR}/bsp/bsp_cm.c"
    "${PLATFORM_DIR}/bsp/bsp_dac.c"
    "${PLATFORM_DIR}/bsp/bsp_eq.c"
    "${PLATFORM_DIR}/bsp/bsp_fmrx.c"
    "${PLATFORM_DIR}/bsp/bsp_gif.c"
    "${PLATFORM_DIR}/bsp/bsp_gpio.c"
    "${PLATFORM_DIR}/bsp/bsp_halt.c"
    "${PLATFORM_DIR}/bsp/bsp_huart.c"
    "${PLATFORM_DIR}/bsp/bsp_hw_timer.c"
    "${PLATFORM_DIR}/bsp/bsp_i2c.c"
    "${PLATFORM_DIR}/bsp/bsp_key.c"
    "${PLATFORM_DIR}/bsp/bsp_pt8028_key.c"
    "${PLATFORM_DIR}/bsp/bsp_map.c"
    "${PLATFORM_DIR}/bsp/bsp_mav.c"
    "${PLATFORM_DIR}/bsp/bsp_opus.c"
    "${PLATFORM_DIR}/bsp/bsp_param.c"
    "${PLATFORM_DIR}/bsp/bsp_piano.c"
    "${PLATFORM_DIR}/bsp/bsp_port_int.c"
    "${PLATFORM_DIR}/bsp/bsp_pwm.c"
    "${PLATFORM_DIR}/bsp/bsp_qdec.c"
    "${PLATFORM_DIR}/bsp/bsp_ram_backup.c"
    "${PLATFORM_DIR}/bsp/bsp_rtc.c"
    "${PLATFORM_DIR}/bsp/bsp_saradc.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor_hub.c"
    "${PLATFORM_DIR}/bsp/bsp_sleep.c"
    "${PLATFORM_DIR}/bsp/bsp_spi.c"
    "${PLATFORM_DIR}/bsp/bsp_spi1flash.c"
    "${PLATFORM_DIR}/bsp/bsp_spp.c"
    "${PLATFORM_DIR}/bsp/bsp_sys.c"
    "${PLATFORM_DIR}/bsp/bsp_uart.c"
    "${PLATFORM_DIR}/bsp/bsp_uitool_phrase.c"
    "${PLATFORM_DIR}/bsp/bsp_vbat.c"
    # platform/bsp/bsp_app
    "${PLATFORM_DIR}/bsp/bsp_app/app_ancs.c"
    "${PLATFORM_DIR}/bsp/bsp_app/app_platform.c"
    "${PLATFORM_DIR}/bsp/bsp_app/weak_symbol.c"
    "${PLATFORM_DIR}/bsp/bsp_app/ab_command/ab_common.c"
    "${PLATFORM_DIR}/bsp/bsp_app/ab_command/app_data.c"
    "${PLATFORM_DIR}/bsp/bsp_app/ab_command/data_storage.c"
    "${PLATFORM_DIR}/bsp/bsp_app/ab_command/data_transmit.c"
    "${PLATFORM_DIR}/bsp/bsp_app/ab_command/device_attr.c"
    "${PLATFORM_DIR}/bsp/bsp_app/ab_command/health_data.c"
    "${PLATFORM_DIR}/bsp/bsp_app/ab_command/msg_notification.c"
    "${PLATFORM_DIR}/bsp/bsp_app/ab_command/personal_info.c"
    "${PLATFORM_DIR}/bsp/bsp_app/ab_command/status_control.c"
    # platform/bsp/bsp_ble
    "${PLATFORM_DIR}/bsp/bsp_ble/adv0.c"
    "${PLATFORM_DIR}/bsp/bsp_ble/app.c"
    "${PLATFORM_DIR}/bsp/bsp_ble/app_ab_link.c"
    "${PLATFORM_DIR}/bsp/bsp_ble/app_blue_fit.c"
    "${PLATFORM_DIR}/bsp/bsp_ble/ble.c"
    "${PLATFORM_DIR}/bsp/bsp_ble/bsp_ams.c"
    "${PLATFORM_DIR}/bsp/bsp_ble/bsp_ancs.c"
    "${PLATFORM_DIR}/bsp/bsp_ble/bt_fota.c"
    "${PLATFORM_DIR}/bsp/bsp_ble/hid/ble_hid_event.c"
    "${PLATFORM_DIR}/bsp/bsp_ble/hid/ble_hid_service.c"
    "${PLATFORM_DIR}/bsp/bsp_ble/hid/ble_hid_tiktok.c"
    # platform/bsp/bsp_bt
    "${PLATFORM_DIR}/bsp/bsp_bt/bsp_bt.c"
    "${PLATFORM_DIR}/bsp/bsp_bt/bt.c"
    "${PLATFORM_DIR}/bsp/bsp_bt/bt_id3_tag.c"
    # platform/bsp/bsp_effect
    "${PLATFORM_DIR}/bsp/bsp_effect/mic_effect.c"
    # platform/bsp/bsp_fs
    "${PLATFORM_DIR}/bsp/bsp_fs/bsp_disk.c"
    "${PLATFORM_DIR}/bsp/bsp_fs/example/fs_example_findall_frist.c"
    "${PLATFORM_DIR}/bsp/bsp_fs/example/fs_example_findfirst.c"
    "${PLATFORM_DIR}/bsp/bsp_fs/example/fs_example_scan_disk.c"
    "${PLATFORM_DIR}/bsp/bsp_fs/fs_drv_sd.c"
    "${PLATFORM_DIR}/bsp/bsp_fs/fs_drv_spiflash.c"
    # platform/bsp/bsp_mem
    "${PLATFORM_DIR}/bsp/bsp_mem/bsp_mem.c"
    "${PLATFORM_DIR}/bsp/bsp_mem/heap.c"
    "${PLATFORM_DIR}/bsp/bsp_mem/tlsf.c"
    # platform/bsp/bsp_music
    "${PLATFORM_DIR}/bsp/bsp_music/bsp_music.c"
    "${PLATFORM_DIR}/bsp/bsp_music/music_id3_tag.c"
    "${PLATFORM_DIR}/bsp/bsp_music/music_lrc.c"
    "${PLATFORM_DIR}/bsp/bsp_music/music_res.c"
    # platform/bsp/bsp_net
    "${PLATFORM_DIR}/bsp/bsp_net/bsp_lwip.c"
    "${PLATFORM_DIR}/bsp/bsp_net/common.c"
    # platform/bsp/bsp_noise
    "${PLATFORM_DIR}/bsp/bsp_noise/bsp_noise_dnn.c"
    # platform/bsp/bsp_record
    "${PLATFORM_DIR}/bsp/bsp_record/bsp_mic_record.c"
    "${PLATFORM_DIR}/bsp/bsp_record/bsp_record_fs.c"
    # platform/bsp/bsp_sensor
    "${PLATFORM_DIR}/bsp/bsp_sensor/bsp_sensor.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hrs3300/fit_hrs3300.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hrs3300/hrs3300.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hx3602/fit_hx3602.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hx3602/hx3602.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hx3602/hx3602_hrs_driv.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hx3605/fit_hx3605.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hx3605/hx3605.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hx3605/hx3605_factory_test.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hx3605/hx3605_hrs_agc.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/hx3605/hx3605_spo2_agc.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/internal_sensor/bsp_internal_sensor.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/msa310/msa310.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/msa310/msa_app.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/qmc6309/qmc6309.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/sc7a20/sc7a20.c"
    "${PLATFORM_DIR}/bsp/bsp_sensor/sc7a20/sl_watch_application.c"
    # platform/gui
    "${PLATFORM_DIR}/gui/api_gui.c"
    "${PLATFORM_DIR}/gui/gui.c"
    "${PLATFORM_DIR}/gui/rotate_disp/rotate_disp.c"
    "${PLATFORM_DIR}/gui/rotate_disp/rotate_widget.c"
    # platform/gui/components
    "${PLATFORM_DIR}/gui/components/compo_animation.c"
    "${PLATFORM_DIR}/gui/components/compo_arc.c"
    "${PLATFORM_DIR}/gui/components/compo_butterfly.c"
    "${PLATFORM_DIR}/gui/components/compo_button.c"
    "${PLATFORM_DIR}/gui/components/compo_camera.c"
    "${PLATFORM_DIR}/gui/components/compo_cardbox.c"
    "${PLATFORM_DIR}/gui/components/compo_chartbox.c"
    "${PLATFORM_DIR}/gui/components/compo_cube.c"
    "${PLATFORM_DIR}/gui/components/compo_datetime.c"
    "${PLATFORM_DIR}/gui/components/compo_disklist.c"
    "${PLATFORM_DIR}/gui/components/compo_fish.c"
    "${PLATFORM_DIR}/gui/components/compo_football.c"
    "${PLATFORM_DIR}/gui/components/compo_form.c"
    "${PLATFORM_DIR}/gui/components/compo_gif.c"
    "${PLATFORM_DIR}/gui/components/compo_iconlist.c"
    "${PLATFORM_DIR}/gui/components/compo_jpg.c"
    "${PLATFORM_DIR}/gui/components/compo_label.c"
    "${PLATFORM_DIR}/gui/components/compo_listbox.c"
    "${PLATFORM_DIR}/gui/components/compo_move_ctr.c"
    "${PLATFORM_DIR}/gui/components/compo_number.c"
    "${PLATFORM_DIR}/gui/components/compo_picturebox.c"
    "${PLATFORM_DIR}/gui/components/compo_qrcodebox.c"
    "${PLATFORM_DIR}/gui/components/compo_rings.c"
    "${PLATFORM_DIR}/gui/components/compo_rotary.c"
    "${PLATFORM_DIR}/gui/components/compo_rowbox.c"
    "${PLATFORM_DIR}/gui/components/compo_scrollbar.c"
    "${PLATFORM_DIR}/gui/components/compo_shape.c"
    "${PLATFORM_DIR}/gui/components/compo_stacklist.c"
    "${PLATFORM_DIR}/gui/components/compo_textbox.c"
    "${PLATFORM_DIR}/gui/components/compo_windmill.c"
    "${PLATFORM_DIR}/gui/components/component_func.c"
    "${PLATFORM_DIR}/gui/components/components.c"
    # platform/gui/ctp
    "${PLATFORM_DIR}/gui/ctp/axs152x/axs152x.c"
    "${PLATFORM_DIR}/gui/ctp/axs5106/axs5106.c"
    "${PLATFORM_DIR}/gui/ctp/ctp.c"
    "${PLATFORM_DIR}/gui/ctp/ctp_chsc6x.c"
    "${PLATFORM_DIR}/gui/ctp/ctp_cst8x.c"
    # platform/gui/tft
    "${PLATFORM_DIR}/gui/tft/oled_368_st7801n.c"
    "${PLATFORM_DIR}/gui/tft/oled_466_icna3310b.c"
    "${PLATFORM_DIR}/gui/tft/tft.c"
    "${PLATFORM_DIR}/gui/tft/tft_240_st7789_i80.c"
    "${PLATFORM_DIR}/gui/tft/tft_320_st77916.c"
    "${PLATFORM_DIR}/gui/tft/tft_480_st7283_srgb.c"
    "${PLATFORM_DIR}/gui/tft/tft_800_st7265_prgb.c"
    "${PLATFORM_DIR}/gui/tft/tft_drv.c"
    "${PLATFORM_DIR}/gui/tft/tft_vga012a_640.c"
    # platform/libs
    "${PLATFORM_DIR}/libs/strong_ble.c"
    "${PLATFORM_DIR}/libs/strong_bt.c"
    "${PLATFORM_DIR}/libs/strong_symbol.c"
    "${PLATFORM_DIR}/libs/libfixmath/fixmath.c"
)

# ---- 颜色输出 ------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

info()  { echo -e "${CYAN}[INFO]${NC}  $*"; }
ok()    { echo -e "${GREEN}[OK]${NC}    $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC}  $*"; }
err()   { echo -e "${RED}[ERR]${NC}   $*"; }

# ---- 检查工具链 ----------------------------------------------------------
check_toolchain() {
    if [ ! -f "$CC" ]; then
        err "找不到编译器: $CC"
        err "请确认 RISC-V V3 工具链已安装到:"
        err "  D:\\Program Files (x86)\\RV32-Toolchain\\RV32-V3\\"
        exit 1
    fi
    info "编译器: $CC"
    info "版本: $("$CC" --version 2>&1 | head -1)"
}

# ---- 清理 ----------------------------------------------------------------
do_clean() {
    info "清理编译产物..."
    rm -rf Output/obj/*.o
    rm -f Output/bin/app.rv32 Output/bin/app.bin Output/bin/app.dcf
    rm -f Output/bin/appxm.o Output/bin/download.bin
    rm -f Output/bin/map.txt
    ok "清理完成"
}

# ---- 预编译 (prebuild) ---------------------------------------------------
do_prebuild() {
    info "执行预编译步骤..."

    # 1. 运行 prebuild_asr.bat (ASR 语音资源配置)
    if [ -f "${BIN_DIR}/prebuild_asr.bat" ]; then
        info "  运行 prebuild_asr.bat ..."
        (cd "$BIN_DIR" && cmd.exe /c "prebuild_asr.bat") || {
            warn "  prebuild_asr.bat 返回非零，继续..."
        }
    fi

    # 2. xmaker 编译资源文件
    info "  xmaker -b ui.xm ..."
    (cd "$BIN_DIR" && "$XMAKER" -b ui.xm) || { err "ui.xm 失败"; exit 1; }

    info "  xmaker -b ui_external.xm ..."
    (cd "$BIN_DIR" && "$XMAKER" -b ui_external.xm) || { err "ui_external.xm 失败"; exit 1; }

    info "  xmaker -b res.xm ..."
    (cd "$BIN_DIR" && "$XMAKER" -b res.xm) || { err "res.xm 失败"; exit 1; }

    info "  xmaker -b xcfg.xm ..."
    (cd "$BIN_DIR" && "$XMAKER" -b xcfg.xm) || { err "xcfg.xm 失败"; exit 1; }

    # 3. 复制 ui_external.bin → app.xbf
    if [ -f "${BIN_DIR}/ui_external.bin" ]; then
        cp "${BIN_DIR}/ui_external.bin" "${BIN_DIR}/app.xbf"
        ok "  ui_external.bin → app.xbf"
    fi

    ok "预编译完成"
}

# ---- 编译单个源文件 ------------------------------------------------------
compile_one() {
    local src="$1"
    # 生成 .o 路径 (保持目录结构)
    local obj="${OBJ_DIR}/${src}.o"
    local obj_dir="$(dirname "$obj")"

    mkdir -p "$obj_dir"

    # 仅当源文件比目标文件新时才编译
    if [ -f "$obj" ] && [ "$src" -ot "$obj" ]; then
        return 0
    fi

    echo -e "  ${CYAN}CC${NC}    $src"
    "$CC" "${CFLAGS[@]}" "${INCLUDES[@]}" -o "$obj" "$src" || {
        err "编译失败: $src"
        return 1
    }
}

# ---- 编译所有源文件 ------------------------------------------------------
do_compile() {
    info "编译源文件 (共 ${#SOURCES[@]} 个)..."

    local count=0
    local failed=0

    for src in "${SOURCES[@]}"; do
        if [ ! -f "$src" ]; then
            warn "源文件不存在，跳过: $src"
            continue
        fi

        if compile_one "$src"; then
            count=$((count + 1))
        else
            failed=$((failed + 1))
        fi
    done

    ok "编译完成: ${count} 成功, ${failed} 失败"

    if [ "$failed" -gt 0 ]; then
        return 1
    fi
}

# ---- 预处理链接脚本 ------------------------------------------------------
do_linker_script() {
    info "预处理链接脚本 ram.ld → ${OBJ_DIR}/ram.o ..."

    "$CC" -E -P -x c "${INCLUDES[@]}" -c ram.ld -o "${OBJ_DIR}/ram.o" || {
        err "链接脚本预处理失败"
        return 1
    }
    ok "ram.o 生成完成"
}

# ---- 预处理 app.xm -------------------------------------------------------
do_appxm() {
    info "预处理 app.xm → ${BIN_DIR}/appxm.o ..."

    mkdir -p "$BIN_DIR"
    "$CC" -E -P -x c "${INCLUDES[@]}" -c "${BIN_DIR}/app.xm" -o "${BIN_DIR}/appxm.o" || {
        err "app.xm 预处理失败"
        return 1
    }
    ok "appxm.o 生成完成"
}

# ---- 链接 ----------------------------------------------------------------
do_link() {
    info "链接生成 app.rv32 ..."

    mkdir -p "$BIN_DIR"

    # 收集所有 .o 文件
    local obj_files=()
    for src in "${SOURCES[@]}"; do
        local obj="${OBJ_DIR}/${src}.o"
        if [ -f "$obj" ]; then
            obj_files+=("$obj")
        fi
    done

    info "  链接 ${#obj_files[@]} 个对象文件..."

    "$CC" "${LDFLAGS[@]}" "${LIB_PATHS[@]}" "${obj_files[@]}" "${LIBS[@]}" \
        -o "${BIN_DIR}/app.rv32" || {
        err "链接失败"
        return 1
    }

    ok "链接完成 → ${BIN_DIR}/app.rv32"
}

# ---- 后处理 (postbuild) --------------------------------------------------
do_postbuild() {
    info "执行后处理步骤..."

    cd "$BIN_DIR"

    # 1. 创建占位 .o 文件 (兼容 CodeBlocks 后处理流程)
    local proj_name="app"
    local proj_obj_dir="../../${OBJ_DIR}/projects/watch466"
    mkdir -p "$proj_obj_dir/Output/bin/ui/0gpu"
    echo "1" > "${proj_obj_dir}/ram.o"
    echo "1" > "${proj_obj_dir}/Output/bin/app.o"
    echo "1" > "${proj_obj_dir}/Output/bin/download.o"
    echo "1" > "${proj_obj_dir}/Output/bin/res.o"
    echo "1" > "${proj_obj_dir}/Output/bin/xcfg.o"
    echo "1" > "${proj_obj_dir}/Output/bin/ui/0gpu/gpu.o"

    # 2. objcopy 生成 .bin
    info "  objcopy → app.bin ..."
    "$OBJCOPY" -O binary "${proj_name}.rv32" "${proj_name}.bin" || {
        err "objcopy 失败"
        return 1
    }
    ok "  app.bin 生成完成"

    # 3. xmaker 生成 app.dcf (下载配置)
    info "  xmaker -b appxm.o → app.dcf ..."
    "$XMAKER" -b appxm.o || {
        warn "  xmaker appxm.o 失败 (如不需 DCF 可忽略)"
    }

    # 4. 可选: 上传到设备
    if [ -f "C:/upload/upload.bat" ]; then
        info "  调用 upload.bat ..."
        cmd.exe /c "C:\\upload\\upload.bat -D AB5790 ${proj_name}.dcf" || true
    fi

    # 5. xmaker 生成 download.bin
    info "  xmaker -b download.xm ..."
    "$XMAKER" -b download.xm || {
        warn "  xmaker download.xm 失败 (如不需 download 可忽略)"
    }

    cd "$SCRIPT_DIR"
    ok "后处理完成"
}

# ---- 主流程 --------------------------------------------------------------
main() {
    echo ""
    echo -e "${GREEN}╔══════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║   watch466 RISC-V V3 编译 (VS Code)         ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════╝${NC}"
    echo ""

    check_toolchain

    case "${1:-build}" in
        clean)
            do_clean
            ;;
        rebuild)
            do_clean
            do_prebuild
            do_linker_script
            do_compile
            do_appxm
            do_link
            do_postbuild
            ;;
        build|*)
            mkdir -p "$OBJ_DIR" "$BIN_DIR"
            do_prebuild
            do_linker_script
            do_compile
            do_appxm
            do_link
            do_postbuild
            ;;
    esac

    echo ""
    echo -e "${GREEN}╔══════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║   ✅ 构建完成!                              ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "  输出文件:"
    echo -e "    ${BIN_DIR}/app.rv32   (ELF)"
    echo -e "    ${BIN_DIR}/app.bin    (BIN)"
    echo -e "    ${BIN_DIR}/app.dcf    (下载配置)"
    echo -e "    ${BIN_DIR}/map.txt    (符号映射)"
    echo ""
}

main "$@"
