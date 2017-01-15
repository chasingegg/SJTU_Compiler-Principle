%{
/*
  Author: Chao Gao
  file: small.y
  This program is the syntax analyzer part of the compiler.
  The entrance of the whole program.
*/
#include "def.h"
#include "lex.yy.c"
#include "node.h"
#include "tree.h"
#include "semantics.h"
#include "translate.h"
#include "interprete.h"
#include "optimize.h"
using namespace std;
void yyerror(char*);
extern int linenum;
%}

%union {
    TreeNode* node;
    char* string;
}

%token <string> INT ID SEMI COMMA
%token <string> TYPE LP RP LB RB LC RC STRUCT RETURN
%token <string> IF ELSE BREAK CONT FOR DOT ASSIGN
%token <string> ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN AND_ASSIGN MOD_ASSIGN
%token <string> XOR_ASSIGN OR_ASSIGN RIGHT_OP LEFT_OP //INC_OP DEC_OP
%token <string> AND_OP OR_OP EQ_OP NE_OP RIGHT_ASSIGN LEFT_ASSIGN //LE_OP GE_OP
%token <string> '!' '~' READ WRITE
%type <node> PROGRAM EXTDEFS EXTDEF SEXTVARS EXTVARS STSPEC FUNC PARAS STMTBLOCK STMTS
%type <node> STMT DEFS SDEFS SDECS DECS VAR INIT EXP EXPS ARRS ARGS UNARYOP

%nonassoc  IFX
%nonassoc ELSE
%right ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN AND_ASSIGN MOD_ASSIGN XOR_ASSIGN OR_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN
%left  OR_OP
%left  AND_OP
%left <string> '|'
%left <string> '^'
%left <string> '&'
%left  EQ_OP NE_OP
%left <string> GE_OP LE_OP '>' '<'
%left LEFT_OP RIGHT_OP
%left <string> '+' '-'
%left <string> '*' '/' '%'
%right <string> INC_OP DEC_OP UNARY 
%left  DOT LP LB
%%
PROGRAM: EXTDEFS {treeroot = $$ = create_node(linenum,_PROGRAM,"program",1,$1);}
;
EXTDEFS: EXTDEF EXTDEFS {$$ = create_node(linenum,_EXTDEFS,"extdefs",2,$1,$2);}
| {$$ = create_node(linenum,_NULL, "null", 0);}
;

EXTDEF: TYPE EXTVARS SEMI { $$ = create_node(linenum,_EXTDEF, "extdef1", 2, create_node(linenum,_TYPE, $1, 0),$2); }
| STSPEC SEXTVARS SEMI { $$ = create_node(linenum,_EXTDEF, "extdef2", 2, $1,$2); }
| TYPE FUNC STMTBLOCK { $$ = create_node(linenum,_EXTDEF, "extdef func", 3, create_node(linenum,_TYPE, $1, 0),$2,$3); }
;

SEXTVARS: ID { $$ = create_node(linenum,_SEXTVARS, "sextvars",1,create_node(linenum,_ID,$1,0)); }
| ID COMMA SEXTVARS { $$ = create_node(linenum,_SEXTVARS, "sextvars", 2, create_node(linenum,_ID, $1, 0),$3); }
| {$$ = create_node(linenum,_NULL, "null", 0);}
;

EXTVARS: VAR { $$ = create_node(linenum,_EXTVARS, "extvars", 1, $1); }
|VAR COMMA EXTVARS { $$ = create_node(linenum,_EXTVARS, "extvars", 2, $1,$3); }
|VAR ASSIGN INIT { $$ = create_node(linenum,_EXTVARS, "extvars", 3, $1,create_node(linenum,_OPERATOR,$2,0),$3); }
|VAR ASSIGN INIT COMMA EXTVARS { $$ = create_node(linenum,_EXTVARS, "extvars", 4, $1,create_node(linenum,_OPERATOR,$2,0),$3,$5); }
| {$$ = create_node(linenum,_NULL, "null", 0);}
;

STSPEC: STRUCT ID LC SDEFS RC { $$ = create_node(linenum,_STSPEC, "stspec identifier {}", 3, create_node(linenum,_OPERATOR,$1,0),create_node(linenum,_ID, $2, 0),$4); }
| STRUCT LC SDEFS RC { $$ = create_node(linenum,_STSPEC, "stspec {}", 2, create_node(linenum,_OPERATOR,$1,0),$3); }
| STRUCT ID { $$ = create_node(linenum,_STSPEC, "stspec", 2, create_node(linenum,_OPERATOR,$1,0),create_node(linenum,_ID, $2, 0)); }
;

