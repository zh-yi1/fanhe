#!/usr/bin/env bash
# 饭盒协议 — 帧自动解析工具
# 用法:
#   frame 55AA0000040000170204000103050200040000003C07040001050A0100010189
#   frame "55 AA 00 00 04 00 00 17 02 04 00 01 03 ..."
#   frame <<< "55 AA 00 00 ..."

set -euo pipefail

# ============================================================
# 属性数据
# ============================================================

declare -A DPID_NAME=(
    [1]="总开关"    [2]="加热模式"  [3]="电量"
    [4]="充电状态"  [5]="加热时长"  [6]="剩余时间"
    [7]="加热温度"  [8]="语言"      [9]="故障提示"
    [10]="是否加热" [11]="时间戳"
)

declare -A DP_TYPE_NAME=(
    ["01"]="bool"  ["02"]="value"  ["04"]="enum"
)

declare -A MODE_NAME=(
    [0]="关闭" [1]="自定义加热" [2]="鸡腿模式" [3]="意面模式" [4]="预约模式" [5]="保温模式"
)

declare -A TEMP_NAME=(
    [0]="40°C" [1]="50°C" [2]="60°C" [3]="70°C" [4]="80°C" [5]="90°C"
)

declare -A BATTERY_NAME=(
    [1]="低(<25%)" [2]="中(25-50%)" [3]="高(50-75%)" [4]="满(>75%)"
)

declare -A CHARGE_NAME=(
    [0]="未充电" [1]="充电中" [2]="已充满"
)

declare -A BOOL_NAME=(
    [0]="关/否" [1]="开/是"
)

declare -A FAULT_NAME=(
    [0]="正常" [1]="高温告警"
)

declare -A SLEEP_NAME=(
    [0]="MCU可休眠" [1]="MCU使能开机"
)

# ============================================================
# 工具函数
# ============================================================

h2d() { printf "%d" "0x$1" 2>/dev/null; }
d2h() { printf "%02X" "$1"; }

norm() {
    local s="$*"
    s="${s// /}"; s="${s//$'\n'/}"; s="${s//$'\r'/}"; s="${s//$'\t'/}"
    s="${s//0x/}"; s="${s//0X/}"; s="${s//,/}"
    echo "$s" | tr '[:lower:]' '[:upper:]' | sed 's/\(..\)/\1 /g'
}

bytes_to_int() {
    local hex="$*"
    hex="${hex// /}"
    printf "%d" "0x$hex" 2>/dev/null
}

# ============================================================
# DataPoint 解析
# ============================================================

DP_CONSUMED=0    # 全局: parse_datapoints 消耗的字节数

