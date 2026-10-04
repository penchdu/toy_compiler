/*
 * mc_x86_64.h
 *
 *  Created on: 2026年9月11日
 *      Author: x
 */

#ifndef X64_BACK_END_H_
#define X64_BACK_END_H_

#include "enums.h"

/*
 *
 st[n] = dword ptr [n]

 push rbp
 mov rbp, rsp
 sub rsp, N


 li
 sem_const_num,
 load %s1, st[s1]
 mov %dst, <num>

 load,
 mov %dst, st[dst]

 store,
 mov st[dst], %dst

 op_assign,
 %dst = %s1
 mov %dst, %s1

 op_+-*,
 %dst = %s1 + %s2
 mov %dst, %s1
 add %dst, %s2

 op_+-*,
 %dst = %s1 + %s2	mov %dst, st[s1]
					add %dst, st[s2]

 op_div,
 %dst = %s1 / %s2
 mov %dst, %s1
 mov eax, %dst
 cdq
 idiv %s2
 mov %dst, eax

 sem_return
 mov eax, %dst
 mov rsp, rbp
 pop rbp
 ret
 */

#if 1
	#define PRINT_MORE	\
			printf("\t\t#%s", mc.ori_sem.c_str());	\
			printf(", cyc %d", mc.start_cycle);	\
		/*	PRINT_ASM(", off %d\n", mc.of1);	*/	\
			printf("\n");
#else
#define PRINT_MORE	\
//		printf("\n");
#endif

enum X64pr {
	R10D,
	R11D,
	R12D,
	R13D,
	R14D,
	R15D,
	//	eax,
	X64PR_MAX,
};

static const char *pr_name[] = {
    [R10D] = "r10d",
    [R11D] = "r11d",
    [R12D] = "r12d",
    [R13D] = "r13d",
    [R14D] = "r14d",
    [R15D] = "r15d",
//	[eax] = "eax",
    };

static const char* pr_name_byte(int pr)
{
	switch (pr)
	{
	case R10D:
		return "r10b";
	case R11D:
		return "r11b";
	case R12D:
		return "r12b";
	case R13D:
		return "r13b";
	case R14D:
		return "r14b";
	case R15D:
		return "r15b";
	}

	ERR("no byte register");
	return 0;
}


enum VrUsage
{
	VR_USAGE_READ = 1 << 0,
	VR_USAGE_WRITE = 1 << 1,
	VR_USAGE_READ_WRITE = VR_USAGE_READ | VR_USAGE_WRITE,

	VR_USEAGE_INVALID = 1 << 31,
};

struct VrToPr
{
	int pr = X64PR_MAX;
	int u = VR_USEAGE_INVALID;
	int score = 0;
};
extern vector<VrToPr> vr2pr;

constexpr int INVALID__VR = -1;
struct PrToVr
{
	int vr = INVALID__VR;
//	int u = VR_USEAGE_INVALID;
};
extern vector<PrToVr> pr2vr;

struct McDepend {
	vector<int> mcs;
	int edges = 0;
};

enum MachineCodeStamp {
	MC_LI,	// reg-num

	MC_LD,	// reg <- ptr
	MC_ST,	// reg -> ptr

	MC_ASSIGN,	// reg-reg
	MC_ADD,	// reg-reg
	MC_SUB,
	MC_IMUL,
	MC_DIV,

//	MC_LOGIC_AND,
//	MC_LOGIC_OR,

	MC_CMP,	// setl cl

	MC_CMP_E,
	MC_CMP_NE,
	MC_CMP_L,
	MC_CMP_LE,
	MC_CMP_G,
	MC_CMP_GE,

	MC_SET_E,
	MC_SET_NE,
	MC_SET_L,
	MC_SET_LE,
	MC_SET_G,
	MC_SET_GE,

	MC_JMP,
	MC_JE,
	MC_JNE,
	MC_JL,
	MC_JLE,
	MC_JG,
	MC_JGE,

	MC_SAVE_RET,	// reg-reg
	MC_INVALID,
};
struct Mc2mc {
	MachineCodeStamp mc_cmp;
	MachineCodeStamp mc_set;
};


struct McInfo {
	int mc_latency = -1;
	string mc_code;	// just for print
};
extern const McInfo mc_info[];
extern VirtualRegDeclareManager vr_declare_manager;

