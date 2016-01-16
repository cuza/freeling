//////////////////////////////////////////////////////////////////
//
//    STL-like n-ary tree template 
//
//    Copyright (C) 2006   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This program is free software; you can redistribute it 
//    and/or modify it under the terms of the GNU General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    General Public License for more details.
//
//    You should have received a copy of the GNU General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110-1301 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx Omega.S112 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#ifndef _TREE_TEMPLATE
#define _TREE_TEMPLATE

namespace freeling {

  // predeclarations
  template <class T> class tree;

  template <class T> class generic_iterator;
  template <class T> class generic_const_iterator;

  template <class T> class preorder_iterator;
  template <class T> class const_preorder_iterator;
  template <class T> class sibling_iterator;
  template <class T> class const_sibling_iterator;

  /// Generic iterator, to derive all the others
  template<class T, class N>
    class tree_iterator {
  protected:
    N *pnode;
  public: 
    tree_iterator();
    tree_iterator(tree<T> *);
    tree_iterator(const tree_iterator<T,N> &);
    ~tree_iterator();

    const tree<T>& operator*() const;
    const tree<T>* operator->() const;
    bool operator==(const tree_iterator<T,N> &) const;
    bool operator!=(const tree_iterator<T,N> &) const;
  };

  template<class T>
    class generic_iterator : public tree_iterator<T,tree<T> > {
    friend class generic_const_iterator<T>;
  public:
    generic_iterator();
    generic_iterator(tree<T> *);
    generic_iterator(const generic_iterator<T> &);
    tree<T>& operator*() const;
    tree<T>* operator->() const;
    ~generic_iterator();
  };

  template<class T>
    class generic_const_iterator : public tree_iterator<T,const tree<T> >  {
  public:
    generic_const_iterator();
    generic_const_iterator(const generic_iterator<T> &);
    generic_const_iterator(const generic_const_iterator<T> &);
    generic_const_iterator(tree<T> *);
    generic_const_iterator(const tree<T> *);
    ~generic_const_iterator();
  };


  /// sibling iterator: traverse all children of the same node

  template<class T>
    class sibling_iterator : public generic_iterator<T> {
  public:
    sibling_iterator();
    sibling_iterator(const sibling_iterator<T> &);
    sibling_iterator(tree<T> *);
    ~sibling_iterator();

    sibling_iterator& operator++();
    sibling_iterator& operator--();
    sibling_iterator operator++(int);
    sibling_iterator operator--(int);
  };

  template<class T>
    class const_sibling_iterator : public generic_const_iterator<T> {
  public:
    const_sibling_iterator();
    const_sibling_iterator(const const_sibling_iterator<T> &);
    const_sibling_iterator(const sibling_iterator<T> &);
    const_sibling_iterator(tree<T> *);
    ~const_sibling_iterator();

    const_sibling_iterator& operator++();
    const_sibling_iterator& operator--();
    const_sibling_iterator operator++(int);
    const_sibling_iterator operator--(int);
  };


  /// traverse the tree in preorder (parent first, then children)
  template<class T>
    class preorder_iterator : public generic_iterator<T> {
  public:
    preorder_iterator();
    preorder_iterator(const preorder_iterator<T> &);
    preorder_iterator(tree<T> *);
    preorder_iterator(const sibling_iterator<T> &);
    ~preorder_iterator();

    preorder_iterator& operator++();
    preorder_iterator& operator--();
    preorder_iterator operator++(int);
    preorder_iterator operator--(int);
  };

  template<class T>
    class const_preorder_iterator : public generic_const_iterator<T> {
  public:
    const_preorder_iterator();
    const_preorder_iterator(tree<T> *);
    const_preorder_iterator(const tree<T> *);
    const_preorder_iterator(const const_preorder_iterator<T> &);
    const_preorder_iterator(const preorder_iterator<T> &);
    const_preorder_iterator(const const_sibling_iterator<T> &);
    const_preorder_iterator(const sibling_iterator<T> &);
    ~const_preorder_iterator();
  
