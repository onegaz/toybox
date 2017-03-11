#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <iostream> 
#include <sstream> 
#include <iomanip>
#include <algorithm>
#include <fstream>
#include<boost/tokenizer.hpp>
#include <fcntl.h>
#include <chrono>
#include <unordered_map>
// size_t nodecnt=0;
// size_t maxkids=0;
// size_t allkidscnt = 0;
// size_t compactedspace = 0;
std::unordered_map<size_t, size_t> kids_num_map; // key: number of kids, value: number of nodes of the same kids
template<int N, typename CharType>
struct TrieNode 
{
    typedef TrieNode<1, CharType> this_type;
    TrieNode(){   
        std::fill(std::begin(m_children), std::end(m_children), nullptr);
    }     

    ~TrieNode() {
        clear();
    }
    void clear() {
        std::for_each(m_children, end(), [](this_type* kid){delete_by_type(kid);});
        std::fill(m_children, end(), nullptr);
        m_kidnum = 0;
    }
    bool leaf() { return m_isleaf;}
    int size() { return (int)m_kidnum;}
    int capacity() { return (int)m_capacity;}
    
    void setkid(int pos, this_type* kid) {
        m_children[pos] = kid;
    }
    this_type* getkid(int pos) {
        return m_children[pos];
    }
    static const int MAXKIDS=100;
    static this_type* create(int childrencnt) {
        this_type* compactclone=nullptr;
        switch(childrencnt){
            case 0: compactclone=reinterpret_cast<this_type*>(new TrieNode<1, CharType>()); break; // at least allocate one kid
            case 1: compactclone=reinterpret_cast<this_type*>(new TrieNode<1, CharType>()); break;
            case 2: compactclone=reinterpret_cast<this_type*>(new TrieNode<2, CharType>()); break;
            case 3: compactclone=reinterpret_cast<this_type*>(new TrieNode<3, CharType>()); break;
            case 4: compactclone=reinterpret_cast<this_type*>(new TrieNode<4, CharType>()); break;
            case 5: compactclone=reinterpret_cast<this_type*>(new TrieNode<5, CharType>()); break;
            case 6: compactclone=reinterpret_cast<this_type*>(new TrieNode<6, CharType>()); break;
            case 7: compactclone=reinterpret_cast<this_type*>(new TrieNode<7, CharType>()); break;
            case 8: compactclone=reinterpret_cast<this_type*>(new TrieNode<8, CharType>()); break;
            default: compactclone=reinterpret_cast<this_type*>(new TrieNode<MAXKIDS, CharType>()); break;
        }
        return compactclone;
    }
    static void delete_by_type(this_type* obj){
        if (!obj)
            return;  
        obj->clear();          
        switch(obj->capacity()){
            case 0: ::operator delete (reinterpret_cast<TrieNode<1, CharType>*>(obj)); break;
            case 1: ::operator delete (reinterpret_cast<TrieNode<1, CharType>*>(obj)); break;
            case 2: ::operator delete (reinterpret_cast<TrieNode<2, CharType>*>(obj)); break;
            case 3: ::operator delete (reinterpret_cast<TrieNode<3, CharType>*>(obj)); break;
            case 4: ::operator delete (reinterpret_cast<TrieNode<4, CharType>*>(obj)); break;
            case 5: ::operator delete (reinterpret_cast<TrieNode<5, CharType>*>(obj)); break;
            case 6: ::operator delete (reinterpret_cast<TrieNode<6, CharType>*>(obj)); break;
            case 7: ::operator delete (reinterpret_cast<TrieNode<7, CharType>*>(obj)); break;
            case 8: ::operator delete (reinterpret_cast<TrieNode<8, CharType>*>(obj)); break;
            default: ::operator delete (reinterpret_cast<TrieNode<MAXKIDS, CharType>*>(obj)); break;
        }
    }
    static void operator delete(void* ptr, std::size_t sz) {
        if(ptr) {
            this_type* tmp = reinterpret_cast<this_type*>(ptr);
            tmp->clear();
            if(tmp->capacity()!=1)
                delete_by_type(tmp);
            else {
                ::operator delete(tmp);
            }
                
        }
    }
    static this_type* create() {
        return create(MAXKIDS);
    }    
    this_type* find(char c) { //  std::cout <<__func__<<"("<<c<<") " << to_string() << std::endl;
        auto node = std::find_if(m_children, end(), [c](this_type* kid) {return c==kid->m_data;} );
        if(node==end()) return nullptr;
        return *node;
    }
    this_type* insert(char c) { //std::cout <<__func__<<"("<<c<<") before " << to_string() << std::endl;
        this_type* kid = create();
        m_children[size()] = kid;
        kid->m_data = c;
        m_kidnum+=1; //std::cout <<__func__<<"("<<c<<") after " << to_string() << " kid " << kid->to_string() << std::endl;
        return kid;
    }
    std::string to_string() {
        std::stringstream ss;
        ss << "data="<<m_data<<",m_isleaf="<<std::boolalpha<<leaf()<<",m_kidnum="<<size()<<",capacity=";
        return ss.str();
    }
    this_type* clone_no_kid() {
        this_type* compactclone=create(m_kidnum);
        compactclone->m_data = m_data;
        compactclone->m_isleaf = m_isleaf;
        compactclone->m_kidnum = m_kidnum;
        kids_num_map[m_kidnum]++;
        //std::copy(m_children, m_children+size(), compactclone->m_children);
        return compactclone;
    }
    this_type** end() {
        return m_children+size();
    }
    this_type** begin() {
        return m_children;
    }
    CharType m_data=0;
    char m_isleaf=0; // m_isleaf is true if the node represents end of a word
    char m_kidnum=0;
    char m_capacity=N;  
private:
    this_type *m_children[N];
    //std::array<std::unique_ptr<this_type>,N> m_children;
};

