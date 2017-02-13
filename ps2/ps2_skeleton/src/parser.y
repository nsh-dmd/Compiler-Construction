%{
#include <vslc.h>
%}
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%token FUNC PRINT RETURN CONTINUE IF THEN ELSE WHILE DO OPENBLOCK CLOSEBLOCK
%token VAR NUMBER IDENTIFIER STRING

%%

program : global_list                             {$$ = $1;}

global_list : global                              {$$ = $1;}
            | global_list global                  {$$ = $1$2;}
            ;

global : function                                 {$$ = $1;}
        | declaration                             {$$ = $1;}
        ;

statement_list : statement                         {$$ = $1;}
                | statement_list statement         {$$ = $1$2;}
                ;

print_list : print_item
            | print_list ',' print_item
            ;

expression_list : expression
                | expression_list ',' identifier
                ;

variable_list : identifier
                | variable_list ',' identifier
                ;

argument_list : expression_list
                |
                ;

parameter_list : variable_list
                |
                ;

declaration_list : declaration
                | declaration_list declaration
                ;

function : FUNC identifier '(' parameter_list ')' statement;

statement : assignment_statement
        | return_statement
        ;
statement : print_statement
            | if_statement
            ;
statement : while_statement
            | null_statement
            | block
            ;

block : BEGIN declaration_list statement_list END;
block : BEGIN statement_list END;

assignment_statement : identifier ':' '=' expression;

return_statement : RETURN expression;
print_statement : PRINT print_list;
null_statement : CONTINUE;
if_statement : IF relation THEN statement;
if_statement : IF relation THEN statement ELSE statement;
while_statement : WHIEL relation DO statement;
relation : expression '=' expression;
relation : expression '<' expression;
relation : expression '>' expression;
expression : expression '+' expression;
expression : expression '-' expression;
expression : expression '*' expression;
expression : expression '/' expression;
expression : '-' expression;
expression : '(' expression ')';
expression : number
            | identifier
            | identifier '(' argument ')'
            ;
declaration : VAR variable_list;
print_item : expression
            |string
            ;
identifier : IDENTIFIER;
number : NUMBER;
string : STRING;





program :
      FUNC {
        root = (node_t *) malloc ( sizeof(node_t) );
        node_init ( root, PROGRAM, NULL, 0 );
      }
    ;
%%

int
yyerror ( const char *error )
{
    fprintf ( stderr, "%s on line %d\n", error, yylineno );
    exit ( EXIT_FAILURE );
}