    const_preorder_iterator& operator++();
    const_preorder_iterator& operator--();
    const_preorder_iterator operator++(int);
    const_preorder_iterator operator--(int);
  };

  template <class T> 
    class tree { 
    friend class preorder_iterator<T>;
    friend class const_preorder_iterator<T>;
    friend class sibling_iterator<T>;
    friend class const_sibling_iterator<T>;

  private:
    bool isempty;
    tree *parent;        // parent node
    tree *first,*last;   // first/last child
    tree *prev,*next;    // prev/next sibling
    void clone(const tree<T>&);

  public:
    T info;
    typedef class generic_iterator<T> generic_iterator;
    typedef class generic_const_iterator<T> generic_const_iterator;
    typedef class preorder_iterator<T> preorder_iterator;
    typedef class const_preorder_iterator<T> const_preorder_iterator;
    typedef class sibling_iterator<T> sibling_iterator;
    typedef class const_sibling_iterator<T> const_sibling_iterator;
    typedef preorder_iterator iterator;
    typedef const_preorder_iterator const_iterator;

    tree();
    tree(const T&);
    tree(const tree<T>&);
    tree(const typename tree<T>::preorder_iterator&);
    ~tree();
    tree<T>& operator=(const tree<T>&);

    unsigned int num_children() const;
    sibling_iterator nth_child(unsigned int) const;
    iterator get_parent() const;
    tree<T> & nth_child_ref(unsigned int) const;
    T& get_info();
    void append_child(const tree<T> &);
    void hang_child(tree<T> &, bool=true);
    void clear();
    bool empty() const;

    sibling_iterator sibling_begin();
    const_sibling_iterator sibling_begin() const;
    sibling_iterator sibling_end();
    const_sibling_iterator sibling_end() const;
    sibling_iterator sibling_rbegin();
    const_sibling_iterator sibling_rbegin() const;
    sibling_iterator sibling_rend();
    const_sibling_iterator sibling_rend() const;

    preorder_iterator begin();
    const_preorder_iterator begin() const;
    preorder_iterator end();
    const_preorder_iterator end() const;
  };


  /// Method implementations for class tree

  /// constructor: empty tree doesn't exist, it's a one-node tree with no info. Be careful

  template <class T>
    tree<T>::tree() {
    isempty = true;
    parent=NULL;
    first=NULL; last=NULL;
    prev=NULL; next=NULL;
  }


  /// constructor: one-node tree

  template <class T>
    tree<T>::tree(const T &x) : info(x) {
    isempty = false;
    parent=NULL;
    first=NULL; last=NULL;
    prev=NULL; next=NULL;
  }

  /// constructor from an iterator

  template <class T>
    tree<T>::tree(const typename tree<T>::preorder_iterator &p) {
    clone(*p);
  }

  /// copy constructor

  template <class T>
    tree<T>::tree(const tree<T>& t) {
    clone(t);
  }


  /// assignment 

  template <class T>
    tree<T>& tree<T>::operator=(const tree<T>& t) {
    if (this != &t) {
      clear();
      clone(t);
    }
    return (*this);
  }

  ///destructor

  template <class T>
    tree<T>::~tree() {
    tree *p=this->first;
    while (p!=NULL) {
      tree *q=p->next;
      delete p;
      p=q;
    }
  }

  template <class T>
    void tree<T>::clear() {

    // delete children
    tree *p=this->first;
    while (p!=NULL) {
      tree *q=p->next;
      delete p;
      p=q;
    }

    // reset root node
    isempty = true;
    parent=NULL;
    first=NULL; last=NULL;
    prev=NULL; next=NULL;

  }

  /// number of children

  template <class T>
    unsigned int tree<T>::num_children() const {
    tree *s;
    unsigned int n=0;
    for (s=this->first; s!=NULL; s=s->next) n++;
    return n;
  }

