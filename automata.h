#ifndef AUTOMATA_H
#define AUTOMATA_H
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <iostream>
#include "symbol.h"

using namespace std;


template <typename S, typename T>
struct NFA {
    vector<S> states;
    map<S, int> state_map;
    vector<map<int, T>> edges;
    NFA() {}
    void add_edge(S from, S to, T on) {
        if (state_map.find(from) == state_map.end()) {
            state_map[from] = states.size();
            states.push_back(from);
            edges.push_back(map<int, T>());
        }
        if (state_map.find(to) == state_map.end()) {
            state_map[to] = states.size();
            states.push_back(to);
            edges.push_back(map<int, T>());
        }

        edges[state_map[from]][state_map[to]] = on;
    }
    map<T, set<int>> transitions(set<int> state_set) {
        map<T, set<int>> ret;
        for (int state : state_set)
            for (auto &p : edges[state])
                ret[p.second].insert(p.first);
        return ret;
    }
    void dump_eps(int state, set<int> &state_set, T eps, Bitmap &bm) {
        bm.set(state);
        state_set.insert(state);
        for (auto &p : edges[state]) {
            int child = p.first;
            if (p.second == eps && !bm.get(child)) 
                dump_eps(child, state_set, eps, bm);
        }
    }
    set<int> eps_set_fast(set<int> state_set, T eps) {
        Bitmap bm(states.size());
        for (int state : state_set)
            dump_eps(state, state_set, eps, bm);
        return state_set;
    }
    string str() {
        string str;
        for (int p = 0; p < states.size(); ++p) {
            str += to_string(p) + ".\t[ " + to_string(states[p]) + " ] :\n";
            for (auto &keyval : edges[p]) 
                str += "\t\t" + keyval.second.str() + 
                    " -> [ " + to_string(states[keyval.first]) + " ]\n";
        }
        return str;
    }
};

template <typename S, typename T>
struct DFA {
    vector<S> nfa_states;
    vector<set<int>> states;
    map<set<int>, int> state_map;
    vector<map<T, int>> edges;
    
