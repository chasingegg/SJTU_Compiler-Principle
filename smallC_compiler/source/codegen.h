/*
  Author: Chao Gao
  file: codegen.h
  /This program interprets intermediate representation into MIPS codes.
*/
#ifndef FILE_CODEGEN_H
#define FILE_CODEGEN_H

#include "def.h"
#include "tree.h"
#include "node.h"
#include "intermediate.h"
#include "optimize.h"
#define MAX_LENGTH (10000)

char* tempname = "temp";
char* varname = "var";
char* labelname = "l";
char ibuffer[MAX_LENGTH];
int in_main;
int which[32];

int get_register_id(Address ptr, int k) {
    if (ptr.value == -2) {
 	return 4;
    }
    if (ptr.value == -3) {
 	return 2;
    }
    if (ptr.real != -1) {
        return ptr.real;
    }
    if (ptr.value == stack_pointer) {
        return 29;
    }
    if (ptr.value == retad_pointer) {
        return 31;
    }
    if (ptr.value == -3) {
	return 2;
    }
    return k;
}

int interprete_fetch_register(Address ptr, int k) {
    if (ptr.value == -2) {
 	return 4;
    }
    if (ptr.value == -3) {
 	return 2;
    }
    if (ptr.real != -1) {
        return ptr.real;
    }
    int reg = ptr.value;
    if (ptr.value == -3) {
	return 2;
    }
    if (reg == stack_pointer) {
        return 29;
    } else if (reg == retad_pointer) {
        return 31;
    } else {
        if (RegisterOffset[reg] == -1) {
            printf("lw $%d, %d($a3)\n", k, reg * 4);
        } else {
            printf("lw $%d, %d($sp)\n", k, -RegisterOffset[reg]);
        }
    }
    return k;
}

void interprete_store_register(char* s, int reg) {
    if (reg == -2) {
 	printf("move $a0, %s\n", s);
    }
    if (reg == -3) {
	printf("move $v0, %s\n", s);
    }
    if (reg == stack_pointer) {
        printf("move $sp, %s\n", s);
    } else if (reg == retad_pointer) {
        printf("move $ra, %s\n", s);
    } 
	else {
        if (RegisterOffset[reg] == -1) {
            printf("sw %s, %d($a3)\n", s, reg * 4);
        } else {
            printf("sw %s, %d($sp)\n", s, -RegisterOffset[reg]);
        }
    }
}

void interprete_global() {
    	printf(".data\n");
    	printf("%s: .space %d\n", tempname, RegisterState.size() * 4);
    	int i;
    	int count = 0;
    	for (i = 0; i < GIR.size(); i++) {
        	if (GIR[i].op=="gnew") {
            		printf("%s%d: .space %d\n", varname, count++, GIR[i].arguments[0].value);
            	//move to register
        	} else {
            		printf("%s%d: .asciiz \"%s\"\n", varname, count++, GIR[i].arguments[0].name.c_str());
            		printf(".align 2\n");
            		//move to register
        	}
    	}
}