  /// access parent

  template <class T>
    typename tree<T>::iterator tree<T>::get_parent() const { 
    iterator i = this->parent;    
    return i;
  }

  /// access nth child

  template <class T>
    typename tree<T>::sibling_iterator tree<T>::nth_child(unsigned int n) const { 
    sibling_iterator i = this->first;    
    while (n>0 && i!=NULL) {
      i = i->next;
      n--;
    }
    return i;
  }


  /// access nth child ref (useful for Java API)

  template <class T>
    tree<T> & tree<T>::nth_child_ref(unsigned int n) const { 
    sibling_iterator i = this->first;
    while (n>0) {
      i = i->next;
      n--;
    }
    return (*i);
  }

  /// access info (useful for Java API)

  template <class T>
    T& tree<T>::get_info() {
    return info;
  }


  /// detect empty tree

  template <class T>
    bool tree<T>::empty() const {
    return isempty;
  }

  /// begin/end sibling iterator

  template <class T>
    typename tree<T>::sibling_iterator tree<T>::sibling_begin() {
    return freeling::sibling_iterator<T>(this->first);
  }

  template <class T>
    typename tree<T>::const_sibling_iterator tree<T>::sibling_begin() const {
    return freeling::const_sibling_iterator<T>(this->first);
  }

  template <class T>
    typename tree<T>::sibling_iterator tree<T>::sibling_end() {
    return freeling::sibling_iterator<T>(NULL);
  }

  template <class T>
    typename tree<T>::const_sibling_iterator tree<T>::sibling_end() const {
    return freeling::const_sibling_iterator<T>(NULL);
  }


  /// begin/end preorder iterator

  template <class T>
    typename tree<T>::preorder_iterator tree<T>::begin() {
    if (isempty) return freeling::preorder_iterator<T>(NULL);
    else return freeling::preorder_iterator<T>(this);
  }

  template <class T>
    typename tree<T>::const_preorder_iterator tree<T>::begin() const {
    if (isempty) return freeling::const_preorder_iterator<T>((const tree<T>*)NULL);
    else return freeling::const_preorder_iterator<T>(this);
  }

  template <class T>
    typename tree<T>::preorder_iterator tree<T>::end() {
    return freeling::preorder_iterator<T>((tree<T>*)NULL);
  }

  template <class T>
    typename tree<T>::const_preorder_iterator tree<T>::end() const {
    return freeling::const_preorder_iterator<T>((const tree<T>*)NULL);
  }

  template <class T>
    typename tree<T>::sibling_iterator tree<T>::sibling_rbegin() {
    return freeling::sibling_iterator<T>(this->last);
  }

  template <class T>
    typename tree<T>::const_sibling_iterator tree<T>::sibling_rbegin() const {
    return freeling::const_sibling_iterator<T>(this->last);
  }

  template <class T>
    typename tree<T>::sibling_iterator tree<T>::sibling_rend() {
    return freeling::sibling_iterator<T>(NULL);
  }

  template <class T>
    typename tree<T>::const_sibling_iterator tree<T>::sibling_rend() const {
    return freeling::const_sibling_iterator<T>(NULL);
  }


  /// append child to a tree

  template <class T>
    void tree<T>::append_child(const tree<T>& child) {

    // make a copy
    tree<T> *x = new tree<T>;
    x->clone(child);

    x->next = NULL;  x->prev = NULL;
    x->parent = this;

    if (this->first != NULL) {  // there are already children, join them
      x->prev = this->last;
      this->last->next = x;
      this->last = x;
    }
    else {
      // no children, new is the only one
      this->first = x; this->last = x;
    }
  }

  /// hang a tree as last child of another (no copying!!)