    DFA(NFA<S, T> nfa): nfa_states(nfa.states) {
        set<int> start_set = nfa.eps_set_fast({0}, T(0));

        state_map[start_set] = states.size();
        states.push_back(start_set);
        edges.push_back(map<T, int>());
        queue<set<int>> new_states;
        new_states.push(start_set);

        while (!new_states.empty()) {
            set<int> state_set = new_states.front();
            new_states.pop();
            map<T, set<int>> trans = nfa.transitions(state_set);
            for (auto &p : trans) {
                if (p.first == T(0)) continue;
                set<int> new_state = nfa.eps_set_fast(p.second, 0);
                if (state_map.find(new_state) == state_map.end()) {
                    state_map[new_state] = states.size();
                    states.push_back(new_state);
                    edges.push_back(map<T, int>());
                    new_states.push(new_state);
                }
                edges[state_map[state_set]][p.first] =
                    state_map[new_state];
            }
        }
    }
    int run_on(const T &x, int state) {
        bool reject = (edges[state].find(x) == edges[state].end());
        return reject ? -1 : edges[state][x];
    }
    int run_on(const list<T> &xv, int state = 0) {
        for (const T &x : xv) {
            state = run_on(x, state);
            if (state == -1) return -1;
        }
        return state;
    }
    bool accepting(int state) {
        return contains<int>(states[state], nfa_states.size()-1);
    }
    int munch(const list<T> &xv, int state = 0) {
        int m = 0;
        for (const T &x : xv) {
            int new_state = run_on(x, state);
            if (new_state == -1) break;
            state = new_state;
            ++m;
        }
        return accepting(state) ? m : -1;
    }
    string str() {
        string str = "--------------------------------\n";
        for (int p = 0; p < states.size(); ++p) {
            str += to_string(p) + ".\n" + set_str(states[p], nfa_states) +
            "\n";
            for (auto &keyval : edges[p]) 
                str += "\t" + keyval.first.str() + 
                    " -> #" + to_string(keyval.second) + "\n";
            str += "--------------------------------\n";
        }
        return str;
    }
};
/*
template <typename S, typename T>
struct NFA {
    vector<S> states;
    map<S, int> state_map;
    vector<map<int, T>> edges;
    NFA() {}
    void add_edge(S from, S to, T on) {
        if (state_map.find(from) == state_map.end()) {
            state_map[from] = states.size();
            states.push_back(from);
            edges.push_back(map<int, T>());
        }
        if (state_map.find(to) == state_map.end()) {
            state_map[to] = states.size();
            states.push_back(to);
            edges.push_back(map<int, T>());
        }

        edges[state_map[from]][state_map[to]] = on;
    }
    map<T, set<int>> transitions(set<int> state_set) {
        map<T, set<int>> ret;
        for (int state : state_set)
            for (auto &p : edges[state])
                ret[p.second].insert(p.first);
        return ret;
    }
    void dump_eps(int state, Bitmap &bm, T eps) {
        bm.set(state);
        for (auto &p : edges[state]) {
            int child = p.first;
            if (p.second == eps && !bm.get(child)) 
                dump_eps(child, bm, eps);
        }
    }
    Bitmap eps_set_fast(Bitmap bm, T eps) {
        for (int i = 0; i < states.size(); ++i)
            if (bm.get(i))
                dump_eps(i, bm, eps);
        return bm;
    }
    string str() {
        string str;
        for (int p = 0; p < states.size(); ++p) {
            str += to_string(p) + ".\t[ " + to_string(states[p]) + " ] :\n";
            for (auto &keyval : edges[p]) 
                str += "\t\t" + keyval.second.str() + 
                    " -> [ " + to_string(states[keyval.first]) + " ]\n";
        }
        return str;
    }
};

template <typename S, typename T>
struct DFA {
    vector<S> nfa_states;
    vector<Bitmap> states;
    map<Bitmap, int> state_map;
    vector<map<T, int>> edges;
    
    DFA(NFA<S, T> nfa): nfa_states(nfa.states) {
        Bitmap start_set = nfa.eps_set_fast({0}, T(0));

        state_map[start_set] = states.size();
        states.push_back(start_set);
        edges.push_back(map<T, int>());
        queue<Bitmap> new_states;
        new_states.push(start_set);

        while (!new_states.empty()) {
            Bitmap state_set = new_states.front();
            new_states.pop();
            map<T, set<int>> trans = nfa.transitions(state_set);
            for (auto &p : trans) {
                if (p.first == T(0)) continue;
                Bitmap new_state = nfa.eps_set_fast(Bitmap(p.second), 0);
                if (state_map.find(new_state) == state_map.end()) {
                    state_map[new_state] = states.size();
                    states.push_back(new_state);
                    edges.push_back(map<T, int>());
                    new_states.push(new_state);
                }
                edges[state_map[state_set]][p.first] =
                    state_map[new_state];
            }
        }
    }
    int run_on(const T &x, int state) {
        bool reject = (edges[state].find(x) == edges[state].end());
        return reject ? -1 : edges[state][x];
    }
    int run_on(const list<T> &xv, int state = 0) {
        for (const T &x : xv) {
            state = run_on(x, state);
            if (state == -1) return -1;
        }
        return state;
    }
    bool accepting(int state) {
        return contains<int>(states[state], nfa_states.size()-1);
    }
    int munch(const list<T> &xv, int state = 0) {
        int m = 0;
        for (const T &x : xv) {
            int new_state = run_on(x, state);
            if (new_state == -1) break;
            state = new_state;
            ++m;
        }
        return accepting(state) ? m : -1;
    }
    string str() {
        string str = "--------------------------------\n";
        for (int p = 0; p < states.size(); ++p) {
            str += to_string(p) + ".\n" + set_str(states[p], nfa_states) +
            "\n";
            for (auto &keyval : edges[p]) 
                str += "\t" + keyval.first.str() + 
                    " -> #" + to_string(keyval.second) + "\n";
            str += "--------------------------------\n";
        }
        return str;
    }

};
*/
#endif
