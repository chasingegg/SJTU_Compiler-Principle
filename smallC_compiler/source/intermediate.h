/*
  Author: Chao Gao
  file: intermediate.h
  This program translate syntax tree into three address codes.
*/

#ifndef FILE_INTERMEDIATE_H
#define FILE_INTERMEDIATE_H
#include "def.h"
#include "node.h"
#include "tree.h"
#include "semantics.h"
#include <cstring>
#define REGISTER_STATE_ADDRESS 1
int quadruple_flag, function_begin_sp, main_flag,global_flag,label_count;
int stack_pointer, retad_pointer,local_register_count = 0, current_sp,tmp_num_var = 0,arrs_cnt = 0,arr_init_cnt = 0;
int translate_level = 0, translate_cnt[MAXSIZE];
ofstream fout("InterCode");
enum RegType {
	ADDRESS_LABEL, ADDRESS_CONSTANT, ADDRESS_TEMP,
        ADDRESS_NAME
};

struct Address{
    RegType type;
    string name;
    int value;
    int real;
    int needload;
    int needclear;
};

struct Quadruple{
    string op; // name of operation
    int active; //whether it's active
    int flag; //whether the arguments should be revised
    Address arguments[3];
};

vector <Quadruple> IR, GIR, MIR;
vector <int> RegisterState, RegisterOffset,LabelCont, LabelBreak, vs_reg; 
vector <string> vs_id;
Quadruple new_quadruple();
Quadruple new_quadruple_arithmetic_im(char *s, int ra, int rb, int im);
void phase3_translate();
int new_register();
void set_register_state_to_address(int k);
Quadruple new_quadruple_function(char *s);
void translate(TreeNode *p);
void translate_extdef(TreeNode *p);
void translate_extdefs(TreeNode *p);
void translate_stmtblock(TreeNode *p);
void translate_args_1(TreeNode *p, int reg);
void translate_exps(TreeNode *p, int reg);
void translate_stmts(TreeNode *p);
void translate_stmt(TreeNode *p);
void translate_assignment_1(TreeNode *p, int flag, int num);
void translate_defs(TreeNode *p);
Quadruple new_quadruple_sw(int ra, int rb);
Quadruple new_quadruple_lw(int ra, int rb);
void translate_assignment(int reg_l, int reg_r);
int get_register_state(int k);
void translate_init(TreeNode *p, int reg);
Quadruple new_quadruple_arithmetic(char *s, int ra, int rb, int rc);
void translate_exps(TreeNode *p, int reg);
void translate_var_global(TreeNode *p, int reg);
void translate_extvars(TreeNode *p);
void translate_args(TreeNode *p);
void translate_exp(TreeNode *p);
void set_register_state_to_value(int k);
Quadruple new_quadruple_goto(int value);
Quadruple new_quadruple_label();
Quadruple new_quadruple_call(char *s);
Quadruple new_quadruple_branch(char *s, int a, int b);
void ir_print(vector <Quadruple>);
void IR_push_back(Quadruple ret);
void translate_var_local(TreeNode *p,int reg);
void translate_decs(TreeNode *p);
Quadruple new_quadruple_move(int ra, int rb);
int vs_fetch_register(char* s);

Quadruple new_quadruple_move(int ra, int rb) {
    Quadruple ret = new_quadruple();
    ret.op = "move";
    ret.arguments[0].type = ADDRESS_TEMP;
    ret.arguments[0].value = ra;
    ret.arguments[1].type = ADDRESS_TEMP;
    ret.arguments[1].value = rb;
    return ret;
}

void IR_push_back(Quadruple ret)
{
	if (global_flag) {
		MIR.push_back(ret);
	}
	else {
		IR.push_back(ret);
	}
}

void ir_print(vector <Quadruple> IR){
	int i,j;
	for (i = 0; i < IR.size(); ++i) {
		if (!IR[i].active) {
			fout<<"#" << i << endl;
			continue;
		}
		if (IR[i].op=="func") {
			fout << "#" << i;
			fout << IR[i].op << "\t";
			fout << IR[i].arguments[0].name << endl;
			continue;
		}
		if (IR[i].op=="call") {
			fout << "#"<<i;
			fout << "\t\t" << IR[i].op << "\t";
			fout << IR[i].arguments[0].name << endl;
			continue;
		}
		fout << "#" << i << "\t\t";
		fout << IR[i].op;
		for (j = 0; j < 3; ++j) {
			fout << "\t";
			if (IR[i].arguments[j].value==-1) break;
			switch(IR[i].arguments[j].type) {
				case ADDRESS_TEMP: 
					if (IR[i].arguments[j].value == stack_pointer) {
                        			fout << "$sp";
                   			} else if (IR[i].arguments[j].value == retad_pointer) {
                        			fout << "$ra";
					}
					else if (IR[i].arguments[j].value == -2) {
						fout << "$a0";
					}
					else if (IR[i].arguments[j].value==-3) {
						fout << "$v0";
					}
                    			else if (IR[i].arguments[j].real != -1) {
                        			//printf( "$%d", IR[i].arguments[j].real);
						fout << "$" << IR[i].arguments[j].real;
                   			} else {
                        			//printf("t%d", IR[i].arguments[j].value);
						fout << "t" << IR[i].arguments[j].value;
                    			}
					break;
				case ADDRESS_CONSTANT:
					//printf("%d", IR[i].arguments[j].value);
					fout << IR[i].arguments[j].value;
					break;
				case ADDRESS_NAME:
					fout << IR[i].arguments[j].name;
					break;
				case ADDRESS_LABEL:
					//printf("l%d", IR[i].arguments[j].value);
					fout << "l"<< IR[i].arguments[j].value;
					break;
				default: break;
			}
		}
		fout << endl;	
	}
}

