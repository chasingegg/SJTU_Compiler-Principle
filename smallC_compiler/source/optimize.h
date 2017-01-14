//This file does some optimizations and allocate registers.
#ifndef FILE_OPTIMIZE_H
#define FILE_OPTIMIZE_H

#include "def.h"
#include "translate.h"
#define MAX_REGISTER 100001
int* next_pos;
int* prev_pos;
int real_register_begin = 11,real_register_end = 26,last_pos[MAX_REGISTER];
int register_liveness_begin[MAX_REGISTER];
int register_liveness_end[MAX_REGISTER];

typedef struct LinkedListNode {
    int value;
    struct LinkedListNode* next;
} LinkedListNode;

typedef struct {
    LinkedListNode* begin;
    LinkedListNode* end;
} LinkedList;

int* label_to_ins;
LinkedListNode* null;
LinkedList** in;
LinkedList** out;
LinkedList** def;
LinkedList** use;

LinkedListNode* new_linked_list_node() {
    return (LinkedListNode*)malloc(sizeof(LinkedListNode));
}

LinkedList** new_linked_list_n(int n) {
    return (LinkedList**)malloc(sizeof(LinkedList*) * n);
}

LinkedList* new_linked_list() {
    LinkedList* ptr = (LinkedList*)malloc(sizeof(LinkedList));
    ptr->begin = null;
    ptr->end = null;
    return ptr;
}

void linked_list_insert(LinkedList* ptr, int value) {
    if (value == stack_pointer || value == retad_pointer) {
        return;
    }
    LinkedListNode* itr = new_linked_list_node();
    itr->value = value;
    itr->next = ptr->begin;
    ptr->begin = itr;
}

int linked_list_find(LinkedList* ptr, int value) {
    LinkedListNode* itr;
    for (itr = ptr->begin; itr != ptr->end; itr = itr->next) {
        if (itr->value == value) {
            return 1;
        }
    }
    return 0;
}

int cmp(const void* a, const void* b) {
    int x = *(int*)a;
    int y = *(int*)b;
    if (register_liveness_begin[x] != register_liveness_begin[y]) {
        return register_liveness_begin[x] < register_liveness_begin[y] ? -1 : 1;
    }
    if (register_liveness_end[x] != register_liveness_end[y]) {
        return register_liveness_end[x] < register_liveness_end[y] ? -1 : 1;
    }
    return 0;
}

int optimize_merge_set(LinkedList* x, LinkedList* y) {
    int ret = 0;
    LinkedListNode* itr;
    for (itr = y->begin; itr != y->end; itr = itr->next) {
        if (!linked_list_find(x, itr->value)) {             //TO_SPEED_UP, using timestamp
            ret = 1;
            linked_list_insert(x, itr->value);
        }
    }
    return ret;
}

int optimize_merge_set_1(LinkedList* x, LinkedList* y, LinkedList* z) {
    static int visited[MAX_REGISTER] = {}, timestamp = 0;
    timestamp++;
    int ret = 0;
    LinkedListNode* itr;
    for (itr = x->begin; itr != x->end; itr = itr->next) {
        visited[itr->value] = timestamp;
    }
    for (itr = z->begin; itr != x->end; itr = itr->next) {
        visited[itr->value] = timestamp;
    }
    for (itr = y->begin; itr != y->end; itr = itr->next) {
        if (visited[itr->value] != timestamp) {
            ret = 1;
            linked_list_insert(x, itr->value);
        }
    }
    return ret;
}

int optimize_calc_in_out(int i, int j) {
    int k, ret = 0;
    for (k = j; k >= i; k = prev_pos[k - 1]) {
        Quadruple t = IR[k];
        if (t.op=="ret" || t.op=="func") {
        } else if (t.op=="goto") {
            ret |= optimize_merge_set(out[k], in[label_to_ins[t.arguments[0].value]]);
        } else if (t.op=="beqz" ||
                   t.op=="bnez" ||
                   t.op=="bgez" ||
                   t.op=="bgtz" ||
                   t.op=="blez" ||
                   t.op=="bltz") {
            ret |= optimize_merge_set(out[k], in[next_pos[k + 1]]);
            ret |= optimize_merge_set(out[k], in[label_to_ins[t.arguments[1].value]]);
        } else {
            ret |= optimize_merge_set(out[k], in[next_pos[k + 1]]);
        }
        ret |= optimize_merge_set_1(in[k], out[k], def[k]);
        //fprintf(stderr, "k=%d\n", k);
    }
    return ret;
}

