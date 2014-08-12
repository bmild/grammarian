#include "grammar.h"
#include "regex.h"
//#include "lexer.h"

static Symbol grammar("grammar");
static Symbol productions("productions");
static Symbol production("production");
static Symbol expansions("expansions");
static Symbol expansion("expansion");
static Symbol token("token");
static Symbol name("name");
static Symbol chsym("char");
static Symbol divider("divider");
static Symbol precedence("precedence");
static Symbol label("label");
static Symbol leftp("leftp");
static Symbol rightp("rightp");
static Symbol level("level");

CFG<Parse_data> *grammar_grammar_create() { return new CFG<Parse_data>{
	/*
    grammar,
    {grammar, productions, production, expansions, expansion, token},
    {name, chsym, ':','|',';'}, */
    {
        {grammar, {
            { {productions}, [](VPD d) { 
                d[0]->cfg = new CFG<int>(d[1]->productions); 
            } },     
            { {precedence, divider, productions}, [](VPD d) { 
                d[0]->cfg = new CFG<int>(d[3]->productions, *d[1]->pm, *d[1]->am); 
            } }        
        }},
        {precedence, {
        	{ {precedence, level}, [](VPD d) {
        		d[0]->pm = d[1]->pm;
        		d[0]->am = d[1]->am;
        		d[0]->prec = d[1]->prec;
        		for (Symbol &s : *d[2]->precsym) {
        			(*(d[0]->am))[s] = d[2]->assoc;
        			(*(d[0]->pm))[s] = d[0]->prec;
        		}
        		d[0]->prec++;
        	} },
        	{ {}, [](VPD d) {
        		d[0]->pm = new SymMap(); 
        		d[0]->am = new SymMap();
        		d[0]->prec = 0;
        	} }
        }},
        {level, {
        	{ {level, token}, [](VPD d) {
        		d[0]->assoc = d[1]->assoc;
        		d[0]->precsym = d[1]->precsym;
        		d[0]->precsym->push_back(d[2]->sym);
        	} },
        	{ {label}, [](VPD d) {
        		d[0]->assoc = (d[1]->sym == leftp ? LEFT:RIGHT);
        		d[0]->precsym = new Symbols();
        	} }
        }},
        {label, {
        	{ {rightp}, [](VPD d) {d[0]->sym = rightp;} },
        	{ {leftp}, [](VPD d) {d[0]->sym = leftp;} }
        }},
        {productions, {
            { {productions, production}, [](VPD d) {
                d[0]->productions = d[1]->productions;
                d[0]->productions->insert(d[0]->productions->end(),
                                          d[2]->productions->begin(),
                                          d[2]->productions->end());
                } },
            { {production}, [](VPD d) {
                d[0]->productions = d[1]->productions;
                } }
        }},
        {production, {
            { {name, ':', expansions, ';'}, [](VPD d) {
                d[0]->productions = new vector<Production<int>>();
                for (Symbols *rhs : *(d[3]->rhs_list)) {
                    d[0]->productions->emplace_back(d[1]->sym, *rhs);
                }
                } }
        }},
        {expansions, {
            { {expansions, '|', expansion}, [](VPD d) {
                d[0]->rhs_list = d[1]->rhs_list;
                d[0]->rhs_list->push_back(d[3]->rhs);
                } },
            { {expansion}, [](VPD d) {
                d[0]->rhs_list = new vector<Symbols *>{ d[1]->rhs };
                } }
        }},
        {expansion, {
            { {expansion, token}, [](VPD d) {
                d[0]->rhs = d[1]->rhs;
                if (d[2]->sym != eps) 
                    d[0]->rhs->push_back(d[2]->sym);
                } },
            { {}, [](VPD d) {
                d[0]->rhs = new Symbols;
                } }
        }},
        {token, {
            { {name}, [](VPD d) {
                d[0]->sym = d[1]->sym; 
                } },
            { {chsym}, [](VPD d) {
                d[0]->sym = d[1]->sym;
                } }
        }}
    }
};
}


CFG<Parse_data> *gram_gram_cached = NULL;
CFG<Parse_data> *grammar_grammar() {
	if (!gram_gram_cached)
		gram_gram_cached = grammar_grammar_create();
	return gram_gram_cached;
}

CFG<int> *read_grammar(string input, SymbolTable *symtab = new SymbolTable()) {
//	cout << "Making grammar lexer" << endl;
    Lexer<Parse_data> GrammarLexer = {{
        {"[:\\|;]"},
        {"\%left", leftp},
        {"\%right", rightp}, 
        {"\%\%", divider}, 
        {"\\'.\\'", chsym, 
            [symtab](Parse_data *d,string s) {d->sym = Symbol(s[1]); } },
        {"[a-zA-Z][a-zA-Z0-9_]*", name, 
            [symtab](Parse_data *d,string s) {d->sym = symtab->get(s); } }
    }};
    Parse_data *d = grammar_grammar()->parse(GrammarLexer.lex(input, false), false);
    return d?d->cfg:NULL; 
}