int new_register() {
    RegisterState.push_back(0);
    if (function_begin_sp == -1) {
        RegisterOffset.push_back(-1);
    } else {
        local_register_count++;
        RegisterOffset.push_back(local_register_count * 4);
    }
    return RegisterState.size()-1;
}

void set_register_state_to_address(int k) {
	if (k<0) return;
	RegisterState[k] = 1;
}

void set_register_state_to_value(int k) {
	if (k<0) return;
	RegisterState[k] = 0;
}

void catch_value_self(int k) {  //if reg k refers to an address, then transform it to the value it stores
    if (get_register_state(k) == REGISTER_STATE_ADDRESS) {  
        IR.push_back(new_quadruple_lw(k, k));
        set_register_state_to_value(k);
    }
}

Quadruple new_quadruple() {
    Quadruple ret;
    ret.flag = quadruple_flag;
    ret.active = 1;
    ret.arguments[0].value = -1;
    ret.arguments[0].real = -1;
    ret.arguments[0].needload = 0;
    ret.arguments[0].needclear = 0;

    ret.arguments[1].value = -1;
    ret.arguments[1].real = -1;
    ret.arguments[1].needload = 0;
    ret.arguments[1].needclear = 0;

    ret.arguments[2].value = -1;
    ret.arguments[2].real = -1;
    ret.arguments[2].needload = 0;
    ret.arguments[2].needclear = 0;
    return ret;
}

Quadruple new_quadruple_sw_offset(int ra, int rb, int offset) {
    Quadruple ret = new_quadruple();
    ret.op = "sw";
    ret.arguments[0].type = ADDRESS_TEMP;
    ret.arguments[0].value = ra;
    ret.arguments[1].type = ADDRESS_TEMP;
    ret.arguments[1].value = rb;
    ret.arguments[2].type = ADDRESS_CONSTANT;
    ret.arguments[2].value = offset;
    return ret;
}

Quadruple new_quadruple_gnew(int size, int reg) {  //为了方便调试, 或许可以把名字打进去
    Quadruple ret = new_quadruple();
    ret.op = "gnew";
    ret.arguments[0].type = ADDRESS_CONSTANT;
    ret.arguments[0].value = size;
    ret.arguments[1].type = ADDRESS_TEMP;
    ret.arguments[1].value = reg;
    return ret;
}

Quadruple new_quadruple_function(char *s) {
    Quadruple ret = new_quadruple();
    ret.op = "func";
    ret.arguments[0].type = ADDRESS_NAME;
    if (strcmp(s, "main") != 0) {
        int n = strlen(s);
        char* t = new char[n+3];
        t[0] = t[1] = '_';
        memcpy(t + 2, s, sizeof(char) * n);
        t[n + 2] = '\0';
        ret.arguments[0].name = t;
	ret.arguments[0].value = -2;
    } else {
        ret.arguments[0].name = s;
	ret.arguments[0].value = -2;
    }
    return ret;
}

int get_label_number(Quadruple qr) {
    assert(qr.op=="label");
    return qr.arguments[0].value;
}

Quadruple new_quadruple_lw(int ra, int rb) {
    Quadruple ret = new_quadruple();
    ret.op = "lw";
    ret.arguments[0].type = ADDRESS_TEMP;
    ret.arguments[0].value = ra;
    ret.arguments[1].type = ADDRESS_TEMP;
    ret.arguments[1].value = rb;
    ret.arguments[2].type = ADDRESS_CONSTANT;
    ret.arguments[2].value = 0;
    return ret;
}

Quadruple new_quadruple_li(int ra, int im) {
    Quadruple ret = new_quadruple();
    ret.op = "li";
    ret.arguments[0].type = ADDRESS_TEMP;
    ret.arguments[0].value = ra;
    ret.arguments[1].type = ADDRESS_CONSTANT;
    ret.arguments[1].value = im;
    return ret;
}

Quadruple new_quadruple_ret() {
    Quadruple ret = new_quadruple();
    ret.op = "ret";
    return ret;
}

Quadruple new_quadruple_arithmetic(char *s, int ra, int rb, int rc) {
    Quadruple ret = new_quadruple();
    ret.op = s;
    ret.arguments[0].type = ADDRESS_TEMP;
    ret.arguments[0].value = ra;
    ret.arguments[1].type = ADDRESS_TEMP;
    ret.arguments[1].value = rb;
    ret.arguments[2].type = ADDRESS_TEMP;
    ret.arguments[2].value = rc;
    return ret;
}