void optimize_register_allocate(int i, int j) {
    static int which[32];
    memset(which, -1, sizeof(which));
    static int used[MAX_REGISTER] = {}, used_timestamp = 0;
    used_timestamp++;
    static int list[MAX_REGISTER], n;
    n = 0;
    static int loaded[MAX_REGISTER];
    static int real[MAX_REGISTER];
    int k, l;
    for (k = i; k <= j; k = next_pos[k + 1]) {
        Quadruple t = IR[k];
        for (l = 0; l < 3; l++) {
            if (t.arguments[l].type == ADDRESS_TEMP) {
                int x = t.arguments[l].value;
                real[x] = -1;
                if (x != stack_pointer && x != retad_pointer && used[x] != used_timestamp) {
                    used[x] = used_timestamp;
                    list[n++] = x;
                    register_liveness_begin[x] = k;
                    register_liveness_end[x] = k;
                }
            }
        }
    }
    for (k = i; k <= j; k = next_pos[k + 1]) {
        LinkedListNode* itr;
        for (itr = in[k]->begin; itr != in[k]->end; itr = itr->next) {
            int x = itr->value;
            if (k < register_liveness_begin[x]) {
                register_liveness_begin[x] = k;
            }
            if (k > register_liveness_end[x]) {
                register_liveness_end[x] = k;
            }
        }
        for (itr = out[k]->begin; itr != out[k]->end; itr = itr->next) {
            int x = itr->value;
            if (k < register_liveness_begin[x]) {
                register_liveness_begin[x] = k;
            }
            if (k > register_liveness_end[x]) {
                register_liveness_end[x] = k;
            }
        }
    }
    qsort(list, n, sizeof(int), cmp);
    for (k = 0; k < n; k++) {
        int x = list[k];
        int p = -1;
        for (l = real_register_begin; l < real_register_end; l++) {
            if (which[l] == -1) {
                p = l;
                break;
            }
            if (register_liveness_end[which[l]] < register_liveness_begin[x]) {
                p = l;
                break;
            }
        }
        if (p == -1) {
            p = real_register_begin;
            for (l = real_register_begin + 1; l < real_register_end; l++) {
                if (register_liveness_end[which[l]] > register_liveness_end[which[p]]) {
                    p = l;
                }
            }
            real[which[p]] = -1;
        }
        real[which[p] = x] = p;
        loaded[x] = 0;
    }
    for (k = 0; k < n; k++) {
        int x = list[k];
        //fprintf(stderr, "t%d %d %d->%d\n", x, real[x], register_liveness_begin[x], register_liveness_end[x]);
    }
    for (k = i; k <= j; k = next_pos[k + 1]) {
        Quadruple t = IR[k];
        for (l = 0; l < 3; l++) {
            if (t.arguments[l].type == ADDRESS_TEMP) {
                int x = t.arguments[l].value;
                if (real[x] != -1) {
                    IR[k].arguments[l].real = real[x];
		    //fprintf(stderr, "real[%d] %d\n",x,real[x]);
                    if (RegisterOffset[x] == -1 && !loaded[x]) {
                        loaded[x] = 1;
                        IR[k].arguments[l].needload = register_liveness_begin[x] == i + 1;
                    }
                    if (register_liveness_end[x] == k) {
                        IR[k].arguments[l].needclear = 1;
                    }
                }
            }
        }
    }
}

