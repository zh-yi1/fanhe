#!/usr/bin/env bash
# 饭盒蓝牙协议 — 帧解析器
# 用法: parse 55 aa 00 00 0e 00 00 01 01 0f
#       parse "55 aa 00 00 0e 00 00 01 01 0f"
#       parse 55aa00000e000001010f

# ============================================================
# 工具函数
# ============================================================

h2d() { printf "%d" "0x$1" 2>/dev/null; }
d2h() { printf "%02X" "$1"; }

# 获取数组中的字节
get_byte() { echo "${bytes[$1]}"; }

# 取范围 bytes[from..to]（含两端）
get_range() {
    local from=$1 to=$2 out=""
    for ((i=from; i<=to; i++)); do out+="${bytes[$i]} "; done
    echo "${out% }"
}

# 大端多字节 → 整数值
be2int() {
    local from=$1 len=$2 val=0
    for ((i=0; i<len; i++)); do
        val=$(( (val << 8) | 0x$(get_byte $((from+i))) ))
    done
    echo "$val"
}

# 十六进制字符串 → 空格分隔
normalize() {
    local s="$1"
    s="${s// /}"
    s="${s//0x/}"; s="${s//0X/}"
    s="${s,,}"
    echo "$s" | sed 's/\(..\)/\1 /g'
}

# ============================================================
# 常量
# ============================================================

ERR_NONE=0x00
TYPE_BOOL=0x01
TYPE_VALUE=0x02
TYPE_ENUM=0x04

DPID_MASTER=1;   DPID_MODE=2;      DPID_BATTERY=3
DPID_CHARGE=4;   DPID_DURATION=5;  DPID_REMAIN=6
DPID_TEMP=7;     DPID_LANGUAGE=8;  DPID_FAULT=9

# ============================================================
# 含义解析
# ============================================================

cmd_name() {
    case $1 in
        01) echo "查询产品信息" ;;
        02) echo "查询设备动态属性" ;;
        03) echo "状态上报" ;;
        04) echo "查询预约列表" ;;
        05) echo "新增预约" ;;
        06) echo "修改预约" ;;
        07) echo "删除预约" ;;
        08) echo "获取指定模式信息" ;;
        09) echo "修改指定模式信息" ;;
        0A|0a) echo "升级查询" ;;
        0B|0b) echo "升级启动" ;;
        0C|0c) echo "升级包传输" ;;
        0D|0d) echo "升级结束" ;;
        0E|0e) echo "立即加热/停止加热" ;;
        *) echo "未知命令" ;;
    esac
}

color_name() {
    case $1 in
        00) echo "白色" ;; 01) echo "黑色" ;; 02) echo "红色" ;;
        03) echo "蓝色" ;; 04) echo "绿色" ;; 05) echo "金色" ;;
        *) echo "未知" ;;
    esac
}

temp_celsius() {
    case $1 in
        00) echo "40°C" ;; 01) echo "50°C" ;; 02) echo "60°C" ;;
        03) echo "70°C" ;; 04) echo "80°C" ;; 05) echo "90°C" ;;
        *) echo "$(( 90 + ($1-5)*5 ))°C" ;;
    esac
}

mode_name() {
    case $1 in
        01) echo "自定义加热" ;; 02) echo "鸡腿模式" ;;
        03) echo "意面模式" ;; 04) echo "预约模式" ;;
        05) echo "保温模式" ;; *) echo "未知" ;;
    esac
}

battery_name() {
    case $1 in
        01) echo "低电量(<25%)" ;; 02) echo "中电量(25%-50%)" ;;
        03) echo "高电量(50%-75%)" ;; 04) echo "满电量(≥75%)" ;;
        *) echo "未知" ;;
    esac
}

charge_name() {
    case $1 in
        00) echo "未充电" ;; 01) echo "充电中" ;;
        02) echo "已充满" ;; *) echo "未知" ;;
    esac
}

