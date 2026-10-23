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

enum X64PhysicReg {
	EDI,
	R10D,
	R11D,
	R12D,
	R13D,
	R14D,
	//	R15D,

//	EAX,
//	EBX,
	X64PR_MAX,

	PR_IMM,
};

enum AllocMethod {
	O0,
	WAVE,
};

struct VrState
{
	int pr = X64PR_MAX;
	int u = VR_USAGE_INVALID;
	bool dirty = 0;
	int expected_usage = VR_USAGE_INVALID;
	int score = 0;
};
extern vector<VrState> vr_state;

constexpr int INVALID_VR = -1;
struct PrState
{
	int vr = INVALID_VR;
//	int u = VR_USEAGE_INVALID;
};
extern vector<PrState> pr_state;

struct McDepend {
	vector<int> mcs;
	int edges = 0;
};

enum MachineCodeStamp {
	MC_LI,	// reg-num

	MC_LD,	// reg <- ptr
	MC_victim,
	MC_ST,	// reg -> ptr

	MC_ASSIGN,	// reg-reg
	MC_NEG,	// reg
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
	MC_TEST,

	MC_SET_E,
	MC_SET_NE,
	MC_SET_L,
	MC_SET_LE,
	MC_SET_G,
	MC_SET_GE,
	MC_SET_TEST,

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
	int latency = -1;
	const char *mc_code;	// just for print
};
extern const McInfo mc_info[];
extern VrDeclareRecord declare_record;

static int mcidx = 0;
struct X64mc {
	X64mc(MachineCodeStamp mc_stamp, int tac_dst, int tac_s1, int tac_s2)
	{
		stamp = mc_stamp;
		dst = tac_dst;
		s1 = tac_s1;
		s2 = tac_s2;

		idx = mcidx++;
	}
	X64mc(MachineCodeStamp mc_stamp, int tac_dst, int tac_s1)
	{
		stamp = mc_stamp;
		dst = tac_dst;
		s1 = tac_s1;

		idx = mcidx++;
	}
	X64mc(MachineCodeStamp mc_stamp, int tac_s1)
	{
		stamp = mc_stamp;
		s1 = tac_s1;

		idx = mcidx++;
	}
	X64mc(MachineCodeStamp mc_stamp)
	{
		stamp = mc_stamp;
		idx = mcidx++;
	}
	X64mc()
	{
		stamp = MC_INVALID;
	}

	MachineCodeStamp stamp = MC_INVALID;

	union {
		struct {
			int dst = INVALID_VR;
			int s1 = INVALID_VR;
			int s2 = INVALID_VR;
		};
		int vr_list[3];
	};

//	VrUsage s1_usage;
//	VrUsage s2_usage;

	// alloced for vr
	int pr_dst = X64PR_MAX;
	int pr1 = X64PR_MAX;	//todo rename
	int pr2 = X64PR_MAX;

	int const_num = 0;

	// put these here to easy dump
	string ori_sem;
	int chain_latency = -1;
	int start_cycle = -1;
	int idx = -1;
};

struct BasicBlock {
	bool is_entry_label = 0;
	vector<X64mc> x64mc;
	vector<X64mc> x64mc_schedu;
	vector<int> vr_unique;

//	Scope *exit_jmp = 0;
//	Scope *jmp_to = 0;
	MachineCodeStamp jmp_mc_stamp = MC_INVALID;

	//
	vector<X64mc> x64mc_alloc_o0;
	vector<X64mc> x64mc_alloc_wave;
//	float max_wave_value = 0;
};

struct Wave {
	int vr;
	int mc_list_size;		// == score.size() == appear_cnt.size()
	vector<float> score;
	vector<int> appear_cnt;		// one vr use cnt in a mc, 0 ~ 3,  3->"a = a + a"; 2->"b = a + a"
};
extern vector<Wave> bb_vr_wave;

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
void update_vr_consume_cnt(Scope *scp, int vr, int update_value = 1);
void update_mc_consume_cnt(Scope *scp, const X64mc &mc, int update_value = 1);
void reset_scope_symb_consume_cnt(Scope *scp, int value);
void clear_all_vr_consume_cnt(Scope *scp);
void check_vr_consume_cnt(Scope *scp);
void mc_schedule();

const char* pr_name(int pr);
const char* pr_name_byte(int pr);
const char* pr_name_asm(int pr, int vr);

void spill_vr(
    enum AllocMethod method,
    vector<VrState> &vr_to_pr,
    vector<PrState> &pr_to_vr,
    int vr, vector<X64mc> &mcs,
    const string &tag = "");

int get_pr_o0();
int get_pr_wave(BasicBlock *bb, int mc_idx);
int get_pr(enum AllocMethod method,
    vector<X64mc> &mcs,
    BasicBlock *bb, int mc_idx,
    int vr, int u,
    bool alloc_immediate = false);

int get_pr_alloc_imm(enum AllocMethod method, vector<X64mc> &mcs, BasicBlock *bb, int mc_idx, int vr);

void x64_reg_alloc();

void dump_vr_state(const vector<VrState> &vp, const string &tag);
void check_pr_vr_consistency();
void check_dead_vr(Scope *scp, vector<X64mc> &mcs, int vr);
void init_wave(BasicBlock *bb);

int get_free_pr_num(const vector<PrState> &prst, int reserved_pr = X64PR_MAX);
void ra_wave__mc(Scope *scp, BasicBlock *bb, const X64mc &schedued, int i);
void x64_reg_alloc_wave();
void ra_wave_regular_scope(Scope *scp);
void ra_wave_scope(Scope *scp);

void while__pre_work(Scope *scp);
void while__end_work(Scope *scp);
int get_swap_pr(BasicBlock *bb);
void recover_pr_state(
    const vector<VrState> &before_vr2pr,
    const vector<PrState> &before_pr2vr,
    int swap_pr,
    vector<VrState> &now_vr2pr,
    vector<PrState> &now_pr2vr,
    Scope *scp_to_gen_mc);

void ra_wave_while(Scope *scp);
void ra_wave_continue_break_save_vr_state(Scope *scp);
//void ra_wave__continue(Scope *while_scp);
void ra_wave_if(Scope *if_scp);

#endif /* X64_BACK_END_H_ */
