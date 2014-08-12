#ifndef REGEX_H
#define REGEX_H
#include "cfg.h"
#include "automata.h"


const char char_start = 1, char_end = 126;

struct Regex {
    virtual void print() = 0;
    virtual int dump(NFA<int,RChar> *nfa, int k, int &ns) = 0;
};  
struct CharRange;

union Regex_data {
    Regex *r;
    CharRange *cr;
    char c;
};

CFG<Regex_data> *regex_grammar();

typedef list<RChar> RString;
static RString str_to_rchar(string s) {
    RString ret;
    for (char c : s) ret.push_back(c);
    return ret; 
}

struct Regex_eval {
    DFA<int, RChar> *dfa;
    int accepting;
    Regex_eval(const char *c_str, bool verbose = false) {
        initialize(string(c_str), verbose);
    }
    Regex_eval(string rgx, bool verbose = false) {
        initialize(rgx, verbose);
    }
    void initialize(string rgx, bool verbose);
    bool accept(string s) {
        return accept(str_to_rchar(s));
    }
    bool accept(const RString &input) {
        int final = dfa->run_on(input);
        return (final >= 0 && contains(dfa->states[final], accepting));
    }
    int munch(string s) {
        return munch(str_to_rchar(s));
    }
    int munch(const RString &input) {
        return dfa->munch(input);
    }
    void write_dfa(string filename);
};


template <typename U>
struct Rule {
    Regex_eval regex;
    Symbol token;
    function<void(U*,string)> Semantic;
};

template <typename U>
struct Lexer {
    vector<Rule<U>> rules;
    vector<pair<Symbol, U>> lex(string input_str, bool verbose = false, bool truenames = false) {
        if (verbose)
            cout << "~~~ Lexing " << input_str << " ~~~" << endl;
        RString input = str_to_rchar(input_str);
        vector<pair<Symbol, U>> output;
        while (!input.empty()) {
            int max_munch = 0;
            Symbol max_sym;
            function<void(U*,string)> Semantic;
            for (Rule<U> &rule : rules) {
                int m = rule.regex.munch(input);
                // if (m>0)
                // cout << "Munch from rule " << rule.token.str() << " is " << input_str.substr(0,m) << endl;
                if (m > max_munch) {
                    max_munch = m;
                    max_sym = rule.token;
                    Semantic = rule.Semantic;
                }
            }
            if (max_munch == 0) {
                input_str = input_str.substr(1);
                input.pop_front();
            } else {
                string munch(input_str.substr(0,max_munch));
                input_str = input_str.substr(max_munch);
                for (int m=0; m<max_munch; ++m) 
                    input.pop_front();
                U new_u;
                if (Semantic) Semantic(&new_u, munch);
                else if (max_munch == 1 && max_sym == eps) max_sym = Symbol(munch[0]);
                if (verbose)
                    cout << "<" << max_sym.str() << "> \"" << munch << "\"" << endl;
                // TURN ON OR OFF FOR SHOWING ACTUAL SYMBOL STRINGS:
                if (truenames) max_sym.name = munch;
                output.push_back({max_sym, new_u});
            }
        }
        return output;
    }
};

#endif