lang_name() {
    case $1 in
        00) echo "中文" ;; 01) echo "英文" ;; 02) echo "德语" ;;
        03) echo "法语" ;; 04) echo "西班牙语" ;; 05) echo "意大利语" ;;
        06) echo "日语" ;; 07) echo "俄语" ;; *) echo "未知" ;;
    esac
}

repeat_days() {
    local mask=$(h2d "$1") out=""
    local days=("周日" "周一" "周二" "周三" "周四" "周五" "周六")
    for i in {0..6}; do
        if (( mask & (1<<i) )); then out+="${days[$i]},"; fi
    done
    echo "${out%,}"  # 去掉末尾逗号
}

ota_query_status() {
    case $1 in
        00) echo "不支持MCU升级" ;; 01) echo "MCU未就绪" ;;
        02) echo "支持升级" ;; *) echo "未知" ;;
    esac
}

ota_start_status() {
    case $1 in
        00) echo "收到升级指令" ;; 01) echo "MCU擦除flash中" ;;
        02) echo "擦除完成，可以传输升级包" ;; *) echo "未知" ;;
    esac
}

# ============================================================
# 帧头解析
# ============================================================

print_frame_header() {
    local b=("${bytes[@]}")
    local header=$(be2int 0 2)
    local ver=$(be2int 2 1)
    local msgf=$(be2int 3 1)
    local cmd=$(be2int 4 1)
    local err=$(be2int 5 1)
    local dlen=$(be2int 6 2)
    local data_start=8
    local chk_pos=$((8 + dlen))
    local calc_chk=0
    for ((i=0; i<chk_pos; i++)); do
        calc_chk=$(( (calc_chk + 0x${b[$i]}) % 256 ))
    done
    local got_chk=$(h2d "${b[$chk_pos]}")
    local chk_ok="正确"
    (( calc_chk != got_chk )) && chk_ok="错误(应为$(d2h $calc_chk))"

    echo "========== 帧结构解析 =========="
    echo "帧头:       0x$(d2h $header)"
    echo "版本:       0x$(d2h $ver)"
    echo "消息标志:   0x$(d2h $msgf)"
    echo "命令字:     0x$(d2h $cmd) ($(cmd_name $(d2h $cmd)))"
    echo "错误标志:   0x$(d2h $err) $([ $(d2h $err) = "00" ] && echo '(正常)' || echo '(异常)')"
    echo "数据长度:   $dlen (0x$(d2h $dlen) (0x$(d2h $((dlen>>8))) $(d2h $((dlen&0xFF)))))"
    echo "校验和:     0x$(d2h $got_chk) ($chk_ok)"
    if (( dlen > 0 )); then
        echo -n "数据($dlen字节): "
        for ((i=data_start; i<chk_pos; i++)); do echo -n "${b[$i]} "; done
        echo
    else
        echo "数据:       无"
    fi
    echo "================================="
}

# ============================================================
# 业务数据解析
# ============================================================

