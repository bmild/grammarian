#ifndef CFG_H
#define CFG_H
#include "symbol.h"
#include "automata.h"
#include <functional>

static Symbol eps(0, "eps");
static Symbol mark(-1, ".");
static Symbol dummy(-2, "S'");
static Symbol fin(-3,"$");

template <typename U>
struct Production {
    Symbol lhs;
    Symbols rhs;
    Symbol prec_sym;

    function<void(vector<U*>)> Semantic;
    //void (*Semantic)(vector<U>);

    Production() {}
    Production(Symbol _lhs, Symbols _rhs)
    : lhs(_lhs), rhs(_rhs), prec_sym(eps) {}
    Production(Symbol _lhs, Symbols _rhs, Symbol pc)
    : lhs(_lhs), rhs(_rhs), prec_sym(pc)  {}
    Production(Symbol _lhs, Symbols _rhs, function<void(vector<U*>)> S)
    : lhs(_lhs), rhs(_rhs), prec_sym(eps) { Semantic = S; }
    Production(Symbol _lhs, Symbols _rhs, Symbol pc, function<void(vector<U*>)> S)
    : lhs(_lhs), rhs(_rhs), prec_sym(pc) { Semantic = S; }

    string str() const {
        string str = lhs.str() + " ->";
        for (const Symbol &sym : rhs)
            str += " " + sym.str();
        return str;
    }
    bool is_reduce() { return rhs.back() == eps; }
    bool operator==(const Production<U> &p) const 
    { return lhs == p.lhs && rhs == p.rhs; }
    bool operator<(const Production<U> &p) const 
    { return lhs < p.lhs || (lhs == p.lhs && rhs < p.rhs); }
};

template <typename U>
string to_string(const Production<U> &p) { return p.str(); }

template <typename U>
struct RHS {
    Symbols rhs;
    Symbol prec_sym;
    function<void(vector<U*>)> Semantic;
    RHS() {};
    RHS(Symbols _rhs): rhs(_rhs), prec_sym(eps) {}
    RHS(Symbols _rhs, function<void(vector<U*>)> S): rhs(_rhs), prec_sym(eps) { Semantic = S; }
    RHS(Symbols _rhs, Symbol pc): rhs(_rhs), prec_sym(pc) {}
    RHS(Symbols _rhs, Symbol pc, function<void(vector<U*>)> S): rhs(_rhs), prec_sym(pc) { Semantic = S; }
};

template <typename U>
struct StackElem {
    Symbol sym;
    int state;
    U data;
};
template <typename U>
struct ParseStack : public vector<StackElem<U>*> {
    Symbol sym() {
        return vector<StackElem<U>*>::empty() ? eps : 
            vector<StackElem<U>*>::back()->sym;
    }
    int state() { 
        return vector<StackElem<U>*>::empty() ? 0 : 
            vector<StackElem<U>*>::back()->state;
    }
};

enum { LEFT, RIGHT };
typedef map<Symbol,int> SymMap;
template <typename U>
struct CFG {
    Symbol start;
    Symbols nonterms;
    Symbols terms;
    vector<Production<U>> rules;
    CFG() {}
    // Directly initialize
    CFG(Symbol _start, Symbols _nonterms, Symbols _terms, vector<Production<U>> _rules, 
        Symbols pl = Symbols(), const SymMap &am = SymMap() )
        : start(_start), nonterms(_nonterms), terms(_terms),
          rules(_rules), prec_list(pl), assoc_map(am)
    { initialize(); }
    // Extract productions from rule_map, sending a
    // LHS symbol to a list of RHS it can expand to,
    // optionally incl. precedence and a callback for the rule.
    CFG(Symbol _start, Symbols _nonterms, Symbols _terms,
        map<Symbol, vector<RHS<U>>> rule_map,
        Symbols pl = Symbols(), const SymMap &am = SymMap() )
        : start(_start), nonterms(_nonterms), terms(_terms), 
          prec_list(pl), assoc_map(am)
    {
        for (auto &p : rule_map)
            for (RHS<U> &rs : p.second) 
                rules.push_back({p.first, rs.rhs, rs.prec_sym, rs.Semantic});
        initialize();
    }
    // Auto extract the NT/T from rule_map
    CFG(vector<pair<Symbol, vector<RHS<U>>>> rule_map,
        Symbols pl = Symbols(), const SymMap &am = SymMap() )
        : prec_list(pl), assoc_map(am)
    {
        for (auto &p : rule_map)
            for (RHS<U> &rs : p.second) 
                rules.push_back({p.first, rs.rhs, rs.prec_sym, rs.Semantic});
        autoextract_symbols();
        initialize();
    }
    // Auto extract the NT/T from rules list
    CFG(vector<Production<U>> *rules,
        const SymMap &pm = SymMap(), const SymMap &am = SymMap() )
        : prec_map(pm), assoc_map(am)
    {
        if (rules->empty()) return;
        this->rules = *rules;
        autoextract_symbols();
        initialize();
    }
    void autoextract_symbols() {
        start = rules.at(0).lhs;
        for (Production<U> &p : rules) {
            if (syms.find(p.lhs) == syms.end()) {
                syms[p.lhs] = NONTERM;
                nonterms.push_back(p.lhs);
            }
        }
        for (Production<U> &p : rules) {
            for (Symbol &s : p.rhs) {
                if (syms.find(s) == syms.end()) {
                    syms[s] = TERM;
                    terms.push_back(s);
                }
            }
        }
    }

