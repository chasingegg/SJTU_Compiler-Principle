/*
  Author: Chao Gao
  file: node.h
  This program is the definition of the abstract syntax tree.
*/

#ifndef FILE_NODE_H
#define FILE_NODE_H

typedef enum {
	_PROGRAM,
	_EXTDEFS,
	_EXTDEF,
	_SEXTVARS,
	_EXTVARS,
	_STSPEC,
	_FUNC,
	_PARAS,
	_STMTBLOCK,
	_STMTS,
	_STMT,
	_DEFS,
	_SDEFS,
	_SDECS,
	_DECS,
	_VAR,
	_INIT,
	_EXP,
	_EXPS,
	_ARRS,
	_ARGS,
	_UNARYOP,
	_ID,
	_OPERATOR,
	_KEYWORDS,
	_TYPE,
	_INT,
	_NULL
} TreeNodeType;

typedef struct TreeNode {
    TreeNodeType type; //type of the treenodes
    int line_num;
    char* data;
    int size, capacity;
    struct TreeNode** children;
    //struct InfoNode* info;
} TreeNode;

#endif
