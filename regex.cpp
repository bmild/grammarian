#include "regex.h"

struct Regex_star : public Regex {
    Regex *r;
    Regex_star(Regex *rr): r(rr) {}
    void print() { 
    	cout << "("; r->print(); cout << ")*"; 
    }
    int dump(NFA<int,RChar> *nfa, int k, int &ns) {
        int k1 = ns++;
        int k2 = r->dump(nfa, k1, ns);
        int f = ns++;
        nfa->add_edge(k,k1,0);
        nfa->add_edge(k2,k1,0);
        nfa->add_edge(k1,f,0);
        return f;
    }
};

struct Regex_concat : public Regex {
    Regex *r, *s;
    Regex_concat(Regex *rr, Regex *ss): r(rr), s(ss) {}
    void print() {
    	cout << "("; r->print();
    	cout << ")("; s->print(); 
    	cout << ")";
	}
    int dump(NFA<int,RChar> *nfa, int k, int &ns) {
        k = r->dump(nfa, k, ns);
        k = s->dump(nfa, k, ns);
        return k;
    }
};

struct Regex_or : public Regex {
    Regex *r, *s;
    Regex_or(Regex *rr, Regex *ss): r(rr), s(ss) {}
    void print() {
    	cout << "("; r->print(); 
    	cout << ")|("; s->print();
        cout << ")"; 
	}
    int dump(NFA<int,RChar> *nfa, int k, int &ns) {
        int k1 = ns++;
        nfa->add_edge(k,k1,0);
        int f1 = r->dump(nfa, k1, ns);
        int k2 = ns++;
        nfa->add_edge(k,k2,0);
        int f2 = s->dump(nfa, k2, ns);
        int f = ns++;
        nfa->add_edge(f1,f,0);
        nfa->add_edge(f2,f,0);
        return f;
    }
};

struct Regex_char : public Regex {
    char c;
    Regex_char(char cc): c(cc) {}
    void print() { cout << c; }
    int dump(NFA<int,RChar> *nfa, int k, int &ns) {
        int next = ns++;
        nfa->add_edge(k, next, c);
        return next;
    }
};

struct Regex_null : public Regex {
    Regex_null() {}
    void print() { cout << "EPS"; }
    int dump(NFA<int,RChar> *nfa, int k, int &ns) {
        return k;
    }
};

struct CharRange {
	Bitmap bm;
	CharRange(): bm(char_end) {}
	CharRange *set(char c) { bm.set(c); return this; }
	CharRange *set(char a, char b) {
		for (char c = a; c <= b; ++c)
			set(c);
		return this;
	}
	CharRange *invert() { bm.invert(); return this; }
	bool get(char c) { return bm.get(c); }
};

Regex *regex_star(Regex *r) { return new Regex_star(r); }
Regex *regex_concat(Regex *r, Regex *s) { return new Regex_concat(r, s); }
Regex *regex_or(Regex *r, Regex *s) { return new Regex_or(r, s); }
Regex *regex_char(char c) { return new Regex_char(c); };
Regex *regex_null() { return new Regex_null(); };
CharRange *char_range() { return new CharRange(); }
Regex *regex_range(CharRange *cr) {
    Regex *ret = NULL;
    for (char c = char_start; c <= char_end; ++c) 
    	if (cr->get(c)) 
    		ret = ret ? regex_or(ret, regex_char(c)) : regex_char(c);
    //delete [] cr;
    return ret ? ret : regex_null();
}

typedef vector<Regex_data*> VRD;

