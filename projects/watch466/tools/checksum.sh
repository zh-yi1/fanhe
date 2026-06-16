#!/usr/bin/env bash
# 饭盒蓝牙协议 — 校验和计算工具 (bash版)
# 用法: ./checksum.sh 55aa00000e00000101
#       ./checksum.sh "55 aa 00 00 0e 00 00 01 01"

set -e

# 纯 hex 字符串 → 空格分隔的 hex 对
normalize() {
    local s="$1"
    s="${s// /}"                         # 去空格
    s="${s//0x/}"                        # 去 0x
    s="${s//0X/}"                        # 去 0X
    s="${s,,}"                           # 转小写
    echo "$s" | sed 's/\(..\)/\1 /g'     # 两两分组加空格
}

# hex → dec
h2d() { printf "%d" "0x$1" 2>/dev/null; }

# dec → hex (两字节大写)
d2h() { printf "%02X" "$1"; }

# 主逻辑
main() {
    local input="$*"
    if [ -z "$input" ]; then
        echo "用法: ./checksum.sh <hex帧不含校验和>"
        echo "例:   ./checksum.sh 55aa00000e00000101"
        echo "      ./checksum.sh \"55 aa 00 00 0e 00 00 01 01\""
        exit 1
    fi

    local norm
    norm=$(normalize "$input")

    # 拆成数组
    local bytes=($norm)
    local count=${#bytes[@]}

    if [ $count -lt 8 ]; then
        echo "错误: 帧至少 8 字节"
        exit 1
    fi

    # 累加
    local sum=0
    for b in "${bytes[@]}"; do
        sum=$(( sum + $(h2d "$b") ))
    done

    local chk=$(( sum % 256 ))

    # 输出
    echo "=============================="
    echo "输入: ${bytes[*]^^}"
    echo "累加和: ${sum} (0x$(d2h $sum))"
    echo "校验和: 0x$(d2h $chk) (${chk})"
    echo "------------------------------"
    echo "完整帧: ${bytes[*]^^} $(d2h $chk)"
    echo "=============================="
}

main "$@"
