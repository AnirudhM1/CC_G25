%option prefix="pr"
%option noyywrap

%{
#include "parser.hh"
#include <string>
#include <unordered_map>     
#include <string>     
#include <cstring>     
#include <tuple>      
 
std::unordered_map<std::string, std::string> hashmap;   
std::string temp = "";   
 
extern int prerror(std::string msg);
 
%}

%%
 
"//".*    { }
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]       { }
[/][*]    {yy_fatal_error("Comment not terminated");}
"#def"[ ]+[a-zA-Z]+[ a-zA-Z0-9+-/*();=]* {
    std::string s = std::string(yytext);
    std::string key = "";
 
    int i=4;
    while(s[i] == ' ')
        i++;
 
    while (i < (int)s.size() and s[i] != ' ') {
        key += s[i];
        i++;
    }
 
    while(i<(int)s.size() and s[i] == ' ')
        i++;
    
    std::string value = "";
    std::string TempString = "";
    for( ; i<(int)s.size() ; i++){
        if(!((s[i] >= 'a' and s[i] <= 'z') or (s[i] >= 'A' and s[i] <= 'Z'))){
            if(TempString.size() > 0){
                if(hashmap.find(TempString) != hashmap.end()){
                    if(hashmap[TempString] == key)
                        yy_fatal_error("Syntax Error\n");
                    else
                        value += hashmap[TempString];
                }
                else
                    value += TempString;
                TempString = "";
            }
            value += s[i];
        }
        else
            TempString += s[i];
    }
 
    if(TempString.size() > 0){
        if(hashmap.find(TempString) != hashmap.end()){
            if(hashmap[TempString] == key)
                yy_fatal_error("Syntax Error\n");
            else
                value += hashmap[TempString];
        }
        else
            value += TempString;
        TempString = "";
    }
 
    if(value.size() == 0)
        value = "1";
    
    if(value == key)
        yy_fatal_error("Syntax Error\n");
    hashmap[key] = value;
}
"#undef"[ ]+[a-zA-Z]+ {
    std::string s = std::string(yytext);
    std::string key = "";
    int i=6;
    while(s[i] == ' ')
        i++;
    while (s[i] != '\0') {
        key += s[i];
        i++;
    }
    hashmap.erase(key);
}
[a-zA-Z] {
    std::string s = std::string(yytext);
    temp += s;
}
. { 
    if(temp.size() > 0){
        if(hashmap.find(temp) != hashmap.end())
            fprintf(prout , "%s" , hashmap[temp].c_str());
        else
            fprintf(prout , "%s" , temp.c_str());
        temp = "";
    }
    fprintf(prout , "%s" , yytext);
}
%%