parse_datapoint() {
    local off=$1
    local dpid=$(h2d "${bytes[$off]}")
    local type=$(h2d "${bytes[$((off+1))]}")
    local vlen=$(be2int $((off+2)) 2)
    local voff=$((off+4))
    local end=$((voff + vlen))

    echo -n "  dpid=$dpid  type=$type  len=$vlen  value="

    local val_hex=""
    for ((i=voff; i<end; i++)); do val_hex+="${bytes[$i]} "; done

    # 值解析
    case $type in
        $TYPE_BOOL) echo -n "$(h2d "${bytes[$voff]}" | xargs -I{} bash -c '[ {} -eq 1 ] && echo 开启 || echo 关闭')" ;;
        $TYPE_VALUE)
            local v=$(be2int $voff 4)
            echo -n "$v"
            ;;
        $TYPE_ENUM)
            echo -n "$(h2d "${bytes[$voff]}")"
            ;;
    esac

    # 附加含义
    case $dpid in
        $DPID_MASTER)   echo -n " [总开关]" ;;
        $DPID_MODE)     echo -n " [加热模式: $(mode_name "${bytes[$voff]}")]" ;;
        $DPID_BATTERY)  echo -n " [电量: $(battery_name "${bytes[$voff]}")]" ;;
        $DPID_CHARGE)   echo -n " [充电: $(charge_name "${bytes[$voff]}")]" ;;
        $DPID_DURATION) echo -n " [加热时长: $(h2d "${bytes[$voff]}")分钟]" ;;
        $DPID_REMAIN)   echo -n " [剩余时间: $(h2d "${bytes[$voff]}")分钟]" ;;
        $DPID_TEMP)     echo -n " [温度: $(temp_celsius "${bytes[$voff]}")]" ;;
        $DPID_LANGUAGE) echo -n " [语言: $(lang_name "${bytes[$voff]}")]" ;;
        $DPID_FAULT)    echo -n " [故障: $([ $(h2d "${bytes[$voff]}") -eq 1 ] && echo '高温告警' || echo '正常')]" ;;
    esac
    echo "  [$val_hex]"

    echo $end  # 返回下一个 offset
}

parse_datapoints() {
    local dlen=$1 off=0
    echo "--- 动态属性列表 ---"
    while (( off + 4 <= dlen )); do
        local next=$(parse_datapoint $((8 + off)))
        local vlen=$(be2int $((8+off+2)) 2)
        off=$((off + 4 + vlen))
    done
}

parse_product_info() {
    local dlen=$1
    if (( dlen == 4 )); then
        # APP 发送: 时间戳
        local ts=$(be2int 8 4)
        local dt
        dt=$(date -d "@$ts" "+%Y-%m-%d %H:%M:%S" 2>/dev/null || echo "时间戳=$ts秒")
        echo "--- 时间戳 ---"
        echo "时间: $dt ($ts 秒)"
    elif (( dlen == 73 )); then
        # MCU 返回: 产品信息
        echo "--- 产品信息 ---"
        local bt_name=""; for ((i=8; i<24; i++)); do
            local ch=$(h2d "${bytes[$i]}"); [ "$ch" != "0" ] && bt_name+=$(printf "\\x$(d2h $ch)")
        done
        echo "蓝牙名称: $bt_name"

        local ver=""; for ((i=24; i<32; i++)); do
            local ch=$(h2d "${bytes[$i]}"); [ "$ch" != "0" ] && ver+=$(printf "\\x$(d2h $ch)")
        done
        echo "版本号:   $ver"

        local model=""; for ((i=32; i<42; i++)); do
            local ch=$(h2d "${bytes[$i]}"); [ "$ch" != "0" ] && model+=$(printf "\\x$(d2h $ch)")
        done
        echo "型号:     $model"

        echo -n "MAC地址:  "
        printf "%02X:%02X:%02X:%02X:%02X:%02X\n" \
            $(h2d "${bytes[42]}") $(h2d "${bytes[43]}") $(h2d "${bytes[44]}") \
            $(h2d "${bytes[45]}") $(h2d "${bytes[46]}") $(h2d "${bytes[47]}")

        local sn=""; for ((i=48; i<80; i++)); do
            local ch=$(h2d "${bytes[$i]}"); [ "$ch" != "0" ] && sn+=$(printf "\\x$(d2h $ch)")
        done
        echo "SN号:     $sn"
        echo "颜色:     $(color_name "${bytes[80]}") (0x${bytes[80]})"
    else
        echo "产品信息数据长度异常: $dlen 字节"
    fi
}

