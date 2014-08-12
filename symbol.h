#ifndef SYMBOL_H
#define SYMBOL_H
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <set>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <list>
#include <cstring>
using namespace std;

template <typename T>
bool contains(const set<T> &items, const T &item) {
    return (items.find(item) != items.end());
}

template <typename T>
bool contains(const set<T> &items, const set<T> &item_subset) {
    return (includes(items.begin(), items.end(), 
                     item_subset.begin(), item_subset.end()));
}

template <typename T>
void insert(set <T> &items, set <T> &new_items) {
    items.insert(new_items.begin(), new_items.end());
}


static int str_codes = 256;
struct Symbol {
    int s;
    string name;
    Symbol(): s(0) {}
    Symbol(char c): s(c), name(1,c) {}
    Symbol(int _s, string _n): s(_s), name(_n) {}
    Symbol(string _n): s(str_codes++), name(_n) {}
    string str() const { 
        return name;
    }
    bool is_eps() const { return s == 0; }
    bool operator==(const Symbol &sym) const { return s == sym.s; }
    bool operator!=(const Symbol &sym) const { return s != sym.s; }
    bool operator<(const Symbol &sym) const { return s < sym.s; }
};
typedef vector<Symbol> Symbols;

struct SymbolTable : public map<string,Symbol> {
    Symbol &get(const string &s) {
        if (find(s) == end()) 
            (*this)[s] = Symbol(s);
        return (*this)[s];
    }
};

static string to_string(const Symbol &s) {
    return s.str();
}

static string to_string(const char &c) {
    return c?string(1,c):"eps";
}

static string vec_str(const Symbols &vs) {
    string ret;
    for (const Symbol &s : vs)
        ret += s.str();
    return ret;
}
template <typename U>
string vec_str(const vector<pair<Symbol,U>> &vs) {
    string ret;
    for (auto &p : vs)
        ret += p.first.str() + " ";
    return ret;
}

static string set_str(const set<Symbol> &vs) {
    string ret;
    for (const Symbol &s : vs)
        ret += s.str();
    return ret;
}

static string set_str(const set<int> &vs) {
    string ret;
    for (const int i: vs)
        ret += to_string(i) + " ";
    return ret;
}

static string list_str(Symbols::iterator begin, Symbols::iterator end) {
    string ret;
    for (auto i = begin; i < end; ++i)
        ret += i->str();
    return ret;
}
template <typename U>
string list_str(typename vector<pair<Symbol,U>>::iterator begin,
                typename vector<pair<Symbol,U>>::iterator end) {
    string ret;
    for (auto i = begin; i < end; ++i)
        ret += i->first.str() + " ";
    return ret;
}

struct RChar {
    char c;
    RChar() {}
    RChar(char _c): c(_c) {}
    string str() const { return string(1,c); }
    bool operator<(const RChar &r) const { return c < r.c; }
    bool operator==(const RChar &r) const { return c == r.c; }
    bool is_eps() const { return c == 0; }
};

static string to_string(const RChar &r) { return r.str(); }

template <typename S>
string set_str(set<int> keys, vector<S> map) {
    string ret = "{\n";
    for (int i : keys)
        ret += "\t" + to_string(map[i]) + "\n";
    ret += "}";
    return ret;
}

static Symbols string_to_sym(string str) {
    Symbols ret;
    for (char c : str)
        ret.emplace_back(c);
    return ret;
}

struct Bitmap {
    uint32_t *data;
    int length;
    int words;
    Bitmap() {
        words = length = 0;
        data = NULL;
    }
    Bitmap(int length) {
        this->length = length;
        words = ((length-1)/32+1);
        data = new uint32_t [words];
        memset(data, 0, 4*words);
    }
    Bitmap(const set<int> &ints) {
        int max=0;
        for (int i : ints) 
            max = i>max?i:max;
        length = max;
        words = ((length-1)/32+1);
        data = new uint32_t [words];
        memset(data, 0, 4*words);
        for (int i : ints) 
            set(i);
    }
    ~Bitmap() { if (data) delete[] data; }
    Bitmap &operator=(const Bitmap &b) {
        length = b.length;
        words = b.words;
        data = new uint32_t [words];
        memcpy(data, b.data, 4*words);
        return *this;
    }
    string str() {
        string ret;
        for (int i = 0; i < length; ++i)
            ret += string(1,'0'+get(i));
        return ret;
    }
    int get(unsigned int index) {
        int word = index / 32;
        int offset = index % 32;
        return (data[word]>>offset)&1;
    }
    void set(unsigned int index) {
        int word = index / 32;
        int offset = index % 32;
        data[word] |= 1<<offset;
    }
    void invert() {
        for (int i = 0; i < words; ++i)
            data[i] ^= ~0;
    }
    bool operator==(const Bitmap &b) const {
        if (length != b.length)
            return false;
        for (int i = 0; i < words; ++i)
            if (data[i] != b.data[i])
                return false;
        return true;
    }
    bool operator<(const Bitmap &b) const {
        if (length < b.length)
            return true;
        for (int i = words - 1; i >= 0; --i)
            if (data[i] < b.data[i])
                return true;
        return false;
    }
    std::set<int> int_set() {
        std::set<int> ret;
        for (int i = 0; i < length; ++i)
            if (get(i)) ret.insert(i);
        return ret;
    }
};


template <typename S>
string set_str(Bitmap keys, vector<S> map) {
    return set_str(keys.int_set(), map);
}



#endif