Quadruple new_quadruple_sw(int ra, int rb) {
    Quadruple ret = new_quadruple();
    ret.op = "sw";
    ret.arguments[0].type = ADDRESS_TEMP;
    ret.arguments[0].value = ra;
    ret.arguments[1].type = ADDRESS_TEMP;
    ret.arguments[1].value = rb;
    ret.arguments[2].type = ADDRESS_CONSTANT;
    ret.arguments[2].value = 0;
    return ret;
}

Quadruple new_quadruple_goto(int value) {
    Quadruple ret = new_quadruple();
    ret.op = "goto";
    ret.arguments[0].type = ADDRESS_LABEL;
    ret.arguments[0].value = value;
    return ret;
}

Quadruple new_quadruple_label() {
    Quadruple ret = new_quadruple();
    ret.op = "label";
    ret.arguments[0].type = ADDRESS_LABEL;
    ret.arguments[0].value = label_count++;
    return ret;
}

Quadruple new_quadruple_call(char *s) {
    Quadruple ret = new_quadruple();
    ret.op = "call";
    ret.arguments[0].type = ADDRESS_NAME;
    if (strcmp(s, "main") != 0) {
        int n = strlen(s);
        char* t = (char*)malloc(sizeof(char) * (n + 3));
        t[0] = t[1] = '_';
        memcpy(t + 2, s, sizeof(char) * n);
        t[n + 2] = '\0';
        ret.arguments[0].name = t;
    } else {
        ret.arguments[0].name = s;
    }
    return ret;
}

Quadruple new_quadruple_branch(char *s, int a, int b) {
    Quadruple ret = new_quadruple();
    ret.op = s;
    ret.arguments[0].type = ADDRESS_TEMP;
    ret.arguments[0].value = a;
    ret.arguments[1].type = ADDRESS_LABEL;
    ret.arguments[1].value = b;
    return ret;
}

Quadruple new_quadruple_arithmetic_im(char *s, int ra, int rb, int im) {
    Quadruple ret = new_quadruple();
    ret.op = s;
    ret.arguments[0].type = ADDRESS_TEMP;
    ret.arguments[0].value = ra;
    ret.arguments[1].type = ADDRESS_TEMP;
    ret.arguments[1].value = rb;
    ret.arguments[2].type = ADDRESS_CONSTANT;
    ret.arguments[2].value = im;
    return ret;
}

Quadruple new_quadruple_arithmetic_unary(char *s, int ra, int rb) {
    Quadruple ret = new_quadruple();
    ret.op = s;
    ret.arguments[0].type = ADDRESS_TEMP;
    ret.arguments[0].value = ra;
    ret.arguments[1].type = ADDRESS_TEMP;
    ret.arguments[1].value = rb;
    return ret;
}

int get_width(char* s) {
	int tmp_level = translate_level, tmp_depth = translate_cnt[translate_level];
	while (1) {
		if (env[tmp_level][tmp_depth].table.find(s)!=env[tmp_level][tmp_depth].table.end()) { //find
			return env[tmp_level][tmp_depth].width_table[s];
		}
		else {
			int parent = env[tmp_level][tmp_depth].parent_index;
			if (parent==-1||tmp_level==0) {fprintf(stderr,"error! %s\n",s);assert(0);}
			else {
				tmp_level--;
				tmp_depth = parent;
			}
		}
	}
}

int cal_offset(char* s, char *s2) {
	int tmp_level = translate_level, tmp_depth = translate_cnt[translate_level];
	while (1) {
		if (env[tmp_level][tmp_depth].table.find(s)!=env[tmp_level][tmp_depth].table.end()) { //find
			assert(!strcmp("struct",env[tmp_level][tmp_depth].table[s]));
			int i;
			for (i = 0; i < env[tmp_level][tmp_depth].struct_table[s].size(); ++i) {
				if (!strcmp(env[tmp_level][tmp_depth].struct_table[s][i],s2)) return i;
			}
			assert(0);
		}
		else {
			int parent = env[tmp_level][tmp_depth].parent_index;
			if (parent==-1||tmp_level==0) {fprintf(stderr,"error! %s\n",s);assert(0);}
			else {
				tmp_level--;
				tmp_depth = parent;
			}
		}
	}
}

void phase3_translate()
{
    main_flag = 0;
    function_begin_sp = -1;
    quadruple_flag = 0;
    label_count = translate_level = 0;
    global_flag = 0;
    stack_pointer = new_register();
    set_register_state_to_address(stack_pointer);
    retad_pointer = new_register();
    current_sp = 0;
    translate(treeroot);
    fout << "gir print------------------------------------\n";
    ir_print(GIR);
    fout << "mir print------------------------------------\n";
    ir_print(MIR);
    fout << "ir print-------------------------------------\n";
    ir_print(IR);
}

void translate(TreeNode *p) {
	int i;
	switch(p->type) {
		case _PROGRAM: translate(p->children[0]); break;
		case _EXTDEFS: 
			translate_extdefs(p);
		case _EXTDEF: translate_extdef(p); break;
		case _TYPE: break;
		case _NULL: break;
		case _OPERATOR: break;
		default: fout << p->data << endl; assert(0); break;
	}
}