void interprete_generate_mips(Quadruple tuple,int delta) {
	//cout << "tuple.op ";
	//cout << tuple.op << endl;
	string s = tuple.op;
	int i;
	for (i = 0; i < 3; i++) {
        	if (tuple.arguments[i].real != -1&&tuple.arguments[i].value!=-1) {
            		which[tuple.arguments[i].real] = tuple.arguments[i].value;
        	}
        	if (tuple.arguments[i].needload) {
            		printf("lw $%d, %d($a3)\n", tuple.arguments[i].real, 4 * tuple.arguments[i].value);
        	}
    }
	if (s=="label") {
        	printf("%s%d:\n", labelname, tuple.arguments[0].value);
    	} else if (s=="func") { 
        	memset(which, -1, sizeof(which));
        	printf("%s:\n", tuple.arguments[0].name.c_str());
	} else if (s=="ret") {
        	if (in_main) {
            		printf("j __program_end\n");
        	} else {
            		printf("jr $ra\n");
        	}
    	} else if (s=="goto") {
        	printf("j %s%d\n", labelname, tuple.arguments[0].value);
	} else if (s=="beqz"||s=="bnez" || s=="bgez" || s=="bgtz" || s=="blez" || s=="bltz") {
        	int src = interprete_fetch_register(tuple.arguments[0], 8); //$t0
        	printf("%s $%d, %s%d\n", s.c_str(), src, labelname, tuple.arguments[1].value);
    	} else if (s=="add"||s=="sub"||s=="mul"||s=="div"||s=="rem"||s=="or"||s=="xor"||s=="and"||s=="sll"||s=="srl" ) {
        		int dest = get_register_id(tuple.arguments[0], 8);
        		int src1 = interprete_fetch_register(tuple.arguments[1], 9);
        		if (tuple.arguments[2].type == ADDRESS_CONSTANT) {
            			printf("%s $%d, $%d, %d\n", s.c_str(), dest, src1, tuple.arguments[2].value);
        		} else {
            			int src2 = interprete_fetch_register(tuple.arguments[2], 10);
            			printf("%s $%d, $%d, $%d\n", s.c_str(), dest, src1, src2);
        		}
        		if (dest == 8) {
            			interprete_store_register("$8", tuple.arguments[0].value);
        		}
    	} else if (s=="neg"|| s=="not") {
        	int src = interprete_fetch_register(tuple.arguments[1], 9);
        	int dest = get_register_id(tuple.arguments[0], 8);
       		printf("%s $%d, $%d\n", s.c_str(), dest, src);
        	if (dest == 8) {
            		interprete_store_register("$8", tuple.arguments[0].value);
        	}
    	} else if (s=="lnot") {
        	static int lnot_count = 0;
        	int src = interprete_fetch_register(tuple.arguments[1], 9);
        	int dest = get_register_id(tuple.arguments[0], 8);
        	if (dest != src) {
            		printf("li $%d, 1\n", dest);
            		printf("beqz $%d, lnot%d\n", src, lnot_count);
            		printf("li $%d, 0\n", dest);
            		printf("lnot%d:\n", lnot_count);
            		if (dest == 8) {
                		interprete_store_register("$8", tuple.arguments[0].value);
            		}
        	} else {
            		printf("move $a1, $%d\n", dest);
            		printf("li $%d, 1\n", dest);
            		printf("beqz $a1, lnot%d\n", lnot_count);
            		printf("li $%d, 0\n", dest);
            		printf("lnot%d:\n", lnot_count);
            		if (dest == 8) {
                		interprete_store_register("$8", tuple.arguments[0].value);
            		}
        	}
        	lnot_count++;
    	} else if (s=="li") {
        	int dest = get_register_id(tuple.arguments[0], 8);
        	printf("li $%d, %d\n", dest, tuple.arguments[1].value);
        	if (dest == 8) {
            		interprete_store_register("$8", tuple.arguments[0].value);
       	 }
    	} else if (s=="lw") {
        	int src = interprete_fetch_register(tuple.arguments[1], 9);
        	int dest = get_register_id(tuple.arguments[0], 8);
        	printf("%s $%d, %d($%d)\n", s.c_str(), dest, tuple.arguments[2].value, src);
        	if (dest == 8) {
            		interprete_store_register("$8", tuple.arguments[0].value);
        	}
    	} else if (s=="sw") {
        	int src = interprete_fetch_register(tuple.arguments[0], 8);
       	 	int dest = interprete_fetch_register(tuple.arguments[1], 9);
        	printf("%s $%d, %d($%d)\n", s.c_str(), src, tuple.arguments[2].value, dest);
    	} else if (s=="move") {
        	int src = interprete_fetch_register(tuple.arguments[1], 9);
        	int dest = get_register_id(tuple.arguments[0], 8);
        	printf("move $%d, $%d\n", dest, src);
        	if (dest == 8) {
            	interprete_store_register("$8", tuple.arguments[0].value);
        	}
    	} else if (s=="call") {
		int flag = tuple.arguments[0].name!="__scanf_one";
        	flag &= tuple.arguments[0].name!="__printf_one";
        	if (flag) {
            		for (i = real_register_begin; i < real_register_end; i++) {
				//fprintf(stderr,"which [%d] %d \n",i,which[i]);
                		if (which[i] != -1) {	
                    			int x = which[i];
                    			assert(x != stack_pointer && x != retad_pointer);
                    			if (RegisterOffset[x] == -1) {
                        			printf("sw $%d, %d($a3)\n", i, x * 4);
                    			} else {
                       		 		printf("sw $%d, %d($sp)\n", i, delta - RegisterOffset[x]);
                    			}
                		}
            		}
        	}
        	printf("jal %s\n", tuple.arguments[0].name.c_str());
        	if (flag) {
            		for (i = real_register_begin; i < real_register_end; i++) {
                		if (which[i] != -1) {
                    			int x = which[i];
                    			if (RegisterOffset[x] == -1) {
                        			printf("lw $%d, %d($a3)\n", i, x * 4);
                    			} else {
                        			printf("lw $%d, %d($sp)\n", i, delta - RegisterOffset[x]);
                    			}
                		}
            		}
        	}
	}
	else {
        	assert(0);
    	}
	for (i = 0; i < 3; i++) {
        if (tuple.arguments[i].needclear) {
            which[tuple.arguments[i].real] = -1;
        }
    }
}

void interprete_local() {
	printf("\n.text\n");
	FILE* builtin = fopen("w_r.s", "r");
    	while (fgets(ibuffer, MAX_LENGTH, builtin)) {
        	printf("%s", ibuffer);
    	}
	int i = 0;
	in_main = 0;
	for (i = 0; i < IR.size(); ++i) {
		if (!IR[i].active) continue;
		string op = IR[i].op;
		int delta = 0;
		if (op=="call"&&IR[i].arguments[0].name!="__printf_one"&&IR[i].arguments[0].name!="__scanf_one") {
            		assert(IR[prev_pos[i - 1]].arguments[2].type == ADDRESS_CONSTANT);
            		delta = -IR[prev_pos[i - 1]].arguments[2].value;
           		 //fprintf(stderr, "delta = %d\n", delta);
        	}
		interprete_generate_mips(IR[i],delta);
		if (op=="func") {
            		if (IR[i].arguments[0].name=="main") {
                		printf("la $a3, %s\n", tempname);
                		in_main = 1;
                		int j = i;
                		int count = 0;
                		for (i = 0; i < GIR.size(); i++) {
                    			printf("la $t0, %s%d\n", varname, count);
                    			printf("sw $t0, %d($a3)\n", GIR[i].arguments[1].value * 4);
                    			count++;
                		}
               		 for (i = 0; i < MIR.size(); i++) {
                    		interprete_generate_mips(MIR[i], -1);
                	 }
                	 i = j;
            		 }
        	} 
	}
	puts("__program_end:");
    	puts("li $v0, 10");
    	puts("syscall");
}

void interpret() {
	interprete_global();
	interprete_local();
}


#endif