void update_pos() {
    int i;
    next_pos[IR.size()] = IR.size();
    for (i = IR.size() - 1; i >= 0; i--) {
        if (IR[i].active) {
            next_pos[i] = i;
        } else {
            next_pos[i] = next_pos[i + 1];
        }
    }
    prev_pos[-1] = -1;
    for (i = 0; i < IR.size(); i++) {
        if (IR[i].active) {
            prev_pos[i] = i;
        } else {
            prev_pos[i] = prev_pos[i - 1];
        }
    }
    /*for (i = 0; i < IR.size();++i) {
		fprintf(stderr, "prev_pos[%d] %d\n",i,prev_pos[i]);
	}*/
}

void optimize_1() { //through observation
    int i, j;
    Quadruple x, y;
    for (i = next_pos[0]; i < IR.size(); i = next_pos[i + 1]) {
        x = IR[i];
        if (!x.active || x.op!="move") {
            continue;
        }
        int value = x.arguments[0].value;
	if (value==-2) continue;
        int active_flag = 1;
        for (j = next_pos[i + 1]; j < IR.size(); j = next_pos[j + 1]) {
            y = IR[j];
            if (y.op=="goto" || y.op=="label") {
                active_flag = 2;
                break;
            }
            if (y.op=="sw") {
                if (y.arguments[1].type == ADDRESS_TEMP && y.arguments[1].value == value) {
                    active_flag = 2;
                    break;
                }
            }
            if (y.arguments[1].type == ADDRESS_TEMP && y.arguments[1].value == value) { // rundundant instructions
                y.arguments[1].value = x.arguments[1].value;
            }
            if (y.arguments[2].type == ADDRESS_TEMP && y.arguments[2].value == value) {
                y.arguments[2].value = x.arguments[1].value;
            }
            if (y.arguments[0].type == ADDRESS_TEMP && y.arguments[0].value == value) {
                active_flag = 0;
                break;
            }
        }
        if (active_flag == 2) {
            continue;
        }
	//fprintf(stderr,"active_flag %d\n",active_flag);
        //只要会被重新赋值, 就可以扔掉
        int flag = active_flag || (y.op!="sw");
        if (flag) {
            IR[i].active = 0;
        }
    }
    update_pos();
}

void optimize_2() {
    int i, j;
    Quadruple x, y;
    for (i = next_pos[0]; i < IR.size(); i = next_pos[i + 1]) {
        x = IR[i];
        if (!x.active || x.op!="li") {
            continue;
        }
	if (x.arguments[0].value==-2) continue;
        j = next_pos[i + 1];
        if (j < IR.size()) {
            int value = x.arguments[0].value;
            y = IR[j];
            if (y.arguments[0].type == ADDRESS_TEMP && y.arguments[0].value == value && //avoid redundant lis!
                y.arguments[2].type == ADDRESS_TEMP && y.arguments[2].value == value) {
                IR[i].active = 0;
                IR[j].arguments[2].type = ADDRESS_CONSTANT;
                IR[j].arguments[2].value = x.arguments[1].value;
            }
        }
    }
    update_pos();
}

void optimize_3() {
    int i, j;
    Quadruple x, y;
    for (i = 0; i < IR.size(); i++) {
        if (!IR[i].active) {
            continue;
        }
        x = IR[i];
        for (j = 0; j < 3; j++) {
            if (x.arguments[j].type == ADDRESS_TEMP) {
                last_pos[x.arguments[j].value] = i;
            }
        }
    }
    for (i = 0; i < IR.size(); i++) {
        x = IR[i];
        if (!x.active || x.op!="li") {
            continue;
        }
	if (x.arguments[0].value==-2) continue;
        j = next_pos[i + 1];
        if (j < IR.size()) {
            int value = x.arguments[0].value;
            y = IR[j];
            if (y.arguments[2].type == ADDRESS_TEMP && y.arguments[2].value == value) {//avoid redundant lis!
                if (last_pos[value] == j) {
                    IR[i].active = 0;
                    IR[j].arguments[2].type = ADDRESS_CONSTANT;
                    IR[j].arguments[2].value = x.arguments[1].value;
                }
            }
        }
    }
    update_pos();
}

