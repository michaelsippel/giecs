#ifndef _list_h_
#define _list_h_
/*
     Copyright 2012-2014 Infinitycoding all rights reserved
     This file is part of the mercury c-library.

     The mercury c-library is free software: you can redistribute it and/or modify
     it under the terms of the GNU Lesser General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     any later version.

     The mercury c-library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public License
     along with the mercury c-library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>

template <typename T>
class ListNode;

template <typename T>
class List;


template <typename T>
class ListIterator
{
    public:
        ListIterator(List<T> *L);

        void pushFront(T element);
        void pushBack(T element);

        void insertAfter(T element);
        void insertBefore(T element);

        T popBack();
        T popFront();

        T remove();
        void destroy();
        T getCurrent();
        bool isEmpty();
        T getAndNext();

        ListIterator<T> *next();
        ListIterator<T> *previous();
        ListIterator<T> *setLast();
        ListIterator<T> *setFirst();

        bool isLast();
        void set(ListNode<T> *n);

        ListNode<T> *currentNode;
        List<T> *Instance;
};


template <typename T>
class List
{
        friend class ListIterator<T>;

    public:
        List();
        List(List<T> *l);
        List(List<T> *l, void(*copyTree)(T destElement, T srcElement));
        ~List();

        void pushFront(T element);
        void pushBack(T element);
        T popBack();
        T popFront();

        bool isEmpty();
        ListNode<T> *getNode(T element);

        size_t size();
        int numOfElements();

        bool remove(T element);
        T remove(ListNode<T> *n);
        void destroy(ListNode<T> *n);

        bool alreadyAdded(T element);

    private:
        bool lock;
        void (*structCleaner)(T element);

        ListNode<T> *dummy;
};

template <typename T>
class ListNode
{
        friend class List<T>;
        friend class ListIterator<T>;

    public:
        ListNode();
        ListNode(T element_);
        ~ListNode();

        T element;

    private:
        ListNode *next;
        ListNode *prev;
};


#define foreach(LIST,NAME,TYPE) \
	ListIterator<TYPE> Iterator = ListIterator<TYPE>(LIST); \
	for(TYPE NAME = Iterator.getCurrent(); !Iterator.isLast(); NAME = Iterator.getAndNext())

#include "list_impl.h"

#endif