  template <class T>
    void tree<T>::hang_child(tree<T>& child, bool last) {

    // remove child from its current location:
    // 1- remove it from siblings chain
    if (child.prev) child.prev->next=child.next;
    if (child.next) child.next->prev=child.prev;  
    // 2- adujst parent pointers if first or last child
    if (child.parent) {
      if (!child.prev) child.parent->first=child.next;
      if (!child.next) child.parent->last=child.prev;
    }

    // hang child on new location
    child.prev=NULL;
    child.next=NULL;
    child.parent = this;

    if (this->first == NULL) { 
      // there are no children, new is the only one
      this->first = &child;
      this->last = &child;
    }
    else {
      // already children, join them
      if (last) {
        // append new node as last child
        child.prev = this->last;
        this->last->next = &child;
        this->last = &child;
      }
      else {
        // append new node as first child
        child.next = this->first;
        this->first->prev = &child;
        this->first = &child;      
      }
    }
  }

  /// clone an entire tree

  template <class T>
    void tree<T>::clone(const tree<T>& t) {

    this->isempty = t.isempty;
    this->info = t.info;
    this->parent = NULL;
    this->first = NULL;
    this->last = NULL;
    this->prev=NULL;
    this->next=NULL;

    for (tree* p=t.first; p!=NULL; p=p->next) {

      tree<T>* c = new tree<T>;
      c->clone(*p);
      c->next = NULL;  
      c->prev = NULL;
      c->parent = this;

      if (this->first != NULL) {
        c->prev = this->last;
        this->last->next = c;
        this->last = c;
      }
      else {
        this->first = c; 
        this->last = c;
      }
    }
  }


  /////////////
  template <class T, class N>
    tree_iterator<T,N>::tree_iterator() {pnode = NULL;}

  template <class T, class N>
    tree_iterator<T,N>::tree_iterator(tree<T> *t) {pnode = t;}

  template <class T, class N>
    tree_iterator<T,N>::tree_iterator(const tree_iterator<T,N> &o) : pnode(o.pnode) {}

  template <class T, class N>
    tree_iterator<T,N>::~tree_iterator() {}

  template <class T, class N>
    const tree<T>& tree_iterator<T,N>::operator*() const {return (*pnode);}

  template <class T, class N>
    const tree<T>* tree_iterator<T,N>::operator->() const {return pnode;}

  template <class T, class N>
    bool tree_iterator<T,N>::operator==(const tree_iterator<T,N> &t) const {
    return (t.pnode==pnode); 
  }

  template <class T, class N>
    bool tree_iterator<T,N>::operator!=(const tree_iterator<T,N> &t) const {
    return (t.pnode!=pnode); 
  }

  /////////////////// Method implementations for class generic_iterator

  template <class T>
    generic_iterator<T>::generic_iterator() : tree_iterator<T,tree<T> >() {}

  template <class T>
    generic_iterator<T>::generic_iterator(const generic_iterator<T> & o) : tree_iterator<T,tree<T> >(o.pnode) {}

  template <class T>
    generic_iterator<T>::generic_iterator(tree<T> *t) : tree_iterator<T,tree<T> >() {this->pnode=t;}

  template <class T>
    generic_iterator<T>::~generic_iterator() {}

  template <class T>
    tree<T>& generic_iterator<T>::operator*() const {return (*this->pnode);}

  template <class T>
    tree<T>* generic_iterator<T>::operator->() const {return this->pnode;}


  /////////////////// Method implementations for class generic_const_iterator

  template <class T>
    generic_const_iterator<T>::generic_const_iterator() : tree_iterator<T,const tree<T> >() {}

  template <class T>
    generic_const_iterator<T>::generic_const_iterator(const generic_iterator<T> & o) : tree_iterator<T,const tree<T> >(o.pnode) {}

  template <class T>
    generic_const_iterator<T>::generic_const_iterator(const generic_const_iterator<T> & o) : tree_iterator<T,const tree<T> >() {this->pnode=o.pnode;}

