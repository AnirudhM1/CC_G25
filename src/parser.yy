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

extern long yylex();
extern int yyparse();

extern NodeStmts* final_values;

SymbolTableContainer symbol_table;

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
            symbol_table.insert($2, "INT");

            $$ = new NodeDecl($2, $6, NodeDecl::INT);
        }
     }
     | TLET TIDENT TCOL TSHORT_DTYPE TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2, "SHORT");

            $$ = new NodeDecl($2, $6, NodeDecl::SHORT);
        }
     }
     | TLET TIDENT TCOL TLONG_DTYPE TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2, "LONG");

            $$ = new NodeDecl($2, $6, NodeDecl::LONG);
        }
     }
     | TDBG Expr TSCOL
     {  
        $$ = new NodeDebug($2);
     }
     | TIF Expr TLBRACE 
     {
        // SymbolTable *new_table = symbol_table.add_scope();
        // symbol_table = *new_table;

        // printf("======================================\n");
        // for(auto i : new_table->parent->table) {
        //     printf("%s %s\n", i.first.c_str(), i.second.c_str());
        // }
        // printf("======================================\n");

        // symbol_table.parent = new_table->parent;
        // symbol_table.table = new_table->table;

        // printf("======================================\n");
        // printf("%d\n", symbol_table.parent->table.size());
        // for(auto i : symbol_table.parent->table) {
        //     printf("%s %s\n", i.first.c_str(), i.second.c_str());
        // }
        // printf("======================================\n");

        symbol_table.add_scope();

     }
        StmtList TRBRACE TELSE
     {
        // printf("Parser:\n%s\n%s\n%s\n", $2->to_string().c_str(), $4->to_string().c_str(), $8->to_string().c_str());
        // $$ = new NodeIfElse($2, $5, $9);
        // symbol_table = symbol_table.remove_scope();
        symbol_table.remove_scope();
     }
        TLBRACE 
     {
        symbol_table.add_scope();
     }
       StmtList TRBRACE
     {
        $$ = new NodeIfElse($2, $5, $11);
        symbol_table.remove_scope();
     }

     ;

Expr : TINT_LIT          
     { 
        $$ = new NodeInt(stol($1)); }
     | TIDENT
     { 
        if(symbol_table.contains_up($1))
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