    Symbols prec_list;
    SymMap assoc_map;
    SymMap prec_map;

    string str() { 
        string str = "Start: " + start.str();
        str += "\nNonterms: ";
        for (Symbol &nt : nonterms)  str += nt.str() + " ";
        str += "\nTerms: ";
        for (Symbol &t : terms)  str += t.str() + " ";
        string rules_str;
        for (Production<U> &p : rules) rules_str += "\n" + p.str();
        str += "\nRules:" + rules_str;
        return str;
    }
    vector<Production<U>> get_prods(Symbol sym) {
        vector<Production<U>> prods;
        for (Production<U> &rule : rules)
            if (rule.lhs == sym)
                prods.push_back(rule);
        return prods;
    }

    int get_prec(const Symbol &s) { 
        return prec_map.find(s)==prec_map.end() ? 0:prec_map[s]; 
    }
    int get_assoc(const Symbol &s) { return assoc_map[s]; }
    
    map<Symbol, int> syms;
    enum { NONTERM, TERM };
    map<Symbol, set<Symbol>> first_sets;
    map<Symbol, set<Symbol>> follow_sets;

    void initialize() {
        syms.clear(); 
        for (Symbol &t : terms) syms[t] = TERM;
        for (Symbol &n : nonterms) syms[n] = NONTERM;

        set_precedence();
        set_first_sets();
        set_follow_sets();

        set_parser();
    }
    void set_precedence() {
        //prec_map.clear();
        for (int i = 0; i < prec_list.size(); ++i)
            prec_map[prec_list[i]] = i;
        for (Production<U> &prod : rules) {
            if (prod.prec_sym == eps) {
                Symbol s = eps;
                for (const Symbol &x : prod.rhs)
                    if (x != mark && syms[x] == TERM)
                        s = x;
                prod.prec_sym = s;
            }

        }
    }
    void set_first_sets() {
        // First sets
        first_sets.clear(); 
        for (Symbol &t : terms)
            first_sets[t].insert(t);
        bool changing = true;
        while (changing) {
            changing = false;
            for (Symbol &n : nonterms) {
                vector<Production<U>> prods = get_prods(n);
                for (Production<U> &p : prods) {
                    int i;
                    for (i = 0; i < p.rhs.size(); ++i) 
                        if (!contains(first_sets[p.rhs[i]], eps))
                            break;
                    if (i == p.rhs.size() && !contains(first_sets[n], eps)) {
                        changing = true;
                        first_sets[n].insert(eps);
                    } else if (!p.rhs.empty() && i < p.rhs.size() &&
                            !contains(first_sets[n], first_sets[p.rhs[i]])) {
                        changing = true;
                        insert(first_sets[n], first_sets[p.rhs[i]]);
                    } 
                }
            }
        }

        // for (auto &p : first_sets) {
        //     cout << "First of " << p.first.str() << endl;
        //     for (auto &s : p.second) {
        //         cout << s.str() << endl;
        //     }
        // }
    }
    void set_follow_sets() {
        // Follow sets
        follow_sets.clear();
        follow_sets[start].insert(fin);
        for (Production<U> &p : rules) {
            for (int i = 0; i + 1 < p.rhs.size(); ++i) 
                insert(follow_sets[p.rhs[i]], first_sets[p.rhs[i+1]]);
        }
        bool changing = true;
        while (changing) {
            changing = false;
            for (Symbol &n : nonterms) {
                if (follow_sets[n].empty()) continue;
                vector<Production<U>> prods = get_prods(n);
                for (Production<U> &p : prods) {
                    int i;
                    for (i = p.rhs.size(); i > 0; --i) {
                        if (i < p.rhs.size() &&
                            !contains(first_sets[p.rhs[i]], eps))
                            break;

                        if (i >= 0 && !follow_sets[n].empty() && 
                            !contains(follow_sets[p.rhs[i-1]],
                        follow_sets[n])) {
                                changing = true;
                                insert(follow_sets[p.rhs[i-1]],
                        follow_sets[n]);
                        }
                    }
                }
            }
        }

        for (auto &p : follow_sets) {
            auto iter = p.second.find(eps);
            if (iter != p.second.end())
                p.second.erase(iter);
        }

        // for (auto &p : follow_sets) {
        //     cout << "Follow of " << p.first.str() << endl;
        //     for (auto &s : p.second) {
        //         cout << s.str() << endl;
        //     }
        // }
    }
    DFA<Production<U>, Symbol> *dfa;
    void set_parser() {
        NFA<Production<U>, Symbol> nfa;
        rules.insert(rules.begin(), {dummy, {start}});
        for (Production<U> &rule : rules) {
            for (int i = 0; i < rule.rhs.size(); ++i) {
                Symbols rhs(rule.rhs);
                Symbol X = rhs[i];
                rhs.insert(rhs.begin()+i, mark);
                Production<U> mrule0({rule.lhs, rhs, rule.prec_sym, rule.Semantic});
                swap(rhs[i],rhs[i+1]);
                Production<U> mrule1({rule.lhs, rhs, rule.prec_sym, rule.Semantic});
                nfa.add_edge(mrule0, mrule1, X);
                
                vector<Production<U>> pv = get_prods(X);
                for (Production<U> &p : pv) {
                    Symbols rhs(p.rhs);
                    rhs.insert(rhs.begin(), mark);
                    nfa.add_edge(mrule0, {X,rhs,p.Semantic}, eps);
                }
            }
        }
        rules.erase(rules.begin());
        dfa = new DFA<Production<U>, Symbol>(nfa);
    }

