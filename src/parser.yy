%define api.value.type { ParserValue }

%code requires {
#include <iostream>
#include <vector>
#include <string>

#include "parser_util.hh"
#include "symbol.hh"

}

%code {

#include <cstdlib>

extern int yylex();
extern int yyparse();

extern NodeStmts* final_values;

SymbolTable symbol_table;

int yyerror(std::string msg);

}

%token TPLUS TDASH TSTAR TSLASH
%token <lexeme> TINT_LIT TIDENT
%token TLET TDBG
%token TSCOL TLPAREN TRPAREN TEQUAL TCOL TLBRACE TRBRACE
%token TINT_DTYPE TSHORT_DTYPE TLONG_DTYPE
%token TIF TELSE

%type <node> Expr Stmt
%type <stmts> Program StmtList

%left TPLUS TDASH
%left TSTAR TSLASH

%%

Program :                
        { final_values = nullptr; }
        | StmtList 
        { final_values = $1; }
	    ;

StmtList : Stmt                
         { $$ = new NodeStmts(); $$->push_back($1); }
	     | StmtList Stmt 
         { $$->push_back($2); }
	     ;

Stmt : TLET TIDENT TCOL TINT_DTYPE TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2);

            $$ = new NodeDecl($2, $6, NodeDecl::INT);
        }
     }
     | TLET TIDENT TCOL TSHORT_DTYPE TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2);

            $$ = new NodeDecl($2, $6, NodeDecl::SHORT);
        }
     }
     | TLET TIDENT TCOL TLONG_DTYPE TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2);

            $$ = new NodeDecl($2, $6, NodeDecl::LONG);
        }
     }
     | TDBG Expr TSCOL
     { 
        $$ = new NodeDebug($2);
     }
     | TIF Expr TLBRACE StmtList TRBRACE TELSE TLBRACE StmtList TRBRACE
     {
        // printf("Parser:\n%s\n%s\n%s\n", $2->to_string().c_str(), $4->to_string().c_str(), $8->to_string().c_str());
        $$ = new NodeIfElse($2, $4, $8);
     }
     ;

Expr : TINT_LIT               
     { $$ = new NodeInt(stoi($1)); }
     | TIDENT
     { 
        if(symbol_table.contains($1))
            $$ = new NodeIdent($1); 
        else
            yyerror("using undeclared variable.\n");
     }
     | Expr TPLUS Expr
     { $$ = new NodeBinOp(NodeBinOp::PLUS, $1, $3); }
     | Expr TDASH Expr
     { $$ = new NodeBinOp(NodeBinOp::MINUS, $1, $3); }
     | Expr TSTAR Expr
     { $$ = new NodeBinOp(NodeBinOp::MULT, $1, $3); }
     | Expr TSLASH Expr
     { $$ = new NodeBinOp(NodeBinOp::DIV, $1, $3); }
     | TLPAREN Expr TRPAREN { $$ = $2; }
     ;

%%

int yyerror(std::string msg) {
    std::cerr << "Error! " << msg << std::endl;
    exit(1);
}