  template <class T>
    generic_const_iterator<T>::generic_const_iterator(tree<T> *t) : tree_iterator<T,const tree<T> >(t) {}

  template <class T>
    generic_const_iterator<T>::generic_const_iterator(const tree<T> *t) : tree_iterator<T,const tree<T> >() {this->pnode=t;}

  template <class T>
    generic_const_iterator<T>::~generic_const_iterator() {}





  /////////////////// Method implementations for class preorder_iterator

  template <class T>
    preorder_iterator<T>::preorder_iterator() : generic_iterator<T>() {}

  template <class T>
    preorder_iterator<T>::preorder_iterator(const preorder_iterator<T> & o) : generic_iterator<T>(o) {}

  template <class T>
    preorder_iterator<T>::preorder_iterator(const sibling_iterator<T> & o) : generic_iterator<T>(o) {}

  template <class T>
    preorder_iterator<T>::preorder_iterator(tree<T> *t) : generic_iterator<T>(t) {}

  template <class T>
    preorder_iterator<T>::~preorder_iterator() {}



  template <class T>
    preorder_iterator<T>& preorder_iterator<T>::operator++() {
    if (this->pnode->first != NULL) 
      this->pnode=this->pnode->first;
    else {
      while (this->pnode!=NULL && this->pnode->next==NULL) 
        this->pnode=this->pnode->parent;
      if (this->pnode!=NULL) this->pnode=this->pnode->next;
    }
    return *this;
  }

  template <class T>
    preorder_iterator<T>& preorder_iterator<T>::operator--() {
    if (this->pnode->prev!=NULL) {
      this->pnode=this->pnode->prev;
      while (this->pnode->last != NULL)
        this->pnode=this->pnode->last;
    }
    else
      this->pnode = this->pnode->parent;

    return *this;
  }

  template <class T>
    preorder_iterator<T> preorder_iterator<T>::operator++(int) {
    preorder_iterator b=(*this);
    ++(*this);
    return b;
  }

  template <class T>
    preorder_iterator<T> preorder_iterator<T>::operator--(int) {
    preorder_iterator b=(*this);
    --(*this);
    return b;
  }


  /////////////////// Method implementations for class const_preorder_iterator

  template <class T>
    const_preorder_iterator<T>::const_preorder_iterator() : generic_const_iterator<T>() {}

  template <class T>
    const_preorder_iterator<T>::const_preorder_iterator(const preorder_iterator<T> & o) : generic_const_iterator<T>(o) {}

  template <class T>
    const_preorder_iterator<T>::const_preorder_iterator(const sibling_iterator<T> & o) : generic_const_iterator<T>(o) {}

  template <class T>
    const_preorder_iterator<T>::const_preorder_iterator(const const_preorder_iterator<T> & o) : generic_const_iterator<T>(o) {}

  template <class T>
    const_preorder_iterator<T>::const_preorder_iterator(const const_sibling_iterator<T> & o) : generic_const_iterator<T>(o) {}

  template <class T>
    const_preorder_iterator<T>::~const_preorder_iterator() {}

  template <class T>
    const_preorder_iterator<T>::const_preorder_iterator(tree<T> *t) : generic_const_iterator<T>(t) {}

  template <class T>
    const_preorder_iterator<T>::const_preorder_iterator(const tree<T> *t) : generic_const_iterator<T>(t) {}

  template <class T>
    const_preorder_iterator<T>& const_preorder_iterator<T>::operator++() {
    if (this->pnode->first != NULL) 
      this->pnode=this->pnode->first;
    else {
      while (this->pnode!=NULL && this->pnode->next==NULL) 
        this->pnode=this->pnode->parent;
      if (this->pnode!=NULL) this->pnode=this->pnode->next;
    }
    return *this;
  }

  template <class T>
    const_preorder_iterator<T>& const_preorder_iterator<T>::operator--() {
    if (this->pnode->prev!=NULL) {
      this->pnode=this->pnode->prev;
      while (this->pnode->last != NULL)
        this->pnode=this->pnode->last;
    }
    else
      this->pnode = this->pnode->parent;

    return *this;
  }