void translate_paras(TreeNode *p) {
	if (p->size==0) return;
	++tmp_num_var;
	int reg = new_register();
        IR_push_back(new_quadruple_arithmetic_im("add", reg, stack_pointer, 4*tmp_num_var-4));    //pass value of parameters here!
        set_register_state_to_address(reg); 
        vs_id.push_back(p->children[0]->data);
	vs_reg.push_back(reg);
	if (p->size==2) translate_paras(p->children[1]);
}

void translate_sextvars(TreeNode *p) {
	if (p->size==0) return;	
	int reg = new_register();
	GIR.push_back(new_quadruple_gnew(get_width(p->children[0]->data)*4, reg));
	vs_id.push_back(p->children[0]->data);
	vs_reg.push_back(reg);
	if (p->size==2) translate_sextvars(p->children[1]);
}

void translate_extdef(TreeNode *p) {
	int i;
	if (!strcmp(p->data,"extdef func")) {
		global_flag = 0;
		IR_push_back(new_quadruple_function(p->children[1]->children[0]->data));
		if (!strcmp(p->children[1]->children[0]->data,"main")) {main_flag = 1;}
		int j = IR.size()-1;
 		local_register_count = 0;
		function_begin_sp = current_sp;
		tmp_num_var = 0;
		translate_paras(p->children[1]->children[1]);
		translate_stmtblock(p->children[2]);
		IR_push_back(new_quadruple_ret());
		for (i = j; i < IR.size(); i++) {
        		if (IR[i].flag == 1) {
           			IR[i].arguments[2].value -= local_register_count * 4;
        		}
    		}
		current_sp = function_begin_sp;
    		function_begin_sp = -1;
		main_flag = 0;
	}
	else if (!strcmp(p->data,"extdef1")) {
		global_flag = 1;
		translate_extvars(p->children[1]);
	}
	else if (!strcmp(p->data,"extdef2")){ //structs
		translate_sextvars(p->children[1]);
	}
}

int vs_fetch_register(char* s) {
    int i;
    for (i = vs_id.size() - 1; i >= 0; i--) {
        if (vs_id[i]==s) {
            return vs_reg[i];
        }
    }
    assert(0); // we shall never get here
}

int get_register_state(int k) {
    return RegisterState[k];
}

void translate_extvars(TreeNode *p) {
	if (p->size==0) return;
    	if (p->size == 3) {
		int reg = new_register();
		translate_var_global(p->children[0],reg);
		if (!strcmp(p->children[2]->data,"init {}")) {
			arr_init_cnt = 0;
			translate_args_1(p->children[2]->children[0],reg);
		}
		else {
			int tmp = new_register();
        		translate_init(p->children[2], tmp);
        		translate_assignment(reg, tmp);
		}
    	}
    	else if (p->size==2) {
		int reg = new_register();
		translate_var_global(p->children[0],reg);
		translate_extvars(p->children[1]);
	}
	else if (p->size==4){
		int reg = new_register();
		translate_var_global(p->children[0],reg);
		if (!strcmp(p->children[2]->data,"init {}")) {
			arr_init_cnt = 0;
			translate_args_1(p->children[2]->children[0],reg);
		}
		else {
			int tmp = new_register();
        		translate_init(p->children[2], tmp);
        		translate_assignment(reg, tmp);
		}
		translate_extvars(p->children[3]);
	}
	else {
		int reg = new_register();
		translate_var_global(p->children[0],reg);
	}
}

void translate_init(TreeNode *p, int tmp) {
	if (!strcmp(p->data,"init")) {
		translate_exps(p->children[0],tmp);
	}
	else {
		
	}
}

void translate_args_1(TreeNode *p, int reg) {
	if (p->size==1) {
		++arr_init_cnt;
		if (p->children[0]->size==0) return;
		int tmp = new_register();
		translate_exps(p->children[0]->children[0],tmp);
		catch_value_self(tmp);
		IR_push_back(new_quadruple_sw_offset(tmp,reg,4*arr_init_cnt-4));
	}
	else {
		++arr_init_cnt;
		if (p->children[0]->size==0) {translate_args_1(p->children[1],reg);return;}
		int tmp = new_register();
		translate_exps(p->children[0]->children[0],tmp);
		catch_value_self(tmp);
		IR_push_back(new_quadruple_sw_offset(tmp,reg,4*arr_init_cnt-4));
		translate_args_1(p->children[1],reg);
	}
}

void translate_var_global(TreeNode *p,int reg) {
	if (p->size==1) {
            	GIR.push_back(new_quadruple_gnew(get_width(p->children[0]->data)*4, reg));// global new variable
    		set_register_state_to_address(reg);
		vs_id.push_back(p->children[0]->data);
		vs_reg.push_back(reg);
	}
	else {
		translate_var_global(p->children[0],reg);
	}
}

void translate_extdefs(TreeNode *p) {
	if (p->size==0) return;
	else {
		translate_extdef(p->children[0]);
		translate_extdefs(p->children[1]);
	}
}

void translate_stmtblock(TreeNode *p) {
	++translate_level;
	++translate_cnt[translate_level];
	translate_defs(p->children[0]);
	translate_stmts(p->children[1]);
	--translate_level;
}