parse_reservation() {
    local dlen=$1
    if (( dlen == 43 )); then
        # MCU 返回 (0x04)
        echo "--- 预约信息(应答) ---"
        local total=$(h2d "${bytes[8]}")
        local idx=$(h2d "${bytes[9]}")
        local id=$(h2d "${bytes[10]}")
        echo "总条数: $total  当前序号: $idx  预约ID: $id"

        local name=""; for ((i=11; i<43; i++)); do
            local ch=$(h2d "${bytes[$i]}"); [ "$ch" != "0" ] && name+=$(printf "\\x$(d2h $ch)")
        done
        echo "名称:     $name"

        local t=$(be2int 43 4)
        local dt=$(date -d "@$t" "+%H:%M:%S" 2>/dev/null || echo "${t}秒")
        echo "结束时间: $dt"
        echo "温度:     $(temp_celsius "${bytes[47]}")"
        echo "时长:     $(h2d "${bytes[48]}") 分钟"
        echo "启用:     $([ $(h2d "${bytes[49]}") -eq 1 ] && echo '开启' || echo '关闭')"
        echo "重复周期: $(repeat_days "${bytes[50]}") (0x${bytes[50]})"
    elif (( dlen == 41 )); then
        # APP 发送 (0x05/0x06)
        echo "--- 预约信息(请求) ---"
        local id=$(h2d "${bytes[8]}")
        echo "预约ID:   $id $([ "$id" = "0" ] && echo '(MCU自动分配)')"

        local name=""; for ((i=9; i<41; i++)); do
            local ch=$(h2d "${bytes[$i]}"); [ "$ch" != "0" ] && name+=$(printf "\\x$(d2h $ch)")
        done
        echo "名称:     $name"

        local t=$(be2int 41 4)
        local dt=$(date -d "@$t" "+%H:%M:%S" 2>/dev/null || echo "${t}秒")
        echo "结束时间: $dt"
        echo "温度:     $(temp_celsius "${bytes[45]}")"
        echo "时长:     $(h2d "${bytes[46]}") 分钟"
        echo "启用:     $([ $(h2d "${bytes[47]}") -eq 1 ] && echo '开启' || echo '关闭')"
        echo "重复周期: $(repeat_days "${bytes[48]}") (0x${bytes[48]})"
    fi
}

parse_mode_info() {
    local dlen=$1 count=$((dlen/3))
    echo "--- 模式信息 (共${count}条) ---"
    for ((i=0; i<count; i++)); do
        local off=$((8 + i*3))
        local mode="${bytes[$off]}"
        local temp="${bytes[$((off+1))]}"
        local dur=$(h2d "${bytes[$((off+2))]}")
        echo "  [$((i+1))] 模式=$(mode_name "$mode")  温度=$(temp_celsius "$temp")  时长=${dur}分钟"
    done
}

parse_mode_modify() {
    # 0x09: 3 字节
    local mode="${bytes[8]}"
    local temp="${bytes[9]}"
    local dur=$(h2d "${bytes[10]}")
    echo "--- 修改模式 ---"
    echo "模式: $(mode_name "$mode")"
    echo "温度: $(temp_celsius "$temp")"
    echo "时长: ${dur} 分钟"
}

parse_ota() {
    local dlen=$1
    case $dlen in
        1)
            local v=$(h2d "${bytes[8]}")
            echo "$(ota_query_status "${bytes[8]}") (0x${bytes[8]})"
            ;;
        4)  # 固件包大小
            local sz=$(be2int 8 4)
            echo "固件包字节数: $sz (0x$(d2h $((sz>>24))) $(d2h $((sz>>16))) $(d2h $((sz>>8))) $(d2h $sz))"
            ;;
        *)  # 升级数据包
            local offset=$(be2int 8 4)
            local pld=$((dlen - 4))
            echo "包偏移: $offset  数据长度: $pld 字节"
            if (( pld > 0 )); then
                echo -n "数据(前16): "
                local max=$((8+4+(pld<16?pld:16)))
                for ((i=12; i<max; i++)); do echo -n "${bytes[$i]} "; done
                [ $pld -gt 16 ] && echo -n "..."
                echo
            fi
            ;;
    esac
}

# ============================================================
# 主解析
# ============================================================

