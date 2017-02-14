%{
#include <vslc.h>
%}
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%token FUNC PRINT RETURN CONTINUE IF THEN ELSE WHILE DO OPENBLOCK CLOSEBLOCK
%token VAR NUMBER IDENTIFIER STRING

%%

program : global_list {
                      root = (node_t *) malloc ( sizeof(node_t) );
                      node_init ( root, PROGRAM, NULL, 1 );
                      root->children[0] = $1;
                      }
                      ;

global_list : global  {
                      $$ = (node_t *) malloc (sizeof(node_t));
                      node_init($$, GLOBAL_LIST, NULL, 1);
                      $$->children[0] = $1;
                      }
            | global_list global  {
                                  $$ = (node_t *) malloc (sizeof(node_t));
                                  node_init($$, GLOBAL_LIST, NULL, 2);
                                  $$->children[0] = $1;
                                  $$->children[1] = $2;
                                  }
            ;

global : function     {
                      $$ = (node_t *) malloc (sizeof(node_t));
                      node_init($$, GLOBAL, NULL, 1);
                      $$->children[0] = $1;
                      }
        | declaration {
                      $$ = (node_t *) malloc (sizeof(node_t));
                      node_init($$, GLOBAL, NULL, 1);
                      $$->children[0] = $1;
                      }
        ;

statement_list : statement  {
                            $$ = (node_t *) malloc (sizeof(node_t));
                            node_init($$, STATEMENT_LIST, NULL, 1);
                            $$->children[0] = $1;
                            }
                | statement_list statement  {
                                      $$ = (node_t *) malloc (sizeof(node_t));
                                      node_init($$, STATEMENT_LIST, NULL, 2);
                                      $$->children[0] = $1;
                                      $$->children[1] = $2;
                                      }
                ;

print_list : print_item     {
                            $$ = (node_t *) malloc (sizeof(node_t));
                            node_init($$, PRINT_LIST, NULL, 1);
                            $$->children[0] = $1;
                            }
            | print_list ',' print_item {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, PRINT_LIST, NULL, 2);
                                        $$->children[0] = $1;
                                        $$->children[1] = $3;
                                        }
            ;

expression_list : expression  {
                              $$ = (node_t *) malloc (sizeof(node_t));
                              node_init($$, EXPRESSION_LIST, NULL, 1);
                              $$->children[0] = $1;
                              }
                | expression_list ',' expression {
                                      $$ = (node_t *) malloc (sizeof(node_t));
                                      node_init($$, EXPRESSION_LIST, NULL, 2);
                                      $$->children[0] = $1;
                                      $$->children[1] = $3;
                                      }
                ;

variable_list : identifier  {
                            $$ = (node_t *) malloc (sizeof(node_t));
                            node_init($$, VARIABLE_LIST, NULL, 1);
                            $$->children[0] = $1;
                            }
                | variable_list ',' identifier {
                                            $$ = (node_t *) malloc (sizeof(node_t));
                                            node_init($$, VARIABLE_LIST, NULL, 2);
                                            $$->children[0] = $1;
                                            $$->children[1] = $3;
                                            }
                ;

argument_list : expression_list   {
                            $$ = (node_t *) malloc (sizeof(node_t));
                            node_init($$, ARGUMENT_LIST, NULL, 1);
                            $$->children[0] = $1;
                            }
                |           {
                            $$ = (node_t *) malloc (sizeof(node_t));
                            node_init($$, ARGUMENT_LIST, NULL, 0);
                            }
                ;

parameter_list : variable_list  {
                                $$ = (node_t *) malloc (sizeof(node_t));
                                node_init($$, PARAMETER_LIST, NULL, 1);
                                $$->children[0] = $1;
                                }
                    |           {
                                $$ = (node_t *) malloc (sizeof(node_t));
                                node_init($$, ARGUMENT_LIST, NULL, 0);
                                }
                ;

declaration_list : declaration  {
                                $$ = (node_t *) malloc (sizeof(node_t));
                                node_init($$, DECLARATION_LIST, NULL, 1);
                                $$->children[0] = $1;
                                }
                | declaration_list declaration  {
                                                $$ = (node_t *) malloc (sizeof(node_t));
                                                node_init($$, DECLARATION_LIST, NULL, 2);
                                                $$->children[0] = $1;
                                                $$->children[1] = $2;
                                                }
                ;

function : FUNC identifier '(' parameter_list ')' statement
                                                {
                                                $$ = (node_t *) malloc (sizeof(node_t));
                                                node_init($$, FUNCTION, NULL, 3);
                                                $$->children[0] = $2;
                                                $$->children[1] = $4;
                                                $$->children[2] = $6;
                                                }
          ;

statement : assignment_statement    {
                                    $$ = (node_t *) malloc (sizeof(node_t));
                                    node_init($$, STATEMENT, NULL, 1);
                                    $$->children[0] = $1;
                                    }
          | return_statement        {
                                    $$ = (node_t *) malloc (sizeof(node_t));
                                    node_init($$, STATEMENT, NULL, 1);
                                    $$->children[0] = $1;
                                    }
          | print_statement         {
                                    $$ = (node_t *) malloc (sizeof(node_t));
                                    node_init($$, STATEMENT, NULL, 1);
                                    $$->children[0] = $1;
                                    }
          | if_statement            {
                                    $$ = (node_t *) malloc (sizeof(node_t));
                                    node_init($$, STATEMENT, NULL, 1);
                                    $$->children[0] = $1;
                                    }
          | while_statement         {
                                    $$ = (node_t *) malloc (sizeof(node_t));
                                    node_init($$, STATEMENT, NULL, 1);
                                    $$->children[0] = $1;
                                    }
          | null_statement         {
                                    $$ = (node_t *) malloc (sizeof(node_t));
                                    node_init($$, STATEMENT, NULL, 1);
                                    $$->children[0] = $1;
                                    }
          | block                   {
                                    $$ = (node_t *) malloc (sizeof(node_t));
                                    node_init($$, STATEMENT, NULL, 1);
                                    $$->children[0] = $1;
                                    }
          ;