FUNC: ID LP PARAS RP { $$ = create_node(linenum,_FUNC, "func ()", 2, create_node(linenum,_ID, $1, 0),$3); }
;

PARAS: TYPE ID COMMA PARAS { $$ = create_node(linenum,_PARAS, "paras", 2, create_node(linenum,_ID, $2, 0),$4); }
| TYPE ID { $$ = create_node(linenum,_PARAS, "paras", 1,create_node(linenum,_ID, $2, 0)); }
| {$$ = create_node(linenum,_NULL, "null", 0);}
;

STMTBLOCK: LC DEFS STMTS RC { $$ = create_node(linenum,_STMTBLOCK, "stmtblock {}", 2, $2,$3); }
;

STMTS: STMT STMTS { $$ = create_node(linenum,_STMTS, "stmts", 2, $1,$2); }
| {$$ = create_node(linenum,_NULL, "null", 0);}
;

STMT: EXP SEMI { $$ = create_node(linenum,_STMT, "stmt: exp;", 1, $1); }
| STMTBLOCK { $$ = create_node(linenum,_STMT, "stmt", 1, $1); }
| RETURN EXPS SEMI { $$ = create_node(linenum,_STMT, "return stmt", 2, create_node(linenum,_KEYWORDS, $1, 0),$2); }
| IF LP EXPS RP STMT %prec IFX { $$ = create_node(linenum,_STMT, "if stmt", 2, $3,$5); }
| IF LP EXPS RP STMT ELSE STMT %prec ELSE { $$ = create_node(linenum,_STMT, "if stmt", 3, $3,$5,$7);}
| FOR LP EXP SEMI EXP SEMI EXP RP STMT { $$ = create_node(linenum,_STMT, "for stmt", 4, $3,$5,$7,$9); }
| CONT SEMI { $$ = create_node(linenum,_STMT, "cont stmt", 1, create_node(linenum,_KEYWORDS, $1, 0)); }
| BREAK SEMI { $$ = create_node(linenum,_STMT, "break stmt", 1, create_node(linenum,_KEYWORDS, $1, 0)); }
| READ LP EXPS RP SEMI{$$ = create_node(linenum,_STMT,"read stmt",1, $3);}
| WRITE LP EXPS RP SEMI{$$ = create_node(linenum,_STMT,"write stmt",1, $3);}
;

DEFS: TYPE DECS SEMI DEFS { $$ = create_node(linenum,_DEFS, "defs1", 3, create_node(linenum,_TYPE, $1, 0),$2,$4); }
| STSPEC SDECS SEMI DEFS { $$ = create_node(linenum,_DEFS, "defs2", 3, $1,$2,$4); }
| {$$ = create_node(linenum,_NULL, "null", 0);}
;

SDEFS: TYPE SDECS SEMI SDEFS { $$ = create_node(linenum,_SDEFS, "sdefs", 3, create_node(linenum,_TYPE, $1, 0),$2,$4); }
| {$$ = create_node(linenum,_NULL, "null", 0);}
;

SDECS: ID COMMA SDECS { $$ = create_node(linenum,_SDECS, "sdecs", 2, create_node(linenum,_ID, $1, 0),$3); }
| ID { $$ = create_node(linenum,_SDECS, "sdecs", 1,create_node(linenum,_ID,$1,0)); }
;

DECS: VAR { $$ = create_node(linenum,_DECS, "decs", 1, $1); }
| VAR COMMA DECS { $$ = create_node(linenum,_DECS, "decs", 2, $1,$3); }
| VAR ASSIGN INIT COMMA DECS { $$ = create_node(linenum,_DECS, "assign decs", 4, $1,create_node(linenum,_OPERATOR, $2, 0),$3,$5); }
| VAR ASSIGN INIT { $$ = create_node(linenum,_DECS, "assign decs", 3, $1,create_node(linenum,_OPERATOR, $2, 0),$3); }
;

VAR:ID { $$ = create_node(linenum,_VAR, "var", 1,create_node(linenum,_ID, $1, 0)); }
|VAR LB INT RB { $$ = create_node(linenum,_VAR, "var []", 2, $1,create_node(linenum,_INT, $3, 0)); }
;

INIT: EXPS { $$ = create_node(linenum,_INIT, "init", 1, $1); }
| LC ARGS RC { $$ = create_node(linenum,_INIT, "init {}", 1, $2); }
; 

EXP: EXPS { $$ = create_node(linenum,_EXP, "exp", 1, $1); }
| {$$ = create_node(linenum,_NULL, "null", 0);}
;

