%option noyywrap

%{
#include "parser.hh"
#include <string>

extern int yyerror(std::string msg);
%}

%%

"+"       { return TPLUS; }
"-"       { return TDASH; }
"*"       { return TSTAR; }
"/"       { return TSLASH; }
";"       { return TSCOL; }
"("       { return TLPAREN; }
")"       { return TRPAREN; }
"="       { return TEQUAL; }
":"       { return TCOL; }
","       { return TCOMMA; }
"int"     { return TINT_DTYPE; }
"short"   { return TSHORT_DTYPE; }
"long"    { return TLONG_DTYPE; }
"dbg"     { return TDBG; }
"let"     { return TLET; }
"if"      { return TIF; }
"else"    { return TELSE; }
"fun"     { return TFUN; }
"ret"     { return TRET; }
"{"       { return TLBRACE; }
"}"       { return TRBRACE; }
[0-9]+    { yylval.lexeme = std::string(yytext); return TINT_LIT; }
[a-zA-Z_]+ { yylval.lexeme = std::string(yytext); return TIDENT; }
[ \t\n]   { /* skip */ }
.         { yyerror("unknown char"); }

%%

std::string token_to_string(int token, const char *lexeme) {
    std::string s;
    switch (token) {
        case TPLUS: s = "TPLUS"; break;
        case TDASH: s = "TDASH"; break;
        case TSTAR: s = "TSTAR"; break;
        case TSLASH: s = "TSLASH"; break;
        case TSCOL: s = "TSCOL"; break;
        case TLPAREN: s = "TLPAREN"; break;
        case TRPAREN: s = "TRPAREN"; break;
        case TEQUAL: s = "TEQUAL"; break;
        case TCOL: s = "TCOL"; break;
        case TCOMMA: s = "TCOMMA"; break;
        
        case TDBG: s = "TDBG"; break;
        case TLET: s = "TLET"; break;

        case TIF: s = "TIF"; break;
        case TELSE: s = "TELSE"; break;

        case TFUN: s = "TFUN"; break;
        case TRET: s = "TRET"; break;

        case TINT_DTYPE: s = "TINT_DTYPE"; break;
        case TSHORT_DTYPE: s = "TSHORT_DTYPE"; break;
        case TLONG_DTYPE: s = "TLONG_DTYPE"; break;

        case TLBRACE: s = "TLBRACE"; break;
        case TRBRACE: s = "TRBRACE"; break;
        
        case TINT_LIT: s = "TINT_LIT"; s.append("  ").append(lexeme); break;
        case TIDENT: s = "TIDENT"; s.append("  ").append(lexeme); break;
    }

    return s;
}