void translate_sdecs(TreeNode *p) {
	int reg = new_register();	
	current_sp += 4*get_width(p->children[0]->data);
	int t = new_register();
        quadruple_flag = 1;
        IR_push_back(new_quadruple_arithmetic_im("add", t, stack_pointer, -current_sp + function_begin_sp));
        quadruple_flag = 0;
        IR_push_back(new_quadruple_move(reg, t));
	set_register_state_to_address(reg);
	vs_id.push_back(p->children[0]->data);
	vs_reg.push_back(reg);
	if (p->size==2) translate_sdecs(p->children[1]);
}

void translate_defs(TreeNode *p) {
	if (p->size==0) return;
	if (!strcmp(p->data,"defs1")) {
    		global_flag = 0;
		translate_decs(p->children[1]);
		translate_defs(p->children[2]);
    	}
	else if (!strcmp(p->data,"defs2")) {
		translate_sdecs(p->children[1]);
	}
}

void translate_decs(TreeNode *p) {
    	if (p->size == 3) {
		int reg = new_register();
		translate_var_local(p->children[0],reg);
		if (!strcmp(p->children[2]->data,"init {}")) {
			arr_init_cnt = 0;
			translate_args_1(p->children[2]->children[0],reg);
		}
		else {
			int tmp = new_register();
        		translate_init(p->children[2], tmp);
        		translate_assignment(reg, tmp);
		}
    	}
    	else if (p->size==2) {
		int reg = new_register();
		translate_var_local(p->children[0],reg);
		translate_decs(p->children[1]);
	}
	else if (p->size==4){
		int reg = new_register();
		translate_var_local(p->children[0],reg);
		if (!strcmp(p->children[2]->data,"init {}")) {
			arr_init_cnt = 0;
			translate_args_1(p->children[2]->children[0],reg);
		}
		else {
			int tmp = new_register(); 
        		translate_init(p->children[2], tmp);
        		translate_assignment(reg, tmp);
		}
		translate_decs(p->children[3]);
	}
	else {
		int reg = new_register();
		translate_var_local(p->children[0],reg);
	}
}

void translate_var_local(TreeNode *p,int reg) {
	if (p->size==1) {
		current_sp += 4*get_width(p->children[0]->data);
		int t = new_register();
        	quadruple_flag = 1;
        	IR_push_back(new_quadruple_arithmetic_im("add", t, stack_pointer, -current_sp + function_begin_sp));
        	quadruple_flag = 0;
        	IR_push_back(new_quadruple_move(reg, t));
		set_register_state_to_address(reg);
		//fout << "reg" << reg << endl;
		vs_id.push_back(p->children[0]->data);
		vs_reg.push_back(reg);
	}
	else translate_var_local(p->children[0],reg);
}

Quadruple new_quadruple_lw_offset(int ra, int rb, int offset) {
    Quadruple ret = new_quadruple();
    ret.op = "lw";
    ret.arguments[0].type = ADDRESS_TEMP;
    ret.arguments[0].value = ra;
    ret.arguments[1].type = ADDRESS_TEMP;
    ret.arguments[1].value = rb;
    ret.arguments[2].type = ADDRESS_CONSTANT;
    ret.arguments[2].value = offset;
    return ret;
}

void translate_stmts(TreeNode *p) {
	int i;
	if (p->size!=0) {
		translate_stmt(p->children[0]);
		translate_stmts(p->children[1]);
	}
}

