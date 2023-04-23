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

int maxx=3;
SymbolTableContainer symbol_table;

int yyerror(std::string msg);

}

%token TPLUS TDASH TSTAR TSLASH
%token <lexeme> TINT_LIT TIDENT
%token TLET TDBG
%token TSCOL TLPAREN TRPAREN TEQUAL TCOL TLBRACE TRBRACE TCOMMA
%token TINT_DTYPE TSHORT_DTYPE TLONG_DTYPE
%token TIF TELSE TFUN TRET

%type <node> Expr Stmt ParamDecl ParamPass
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

ParamDecl : TIDENT TCOL TINT_DTYPE
          { $$ = new NodeParamDecl($1, NodeDecl::INT); }
          | TIDENT TCOL TSHORT_DTYPE
          { $$ = new NodeParamDecl($1, NodeDecl::SHORT); }
          | TIDENT TCOL TLONG_DTYPE
          { $$ = new NodeParamDecl($1, NodeDecl::LONG); }
          | ParamDecl TCOMMA TIDENT TCOL TINT_DTYPE
          { $$ = new NodeParamDecl($1, $3, NodeDecl::INT); }
          | ParamDecl TCOMMA TIDENT TCOL TSHORT_DTYPE
          { $$ = new NodeParamDecl($1, $3, NodeDecl::SHORT); }
          | ParamDecl TCOMMA TIDENT TCOL TLONG_DTYPE
          { $$ = new NodeParamDecl($1, $3, NodeDecl::LONG); }
         ;

ParamPass : Expr
            { $$ = new NodeParamPass($1); }
            | ParamPass TCOMMA Expr
            { $$ = new NodeParamPass($1, $3); }
            ;

