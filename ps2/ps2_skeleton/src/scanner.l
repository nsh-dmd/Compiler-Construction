%{
  #include <vslc.h>
  /* Reserved words
  FUNC, BEGIN, END, RETURN, PRINT, CONTINUE, IF, THEN, ELSE, WHILE, DO, VAR */

  int yylineno;
%}
%option noyywrap
%option array
%option yylineno

/*regular definitions*/
delim           [ \t\n\r\v]
ws              {delim}+
character       [\40-\176]
comment         {\//}({digit}|{letter})*{\n}
digit           [0-9]
number          {digit}+(\.{digit}+)?
string          \"character*\"
letter          [A-Za-z\_]
identifier      {letter}({letter|digit})*

%%
ws                      { /* no action */ }
comment                 { /* no action */ }
letter+                 { printf("%s", yytext); }
\n                      { yylineno++; }
number                  { return NUMBER; }
identifier              { return IDENTIFIER; }
string                  { return STRING; }
IF                      { return IF; }
THEN                    { return THEN; }
ELSE                    { return ELSE; }
WHILE                   { return WHILE; }
DO                      { return DO; }
VAR                     { return VAR; }
RETURN                  { return RETURN; }
PRINT                   { return PRINT; }
CONTINUE                { return CONTINUE; }
BEGIN                   { return OPENBLOCK; }
END                     { return CLOSEBLOCK; }
FUNC                    { return FUNC; }
.                       { return yytext[0]; }

%%
int yywrap(void) {
    return 1;
}
int main(void) {
    yylex();
    printf("%d\n", yylineno);
    return 0;
}