    Production<U> reduce(int s) {
        set<int> nfa_indices = dfa->states[s];
        for (int i : nfa_indices) {
            if (dfa->nfa_states[i].rhs.back() == mark) 
                return dfa->nfa_states[i];
        }
        return {eps,{}};
    }
    bool shift(int s, Symbol t) {
        auto iter = dfa->edges[s].find(t);
        return iter != dfa->edges[s].end();
    }
    U *parse(Symbols input_, bool verbose = false) {
        vector<pair<Symbol,U>> input;
        for (const Symbol &s : input_)
            input.push_back({s, U()});
        return parse(input, verbose);
    }
    U *parse(vector<pair<Symbol, U>> input, bool verbose = false) {
        if (verbose)
            cout << "~~~ Parsing " << vec_str(input) << " ~~~" << endl;
        ParseStack<U> stack;
        input.push_back({fin,U()});
        int pos = 0;
        while (true) {
            string curr;
            for (auto &se : stack) curr += se->sym.str() + " ";
            if (verbose)
                cout << setw(input.size()+10) << left << 
                    (curr + "| " + list_str<U>(input.begin()+pos,input.end()));
            if (pos == input.size()-1 && stack.size() == 1 && stack.sym() == start) {
                if (verbose)
                    cout << "Success!" << endl;
                return &stack[0]->data;
            }
            Production<U> reduce_prod = reduce(stack.state());
            bool can_reduce = (reduce_prod.lhs != eps &&
                               contains(follow_sets[reduce_prod.lhs],
                                        input[pos].first));
            bool can_shift = shift(stack.state(),input[pos].first);
            string sr_str;
            if (can_reduce && can_shift) {
                Symbol r_sym = reduce_prod.prec_sym;
                Symbol s_sym = input[pos].first;
                if (r_sym == s_sym) {
                    if (r_sym == eps) {
                        if (verbose)
                            cout << "Ambiguity not resolvable in DFA state " <<
                                set_str(dfa->states[stack.state()], dfa->nfa_states) << 
                                " Aborting" << endl;
                        return NULL;
                    }
                    can_reduce = get_assoc(r_sym) == LEFT; 
                    if (verbose)
                        sr_str =  string(" (SR conflict resolved by ") + 
                            (can_reduce?"left":"right") + " associativity of " 
                            + r_sym.str() + ")";
                } else {
                   can_reduce = get_prec(r_sym) < get_prec(s_sym);
                   if (verbose)
                       sr_str = string(" (SR conflict resolved by precedence of ") +
                            (can_reduce?r_sym.str():s_sym.str())
                             + " over " + (can_reduce?s_sym.str():r_sym.str()) + ")";
                }
            }
            if (can_reduce) {
                // Need space for RHS without the '.' marker, plus the LHS 
                int to_pop = reduce_prod.rhs.size();
                vector<U*> data(to_pop);
                // for (int i = reduce_prod.rhs.size()-2; i >= 0; --i) 
                //     data.push_back(&stack[stack.size() - 1 - i]->data);
                //for (int i = 0; i < reduce_prod.rhs.size()-1; ++i) {
                for (--to_pop; to_pop > 0; --to_pop) {
                    data[to_pop] = &stack.back()->data;
                    stack.pop_back();
                }
                stack.push_back(new StackElem<U>{reduce_prod.lhs, 
                                        dfa->run_on(reduce_prod.lhs, stack.state())});
                data[0] = &stack.back()->data;
                if (verbose)
                    cout << "Reduce using " << reduce_prod.str() << sr_str << endl;

                if (reduce_prod.Semantic) 
                   reduce_prod.Semantic(data);
            } else if (can_shift) {
                stack.push_back(new StackElem<U>{input[pos].first, 
                    dfa->run_on(input[pos].first, stack.state()),
                    input[pos].second});
                ++pos;
                if (verbose)
                    cout << "Shift" << sr_str << endl;
            } else {
                if (verbose)
                    cout << "Parsing error. Aborting" << endl;
                return NULL;
            }
        }
    }
};


#endif 