Stmt : TLET TIDENT TCOL TINT_DTYPE { maxx=1; } TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2, "INT");

            $$ = new NodeDecl($2, $7, NodeDecl::INT);
        }
        maxx = 3;
     }
     | TLET TIDENT TCOL TSHORT_DTYPE  { maxx=0; }TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2, "SHORT");

            $$ = new NodeDecl($2, $7, NodeDecl::SHORT);
        }
        maxx = 3;
     }
     | TLET TIDENT TCOL TLONG_DTYPE { maxx=2; } TEQUAL Expr TSCOL
     {
        if(symbol_table.contains($2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        } else {
            symbol_table.insert($2, "LONG");

            $$ = new NodeDecl($2, $7, NodeDecl::LONG);
        }
        maxx = 3;
     }
     | TDBG Expr TSCOL
     {  
        $$ = new NodeDebug($2);
     }
     | TRET Expr TSCOL
     {
        $$ = new NodeReturn($2);
     }
     | TIF Expr TLBRACE 
     {

        symbol_table.add_scope();

     }
        StmtList TRBRACE TELSE
     {

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

     | TFUN TIDENT TLPAREN TRPAREN TCOL TINT_DTYPE TLBRACE 
     {
         symbol_table.insert($2, "FUN");
         symbol_table.add_scope();
     }
     StmtList TRBRACE
     {
        std::vector<NodeDecl::DataType> param_types;
        $$ = new NodeFunDef($2, param_types, std::vector<std::string>(), $9, NodeDecl::INT);
        symbol_table.remove_scope();
     }
     | TFUN TIDENT TLPAREN TRPAREN TCOL TSHORT_DTYPE TLBRACE 
     {
         symbol_table.insert($2, "FUN");
         symbol_table.add_scope();
     }
     StmtList TRBRACE
     {
        std::vector<NodeDecl::DataType> param_types;
        $$ = new NodeFunDef($2, param_types, std::vector<std::string>(), $9, NodeDecl::SHORT);
        symbol_table.remove_scope();
     }
     | TFUN TIDENT TLPAREN TRPAREN TCOL TLONG_DTYPE TLBRACE 
     {
         symbol_table.insert($2, "FUN");
         symbol_table.add_scope();
     }
     StmtList TRBRACE
     {
        std::vector<NodeDecl::DataType> param_types;
        $$ = new NodeFunDef($2, param_types, std::vector<std::string>(), $9, NodeDecl::LONG);
        symbol_table.remove_scope();
     }
     | TFUN TIDENT TLPAREN ParamDecl TRPAREN TCOL TINT_DTYPE TLBRACE 
     {
         symbol_table.insert($2, "FUN");
         symbol_table.add_scope();
         std::vector<std::string> identifiers = ((NodeParamDecl*)$4)->get_node_list();
         std::vector<NodeDecl::DataType> types = ((NodeParamDecl*)$4)->get_type_list();
         for(int i = 0; i < (int)identifiers.size(); i++) {
             std::string dtype;
               switch(types[i]) {
                  case NodeDecl::INT:
                        dtype = "INT";
                        break;
                  case NodeDecl::SHORT:
                        dtype = "SHORT";
                        break;
                  case NodeDecl::LONG:
                        dtype = "LONG";
                        break;
               }
             symbol_table.insert(identifiers[i], dtype);
         }

     }
     StmtList TRBRACE
     {
         std::vector<NodeDecl::DataType> param_types = ((NodeParamDecl*)$4)->get_type_list();
         std::vector<std::string> param_names = ((NodeParamDecl*)$4)->get_node_list();
         $$ = new NodeFunDef($2, param_types, param_names, $10, NodeDecl::INT);
         symbol_table.remove_scope();
     }
     | TFUN TIDENT TLPAREN ParamDecl TRPAREN TCOL TLONG_DTYPE TLBRACE 
     {
         symbol_table.insert($2, "FUN");
         symbol_table.add_scope();
         std::vector<std::string> identifiers = ((NodeParamDecl*)$4)->get_node_list();
         std::vector<NodeDecl::DataType> types = ((NodeParamDecl*)$4)->get_type_list();
         for(int i = 0; i < (int)identifiers.size(); i++) {
             std::string dtype;
               switch(types[i]) {
                  case NodeDecl::INT:
                        dtype = "INT";
                        break;
                  case NodeDecl::SHORT:
                        dtype = "SHORT";
                        break;
                  case NodeDecl::LONG:
                        dtype = "LONG";
                        break;
               }
             symbol_table.insert(identifiers[i], dtype);
         }

     }
     StmtList TRBRACE
     {
         std::vector<NodeDecl::DataType> param_types = ((NodeParamDecl*)$4)->get_type_list();
         std::vector<std::string> param_names = ((NodeParamDecl*)$4)->get_node_list();
         $$ = new NodeFunDef($2, param_types, param_names, $10, NodeDecl::LONG);
         symbol_table.remove_scope();
     }
     | TFUN TIDENT TLPAREN ParamDecl TRPAREN TCOL TSHORT_DTYPE TLBRACE 
     {
         symbol_table.insert($2, "FUN");
         symbol_table.add_scope();
         std::vector<std::string> identifiers = ((NodeParamDecl*)$4)->get_node_list();
         std::vector<NodeDecl::DataType> types = ((NodeParamDecl*)$4)->get_type_list();
         for(int i = 0; i < (int)identifiers.size(); i++) {
             std::string dtype;
               switch(types[i]) {
                  case NodeDecl::INT:
                        dtype = "INT";
                        break;
                  case NodeDecl::SHORT:
                        dtype = "SHORT";
                        break;
                  case NodeDecl::LONG:
                        dtype = "LONG";
                        break;
               }
             symbol_table.insert(identifiers[i], dtype);
         }

     }
     StmtList TRBRACE
     {
         std::vector<NodeDecl::DataType> param_types = ((NodeParamDecl*)$4)->get_type_list();
         std::vector<std::string> param_names = ((NodeParamDecl*)$4)->get_node_list();
         $$ = new NodeFunDef($2, param_types, param_names, $10, NodeDecl::SHORT);
         symbol_table.remove_scope();
     }

     ;

Expr : TINT_LIT          
     { 
        long long int x = stol($1); 
        if(maxx == 0)
            x = (short)x;
        else if(maxx == 1)
            x = (int)x;
        else
            x = (long)x;
        $$ = new NodeInt(x);
     }
     | TIDENT
     { 
        if(symbol_table.contains_up($1))
        {
            $$ = new NodeIdent($1); 
            int curr = 0;
            if(symbol_table.get_type($1) == "INT")
                curr = 1;
            else if(symbol_table.get_type($1) == "SHORT")
                curr = 0;
            else
                curr = 2;
            if(curr > maxx)
                {
                    yyerror("Type coersion detected. \n");
                }
        }           
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
     | TIDENT TLPAREN TRPAREN
     {
        if(symbol_table.contains_up($1))
            $$ = new NodeFunCall($1, std::vector<Node*>());
        else
            yyerror("using undeclared function.\n");
     }
     | TIDENT TLPAREN ParamPass TRPAREN
     {
        if(symbol_table.contains_up($1))
            $$ = new NodeFunCall($1, ((NodeParamPass*)$3)->get_list());
        else
            yyerror("using undeclared function.\n");
     }
     ;

%%

int yyerror(std::string msg) {
    std::cerr << "Error! " << msg << std::endl;
    exit(1);
}