EXPS: EXPS ASSIGN EXPS { $$ = create_node(linenum,_EXPS, $2, 2, $1,$3); }
| EXPS ADD_ASSIGN EXPS { $$ = create_node(linenum,_EXPS, $2, 2, $1,$3); }
| EXPS SUB_ASSIGN EXPS { $$ = create_node(linenum,_EXPS, $2, 2, $1,$3); }
| EXPS MUL_ASSIGN EXPS { $$ = create_node(linenum,_EXPS, $2, 2, $1,$3); }
| EXPS DIV_ASSIGN EXPS { $$ = create_node(linenum,_EXPS, $2, 2, $1,$3); }
| EXPS AND_ASSIGN EXPS { $$ = create_node(linenum,_EXPS, $2, 2, $1,$3); }
| EXPS MOD_ASSIGN EXPS { $$ = create_node(linenum,_EXPS, $2, 2, $1,$3); }
| EXPS XOR_ASSIGN EXPS { $$ = create_node(linenum,_EXPS, $2, 2, $1,$3); }
| EXPS OR_ASSIGN EXPS { $$ = create_node(linenum,_EXPS, $2, 2, $1,$3); }
| EXPS RIGHT_OP EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS LEFT_OP EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS AND_OP EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS OR_OP EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS LE_OP EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS GE_OP EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS EQ_OP EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS NE_OP EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS RIGHT_ASSIGN EXPS { $$ = create_node(linenum,_EXPS, $2, 2, $1,$3); }
| EXPS LEFT_ASSIGN EXPS { $$ = create_node(linenum,_EXPS, $2, 2, $1,$3); }
| EXPS '+' EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS '-' EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS '&' EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS '*' EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS '/' EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS '%' EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS '<' EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS '>' EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS '^' EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| EXPS '|' EXPS { $$ = create_node(linenum,_OPERATOR, $2, 2, $1,$3); }
| UNARYOP EXPS %prec UNARY { $$ = create_node(linenum,_EXPS, "exps unary", 2, $1,$2); }
| LP EXPS RP { $$ = create_node(linenum,_EXPS, "exps ()", 1, $2); }
| ID LP ARGS RP { $$ = create_node(linenum,_EXPS, "exps f()", 2, create_node(linenum,_ID, $1, 0),$3); }
| ID ARRS { $$ = create_node(linenum,_EXPS, "exps arr", 2, create_node(linenum,_ID, $1, 0),$2); }
| ID DOT ID { $$ = create_node(linenum,_EXPS, "exps struct", 3, create_node(linenum,_ID, $1, 0),create_node(linenum,_OPERATOR, $2, 0),create_node(linenum,_ID, $3, 0)); }
| INT { $$ = create_node(linenum,_INT, $1, 0); }
;

ARRS: LB EXP RB ARRS { $$ = create_node(linenum,_ARRS, "arrs []", 2, $2,$4); }
| {$$ = create_node(linenum,_NULL, "null", 0);}
;

ARGS: EXP COMMA ARGS { $$ = create_node(linenum,_ARGS, "args", 2, $1,$3); }
| EXP { $$ = create_node(linenum,_ARGS, "args", 1, $1); }
;

UNARYOP:
	'+' {$$ = create_node(linenum,_UNARYOP, $1, 0);}
	|'-' {$$ = create_node(linenum,_UNARYOP, $1, 0);}
	|'~' {$$ = create_node(linenum,_UNARYOP, $1, 0);}
	|'!' {$$ = create_node(linenum,_UNARYOP, $1, 0);}
	|INC_OP {$$ = create_node(linenum,_UNARYOP, $1, 0);}
	|DEC_OP {$$ = create_node(linenum,_UNARYOP, $1, 0);}
;

%%
#include <stdio.h>
#include "def.h"
#include "semantics.h"
#include "translate.h"
void yyerror(char *s)
{
	fflush(stdout);
	fprintf(stderr,"\n[Line %d]: %s %s\n",linenum,s,yytext);
}
int main(int argc, char *argv[])
{
	freopen(argv[1], "r", stdin);
	if(argc == 2) {
    		freopen("MIPSCode.s", "w", stdout);
	}
	else if(argc == 3){
		freopen(argv[2], "w", stdout);
	}
	if(!yyparse()){
		fprintf(stderr,"Parsing complete.\n");
		//print_tree(treeroot,0);
		semantics(treeroot);
		fprintf(stderr,"Semantics check complete.\n");
		phase3_translate();
		fprintf(stderr,"Translate complete.\n");
		optimize();
		interpret();
	}
	else
		printf("Parsing failed.\n");
	return 0;
}
int yywrap()
{
	return 1;
}
