#ifndef VSLC_H
#define VSLC_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "nodetypes.h"
#include "ir.h"
#include "y.tab.h"

int yyerror ( const char *error );
extern int yylineno;
extern int yylex ( void );
extern char yytext[];

extern node_t *root;

bool list_detected ( node_t *node );
bool is_syntactical ( node_t *node );
node_t* flatten_list ( node_t *node );
node_t* prone_and_connect ( node_t *node );
void prone_printlists( node_t *node, node_t *parent );
void prone_printitems( node_t *node );
void resolve_constant_expressions(node_t *node);
#endif
