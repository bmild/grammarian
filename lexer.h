#ifndef LEXER_H
#define LEXER_H
#include "regex.h"
#include "cfg.h"



struct Lexer_data {
	union {
		Lexer<int> *lexer;
	    vector<Rule<int>> *rules; 
	    Rule<int> *rule;
	    Regex_eval *regex;
	    CharRange *cr;

	};
	Symbol sym;
    char c;
    string str;
};
typedef vector<Lexer_data*> VLD;

static Symbol name("name");
//static Symbol divider("divider");

CFG<Lexer_data> *lexer_grammar_create(SymbolTable *symtab) {

    Symbol flex("flex"), rules("rules"), rule("rule"), token("token");
    Symbol regex("regex"), chars("chars");

    return new CFG<Lexer_data> {
        {
        	{flex, { 
        		{{chars, rules}, [](VLD d) { 
        			d[2]->rules->insert(d[2]->rules->begin(), *d[1]->rule);
        			d[0]->lexer = new Lexer<int>{*d[2]->rules}; } }
        	}}, 
        	{chars, {
        		{{regex}, [](VLD d) { 
        			d[0]->rule = new Rule<int>{*d[1]->regex}; }}
        	}},
        	{rules, {
        		{{rules, rule}, [](VLD d) { 
        			d[0]->rules=d[1]->rules; d[0]->rules->push_back(*d[2]->rule); } },
        		{{rule}, [](VLD d) { d[0]->rules = new vector<Rule<int>>{*d[1]->rule}; } }
        	}},
        	{rule, {
        		{{regex, token}, [](VLD d) {
        			d[0]->rule = new Rule<int>{*d[1]->regex, d[2]->sym};
        		} }
        	}},
        	{regex, {
        		{{name}, [](VLD d) {d[0]->regex = new Regex_eval(d[1]->str);} }
        	}},
        	{token, {
        		{{name}, [symtab](VLD d) {d[0]->sym = symtab->get(d[1]->str);} }
        	}}
        }
    };
}

//template <typename U>
Lexer<int> *read_lexer(string input, SymbolTable *symtab = new SymbolTable()) {
//	cout << "Building lexer lexer" << endl;
	Lexer<Lexer_data> LexerLexer = {{
        //{"[\n]"},
        //{"\%\%", divider},
        {"[^ \t\n]+", name, 
            [](Lexer_data *d,string s) {d->str = s; } }
    }};
//    cout << "Creating lexer grammar" << endl;
    CFG<Lexer_data> *cfg = lexer_grammar_create(symtab);
//    cout << "Parsing the lex input" << endl;
    Lexer_data *d = cfg->parse(LexerLexer.lex(input, false), false);
    return d?d->lexer:NULL; 
}



#endif