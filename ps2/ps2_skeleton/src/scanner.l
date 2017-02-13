%{
  #include <vslc.h>
%}
%option noyywrap
%option array
%option yylineno

/*regular definitions*/
delim           [ \t\n\r\v]
ws              {delim}+
comment         \/\/([^\n])*\n
digit           [0-9]
number          {digit}+
string         \"[^"\n]*["\n]
letter          [A-Za-z_]
identifier      {letter}({letter|digit})*

%%
FUNC                    { return FUNC; }
ws                      { /* no action */ }
comment                 { /* no action */ }
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
.                       { return yytext[0]; }

%%
