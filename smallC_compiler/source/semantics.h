/*
  Author: Chao Gao
  file: semantics.h
  This program checks semantic errors.
*/
#ifndef FILE_SEMANTICS_H
#define FILE_SEMANTICS_H
#include <vector>
#include <map>
#include "def.h"
#include "node.h"
#include "tree.h"

using namespace std;
#define MAXSIZE 200

struct ptr_cmp { //map compare char[] with their address; 
	bool operator()(const char* s1, const char* s2) const {
		return strcmp(s1,s2) < 0;
	}
};
struct SymbolTable {
	map <char*, char*,ptr_cmp> table;
	map <char*, vector<char *>, ptr_cmp > struct_table;
	map <char*, vector<char *>, ptr_cmp > struct_id_table;
	map < char*, int, ptr_cmp > struct_name_width_table;
        map <char*, int, ptr_cmp> width_table; // records the num of "int"s, the actual width should be 4 times of that
	map <char*, vector<int>, ptr_cmp> array_size_table;
	int parent_index;//which record the index of his parent in the upper level; -1 means it's the gloabl scope
} env[MAXSIZE][MAXSIZE];
int level = 0,cnt[MAXSIZE];// keep track num of symbol tables of this level
map <char*, vector<int>, ptr_cmp > func_table;
map <char*, int, ptr_cmp> func_cnt_table;
map <char*,map<int,char*>, ptr_cmp > func_overload_table;
vector <int> tmp_array_size_vector;
vector <int> func_vector[MAXSIZE];
vector <char*> struct_vector[MAXSIZE];
set <char*, ptr_cmp> func_set[MAXSIZE], struct_set[MAXSIZE];
int func_cnt = 0, struct_cnt = 0, tmp_num_of_var = 0, func_vector_cnt = 0, struct_vector_cnt = 0, tmp_num = 1, tmp_num_struct = 0;
bool in_func = false, in_for = false, bool_main = false;//to help check classify whether a 'return' happens in a func

int StringToInt(char *s);
bool isReserved(char* s);
void report_err(char *s1,char *s2, int linenum);
void semantics_check(TreeNode* p);
void semantics_check_program(TreeNode* p);
void semantics_check_extdefs(TreeNode* p);
void semantics_check_init(TreeNode *p);
void semantics_check_exp(TreeNode *p, char *s);
void semantics_check_extdef(TreeNode* p);
void semantics_check_sextvars(TreeNode *p);
void semantics_check_stspec(TreeNode* p);
void semantics_check_paras(TreeNode *p,char *s);
char* semantics_check_type(TreeNode *p);
void semantics_check_func(TreeNode *p);
void semantics_check_extvars(TreeNode *p);
bool semantics_check_id(char* s,int num);
void semantics_check_var(TreeNode *p);
void semantics_check_stmtblock(TreeNode *p);
void semantics_check_decs(TreeNode *p);
void semantics_check_defs(TreeNode* p);
void semantics_check_sdefs(TreeNode *p);
void semantics_check_sdecs(TreeNode *p, int in);
void semantics_check_stmts(TreeNode* p);
void semantics_check_arrs(TreeNode *p);
void semantics_check_args(TreeNode *p, char *s);
void semantics_check_exps(TreeNode *p);
void check_left_value_exps(TreeNode *p);
bool find_id(TreeNode *p);
int find_struct_id(char* s, char*s2);
void semantics_check_stmt(TreeNode *p);
void semantics(TreeNode *p);

