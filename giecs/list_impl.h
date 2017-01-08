#ifndef _list_impl_h_
#define _list_impl_h_
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
ListNode<T>::ListNode()
{
}

template <typename T>
ListNode<T>::ListNode(T element_)
    : element(element_)
{
}

template <typename T>
ListNode<T>::~ListNode()
{
}

template <typename T>
List<T>::List()
{
    lock = true;

    dummy = new ListNode<T>();
    dummy->prev = dummy;
    dummy->next = dummy;
    structCleaner = (void (*)(T))NULL;

    lock = false;
}

template <typename T>
List<T>::List(List<T> *l)
{
    lock = true;

    structCleaner = l->structCleaner;
    ListIterator<T> *lit = new ListIterator<T>(l);

    dummy = new ListNode<T>();
    dummy->element = NULL;
    dummy->next = dummy;
    dummy->prev = dummy;
    while(!lit->IsLast())
    {
        T *currentEntry = new T;
        *currentEntry = *lit->GetCurrent();
        PushFront(currentEntry);
        lit->Next();
    }

    lock = false;
}

template <typename T>
List<T>::List(List<T> *l, void(*copyTree)(T destElement, T srcElement))
{
    lock = true;

    structCleaner = l->structCleaner;
    ListIterator<T> *srcIt = ListIterator<T>(l);

    dummy = new ListNode<T>();
    dummy->element = NULL;
    dummy->next = dummy;
    dummy->prev = dummy;

    while(!srcIt->isLast())
    {
        T currentEntry;
        currentEntry = srcIt->getCurrent();
        copyTree(currentEntry, srcIt->getCurrent());
        pushFront(currentEntry);
        srcIt->nxt();
    }

    lock = false;
}

template <typename T>
List<T>::~List<T>()
{
    lock = true;

    while(!isEmpty())
    {
        T element = popFront();
        if(structCleaner)
            structCleaner(element);
    }

    lock = false;
}

template <typename T>
void List<T>::pushFront(T element)
{
    ListNode<T> *newNode = new ListNode<T>();
    newNode->element = element;

    while(lock) {}
    lock = true;

    newNode->next = dummy->next;
    newNode->prev = dummy;
    dummy->next->prev = newNode;
    dummy->next = newNode;

    lock = false;
}

template <typename T>
void List<T>::pushBack(T element)
{
    ListNode<T> *newNode = new ListNode<T>();
    newNode->element = element;

    while(lock) {}
    lock = true;

    newNode->next = dummy;
    newNode->prev = dummy->prev;
    dummy->prev->next = newNode;
    dummy->prev = newNode;

    lock = false;
}

template <typename T>
T List<T>::popBack()
{
    T element = dummy->prev->element;
    remove(dummy->prev);

    return element;
}

template <typename T>
T List<T>::popFront()
{
    T element = dummy->next->element;
    remove(dummy->next);

    return element;
}

template <typename T>
bool List<T>::isEmpty()
{
    if(dummy->next == dummy && dummy->prev == dummy)
        return true;

    return false;
}

template <typename T>
ListNode<T> *List<T>::getNode(T element)
{
    lock = true;

    ListNode<T> *n = dummy->next;
    if(n->element == element)
    {
        lock = false;
        return n;
    }
    while(n != dummy)
    {
        if(n->element == element)
        {
            lock = false;
            return n;
        }

        n = n->next;
    }

    lock = false;
    return NULL;
}

template <typename T>
size_t List<T>::size()
{
    size_t s = 0;
    lock = true;

    ListIterator<T> i = ListIterator<T>(this);
    while(!i.isLast())
    {

        s++;
        i.next();
    }

    lock = false;
    return s * sizeof(T);
}

template <typename T>
int List<T>::numOfElements()
{
    int s = 0;
    lock = true;

    ListIterator<T> i = ListIterator<T>(this);
    while(!i.isLast())
    {
        s++;
        i.next();
    }

    lock = false;
    return s;
}

template <typename T>
T List<T>::remove(ListNode<T> *n)
{
    lock = true;

    if(n == dummy)
    {
        lock = false;
        return T();
    }

    n->prev->next = n->next;
    n->next->prev = n->prev;
    T element = n->element;
    delete n;

    lock = false;
    return element;
}