parse_datapoints() {
    local data=("$@")
    local total=${#data[@]}
    local pos=0
    local idx=1
    DP_CONSUMED=0

    while [ $pos -lt $total ]; do
        [ $((pos + 4)) -gt $total ] && break

        local dpid=${data[$pos]}
        local dtype=${data[$((pos + 1))]}
        local dlen_h=${data[$((pos + 2))]}
        local dlen_l=${data[$((pos + 3))]}
        local dlen=$(( ( $(h2d "$dlen_h") << 8 ) | $(h2d "$dlen_l") ))

        # 合法检查
        local dpid_d=$(h2d "$dpid")
        [ "$dpid_d" -lt 1 ] 2>/dev/null && break
        [ "$dpid_d" -gt 11 ] 2>/dev/null && break
        # type 合法性
        [ "$dtype" != "01" ] && [ "$dtype" != "02" ] && [ "$dtype" != "04" ] && break
        # len 与 type 匹配
        [ "$dtype" = "01" ] && [ "$dlen" -ne 1 ] && break
        [ "$dtype" = "04" ] && [ "$dlen" -ne 1 ] && break
        [ "$dtype" = "02" ] && [ "$dlen" -ne 4 ] && break

        [ $((pos + 4 + dlen)) -gt $total ] && break

        # 读 value
        local value=""
        for ((i = 0; i < dlen; i++)); do
            value+="${data[$((pos + 4 + i))]}"
        done

        # 解析 value
        local val_str=""
        local dpid_name="${DPID_NAME[$dpid_d]:-未知($dpid)}"
        local type_name="${DP_TYPE_NAME[$dtype]:-$dtype}"

        case "$dtype" in
            "01") # bool
                local v=$(h2d "${data[$((pos + 4))]}")
                if [ "$dpid_d" -eq 1 ]; then
                    val_str="${BOOL_NAME[$v]:-$v}"
                elif [ "$dpid_d" -eq 10 ]; then
                    val_str="${BOOL_NAME[$v]:-$v}"
                else
                    val_str="${BOOL_NAME[$v]:-$v} (0x${data[$((pos + 4))]})"
                fi
                ;;
            "04") # enum
                local v=$(h2d "${data[$((pos + 4))]}")
                case $dpid_d in
                    2) val_str="${MODE_NAME[$v]:-未知} (档位$v)" ;;
                    3) val_str="${BATTERY_NAME[$v]:-未知} (档位$v)" ;;
                    4) val_str="${CHARGE_NAME[$v]:-未知} (档位$v)" ;;
                    7) val_str="${TEMP_NAME[$v]:-未知} (档位$v)" ;;
                    9) val_str="${FAULT_NAME[$v]:-未知} (档位$v)" ;;
                    *) val_str="档位$v" ;;
                esac
                ;;
            "02") # value
                local n=$(bytes_to_int "$value")
                case $dpid_d in
                    5) val_str="$n 分钟" ;;
                    6) val_str="$n 分钟" ;;
                    11) val_str="Unix=$n" ;;
                    *) val_str="$n" ;;
                esac
                ;;
            *) val_str="0x$value" ;;
        esac

        printf "  %-30s ← DP%d: %s\n" \
            "$dpid $dtype $dlen_h $dlen_l $value" \
            "$idx" "$dpid_name=$val_str"

        pos=$((pos + 4 + dlen))
        idx=$((idx + 1))
    done

    DP_CONSUMED=$pos
}

# ============================================================
# 主解析
# ============================================================

