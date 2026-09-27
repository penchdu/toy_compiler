#!/bin/bash
# 检查 asm 里的 spill/reload 栈地址是否配对
# 用法: bash check_spill.sh a1.s (或者你的 asm 输出文件)

ASM=${1:-a1.s}

echo "=== Spill (MC_ST) ==="
grep "spill" "$ASM" | grep -oP '\[rbp - \K\d+' | sort -n | uniq -c | sort -rn

echo ""
echo "=== Reload (MC_LD / alloc) ==="
grep -E "alloc|reload" "$ASM" | grep -oP '\[rbp - \K\d+' | sort -n | uniq -c | sort -rn

echo ""
echo "=== 配对检查（地址 vs 次数） ==="
# 提取每个栈地址，看 ST 和 LD 次数是否匹配
for addr in $(grep -oP '\[rbp - \K\d+' "$ASM" | sort -n | uniq); do
    st_count=$(grep "spill" "$ASM" | grep -oP '$$rbp - '"$addr"'$$' | wc -l)
    ld_count=$(grep -E "alloc|reload" "$ASM" | grep -oP '$$rbp - '"$addr"'$$' | wc -l)
    if [ "$st_count" != "$ld_count" ]; then
        echo "❌ 栈 [rbp - $addr]: ST=$st_count LD=$ld_count 不匹配！"
    fi
done
echo "配对检查完成"