CFG<Regex_data> *regex_grammar_create() {
//	cout << "Building the regex grammar" << endl;
    Symbol cat_prec("cat_prec");
    Symbol char_prec("char_prec");

    vector<RHS<Regex_data>> all_chars;// = { {{'x'}}, {{'y'}}, {{'z'}} };
    set<char> special = {'.','|','[',']','*','(',')','+','?','-','\\','^'};
    for (char c = char_start; c <= char_end; ++c) {
    	if (!c) cout << "DIE DIE DIE" << endl;
        if (!contains(special, c))
            all_chars.push_back(RHS<Regex_data>(Symbols{Symbol(c)}, 
                [=](VRD d) { d[0]->c = c; }  ));
        all_chars.push_back(RHS<Regex_data>(Symbols{Symbol('\\'),Symbol(c)}, 
            [=](VRD d) { d[0]->c = c; }  ));
    }
    
    Symbol regex("R"), factor("F"), range("G"), ch("C");
	Regex *regex_period = regex_or(regex_range(char_range()->set(char_start,'\n'-1)),
                                regex_range(char_range()->set('\n'+1,char_end)));

    return new CFG<Regex_data> {
        {
            {regex, { {{regex,factor}, cat_prec, 
                       [](VRD d) { d[0]->r = regex_concat(d[1]->r,d[2]->r); } },
                  {{regex, '|', regex}, [](VRD d) { d[0]->r =
                                                     regex_or(d[1]->r,d[3]->r);}},
                      { {factor}, [](VRD d) { d[0]->r = d[1]->r;} }
                }
                }, {factor, {

                  {{'(', regex, ')'}, [](VRD d) { d[0]->r = d[2]->r; } },
                  {{factor, '?'}, [](VRD d) { d[0]->r = regex_or(d[1]->r, regex_null()); } },
                  {{factor, '+'}, [](VRD d) { d[0]->r = regex_concat(d[1]->r, regex_star(d[1]->r)); } },
                  {{factor, '*'}, [](VRD d) { d[0]->r = regex_star(d[1]->r); } },
                  {{'[', range, ']'}, [](VRD d) { d[0]->r = regex_range(d[2]->cr); }  },
                  {{'[', '^', range, ']'}, [](VRD d) { d[0]->r = regex_range(d[3]->cr->invert()); }  },
                  {{'.'}, [=](VRD d) { d[0]->r = regex_period; } },
                  {{ch}, [](VRD d) { d[0]->r = regex_char(d[1]->c); } }
                }
            }, 
            {range, { {{range, ch, '-', ch}, 
                       [](VRD d) { d[0]->cr = d[1]->cr->set(d[2]->c, d[4]->c); }  },
                  {{range, ch}, [](VRD d) { d[0]->cr = d[1]->cr->set(d[2]->c); }  },
                  {{ }, [](VRD d) { d[0]->cr = char_range(); }  }
                }
            },
            {ch, {  all_chars  }  }
        }
    };
}

CFG<Regex_data> *rgx_gram_cached = NULL;
CFG<Regex_data> *regex_grammar() {
	if (!rgx_gram_cached) 
		rgx_gram_cached = regex_grammar_create();
	return rgx_gram_cached;
}

void Regex_eval::initialize(string rgx, bool verbose) {
    // cout << "Making regex DFA for " << rgx << endl;
    Regex_data *data = regex_grammar()->parse(string_to_sym(rgx),verbose);
    NFA<int, RChar> nfa;
    accepting = 1;
    //data->r->print(); cout << endl;
    data->r->dump(&nfa, 0, accepting);
    // cout << "NFA created with " << nfa.states.size() << " states" << endl;
    --accepting;
    dfa = new DFA<int, RChar>(nfa);
    // cout << "DFA created with " << dfa->states.size() << " states" << endl;
    //cout << nfa.str() << endl;
    //cout << dfa->str() << endl;
}


void Regex_eval::write_dfa(string filename) {
    ofstream file(filename);
    if (file.fail()) return;
    file << dfa->edges.size() << endl;
    for (int v = 0; v < dfa->edges.size(); ++v) {
        file << v << " " << dfa->edges[v].size();
        if (contains(dfa->states[v],accepting))
            file << " 1" << endl;
        else
            file << " 0" << endl;

        for (auto &p : dfa->edges[v]) {
            file << p.second << " " << p.first.str() << endl;
        }
    }
    file.close();
}