int get_width_semantics(char* s) {
	int tmp_level = level, tmp_depth = cnt[level];
	while (1) {
		if (env[tmp_level][tmp_depth].table.find(s)!=env[tmp_level][tmp_depth].table.end()) { //find
			return env[tmp_level][tmp_depth].width_table[s];
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

int StringToInt(char *s) {
	assert(s!=NULL);
	if (strlen(s)>=2&&s[0]=='0'&&(s[1]=='x'||s[1]=='X')) return strtol(s,NULL,16);
	if (strlen(s)>=2&&s[0]=='0'&&s[1]!='x'&&s[1]!='X') return strtol(s,NULL,8);
	return strtol(s,NULL,10);	
}

bool isReserved(char* s) {
	if (strcmp(s,"int")==0||strcmp(s,"return")==0||strcmp(s,"break")==0||strcmp(s,"for")==0||strcmp(s,"if")==0)
		return true;
	if (strcmp(s,"else")==0||strcmp(s,"continue")==0||strcmp(s,"struct")==0||strcmp(s,"read")==0||strcmp(s,"write")==0) return true;
	return false;
}

void report_err(char *s1, char *s2, int linenum) {
	fprintf(stderr,"at line %d:\t",linenum);
	if (s1!=NULL) fprintf(stderr,"%s ",s1);
	if (s2!=NULL) fprintf(stderr,"%s ",s2);
	fprintf(stderr,"\n");
	exit(1);
}

void semantics_check(TreeNode* p) {
	switch (p->type) {
		case _PROGRAM: semantics_check_program(p); break;
		case _STMTBLOCK: semantics_check_stmtblock(p); break;
		case _EXTDEFS: semantics_check_extdefs(p); break;
		case _EXTDEF: semantics_check_extdef(p); break;
		case _SEXTVARS: semantics_check_sextvars(p); break;
		case _EXTVARS: semantics_check_extvars(p); break;
		case _STSPEC: semantics_check_stspec(p); break;
		case _FUNC: semantics_check_func(p); break;
		case _TYPE: semantics_check_type(p); break;
		case _STMTS: semantics_check_stmts(p); break;
		case _STMT: semantics_check_stmt(p); break;
		case _DEFS: semantics_check_defs(p); break;
		case _SDEFS: semantics_check_sdefs(p); break;
		case _DECS: semantics_check_decs(p); break;
		case _VAR: semantics_check_var(p); break;
		case _INIT: semantics_check_init(p); break;
		case _EXP: semantics_check_exp(p,NULL); break;
		case _EXPS: semantics_check_exps(p); break;
		case _ARRS: semantics_check_arrs(p); break;
		case _OPERATOR: break;
		case _INT: break;
		case _SDECS: semantics_check_sdecs(p,0); break;
		case _NULL: break;//do nothing
		default: fprintf(stderr,"%s \n", p->data); assert(0);
	}
}

void semantics_check_program(TreeNode* p) {//OK
	int i;
	for (i = 0; i < p->size; ++i) {
		semantics_check(p->children[i]);
	}
}

void semantics_check_extdefs(TreeNode* p) {//OK
	int i;
	for (i = 0; i < p->size; ++i) {
		semantics_check(p->children[i]);
	}
}

void semantics_check_init(TreeNode *p) {
	if (!strcmp(p->data,"init")) {
		semantics_check_exps(p->children[0]);
	}
	else {
		semantics_check_args(p,NULL); //array initialization
	}
}

void semantics_check_exp(TreeNode *p,char* s) {
	int i;
	if (s!=NULL) {
		if (p->size==0);
		else {++func_cnt_table[s];}
	}
	if (p->size>0) {
		semantics_check_exps(p->children[0]);
	}
}

void semantics_check_sextvars_1(char *s, TreeNode *p) {
	if (p->size==0) return;
	int tmp_level = level, tmp_depth = cnt[level];
	while (1) {
		if (env[tmp_level][tmp_depth].struct_id_table.find(s)!=env[tmp_level][tmp_depth].struct_id_table.end()) { 
			break;
		}
		else {
			int parent = env[tmp_level][tmp_depth].parent_index;
			if (parent==-1||tmp_level==0) {report_err("No such struct named~",s,p->line_num);}
			else {
				tmp_level--;
				tmp_depth = parent;
			}
		}
	}
	if (semantics_check_id(p->children[0]->data,p->children[0]->line_num)&&func_table.find(p->children[0]->data)!=func_table.end()) {
		report_err("redefinition of",p->children[0]->data,p->children[0]->line_num);
	}
	else {
		env[level][cnt[level]].struct_table[p->children[0]->data] = env[tmp_level][tmp_depth].struct_id_table[s];
		env[level][cnt[level]].table[p->children[0]->data] = "struct";
		//fprintf(stderr,"flag %s  \n", p->children[0]->data);
		env[level][cnt[level]].width_table[p->children[0]->data] = env[tmp_level][tmp_depth].struct_name_width_table[s];
	}
	if (p->size==2) semantics_check_sextvars_1(s,p->children[1]);
}

void semantics_check_extdef(TreeNode* p) {
	if (!strcmp("extdef1",p->data)) {//global variable declearations
		semantics_check_extvars(p->children[1]);
	}
	else if (!strcmp("extdef2",p->data)) {//struct declarations
		semantics_check_stspec(p->children[0]);
		if (p->children[0]->size==3) {
			env[level][cnt[level]].struct_id_table[p->children[0]->children[1]->data] = struct_vector[struct_vector_cnt];
			env[level][cnt[level]].struct_name_width_table[p->children[0]->children[1]->data] = tmp_num_struct;
		}
		if (p->children[0]->size==2&&!strcmp(p->children[0]->data,"stspec")) {
			semantics_check_sextvars_1(p->children[0]->children[1]->data,p->children[1]);
		}
		else {
			semantics_check_sextvars(p->children[1]);
		}
	}
	else { //function declarations
		semantics_check_func(p->children[1]);
		in_func = true;
		semantics_check_stmtblock(p->children[2]);
		in_func = false;
	}
}

void semantics_check_sextvars(TreeNode *p) {
	if (p->size==0) return;
	if (semantics_check_id(p->children[0]->data,p->children[0]->line_num)&&func_table.find(p->children[0]->data)==func_table.end()) {
		env[level][cnt[level]].table[p->children[0]->data] = "struct";
		env[level][cnt[level]].struct_table[p->children[0]->data] = struct_vector[struct_vector_cnt];
		env[level][cnt[level]].width_table[p->children[0]->data] = tmp_num_struct;
		if (level>0)
			env[level][cnt[level]].parent_index = cnt[level-1]; //which stmtblock in the upper level is my parent
		else 	env[level][cnt[level]].parent_index = -1;
	}
	else {
		report_err("redefinition of",p->children[0]->data,p->children[0]->line_num);
	}
	if (p->size==2) { semantics_check_sextvars(p->children[1]);}
}

void semantics_check_stspec(TreeNode* p) {  //struct
	++struct_cnt;
	if (!strcmp(p->data,"stspec identifier {}")) {
		++struct_vector_cnt;
		tmp_num_struct = 0;
		semantics_check_sdefs(p->children[2]);
	}
	else if (!strcmp(p->data,"stspec {}")) {
		++struct_vector_cnt;
		tmp_num_struct = 0;
		semantics_check_sdefs(p->children[1]);
	}
}

void semantics_check_paras(TreeNode *p,char *s) {
	if (p->size>=1) {
		++tmp_num_of_var;
		if (env[level+1][cnt[level+1]+1].table.find(p->children[0]->data)==env[level+1][cnt[level+1]+1].table.end()) {
		 
			env[level+1][cnt[level+1]+1].table[p->children[0]->data] = "int";
			env[level+1][cnt[level+1]+1].width_table[p->children[0]->data] = 1;
		}
		else {
			report_err("redefinition of", p->children[1]->data,p->children[1]->line_num);
		}
	}
	if (p->size==2) {semantics_check_paras(p->children[1],s);}
}


char* semantics_check_type(TreeNode *p) {
	return p->data;
}

bool vector_find(vector<int> p, int num) {
	int i, sz = p.size();
	for (i = 0; i < sz; ++i) {
		if (p[i]==num) return true;
	}
	return false;
}

void rename(TreeNode *p, int num) {
	char *s = new char[strlen(p->data)+2];
	s[0] = '_';
	s[1] = num + '0';
	int i;
	for (i = 2; i < 2+strlen(p->data); ++i) {
		s[i] = p->data[i-2];
	}
	p->data = s;
}

void semantics_check_func(TreeNode *p) { 
	++func_cnt;
	if (semantics_check_id(p->children[0]->data,p->children[0]->line_num)) { // no conflict with the name of int variables and struct variables
		tmp_num_of_var = 0;
		semantics_check_paras(p->children[1],p->children[0]->data);
		if (func_table.find(p->children[0]->data)==func_table.end()) { // new function name
			if (!strcmp(p->children[0]->data,"main")) {bool_main = true;}
			func_table[p->children[0]->data] = func_vector[func_vector_cnt];
			func_table[p->children[0]->data].push_back(tmp_num_of_var);
			++func_vector_cnt;
		}
		else {	// function name exists, check whether it's an overloaded function
			/*just rename it!*/
			if (!vector_find(func_table[p->children[0]->data],tmp_num_of_var)) { //overloaded function
		   		func_table[p->children[0]->data].push_back(tmp_num_of_var);
				if (func_table[p->children[0]->data].size()>=2) {
					func_overload_table[p->children[0]->data][func_table[p->children[0]->data][0]] = p->children[0]->data;
					char *ch = p->children[0]->data;
					rename(p->children[0],tmp_num_of_var);
					func_overload_table[ch][tmp_num_of_var] = p->children[0]->data;
					func_table[p->children[0]->data] = func_vector[func_vector_cnt];
					func_table[p->children[0]->data].push_back(tmp_num_of_var);
					++func_vector_cnt;
				}
			}
			else report_err("multiple definitions of", p->children[0]->data,p->children[0]->line_num);
		}
		
	}
	else {
		report_err("conflicting names of", p->children[0]->data,p->children[0]->line_num);
	}
}

void semantics_check_extvars(TreeNode *p) {
	if (p->type==_NULL) {return;}	
	else {
		int i;
		tmp_num = 1;
		tmp_array_size_vector.clear();
		semantics_check_var(p->children[0]);
		for (i = 1; i < p->size;++i) {
			semantics_check(p->children[i]);
		}
	}
	
}

bool semantics_check_id(char* s,int num) { 
	if (isReserved(s)) {
		report_err("Reserved words can not be used as identifiers", s,num);
		return false;
	}
	if (env[level][cnt[level]].table.find(s)!=env[level][cnt[level]].table.end()){	//already exist
				report_err("redeclearation of",s,num);
				return false; 
	}
	else return true; 
}

void semantics_check_var(TreeNode *p) {
	if (p->size==1) {
		if (semantics_check_id(p->children[0]->data,p->children[0]->line_num)) {  //new variable name
			if (level==0&&func_table.find(p->children[0]->data)!=func_table.end()) 
				{report_err("redefinition of", p->children[0]->data,p->children[0]->line_num);}
			env[level][cnt[level]].table[p->children[0]->data] = "int";
			env[level][cnt[level]].width_table[p->children[0]->data] = tmp_num;
			int i,j;
			vector <int> final_vector;
			for (i = 0; i < tmp_array_size_vector.size(); ++i) {
				int t = 4;
				for (j = i+1; j < tmp_array_size_vector.size(); ++j) {
					t *= tmp_array_size_vector[i];
				}
				final_vector.push_back(t);
			}
			env[level][cnt[level]].array_size_table[p->children[0]->data] = final_vector;
		}
		else {report_err("redefinition of", p->children[0]->data,p->children[0]->line_num);}
	}
	else {
		int intnum = StringToInt(p->children[1]->data);
		if (intnum < 0) report_err("index can't be less than zero",NULL,p->line_num);
		tmp_array_size_vector.push_back(intnum);
		tmp_num *= intnum;
		semantics_check_var(p->children[0]);
	}
}

void semantics_check_stmtblock(TreeNode *p) {
	++level;
	++cnt[level];
	if (level>0)
		env[level][cnt[level]].parent_index = cnt[level-1]; //which stmtblock in the upper level is my parent
	else 	env[level][cnt[level]].parent_index = -1;
	semantics_check_defs(p->children[0]);
	semantics_check_stmts(p->children[1]);
	--level;
}

void semantics_check_defs(TreeNode* p) {
	int i;
	if (p->size==0) return;
	if (!strcmp(p->data,"defs1")) {
		semantics_check(p->children[0]);
		semantics_check(p->children[1]);
		semantics_check(p->children[2]);
		return;
	}
	if (p->children[0]->size==2&&!strcmp(p->children[0]->data,"stspec")) {
			semantics_check_stspec(p->children[0]);
			semantics_check_sextvars_1(p->children[0]->children[1]->data,p->children[1]);
			return;
		}
	for (i = 0; i < p->size; ++i) {
		semantics_check(p->children[i]);
	}
}

void semantics_check_decs(TreeNode *p)
{	
	int i;
	tmp_num = 1;
	tmp_array_size_vector.clear();
	semantics_check_var(p->children[0]);
	for (i = 1; i < p->size;++i) {
		semantics_check(p->children[i]);
	}
}

void semantics_check_sdefs(TreeNode *p) { //inside of a struct
	int i;
	if (p->size==0) return;
	semantics_check_sdecs(p->children[1],1);
	semantics_check_sdefs(p->children[2]);
}

void semantics_check_sdecs(TreeNode *p, int in_struct) {
	if (in_struct) {
		++tmp_num_struct;
		if (struct_set[struct_cnt].find(p->children[0]->data)==struct_set[struct_cnt].end()) {//no multiple declarations in a struct
			struct_set[struct_cnt].insert(p->children[0]->data);
			struct_vector[struct_vector_cnt].push_back(p->children[0]->data);
		}
		else {
			report_err("redefinition of", p->children[0]->data,p->children[0]->line_num);
		}
		if (p->size==2) {
			semantics_check_sdecs(p->children[1], in_struct);
		}
	}
	else {
	if (semantics_check_id(p->children[0]->data,p->children[0]->line_num)&&func_table.find(p->children[0]->data)==func_table.end()) {
			env[level][cnt[level]].table[p->children[0]->data] = "struct";
			env[level][cnt[level]].struct_table[p->children[0]->data] = struct_vector[struct_vector_cnt];
			env[level][cnt[level]].width_table[p->children[0]->data] = tmp_num_struct;
		if (level>0)
			env[level][cnt[level]].parent_index = cnt[level-1]; //which stmtblock in the upper level is my parent
		else 	env[level][cnt[level]].parent_index = -1;
		}
		else {
			report_err("redefinition of",p->children[0]->data,p->children[0]->line_num);
		}
	}
}

void semantics_check_stmts(TreeNode* p) {
	int i;
	for (i = 0; i < p->size; ++i) {
		semantics_check(p->children[i]);
	}
}

void semantics_check_arrs(TreeNode *p) { //arrays
	int i;
	for (i = 0; i < p->size; ++i) {
		semantics_check(p->children[i]);
	}
}

void semantics_check_args(TreeNode *p, char *s) { //arguments of function
	if (s==NULL) return;
	if (p->size==2) {
		semantics_check_exp(p->children[0],s);//1 means it's the check process of arguemnts
		semantics_check_args(p->children[1],s);
	}
	else {
		semantics_check_exp(p->children[0],s);
	}
}

int vector_find_index(vector <int> s, int num) {
	int sz =  s.size();
	int i;
	for (i = 0; i < sz; ++i) {
		if (s[i]==num) return i;
	}
}

void semantics_check_exps(TreeNode *p) {
	if (!strcmp(p->data,"exps f()")) {
		if (func_table.find(p->children[0]->data)==func_table.end()) { //don't find the function
			report_err(p->children[0]->data,"not declared",p->children[0]->line_num);
		}
		else {
			int func_tmp = func_cnt_table[p->children[0]->data];
			func_cnt_table[p->children[0]->data] = 0;
			semantics_check_args(p->children[1],p->children[0]->data);
		if (vector_find(func_table[p->children[0]->data],func_cnt_table[p->children[0]->data])) {
				// num of variables match!
				int v_num = vector_find_index(func_table[p->children[0]->data],func_cnt_table[p->children[0]->data]);
				if (v_num>0) {
					int t_num = func_cnt_table[p->children[0]->data];
					rename(p->children[0],t_num); 
				}
			}
			else {
				report_err("number of arguments doesn't match: function", p->children[0]->data,p->children[0]->line_num);
			}
			func_cnt_table[p->children[0]->data] = func_tmp;
		}
	}
	else if (!strcmp(p->data,"exps ()")) {
		semantics_check_exps(p->children[0]);
	}
	else if (!strcmp(p->data,"exps arr")) { //intergers or arrays
		if (!find_id(p->children[0])) {
			report_err("No variable named",p->children[0]->data,p->children[0]->line_num);
		}
		if (find_struct_id(p->children[0]->data,NULL)) {
				report_err(p->children[0]->data,"is a struct",p->children[0]->line_num);
		}
		if (p->children[1]->size==0) {
			if (get_width_semantics(p->children[0]->data)!=1) {
				report_err(p->children[0]->data,"is not an integer",p->children[0]->line_num);
			}
		}
		if (p->children[1]->size!=0) {  // it should be an array
			if (get_width_semantics(p->children[0]->data)==1) {
				report_err(p->children[0]->data,"is not array",p->children[0]->line_num);
			}
		}
	}
	else if (!strcmp(p->data,"exps unary")) { //unary operations
		if (!strcmp(p->children[0]->data,"++")||!strcmp(p->children[0]->data,"--")) {
			check_left_value_exps(p->children[1]);
		}
		semantics_check_exps(p->children[1]);
	}
	else if (!strcmp(p->data,"exps struct")) { //structs
		int sjtu = find_struct_id(p->children[0]->data,p->children[2]->data);
		if (sjtu==0) {
			report_err("No such struct named", p->children[0]->data,p->children[0]->line_num);
		}
		else if (sjtu==-1){
			report_err("struct has no varibale named", p->children[2]->data,p->children[2]->line_num);
		}
	}
	else if (!strcmp(p->data,"=")) {
		check_left_value_exps(p->children[0]);
		semantics_check_exps(p->children[0]);
		semantics_check_exps(p->children[1]);
	}
	else {
		int i;
		for (i = 0; i < p->size; ++i) {
			semantics_check_exps(p->children[i]);
		}
	}
}

int find_struct_id(char *s,char *s2) {
	int tmp_level = level, tmp_depth = cnt[level];
        int bool_find = 0;
	while (1) {
if (env[tmp_level][tmp_depth].table.find(s)!=env[tmp_level][tmp_depth].table.end()&&!strcmp(env[tmp_level][tmp_depth].table[s],"struct")) { 
			//find
			if (s2==NULL) return 1;
			int i;
			for (i = 0; i < env[tmp_level][tmp_depth].struct_table[s].size(); ++i) {
				
				if (!strcmp(env[tmp_level][tmp_depth].struct_table[s][i],s2)) return 1;
			}
			return -1;
		}
		else {
			int parent = env[tmp_level][tmp_depth].parent_index;
			if (parent==-1||tmp_level==0) break;
			else {
				tmp_level--;
				tmp_depth = parent;
			}
		}
	}
	return bool_find;
}

bool find_id(TreeNode *p) {
	int tmp_level = level, tmp_depth = cnt[level];
	bool bool_find = false;
	while (1) {
		//cout << "tmp_level" << tmp_level << "tmp_depth" << tmp_depth << endl;
		if (env[tmp_level][tmp_depth].table.find(p->data)!=env[tmp_level][tmp_depth].table.end()) { //find
			//cout << "find" << endl;
			return true;
		}
		else {
			int parent = env[tmp_level][tmp_depth].parent_index;
			if (parent==-1||tmp_level==0) break;
			else {
				tmp_level--;
				tmp_depth = parent;
			}
		}
	}
	return bool_find;
}

void check_left_value_exps(TreeNode *p) {
	if (!strcmp(p->data,"=")) {
		check_left_value_exps(p->children[0]);
		semantics_check_exps(p->children[1]);
	}
	else if (!strcmp(p->data,"exps arr")||!strcmp(p->data,"exps struct")||!strcmp(p->data,"exps ()")) {
		return;
	}
	else {report_err("lvalue required as left operand assignment",NULL,p->line_num);}
}

void semantics_check_stmt(TreeNode *p) {
	if (!strcmp("stmt",p->data)||!strcmp(p->data,"stmt: exp;")) {
		int i;
		for (i = 0; i < p->size; ++i) {
			semantics_check(p->children[i]);
		}
	}
	else if (!strcmp("return stmt",p->data)) {
		if (!in_func) report_err("return not in a function",NULL,p->line_num);
		semantics_check_exps(p->children[1]);
	}
	else if (!strcmp("if stmt",p->data)) {
		int i;
		for (i = 0; i < p->size; ++i) {
			semantics_check(p->children[i]);
		}
	}
	else if (!strcmp("for stmt",p->data)) {
		int i;
		for (i = 0; i < p->size-1; ++i) {
			semantics_check_exp(p->children[i],NULL);
			
		}
		in_for = true;
		semantics_check_stmt(p->children[p->size-1]);
		in_for =  false;
	}
	else if (!strcmp("read stmt",p->data)) {
		check_left_value_exps(p->children[0]);  //check if it's a left value
	}
	else if( !strcmp("write stmt",p->data)) {
		semantics_check_exps(p->children[0]);
	}
	else { // break and continue
		if (!in_for) {
			report_err(p->children[0]->data,"not in a loop",p->children[0]->line_num);
		}
	}
}


void semantics(TreeNode *p) {
	semantics_check(p);
	if (!bool_main) {
		fprintf(stderr,"fatal error! undefined reference to main()\n");
		exit(1);
	}
}

#endif