parse_frame() {
    local input="$*"
    if [ -z "$input" ]; then
        echo "用法: parse <hex帧 含校验和>"
        echo "例:   parse 55 aa 00 00 0e 00 00 01 01 0f"
        echo "      parse 55aa00000e000001010f"
        echo "      parse \"55 aa 00 00 0e 00 00 01 01 0f\""
        return 1
    fi

    # 规范化
    local norm=$(normalize "$input")
    bytes=($norm)
    local count=${#bytes[@]}

    if [ $count -lt 9 ]; then
        echo "错误: 帧至少 9 字节（含校验和）"
        return 1
    fi

    # 检查帧头
    if [ "${bytes[0]}" != "55" ] || [ "${bytes[1]}" != "aa" ]; then
        echo "错误: 无效帧头 ${bytes[0]} ${bytes[1]}, 期望 55 AA"
        return 1
    fi

    # 数据长度
    local dlen=$(be2int 6 2)
    local total=$((9 + dlen))
    if [ $count -lt $total ]; then
        echo "错误: 帧不完整，期望 $total 字节，实际 $count 字节"
        return 1
    fi
    if [ $count -gt $total ]; then
        echo "注意: 帧超出期望长度，多余 ${count}/$total 字节将被忽略"
    fi

    # 校验
    local calc_chk=0
    for ((i=0; i<total-1; i++)); do
        calc_chk=$(( (calc_chk + 0x${bytes[$i]}) % 256 ))
    done
    local got_chk=$(h2d "${bytes[$((total-1))]}")
    local cmd_hex="${bytes[4]}"

    # 打印帧头
    echo
    echo "==================== 饭盒协议帧解析 ===================="
    echo -n "原始帧: "
    for ((i=0; i<total; i++)); do echo -n "${bytes[$i]} "; done
    echo
    echo "----------------------------------------------------------"
    echo "帧头:     0x55AA"
    echo "版本:     0x${bytes[2]}"
    echo "消息标志: 0x${bytes[3]}"
    echo "命令字:   0x${cmd_hex} ($(cmd_name "$cmd_hex"))"
    echo "错误标志: 0x${bytes[5]} $([ "${bytes[5]}" = "00" ] && echo '(正常)' || echo '(异常)')"
    echo "数据长度: $dlen"

    # 校验和
    local chk_status="✓ 正确"
    [ $calc_chk -ne $got_chk ] && chk_status="✗ 错误 (应为 $(d2h $calc_chk))"
    echo "校验和:   0x$(d2h $got_chk)  累加和=0x$(d2h $(( (calc_chk - got_chk + 256) % 256 )))...  $chk_status"

    echo

    # 业务数据
    if [ $dlen -gt 0 ]; then
        echo "==================== 业务数据解析 ===================="
        local cmd=$(h2d "$cmd_hex")
        case $cmd in
            1)   parse_product_info $dlen ;;
            2|3) parse_datapoints $dlen ;;
            4)   parse_reservation $dlen ;;
            5|6) parse_reservation $dlen ;;
            7)
                echo "--- 删除预约 ---"
                echo "预约ID: $(h2d "${bytes[8]}")"
                ;;
            8)   parse_mode_info $dlen ;;
            9)   parse_mode_modify $dlen ;;
            10|11)
                echo "--- 升级状态 ---"
                parse_ota $dlen
                ;;
            12)
                echo "--- 升级数据传输 ---"
                parse_ota $dlen
                ;;
            13)
                echo "--- 升级结束 ---"
                local result=$(h2d "${bytes[8]}")
                echo "升级结果: $([ $result -eq 1 ] && echo '成功' || echo '失败') (0x${bytes[8]})"
                ;;
            14)
                echo "--- 立即加热控制 ---"
                local action=$(h2d "${bytes[8]}")
                echo "指令: $([ $action -eq 1 ] && echo '🔥 开启加热' || echo '🛑 停止加热') (0x${bytes[8]})"
                ;;
            *)   echo "未知命令字 0x$(d2h $cmd)" ;;
        esac
    else
        echo "(无业务数据)"
    fi
    echo "========================================================"
}

parse_frame "$@"