  template <class T>
    const_preorder_iterator<T> const_preorder_iterator<T>::operator++(int) {
    const_preorder_iterator b=(*this);
    ++(*this);
    return b;
  }

  template <class T>
    const_preorder_iterator<T> const_preorder_iterator<T>::operator--(int) {
    const_preorder_iterator b=(*this);
    --(*this);
    return b;
  }


  /*
    template <class T>
    const_preorder_iterator<T>& const_preorder_iterator<T>::operator+=(unsigned int n) {
    for (; n>0; n--) ++(*this);
    return *this;
    }

    template <class T>
    const_preorder_iterator<T>& const_preorder_iterator<T>::operator-=(unsigned int n) {
    for (; n>0; n--) --(*this);
    return *this;
    }
  */

  /// Method implementations for class sibling_iterator

  template <class T>
    sibling_iterator<T>::sibling_iterator() : generic_iterator<T>() {}

  template <class T>
    sibling_iterator<T>::sibling_iterator(const sibling_iterator<T> & o) : generic_iterator<T>(o) {}

  template <class T>
    sibling_iterator<T>::sibling_iterator(tree<T> *t) : generic_iterator<T>(t) {}

  template <class T>
    sibling_iterator<T>::~sibling_iterator() {}

  template <class T>
    sibling_iterator<T>& sibling_iterator<T>::operator++() {
    this->pnode = this->pnode->next; 
    return *this;
  }

  template <class T>
    sibling_iterator<T>& sibling_iterator<T>::operator--() {
    this->pnode = this->pnode->prev; 
    return *this;
  }


  template <class T>
    sibling_iterator<T> sibling_iterator<T>::operator++(int) {
    sibling_iterator b=(*this);
    ++(*this);
    return b;
  }

  template <class T>
    sibling_iterator<T> sibling_iterator<T>::operator--(int) {
    sibling_iterator b=(*this);
    --(*this);
    return b;
  }

  /*template <class T>
    sibling_iterator<T>& sibling_iterator<T>::operator+=(unsigned int n) {
    for (; n>0; n--) ++(*this);
    return *this;
    }

    template <class T>
    sibling_iterator<T>& sibling_iterator<T>::operator-=(unsigned int n) {
    for (; n>0; n--) --(*this);
    return *this;
    }
  */

  /// Method implementations for class const_sibling_iterator

  template <class T>
    const_sibling_iterator<T>::const_sibling_iterator() : generic_const_iterator<T>() {}

  template <class T>
    const_sibling_iterator<T>::const_sibling_iterator(const sibling_iterator<T> & o) : generic_const_iterator<T>(o) {}

  template <class T>
    const_sibling_iterator<T>::const_sibling_iterator(const const_sibling_iterator<T> & o) : generic_const_iterator<T>(o) {}

  template <class T>
    const_sibling_iterator<T>::const_sibling_iterator(tree<T> *t) : generic_const_iterator<T>(t) {}

  template <class T>
    const_sibling_iterator<T>::~const_sibling_iterator() {}


  template <class T>
    const_sibling_iterator<T>& const_sibling_iterator<T>::operator++() {
    this->pnode = this->pnode->next; 
    return *this;
  }

  template <class T>
    const_sibling_iterator<T>& const_sibling_iterator<T>::operator--() {
    this->pnode = this->pnode->prev; 
    return *this;
  }


  template <class T>
    const_sibling_iterator<T> const_sibling_iterator<T>::operator++(int) {
    const_sibling_iterator b=(*this);
    ++(*this);
    return b;
  }

  template <class T>
    const_sibling_iterator<T> const_sibling_iterator<T>::operator--(int) {
    const_sibling_iterator b=(*this);
    --(*this);
    return b;
  }


} // namespace

#endif