void translate_stmt(TreeNode *p) {
	if (!strcmp("return stmt",p->data)) {
		int t0 = new_register(), t1 = new_register();
		translate_exps(p->children[1],t1);
		IR_push_back(new_quadruple_arithmetic_im("add", t0, stack_pointer, 4*tmp_num_var + 4));// store the return value
            	translate_assignment(t0, t1);
		IR_push_back(new_quadruple_ret());
	}
	else if (!strcmp("write stmt",p->data)) {
		int reg = new_register();// reserved for $a0 
		translate_exps(p->children[0],reg);
		catch_value_self(reg);
		IR_push_back(new_quadruple_move(-2,reg));
		if (!main_flag) {
            		IR_push_back(new_quadruple_sw_offset(retad_pointer, stack_pointer, -10000));
        	}
		IR_push_back(new_quadruple_call("printf_one"));
		if (!main_flag) {
            		IR_push_back(new_quadruple_lw_offset(retad_pointer, stack_pointer, -10000));
        	}
	}
	else if (!strcmp("read stmt",p->data)) {
		int reg = new_register();
		translate_exps(p->children[0], reg);
    		assert(get_register_state(reg) == REGISTER_STATE_ADDRESS);
		int v0 = -3;
		if (!main_flag) {
            		IR_push_back(new_quadruple_sw_offset(retad_pointer, stack_pointer, -10000));
        	}
		IR_push_back(new_quadruple_call("scanf_one"));
		if (!main_flag) {
            		IR_push_back(new_quadruple_lw_offset(retad_pointer, stack_pointer, -10000));
        	}
		IR_push_back(new_quadruple_sw(v0,reg));
		/* all exps that could be a left value could be here*/
		
	}
	else if (!strcmp("stmt: exp;",p->data)) {
		translate_exp(p->children[0]);
	}
	else if (!strcmp("if stmt",p->data)) {
		//if (a) then b; else c     ->      (a->L1) | b | goto L2 | label: L1 | c | label: L2
    		Quadruple t1 = new_quadruple_label();
   	 	Quadruple t2 = new_quadruple_label();
    		Quadruple t3 = new_quadruple_goto(get_label_number(t2));
    		translate_assignment_1(p->children[0], 0, get_label_number(t1));
    		translate_stmt(p->children[1]);
    		IR_push_back(t3);
    		IR_push_back(t1);
    		if (p->size == 3) {
        		translate_stmt(p->children[2]);
    		}
    		IR_push_back(t2);
	}
	// let's make the logic clear here: if the judgement is right, then we will just execute the following stmt until we meet the goto L2, avoid continuing executing the following; if it's false, we will go to L1 directly
	else if (!strcmp("stmt",p->data)) {
		translate_stmtblock(p->children[0]);
	}
	else if (!strcmp("for stmt",p->data)) {
		Quadruple t1 = new_quadruple_label();
    		Quadruple t2 = new_quadruple_label();
		/*for(a;b;c)d;      ->      a | (b->L3) | label L1 | d | label L2 | c | (!b->L1) | label L3*/
        	TreeNode *a = NULL, *b = NULL, *c = NULL;
		if (p->children[0]->size!=0) a = p->children[0]->children[0];
        	if (p->children[1]->size!=0) b = p->children[1]->children[0];
		if (p->children[2]->size!=0) c = p->children[2]->children[0];
        	int old_current_sp = current_sp;
        	Quadruple t3 = new_quadruple_label();
		LabelCont.push_back(t2.arguments[0].value);
        	LabelBreak.push_back(t3.arguments[0].value);
        	int reg = new_register();
        	if (a!=NULL) translate_exps(a, reg);
		if (b != NULL) {
            		translate_assignment_1(b, 0, get_label_number(t3));
        	} else {
            		IR_push_back(new_quadruple_goto(get_label_number(t3)));
        	}
        	IR_push_back(t1);
        	translate_stmt(p->children[3]);
        	IR_push_back(t2);
        	if (c!=NULL) translate_exps(c, reg);
		if (b != NULL) {
            		translate_assignment_1(b, 1, get_label_number(t1));
        	} else {
            		IR_push_back(new_quadruple_goto(get_label_number(t1)));
        	}
        	IR_push_back(t3);
        	current_sp = old_current_sp;
        	LabelCont.pop_back();
		LabelBreak.pop_back();
	}
	else if (!strcmp("cont stmt",p->data)) {
		IR_push_back(new_quadruple_goto(LabelCont.back()));
	}
	else if (!strcmp("break stmt",p->data)) {
		IR_push_back(new_quadruple_goto(LabelBreak.back()));
	}
}

void translate_assignment_1(TreeNode *p, int flag, int num) {
	int reg = new_register();
	translate_exps(p,reg);
	catch_value_self(reg);
	if (flag==0) {
		IR_push_back(new_quadruple_branch("beqz", reg, num));
	}
	else {
		IR_push_back(new_quadruple_branch("bnez", reg, num));
	}
}

void translate_exp(TreeNode *p) {
	if (p->size==0) return;
	int reg = new_register();
	translate_exps(p->children[0],reg);
}

void translate_assignment(int reg_l, int reg_r) { //problems may arise here
	catch_value_self(reg_r);
        IR_push_back(new_quadruple_sw(reg_r, reg_l));
}

void translate_args(TreeNode *p) {
	if (p->size>1)
		translate_args(p->children[1]);
	if (p->children[0]->size==0) return;
	current_sp += 4;
        int t0 = new_register();
        //fprintf(stderr,"t0 %d\n",t0);
        translate_exps(p->children[0]->children[0], t0);
        int t1 = new_register();
        set_register_state_to_address(t1);
        quadruple_flag = 1;
        IR_push_back(new_quadruple_arithmetic_im("add", t1, stack_pointer, -current_sp + function_begin_sp));//to pass value of the parameters
        quadruple_flag = 0;
        translate_assignment(t1, t0);
}

int find_array_size(char *s) {
	int tmp_level = translate_level, tmp_depth = translate_cnt[translate_level];
	while (1) {
		if (env[tmp_level][tmp_depth].table.find(s)!=env[tmp_level][tmp_depth].table.end()) { //find
			return env[tmp_level][tmp_depth].array_size_table[s][arrs_cnt-1];
		}
		else {
			int parent = env[tmp_level][tmp_depth].parent_index;
			if (parent==-1||tmp_level==0) assert(0);
			else {
				tmp_level--;
				tmp_depth = parent;
			}
		}
	}
}

void translate_arrs(TreeNode *p, int reg, char *s) {
	if (p->size==0) return;
	++arrs_cnt;
	int t0 = new_register(), t1 = new_register();
        translate_exps(p->children[0]->children[0], t0);
        catch_value_self(t0);
        catch_value_self(reg);
	//int zzy = env[translate_level][translate_cnt[translate_level]].array_size_table[s][arrs_cnt-1];
	int zzy = find_array_size(s);
        //if (p->info->type == ARRAY) {
        IR_push_back(new_quadruple_arithmetic_im("mul", t1, t0, zzy));
        set_register_state_to_value(reg);
        /*} else {
            ir_push_back(&ir, new_quadruple_arithmetic_im("mul", t1, t0, p->info->width));
            set_register_state_to_address(reg);
        }*/
        IR_push_back(new_quadruple_arithmetic("add", reg, reg, t1));
	translate_arrs(p->children[1],reg,s);
}

