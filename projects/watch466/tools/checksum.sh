#!/usr/bin/env bash
# 饭盒蓝牙协议 — 校验和计算工具 (bash版)
# 用法:
#   单行:  crc 55aa00000e00000101
#   多行:  crc << 'EOF'
#          55 aa 00 00 ...
#          01 00 03 04 ...
#          EOF
#   粘贴:  crc (回车, 粘贴, Ctrl+D)
#   管道:  echo "55aa..." | crc

normalize() {
    local s="$1"
    s="${s// /}"; s="${s//$'\n'/}"; s="${s//$'\r'/}"; s="${s//$'\t'/}"
    s="${s//0x/}"; s="${s//0X/}"; s="${s,,}"
    echo "$s" | sed 's/\(..\)/\1 /g'
}
h2d() { printf "%d" "0x$1" 2>/dev/null; }
d2h() { printf "%02X" "$1"; }

do_crc() {
    local input="$*"
    if [ -z "$input" ]; then
        echo "错误: 未提供 hex 数据"
        return 1
    fi
    local norm=$(normalize "$input")
    local bytes=($norm)
    local count=${#bytes[@]}
    if [ $count -lt 8 ]; then
        echo "错误: 帧至少 8 字节，当前 $count 字节"
        return 1
    fi
    local sum=0
    for b in "${bytes[@]}"; do sum=$(( sum + $(h2d "$b") )); done
    local chk=$(( sum % 256 ))
    echo "=============================="
    echo "输入(${count}字节): ${bytes[*]^^}"
    echo "累加和: ${sum} (0x$(d2h $sum))"
    echo "校验和: 0x$(d2h $chk) (${chk})"
    echo "------------------------------"
    echo "完整帧: ${bytes[*]^^} $(d2h $chk)"
    echo "=============================="
}

print_usage() {
    echo "crc — 饭盒协议校验和计算"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "单行:  crc 55aa00000e00000101"
    echo "多行:  crc << 'EOF'"
    echo "       55 aa 00 00 02 00 00 56 ..."
    echo "       01 00 03 04 ..."
    echo "       EOF"
    echo "粘贴:  crc (回车→粘贴→Ctrl+D)"
    echo "管道:  echo '55aa...' | crc"
}

main() {
    local input=""

    if [ $# -gt 0 ]; then
        # 有命令行参数 → 直接处理
        input="$*"
    elif [ ! -t 0 ]; then
        # 管道/重定向 → 读 stdin
        input=$(cat)
    else
        # 终端交互模式 → 提示用户输入
        print_usage
        echo ""
        echo "↓↓↓ 请在下方粘贴 hex 数据，然后按 Ctrl+D ↓↓↓"
        input=$(cat)
    fi

    input="${input#"${input%%[![:space:]]*}"}"
    input="${input%"${input##*[![:space:]]}"}"

    if [ -z "$input" ]; then
        print_usage
        exit 1
    fi

    # 去可能的子命令前缀
    local first="${input%% *}"
    case "$first" in crc|frame) input="${input#* }" ;; esac

    do_crc "$input"
}

main "$@"
