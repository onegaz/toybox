// http://collaboration.cmc.ec.gc.ca/science/rpn/biblio/ddj/Website/articles/CUJ/2001/0101/austern/list1.htm
// https://stackoverflow.com/questions/2150192/how-to-avoid-code-duplication-implementing-const-and-non-const-iterators

#include <type_traits>
#include <iterator>
#include <iostream>

template <class T>
struct slist_node
{
    T val;
    slist_node* next;
    slist_node(const T& t, slist_node* p) : val(t), next(p)
    {
    }
};

template <class T, bool isconst = false>
struct slist_iterator
{
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = typename std::conditional<isconst, const T&, T&>::type;
    using pointer = typename std::conditional<isconst, const T*, T*>::type;
    using nodeptr =
        typename std::conditional<isconst, const slist_node<T>*, slist_node<T>*>::type;

    slist_iterator(nodeptr x = 0) : p(x)
    {
    }
    slist_iterator(const slist_iterator<T, false>& i) : p(i.p)
    {
    }
    reference operator*() const
    {
        return p->val;
    }
    pointer operator->() const
    {
        return &(p->val);
    }
    slist_iterator& operator++()
    {
        p = p->next;
        return *this;
    }
    slist_iterator operator++(int)
    {
        slist_iterator tmp(*this);
        ++*this;
        return tmp;
    }
    bool operator!=(const slist_iterator& rhs)
    {
        return p != rhs.p;
    }
    nodeptr p;
};

template <class T>
struct slist
{
    slist_node<T>* head;
    using iterator = slist_iterator<T, false>;
    using const_iterator = slist_iterator<T, true>;
    iterator begin()
    {
        return iterator{(slist_node<T>*) head};
    }
    iterator end()
    {
        return iterator(nullptr);
    }
    const_iterator cbegin()
    {
        return const_iterator(head);
    }
    const_iterator cend()
    {
        return const_iterator(nullptr);
    }
    void insert(const T& data)
    {
        auto node = new slist_node<T>(data, head);
        head = node;
    }
    //  ...
};

int main()
{
    slist<int> nlist;
    for (int i = 0; i < 4; i++)
        nlist.insert(i);
    std::cout << "access via iterator\n";
    for (slist_iterator<int, false> iter = nlist.begin(); iter != nlist.end(); iter++)
        std::cout << (*iter) << "\t";
    std::cout << std::endl;
    std::cout << "access via const_iterator\n";
    for (auto iter = nlist.cbegin(); iter != nlist.cend(); iter++)
        std::cout << (*iter) << "\t";
    std::cout << std::endl;
    return 0;
}