template <typename T>
bool List<T>::remove(T element)
{
    ListIterator<T> i = *ListIterator<T>(this).setFirst();
    while(!i.isLast())
    {
        if(i.getCurrent() == element)
        {
            i.remove();
            return true;
        }
        i.next();
    }

    return false;
}

template <typename T>
void List<T>::destroy(ListNode<T> *n)
{
    lock = true;

    if(n == dummy)
    {
        lock = false;
        return;
    }

    if(structCleaner)
        structCleaner(n->element);

    n->prev->next = n->next;
    n->next->prev = n->prev;

    delete n->element;
    delete n;

    lock = false;
}

template <typename T>
bool List<T>::alreadyAdded(T element)
{
    ListIterator<T> i = *ListIterator<T>(this).setFirst();
    while(!i.isLast())
    {
        if(i.getCurrent() == element)
            return true;

        i.next();
    }
    return false;
}


template <typename T>
ListIterator<T>::ListIterator(List<T> *L)
{
    Instance = L;
    currentNode = L->dummy->next;
}

template <typename T>
void ListIterator<T>::pushFront(T element)
{
    ListNode<T> *newNode = new ListNode<T>();
    newNode->element = element;

    while(Instance->lock) {}
    Instance->lock = true;

    newNode->next = Instance->dummy->next;
    newNode->prev = Instance->dummy;
    Instance->dummy->next->prev = newNode;
    Instance->dummy->next = newNode;

    Instance->lock = false;
}

template <typename T>
void ListIterator<T>::pushBack(T element)
{
    ListNode<T> *newNode = new ListNode<T>();
    newNode->element = element;

    while(Instance->lock) {}
    Instance->lock = true;

    newNode->prev = Instance->dummy;
    newNode->next = Instance->dummy->next;
    Instance->dummy->next->prev = newNode;
    Instance->dummy->next = newNode;

    Instance->lock = false;
}

template <typename T>
void ListIterator<T>::insertAfter(T element)
{
    ListNode<T> *newNode = new ListNode<T>();
    newNode->element = element;

    while(Instance->lock) {}
    Instance->lock = true;

    newNode->next = currentNode->next;
    newNode->prev = currentNode;
    currentNode->next->prev = newNode;
    currentNode->next = newNode;

    Instance->lock = false;
}

template <typename T>
void ListIterator<T>::insertBefore(T element)
{
    ListNode<T> *newNode = new ListNode<T>();
    newNode->element = element;

    while(Instance->lock) {}
    Instance->lock = true;

    newNode->prev = currentNode;
    newNode->next = currentNode->next;
    currentNode->next->prev = newNode;
    currentNode->next = newNode;

    Instance->lock = false;
}

template <typename T>
T ListIterator<T>::popBack()
{
    T element = Instance->dummy->prev->element;
    Instance->remove(Instance->dummy->prev);

    return element;
}

template <typename T>
T ListIterator<T>::popFront()
{
    T element = Instance->dummy->next->element;
    Instance->remove(Instance->dummy->next);

    return element;
}


template <typename T>
T ListIterator<T>::remove()
{
    T element = currentNode->element;
    currentNode = currentNode->next;
    Instance->remove(currentNode->prev);

    return element;
}

template <typename T>
void ListIterator<T>::destroy()
{
    currentNode = currentNode->next;
    Instance->destroy(currentNode->prev);
}

template <typename T>
T ListIterator<T>::getCurrent()
{
    return currentNode->element;
}

template <typename T>
T ListIterator<T>::getAndNext()
{
    T element = currentNode->element;
    next();

    return element;
}


template <typename T>
ListIterator<T> *ListIterator<T>::next()
{
    currentNode = currentNode->next;
    return this;
}

template <typename T>
ListIterator<T> *ListIterator<T>::previous()
{
    currentNode = currentNode->prev;
    return this;
}

template <typename T>
ListIterator<T> *ListIterator<T>::setLast()
{
    currentNode = Instance->dummy->prev;
    return this;
}

template <typename T>
ListIterator<T> *ListIterator<T>::setFirst()
{
    currentNode = Instance->dummy->next;
    return this;
}


template <typename T>
bool ListIterator<T>::isLast()
{
    if(currentNode == Instance->dummy)
        return true;

    return false;
}

template <typename T>
void ListIterator<T>::set(ListNode<T> *n)
{
    currentNode = n;
    return this;
}

template <typename T>
bool ListIterator<T>::isEmpty()
{
    return Instance->isEmpty();
}

#endif

