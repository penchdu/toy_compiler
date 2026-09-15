
#========== asm ==========
.intel_syntax noprefix
.extern printf 
.section .rodata 
fmt: 
	.string "Result: %d\n" 

.section .text
.global main

main: 
	push rbp
	mov rbp, rsp
	sub rsp, 48

	mov r10d, 0
	mov dword ptr [rbp - 20], r10d
	mov r10d, 2
	mov dword ptr [rbp - 28], r10d
	mov r10d, 1
	mov dword ptr [rbp - 12], r10d
	mov r10d, 1
	mov dword ptr [rbp - 40], r10d
	mov r10d, 2
	mov dword ptr [rbp - 44], r10d
	mov r10d, 0
	mov dword ptr [rbp - 4], r10d
	mov r11d, dword ptr [rbp - 20]
	mov r10d, r11d
	mov dword ptr [rbp - 24], r10d
	mov r11d, dword ptr [rbp - 12]
	mov r10d, r11d
	mov dword ptr [rbp - 16], r10d
	mov r11d, dword ptr [rbp - 4]
	mov r10d, r11d
	mov dword ptr [rbp - 8], r10d
	mov r11d, dword ptr [rbp - 24]
	mov r10d, r11d
	mov dword ptr [rbp - 32], r10d
	mov r11d, dword ptr [rbp - 40]
	mov r10d, r11d
	mov dword ptr [rbp - 16], r10d
	mov r10d, dword ptr [rbp - 32]
	mov r11d, dword ptr [rbp - 28]
	add r10d, r11d
	mov dword ptr [rbp - 32], r10d
	mov r11d, dword ptr [rbp - 16]
	mov r10d, r11d
	mov dword ptr [rbp - 48], r10d
	mov r11d, dword ptr [rbp - 32]
	mov r10d, r11d
	mov dword ptr [rbp - 36], r10d
	mov r11d, dword ptr [rbp - 44]
	mov r10d, r11d
	mov dword ptr [rbp - 36], r10d
	mov r10d, dword ptr [rbp - 48]
	mov r11d, dword ptr [rbp - 36]
	add r10d, r11d
	mov dword ptr [rbp - 48], r10d
	mov r10d, dword ptr [rbp - 48]
	# ---------------- 打印结果 ----------------
	mov esi, r10d		# 第 2 个参数：要打印的整数 (放在 %esi / %rsi)
	lea rdi, [rip + fmt]		# 第 1 个参数：格式化字符串地址 (放在 %rdi)
	mov eax, 0		# x86-64 ABI 规定：变长参数调用前将 eax 清零
	call printf@PLT		# 调用 C 语言的 printf
	# ------------------------------------------
	mov eax, r10d
	mov rsp, rbp
	pop rbp
	ret 

.section .note.GNU-stack,"",@progbits