void optimize_4() {
    label_to_ins = (int*)malloc(sizeof(int) * IR.size());
    null = new_linked_list_node();
    null->next = null;
    in = new_linked_list_n(IR.size() + 1);
    out = new_linked_list_n(IR.size() + 1);
    def = new_linked_list_n(IR.size() + 1);
    use = new_linked_list_n(IR.size() + 1);
    int i, j, k;
    for (i = prev_pos[IR.size() - 1]; i >= 0; i = prev_pos[j - 1]) {
        for (j = i; j >= 0; j = prev_pos[j - 1]) {
            if (IR[j].op=="func") {
                break;
            }
        }
        assert(j >= 0);
        for (k = i; k >= j; k = prev_pos[k - 1]) {
            in[k] = new_linked_list();
            out[k] = new_linked_list();
            def[k] = new_linked_list();
            use[k] = new_linked_list();
            Quadruple t = IR[k];
            if (t.op=="label") {
                label_to_ins[t.arguments[0].value] = k;
            } else if (t.op=="func") {
            } else if (t.op=="call") {
            } else if (t.op=="ret") {
            } else if (t.op=="goto") {
            } else if (t.op=="beqz" ||
                       t.op=="bnez" ||
                       t.op=="bgez" ||
                       t.op=="bgtz" ||
                       t.op=="blez" ||
                       t.op=="bltz") {
                linked_list_insert(use[k], t.arguments[0].value);
                linked_list_insert(in[k], t.arguments[0].value);
            } else if (t.op=="add" ||
                       t.op=="sub" ||
                       t.op=="mul" ||
                       t.op=="div" ||
                       t.op=="rem" ||
                       t.op=="or" ||
                       t.op=="xor" ||
                       t.op=="and" ||
                       t.op=="sll" ||
                       t.op=="srl") {
                if (t.arguments[2].type == ADDRESS_TEMP) {
                    linked_list_insert(use[k], t.arguments[2].value);
                    linked_list_insert(in[k], t.arguments[2].value);
                }
                linked_list_insert(use[k], t.arguments[1].value);
                linked_list_insert(in[k], t.arguments[1].value);
                int flag = t.arguments[1].value != t.arguments[0].value;
                if (t.arguments[2].type == ADDRESS_TEMP) {
                    flag &= t.arguments[2].value != t.arguments[0].value;
                }
                if (flag) {
                    linked_list_insert(def[k], t.arguments[0].value);
                }
            } else if (t.op=="neg" ||
                       t.op=="not" ||
                       t.op=="lnot" ||
                       t.op=="lw") {
                linked_list_insert(use[k], t.arguments[1].value);
                linked_list_insert(in[k], t.arguments[1].value);
                if (t.arguments[1].value != t.arguments[0].value) {
                    linked_list_insert(def[k], t.arguments[0].value);
                }
            } else if (t.op=="li") {
                linked_list_insert(def[k], t.arguments[0].value);
            } else if (t.op=="sw") {
                linked_list_insert(use[k], t.arguments[0].value);
                linked_list_insert(in[k], t.arguments[0].value);
                linked_list_insert(use[k], t.arguments[1].value);
                linked_list_insert(in[k], t.arguments[1].value);
            } else if (t.op=="move") {
                linked_list_insert(use[k], t.arguments[1].value);
                linked_list_insert(in[k], t.arguments[1].value);
                if (t.arguments[1].value != t.arguments[0].value) {
                    linked_list_insert(def[k], t.arguments[0].value);
                }
            } else {
		cout << t.op << endl;
                assert(0);
            }
            //fprintf(stderr, "k=%d\n",k);
        }
        while (optimize_calc_in_out(j, i));
        optimize_register_allocate(j, i);
    }
}


void optimize() {
	next_pos = (int*)malloc(sizeof(int) * (IR.size() + 1));
    	prev_pos = (int*)malloc(sizeof(int) * (IR.size() + 1)) + 1;
	update_pos();
	//optimize_1();
	optimize_2();
	optimize_3();
	optimize_4();
}

#endif
