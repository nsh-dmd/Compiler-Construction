%{
  #include <vslc.h>
%}
%option noyywrap
%option array
%option yylineno

/*regular definitions*/
delim                   [ \t\n\r\v]
ws                      {delim}+
comment                 \/\/[^\n]*\n
digit                   [0-9]
number                  {digit}+
letter                  [A-Za-z_]
identifier              {letter}({letter}|{digit})*
string                  \"([^\n\"]|\\\")*\"

%%
FUNC                    { return FUNC; }
BEGIN                   { return OPENBLOCK; }
END                     { return CLOSEBLOCK; }
RETURN                  { return RETURN; }
CONTINUE                { return CONTINUE; }
PRINT                   { return PRINT; }
IF                      { return IF; }
THEN                    { return THEN; }
ELSE                    { return ELSE; }
WHILE                   { return WHILE; }
DO                      { return DO; }
VAR                     { return VAR; }
{ws}                    { /* no action */ }
{comment}               { /* no action */ }
{number}                { return NUMBER; }
{identifier}            { return IDENTIFIER; }
{string}                { return STRING; }
.                       { return yytext[0]; }
%%