void translate_sdefs(TreeNode *p) {

}

void translate_exps(TreeNode *p, int reg) {
	if (p->type == _INT) {
		IR_push_back(new_quadruple_li(reg, StringToInt(p->data)));
        	set_register_state_to_value(reg);
	}
	else if (!strcmp(p->data,"=")) {
			translate_exps(p->children[0], reg);
    			int tmp = new_register();
    			translate_exps(p->children[1], tmp);
    			if (get_register_state(reg) == REGISTER_STATE_ADDRESS)
        			translate_assignment(reg, tmp);
			else report_err("lvalue error",NULL,p->line_num);
   	}
	else if (p->type== _OPERATOR) {
		translate_exps(p->children[0], reg);
		catch_value_self(reg);
    		int tmp = new_register();
    		translate_exps(p->children[1], tmp);
		catch_value_self(tmp);
		if (!strcmp(p->data,"*"))
    			IR_push_back(new_quadruple_arithmetic("mul", reg, reg, tmp));
		else if (!strcmp(p->data,"/"))
    			IR_push_back(new_quadruple_arithmetic("div", reg, reg, tmp));
		else if (!strcmp(p->data,"%"))
    			IR_push_back(new_quadruple_arithmetic("rem", reg, reg, tmp));
		else if (!strcmp(p->data,"+"))
    			IR_push_back(new_quadruple_arithmetic("add", reg, reg, tmp));
		else if (!strcmp(p->data,"-"))
    			IR_push_back(new_quadruple_arithmetic("sub", reg, reg, tmp));
		else if (!strcmp(p->data,"*"))
    			IR_push_back(new_quadruple_arithmetic("mul", reg, reg, tmp));
		else if (!strcmp(p->data,"|"))
    			IR_push_back(new_quadruple_arithmetic("or", reg, reg, tmp));
		else if (!strcmp(p->data,"&"))
    			IR_push_back(new_quadruple_arithmetic("and", reg, reg, tmp));
		else if (!strcmp(p->data,"^"))
    			IR_push_back(new_quadruple_arithmetic("xor", reg, reg, tmp));
		else if (!strcmp(p->data,"<<")) {
			IR_push_back(new_quadruple_arithmetic("sll", reg, reg, tmp));
		}
		else if (!strcmp(p->data,">>")) {
			IR_push_back(new_quadruple_arithmetic("srl", reg, reg, tmp));
		}
		else if (!strcmp(p->data,"&&")) {
			IR_push_back(new_quadruple_arithmetic("and",tmp,reg,tmp));
			IR_push_back(new_quadruple_li(reg, 1));
    			Quadruple l = new_quadruple_label();
			IR_push_back(new_quadruple_branch("bnez", tmp, get_label_number(l)));
			IR_push_back(new_quadruple_li(reg, 0));
    			IR_push_back(l);
		}
		else if (!strcmp(p->data,"||")) {
			IR_push_back(new_quadruple_arithmetic("or",tmp,reg,tmp));
			IR_push_back(new_quadruple_li(reg, 1));
    			Quadruple l = new_quadruple_label();
			IR_push_back(new_quadruple_branch("bnez", tmp, get_label_number(l)));
			IR_push_back(new_quadruple_li(reg, 0));
    			IR_push_back(l);
		}
		else {
    			IR_push_back(new_quadruple_arithmetic("sub", tmp, reg, tmp));
    			IR_push_back(new_quadruple_li(reg, 1));
    			Quadruple l = new_quadruple_label();
    			if (strcmp(p->data, ">=") == 0) {
        			IR_push_back(new_quadruple_branch("bgez", tmp, get_label_number(l)));
    			} else if (strcmp(p->data, ">") == 0) {
        			IR_push_back(new_quadruple_branch("bgtz", tmp, get_label_number(l)));
    			} else if (strcmp(p->data, "<=") == 0) {
        			IR_push_back(new_quadruple_branch("blez", tmp, get_label_number(l)));
    			} else if (strcmp(p->data,"<")==0) {
        			IR_push_back(new_quadruple_branch("bltz", tmp, get_label_number(l)));
    			} else if (strcmp(p->data,"==")==0) {
				IR_push_back(new_quadruple_branch("beqz", tmp, get_label_number(l)));
			}
			  else if (strcmp(p->data,"!=")==0) {
				IR_push_back(new_quadruple_branch("bnez", tmp, get_label_number(l)));
			}
    			IR_push_back(new_quadruple_li(reg, 0));
    			IR_push_back(l);
		}
	}
	else if (!strcmp("exps unary",p->data)) {
		translate_exps(p->children[1],reg);
		switch(p->children[0]->data[0]) {
			case '~': catch_value_self(reg);IR_push_back(new_quadruple_arithmetic_unary("not", reg, reg)); break;
			case '!': catch_value_self(reg);IR_push_back(new_quadruple_arithmetic_unary("lnot", reg, reg)); break;
			case '+': if (p->children[0]->data[1]=='+') { //++ case
        			assert(get_register_state(reg) == REGISTER_STATE_ADDRESS);
        			int t0 = new_register();// use t0 as a temporary reg to store the original address
        			IR_push_back(new_quadruple_move(t0, reg));
        			set_register_state_to_value(reg);
        			IR_push_back(new_quadruple_lw(reg, reg));
        			IR_push_back(new_quadruple_arithmetic_im("add", reg, reg, 1));
        			IR_push_back(new_quadruple_sw(reg, t0));
			}
			break;
			case '-': if (p->children[0]->data[1]=='-') { //-- case
				assert(get_register_state(reg) == REGISTER_STATE_ADDRESS);
        			int t0 = new_register();
        			IR_push_back(new_quadruple_move(t0, reg));
        			set_register_state_to_value(reg);
        			IR_push_back(new_quadruple_lw(reg, reg));
        			IR_push_back(new_quadruple_arithmetic_im("add", reg, reg, -1));
        			IR_push_back(new_quadruple_sw(reg, t0));
			} else {
				catch_value_self(reg);
				IR_push_back(new_quadruple_arithmetic_unary("neg", reg, reg)); break;
			}
			break;
			default: assert(0); break;
		}
	}
	else if (!strcmp("exps ()",p->data)) {
		translate_exps(p->children[0],reg);
	}
	else if (!strcmp("exps f()",p->data)) {//function here
		current_sp += 8;
		quadruple_flag = 1;
		if (!main_flag) {
            		IR_push_back(new_quadruple_sw_offset(retad_pointer, stack_pointer, -current_sp + function_begin_sp));
        	}
        	quadruple_flag = 0;
        	translate_args(p->children[1]);
        	quadruple_flag = 1;
        	IR_push_back(new_quadruple_arithmetic_im("add", stack_pointer, stack_pointer, -current_sp + function_begin_sp));
        	quadruple_flag = 0;
            	IR_push_back(new_quadruple_call(p->children[0]->data));
        	quadruple_flag = 1;
        	IR_push_back(new_quadruple_arithmetic_im("sub", stack_pointer, stack_pointer, -current_sp + function_begin_sp));
        	quadruple_flag = 0;
        	current_sp -= func_table[p->children[0]->data][0]*4;
        	quadruple_flag = 1;
        	if (!main_flag) {
            		IR_push_back(new_quadruple_lw_offset(retad_pointer, stack_pointer, -current_sp + function_begin_sp));
        	}
        	quadruple_flag = 0;
        	current_sp -= 4;
            	set_register_state_to_address(reg);
            	int t = new_register();
            	set_register_state_to_address(t);
            	quadruple_flag = 1;
            	IR_push_back(new_quadruple_arithmetic_im("add", t, stack_pointer, -current_sp + function_begin_sp));
            	quadruple_flag = 0;
            	IR_push_back(new_quadruple_move(reg, t));
            	set_register_state_to_address(reg);
            	current_sp -= 4;
	}
	else if (!strcmp("exps arr",p->data)) { //id here
		int tmp = vs_fetch_register(p->children[0]->data);
		if (get_register_state(tmp) == REGISTER_STATE_ADDRESS) {
			IR_push_back(new_quadruple_move(reg, tmp));
                	if (get_width(p->children[0]->data)!=1) { // an variable of type array
				arrs_cnt = 0;
                    		translate_arrs(p->children[1],reg,p->children[0]->data);
				set_register_state_to_address(reg);
                	} else { // an varaible of type int
                    		set_register_state_to_address(reg);
                	}
            	} else {
		       //array pointer
                //set_register_state_to_value(reg);
                //IR_push_back(new_quadruple_lw(reg, tmp));
            	}
	}
	else if (!strcmp("exps struct",p->data)) { //dot op
		int tmp = vs_fetch_register(p->children[0]->data);
		IR_push_back(new_quadruple_move(reg, tmp));
		set_register_state_to_address(reg);
		int offset = 4*cal_offset(p->children[0]->data,p->children[2]->data);
		IR_push_back(new_quadruple_arithmetic_im("add", reg, reg, offset));
            	set_register_state_to_address(reg);
	}
	else { //all kinds of assign expressions
		translate_exps(p->children[0],reg);
		int tmp = new_register();
    		translate_exps(p->children[1], tmp);
		assert(get_register_state(reg) == REGISTER_STATE_ADDRESS);
		int t = new_register();
		char* op;
		switch (p->data[0]) {
            		case '*': op = "mul"; break;
            		case '/': op = "div"; break;
            		case '%': op = "rem"; break;
            		case '+': op = "add"; break;
            		case '-': op = "sub"; break;
            		case '<': op = "sll"; break;
            		case '>': op = "srl"; break;
            		case '&': op = "and"; break;
            		case '^': op = "xor"; break;
            		case '|': op = "or"; break;
			default: {fprintf(stderr,"p->data %s\n",p->data);assert(0); break;}
        	}
		assert(get_register_state(reg) == REGISTER_STATE_ADDRESS);
        	IR_push_back(new_quadruple_lw(t, reg));
        	catch_value_self(tmp);  //problems may arise here
        	IR_push_back(new_quadruple_arithmetic(op, t, t, tmp));
        	IR_push_back(new_quadruple_sw(t, reg));
	}
}
#endif