typedef TrieNode<1, char> trienode; 
using compactnode=TrieNode<1, char>; 
// If not present, inserts key into trie   http://www.geeksforgeeks.org/trie-insert-and-search/
// If the key is prefix of trie node, just marks leaf node
void insert(trienode *root, const char *key)
{
//    std::cout << __func__ << " " << key << " to " << root->to_string() << std::endl;
    int length = strlen(key);
    for (int level = 0; level < length; level++)
    {
        char curchar = key[level]; //std::cout << "level " << level << " current node " << pCrawl->to_string() << std::endl;
        trienode *next = root->find(curchar); 
        if(next == nullptr) { //  std::cout << "not found " << curchar << std::endl;
            next = root->insert(curchar);
        }
        root=next;
    } 
    root->m_isleaf = 1;
}
 
// Returns true if key presents in trie, else false
bool search(trienode *root, const char *key)
{std::cout << __func__ << " " << key << std::endl;
    int length = strlen(key);
    for (int level = 0; level < length; level++)
    {
        trienode* next = root->find(key[level]);
        if(next==nullptr) {
            return false;
        }
        root = next;
    } 
    return (root != nullptr && root->m_isleaf);
}

compactnode* compact(trienode *root) {
    compactnode* compactroot = root->clone_no_kid();
    std::transform(root->begin(), root->end(), compactroot->begin(), compact);
    return compactroot;
}

template<typename F, class ...Args1>                                                                            
void checkperf(const std::string& prompt, F& func, Args1... args) {                                                    
    auto t1 = std::chrono::high_resolution_clock::now();  
    func(args...) ;                                                                                             
    auto t2 = std::chrono::high_resolution_clock::now();                                                        
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);                               
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;                                                  
    std::cout << prompt << " took " << fp_ms.count() << " ms, or " << int_ms.count() << " whole milliseconds\n";
} 

void processfile(const std::string& filepath) {
   std::fstream srcfile(filepath);
   trienode *rootf = trienode::create(100);
   std::unique_ptr<trienode, decltype(&trienode::delete_by_type) >rootf_owner(rootf, &trienode::delete_by_type);
   while(srcfile.good()) {
        std::string line;
       std::getline(srcfile, line);
       boost::tokenizer<> tok(line);
        for(boost::tokenizer<>::iterator beg=tok.begin(); beg!=tok.end();++beg){
            insert(rootf, beg->c_str());
        }
   }
   auto t1 = std::chrono::high_resolution_clock::now();
   trienode *crootf = compact(rootf);
//    std::unique_ptr<trienode, decltype(&trienode::delete_by_type) >crootf_owner(crootf, &trienode::delete_by_type);
       auto t2 = std::chrono::high_resolution_clock::now();                                                        
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);                               
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;                                                  
    std::cout << "compact trie took " << fp_ms.count() << " ms, or " << int_ms.count() << " whole milliseconds\n";
   const char* words[]={"United", "NotExistZ"};
   for(auto iter=std::begin(words); iter!= std::end(words); iter++){
       std::cout << *iter << " is " << (search(crootf, *iter)?"found":"not found") << std::endl;
   }
   delete crootf;
   std::cout << "after search nodecnt for file "<< filepath<< " " 
    << std::endl;
    size_t node_cnt_with_more_kids = 0;
    size_t avgkids = 3;
    double totalcnt = 0;
    for(auto& iter:kids_num_map) {
        std::cout << iter.second << " nodes have " << iter.first << " kids"<<std::endl;
        if(iter.first>avgkids)
            node_cnt_with_more_kids+= iter.second;
        totalcnt+=iter.second;
    }
    std::cout << node_cnt_with_more_kids << " nodes have more than " << avgkids << " kids, " << (node_cnt_with_more_kids*100)/totalcnt << "%" << std::endl;
}

int main(int argc, char* argv[])
{   // /home/onega/Downloads/54294-0.txt  https://www.gutenberg.org/
    std::cout << "sizeof 0 " << sizeof(TrieNode<0, char>) << std::endl;
    std::cout << "sizeof 1 " << sizeof(TrieNode<1, char>) << std::endl;
    std::cout << "sizeof 2 " << sizeof(TrieNode<2, char>) << std::endl;
    std::cout << "sizeof 26 " << sizeof(TrieNode<26, char>) << std::endl;
   if(argc>1) {
       checkperf("processfile", processfile, std::string(argv[1]));    
        return 0;
   }
    
    const char* keys[] = {"the", "there", "answer", "any", "by", "their"}; 
    trienode *root = trienode::create(127); // root node has more kids
    // int cnt = std::extent< decltype(keys)>::value;
    for(auto iter=std::begin(keys); iter!=std::end(keys); iter++)
        insert(root, *iter);
    trienode * orig = root;
    // root = compact(orig);
    const char* patterns[] = {"the", "these", "warning", "thaw", "their"}; 
    for(auto iter=std::begin(patterns); iter!=std::end(patterns); iter++)
        std::cout << *iter << " is " << (search(root, *iter)?"found":"not found") << std::endl;
    delete root;
    // delete orig;
    return 0;
}