static int mcidx = 0;
struct X64mc {
	X64mc(MachineCodeStamp _mc_stamp, int tac_dst, int tac_s1, int tac_s2)
	{
		mc_stamp = _mc_stamp;
		dst = tac_dst;
		s1 = tac_s1;
		s2 = tac_s2;

		of_dst = vr_declare_manager.get_vr_off(dst);
		of1 = vr_declare_manager.get_vr_off(s1);
		of2 = vr_declare_manager.get_vr_off(s2);

		asm_code = mc_info[_mc_stamp].mc_code;
		latency = mc_info[_mc_stamp].mc_latency;
		idx = mcidx++;
	}
	X64mc(MachineCodeStamp _mc_stamp, int tac_dst, int tac_s2)
	{
		mc_stamp = _mc_stamp;
		s1 = tac_dst;
		s2 = tac_s2;

		of1 = vr_declare_manager.get_vr_off(s1);
		of2 = vr_declare_manager.get_vr_off(s2);

		asm_code = mc_info[_mc_stamp].mc_code;
		latency = mc_info[_mc_stamp].mc_latency;
		idx = mcidx++;
	}
	X64mc(MachineCodeStamp _mc_stamp, int tac_s1)
	{
		mc_stamp = _mc_stamp;
		s1 = tac_s1;
		of1 = vr_declare_manager.get_vr_off(tac_s1);

		asm_code = mc_info[_mc_stamp].mc_code;
		latency = mc_info[_mc_stamp].mc_latency;
		idx = mcidx++;
	}
	X64mc(MachineCodeStamp _mc_stamp)
	{
		mc_stamp = _mc_stamp;
	}
	X64mc()
	{
		mc_stamp = MC_INVALID;
	}

	MachineCodeStamp mc_stamp = MC_INVALID;

	union {
		struct {
			int dst = -1;
			int s1 = -1;
			int s2 = -1;
		};
		int vr_list[3];
	};
	int of_dst = -1;
	int of1 = -1;
	int of2 = -1;

//	VrUsage s1_usage;
//	VrUsage s2_usage;

	// alloced for vr
	int pr_dst = -1;
	int pr1 = -1;	//todo rename
	int pr2 = -1;

	int const_num = 0;

	// put these here to easy dump
	string asm_code;
	string ori_sem;
	int latency = -1;
	int chain_latency = -1;
	int start_cycle = -1;
	int idx = -1;
};


struct BasicBlock {
	Scope *entry_label = 0;
	vector<X64mc> x64mc;
	vector<X64mc> x64mc_schedu;
	vector<int> vrids;

//	Scope *exit_jmp = 0;
	Scope *jmp_to = 0;
	MachineCodeStamp jmp_mc_stamp = MC_INVALID;

	//
	vector<X64mc> x64mc_alloc_o0;
	vector<X64mc> x64mc_alloc_wave;
//	float max_wave_value = 0;
};

struct Wave {
	int vr;
	vector<float> score;
	vector<int> insts;		// >=0, <= 3
};
extern vector<Wave> vrwave;

void dump_mc(Scope *scp, string mc_list_name);
void gen_wave(BasicBlock *bb);
void dump_wave(BasicBlock *bb);
void dump_wave_gnuplot(Wave &w, int vr);
//void dump_asm();

//void spill_pr(vector<X64mc> &x64mc_alloced, int vr);
//void spill_pr(vector<X64mc> &x64mc_alloced, int vr1, int vr2);


void gen_machine_code();
void print_mc_err(const char *prefix, Scope *scp, const X64mc &mc, int idx);

void gen_use_def_chain(vector<X64mc> &x64mc);
void dump_chain(vector<X64mc> &x64mc);
void gen_schdu_chain_latency(vector<X64mc> &x64mc);

bool is_private_symb(Scope *scp, int vr);
void update_vr_consume_cnt(Scope *scp, int vr, int target_symb_stamp, int usage = VR_USAGE_READ);
void update_mc_consume_cnt(Scope *scp, const X64mc &mc, int target_symb_stamp = SYMB_ALL);
void clear_vr_consume_cnt(Scope *scp);
void check_vr_consume_cnt(Scope *scp);
void mc_schedule();

void spill_vr(vector<X64mc> &x64mc_alloced, int vr);
void spill_all_pr(BasicBlock *bb);
void x64_reg_alloc();

void check_and_clear_vr(Scope *scp, int vr);
int wave_get_pr(BasicBlock *bb, int mc_idx);
void x64_reg_alloc_wave();
void reg_alloc_wave__scope(Scope *scp);
void reg_alloc_wave__while(Scope *scp);


#endif /* X64_BACK_END_H_ */