parse_frame() {
    local raw_bytes=("$@")
    local count=${#raw_bytes[@]}

    # 最小帧检查
    if [ $count -lt 9 ]; then
        echo "❌ 数据太短: 至少 9 字节，当前 $count 字节"
        return 1
    fi

    local h0=${raw_bytes[0]}
    local h1=${raw_bytes[1]}

    if [ "$h0" != "55" ] || [ "$h1" != "AA" ]; then
        echo "❌ 帧头错误: 期望 55 AA, 实际 $h0 $h1"
        return 1
    fi

    local ver=${raw_bytes[2]}
    local msg=${raw_bytes[3]}
    local cmd=${raw_bytes[4]}
    local err=${raw_bytes[5]}
    local dlen_h=${raw_bytes[6]}
    local dlen_l=${raw_bytes[7]}
    local dlen=$(( ( $(h2d "$dlen_h") << 8 ) | $(h2d "$dlen_l") ))

    local expect=$((9 + dlen))
    if [ $count -lt $expect ]; then
        echo "❌ 数据不完整: 期望 $expect 字节, 实际 $count"
        return 1
    fi

    # 提取 payload
    local payload=()
    if [ $dlen -gt 0 ]; then
        for ((i = 0; i < dlen; i++)); do
            payload+=("${raw_bytes[$((8 + i))]}")
        done
    fi

    # 校验和
    local sum=0
    for ((i = 0; i < 8 + dlen; i++)); do
        sum=$((sum + $(h2d "${raw_bytes[$i]}")))
    done
    local calc_chk=$((sum % 256))

    local rcv_chk_hex="${raw_bytes[$((8 + dlen))]:-??}"
    local rcv_chk=$(h2d "$rcv_chk_hex" 2>/dev/null || echo -1)

    # ── 输出 ──
    echo "╔══════════════════════════════════════════════════╗"
    echo "║          饭盒协议帧自动解析                      ║"
    echo "╠══════════════════════════════════════════════════╣"
    printf "║ 帧头      : 55 AA                                 ║\n"
    printf "║ 版本      : %s                                     ║\n" "$ver"
    printf "║ 消息标志  : %s                                     ║\n" "$msg"
    printf "║ 命令字    : %s                                     ║\n" "$cmd"
    printf "║ 错误标志  : %s                                     ║\n" "$err"
    printf "║ 数据长度  : %d (0x%s%s)                            ║\n" "$dlen" "$dlen_h" "$dlen_l"

    # 校验
    if [ "$rcv_chk" != "-1" ]; then
        if [ "$calc_chk" -eq "$rcv_chk" ]; then
            printf "║ 校验和    : 计算=0x%02X 收到=0x%s → ✅ 通过       ║\n" "$calc_chk" "$rcv_chk_hex"
        else
            printf "║ 校验和    : 计算=0x%02X 收到=0x%s → ❌ 不匹配     ║\n" "$calc_chk" "$rcv_chk_hex"
            printf "║            累加和=%d (0x%04X)                     ║\n" "$sum" "$sum"
        fi
    fi

    echo "╠══════════════════════════════════════════════════╣"

    if [ $dlen -eq 0 ]; then
        echo "║ 数据区    : 无                                   ║"
    else
        # 优先尝试纯 DataPoints 解析(静默探测)
        parse_datapoints "${payload[@]}" > /dev/null
        local dp_consumed=$DP_CONSUMED

        if [ "$dp_consumed" -eq "$dlen" ] && [ "$dlen" -gt 0 ]; then
            # 纯 DataPoints 解析成功
            echo "║ DataPoints:                                      ║"
            parse_datapoints "${payload[@]}"
        elif [ $dlen -ge 5 ]; then
            # DataPoints 解析失败, 尝试 timestamp+sleep_flag+DPs 格式
            local sf=$(h2d "${payload[4]}" 2>/dev/null || echo 255)
            if [ "$sf" = "0" ] || [ "$sf" = "1" ]; then
                local ts="${payload[0]}${payload[1]}${payload[2]}${payload[3]}"
                local ts_n=$(bytes_to_int "$ts")
                local sf_v="$sf"
                echo "║ [timestamp+sleep_flag+DataPoints 格式]          ║"
                printf "║ 时间戳    : 0x%s (%d)     ║\n" "$ts" "$ts_n"
                printf "║ sleep_flag: %s → %s                               ║\n" "${payload[4]}" "${SLEEP_NAME[$sf_v]:-未知}"
                echo "╠══════════════════════════════════════════════════╣"

                local dp_data=("${payload[@]:5}")
                if [ ${#dp_data[@]} -gt 0 ]; then
                    echo "║ DataPoints:                                      ║"
                    DP_CONSUMED=0
                    parse_datapoints "${dp_data[@]}"
                fi
            else
                echo "║ [无法解析为DataPoints]                          ║"
                printf "║ 原始数据: %s\n" "$(echo "${payload[*]}" | tr '[:lower:]' '[:upper:]')"
            fi
        else
            echo "║ [无法解析]                                       ║"
        fi
    fi

    echo "╚══════════════════════════════════════════════════╝"

    local full=""
    for ((i = 0; i < 8 + dlen; i++)); do
        full+="${raw_bytes[$i]} "
    done
    echo "完整帧: $full$(d2h $calc_chk)"
}


# ============================================================
# 入口
# ============================================================

main() {
    local input=""

    if [ $# -gt 0 ]; then
        input="$*"
    elif [ ! -t 0 ]; then
        input=$(cat)
    else
        echo "frame — 饭盒协议帧自动解析"
        echo "━━━━━━━━━━━━━━━━━━━━━━━━━"
        echo "单行:  frame 55AA0000040000170204..."
        echo "空格:  frame '55 AA 00 00 04 00 ...'"
        echo "管道:  echo '55 AA ...' | frame"
        echo "粘贴:  frame (回车→粘贴→Ctrl+D)"
        echo ""
        echo "↓↓↓ 粘贴 hex 数据，然后按 Ctrl+D ↓↓↓"
        input=$(cat)
    fi

    # 清洗
    input="${input#"${input%%[![:space:]]*}"}"
    input="${input%"${input##*[![:space:]]}"}"

    if [ -z "$input" ]; then
        echo "❌ 未输入数据"
        exit 1
    fi

    # 去掉可能的 "frame" 前缀
    local first="${input%% *}"
    case "$first" in frame|parse) input="${input#* }" ;; esac

    local norm=$(norm "$input")
    local bytes=($norm)

    parse_frame "${bytes[@]}"
}

main "$@"