block : OPENBLOCK declaration_list statement_list CLOSEBLOCK {
                                    $$ = (node_t*) malloc (sizeof(node_t));
                                    node_init($$, BLOCK, NULL, 2);
                                    $$->children[0] = $2;
                                    $$->children[1] = $3;
                                    }
      | OPENBLOCK statement_list CLOSEBLOCK{
                                    $$ = (node_t*) malloc (sizeof(node_t));
                                    node_init($$, BLOCK, NULL, 1);
                                    $$->children[0] = $2;
                                    };

assignment_statement : identifier ':' '=' expression   {
                                  $$ = (node_t *) malloc (sizeof(node_t));
                                  node_init($$, ASSIGNMENT_STATEMENT, NULL, 2);
                                  $$->children[0] = $1;
                                  $$->children[1] = $4;
                                  };

return_statement : RETURN expression   {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, RETURN_STATEMENT, NULL, 1);
                                        $$->children[0] = $2;
                                        };

print_statement : PRINT print_list      {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, PRINT_STATEMENT, NULL, 1);
                                        $$->children[0] = $2;
                                        };

null_statement : CONTINUE               {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, NULL_STATEMENT, NULL, 0);
                                        };

if_statement : IF relation THEN statement   {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, IF_STATEMENT, NULL, 2);
                                        $$->children[0] = $2;
                                        $$->children[1] = $4;
                                        }
             | IF relation THEN statement ELSE statement   {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, IF_STATEMENT, NULL, 3);
                                        $$->children[0] = $2;
                                        $$->children[1] = $4;
                                        $$->children[2] = $6;
                                        };

while_statement : WHILE relation DO statement   {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, WHILE_STATEMENT, NULL, 2);
                                        $$->children[0] = $2;
                                        $$->children[1] = $4;
                                        };

relation : expression '=' expression   {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, RELATION, strdup("="), 2);
                                        $$->children[0] = $1;
                                        $$->children[1] = $3;
                                        }
          | expression '<' expression   {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, RELATION, strdup("<"), 2);
                                        $$->children[0] = $1;
                                        $$->children[1] = $3;
                                        }
          | expression '>' expression   {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, RELATION, strdup(">"), 2);
                                        $$->children[0] = $1;
                                        $$->children[1] = $3;
                                        }
                                        ;
expression : expression '+' expression  {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, EXPRESSION, strdup("+"), 2);
                                        $$->children[0] = $1;
                                        $$->children[1] = $3;
                                        }
          | expression '-' expression   {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, EXPRESSION, strdup("-"), 2);
                                        $$->children[0] = $1;
                                        $$->children[1] = $3;
                                        }
          | expression '*' expression   {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, EXPRESSION, strdup("*"), 2);
                                        $$->children[0] = $1;
                                        $$->children[1] = $3;
                                        }
          | expression '/' expression   {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, EXPRESSION, strdup("/"), 2);
                                        $$->children[0] = $1;
                                        $$->children[1] = $3;
                                        }
          | '-' expression %prec UMINUS          {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, EXPRESSION, strdup("-"), 1);
                                        $$->children[0] = $2;
                                        }
          | '(' expression ')'          {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        $$ = $2;
                                        };

          | number                     {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, EXPRESSION, NULL, 1);
                                        $$->children[0] = $1;
                                        }
          | identifier                 {
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, EXPRESSION, NULL, 1);
                                        $$->children[0] = $1;
                                        }
          | identifier '(' argument_list ')'{
                                        $$ = (node_t *) malloc (sizeof(node_t));
                                        node_init($$, EXPRESSION, NULL, 2);
                                        $$->children[0] = $1;
                                        $$->children[1] = $3;
                                        };


declaration : VAR variable_list        {
                                       $$ = (node_t *) malloc (sizeof(node_t));
                                       node_init($$, DECLARATION, NULL, 1);
                                       $$->children[0] = $2;
                                       };

print_item : expression                {
                                       $$ = (node_t *) malloc (sizeof(node_t));
                                       node_init($$, PRINT_ITEM, NULL, 1);
                                       $$->children[0] = $1;
                                       }
            |string                    {
                                       $$ = (node_t *) malloc (sizeof(node_t));
                                       node_init($$, PRINT_ITEM, NULL, 1);
                                       $$->children[0] = $1;
                                       };

identifier : IDENTIFIER                {
                                       $$ = (node_t *) malloc (sizeof(node_t));
                                       node_init($$, IDENTIFIER_DATA, NULL, 1);
                                       $$->data = strdup(yytext);
                                       };

number : NUMBER                        {
                                       $$ = (node_t *) malloc (sizeof(node_t));
                                       int64_t* data = malloc(sizeof(int64_t));
                                       *data = strtol ( yytext, NULL, 10 );
                                       node_init($$, NUMBER_DATA, data, 0);
                                       };

string : STRING                        {
                                       $$ = (node_t*) malloc(sizeof(node_t));
                                       node_init($$, STRING_DATA, NULL, 0);
                                       $$->data = strdup(yytext);
                                       };

%%

int
yyerror ( const char *error )
{
    fprintf ( stderr, "%s on line %d\n", error, yylineno );
    exit ( EXIT_FAILURE );
}
