%option prefix="ppr"
%option noyywrap

%{
#include "parser.hh"
#include <string>
#include <set>        
#include <cstring>
#include <tuple>

std::set <std::string> keywords;
int check = 0;

void print_string(std::string s , int i);
%}

%%

"//".*    { }
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]       { }
[/][*]    {yy_fatal_error("Comment not terminated");}
"#def"[ ]+[a-zA-Z0-9]+([ a-zA-Z0-9+-/*();=\\]*\\[ ]*\n)*[ a-zA-Z0-9+-/*();=\\]* {

    std::string s = std::string(yytext);
    std::string key = "";
 
    int i=4;
    while(s[i] == ' ')
        i++;
 
    while (i < (int)s.size() and ((s[i] >= 'a' and s[i] <= 'z') or (s[i] >= 'A' and s[i] <= 'Z') or (s[i] >= '0' and s[i] <= '9')) ) {
        key += s[i];
        i++;
    }
    
    keywords.insert(key);
    fprintf(pprout , "%s" , yytext);
}
"#ifdef"[ ]+[a-zA-Z0-9]+[ a-zA-Z0-9+\-\/*();=\n\t\\]*"#d"*[ a-zA-Z0-9+\-\/*();=\n\t\\]*"#u"*[ a-zA-Z0-9+\-\/*();=\n\t\\]*"#e" {
    check = 0;
    std::string s = std::string(yytext);
    std::string key = "";

    int i=6;
    while(s[i] == ' ')
        i++;
 
    while (i < (int)s.size() and ((s[i] >= 'a' and s[i] <= 'z') or (s[i] >= 'A' and s[i] <= 'Z') or (s[i] >= '0' and s[i] <= '9')) ) {
        key += s[i];
        i++;
    }

    if(keywords.find(key) != keywords.end()){
        check = 1;
        print_string(s , i);
    }
}
"lif"[ ]+[a-zA-Z0-9]+[ a-zA-Z0-9+\-\/*();=\n\t\\]*"#d"*[ a-zA-Z0-9+\-\/*();=\n\t\\]*"#u"*[ a-zA-Z0-9+\-\/*();=\n\t\\]*"#e" {
    if(check == 0){
        std::string s = std::string(yytext);
        std::string key = "";

        int i=3;
        while(s[i] == ' ')
            i++;
    
        while (i < (int)s.size() and ((s[i] >= 'a' and s[i] <= 'z') or (s[i] >= 'A' and s[i] <= 'Z') or (s[i] >= '0' and s[i] <= '9')) ) {
            key += s[i];
            i++;
        }

        if(keywords.find(key) != keywords.end()){
            check = 1;
            print_string(s , i);
        }
    }
}
"lse"[ ]+[a-zA-Z0-9]+[ a-zA-Z0-9+\-\/*();=\n\t\\]*"#d"*[ a-zA-Z0-9+\-\/*();=\n\t\\]*"#u"*[ a-zA-Z0-9+\-\/*();=\n\t\\]*"#e" {
    if(check == 0){
        std::string s = std::string(yytext);
        check = 1;
        print_string(s , 3);
    }
}
"ndif" {

}
"#undef"[ ]+[a-zA-Z]+ {
    std::string s = std::string(yytext);
    std::string key = "";
    int i=6;
    while(s[i] == ' ')
        i++;
    while (i < (int)s.size() and ((s[i] >= 'a' and s[i] <= 'z') or (s[i] >= 'A' and s[i] <= 'Z') or (s[i] >= '0' and s[i] <= '9')) ) {
        key += s[i];
        i++;
    }
    keywords.erase(key);
    fprintf(pprout , "%s" , yytext);
}
. { 
    fprintf(pprout , "%s" , yytext);
}

%%

void print_string(std::string s , int i)
{
    for( ; i<(int)s.size()-2 ; i++){
        if(s[i] == '#' and s[i+1] == 'd'){
            int curri = i;
            i += 4;

            std::string key = "";
            while(s[i] == ' ')
                i++;
            while (i < (int)s.size() and ((s[i] >= 'a' and s[i] <= 'z') or (s[i] >= 'A' and s[i] <= 'Z') or (s[i] >= '0' and s[i] <= '9')) ) {
                key += s[i];
                i++;
            }
            keywords.insert(key);

            i = curri;
        }
        else if(s[i] == '#' and s[i+1] == 'u'){
            int curri = i;
            i += 6;

            std::string key = "";
            while(s[i] == ' ')
                i++;
            while (i < (int)s.size() and ((s[i] >= 'a' and s[i] <= 'z') or (s[i] >= 'A' and s[i] <= 'Z') or (s[i] >= '0' and s[i] <= '9')) ) {
                key += s[i];
                i++;
            }
            keywords.erase(key);

            i = curri;
        }

        fprintf(pprout , "%c" , s[i]);
    }
}