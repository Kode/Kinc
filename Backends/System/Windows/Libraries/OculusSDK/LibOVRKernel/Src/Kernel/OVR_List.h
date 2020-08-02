/************************************************************************************

PublicHeader:   OVR
Filename    :   OVR_List.h
Content     :   Template implementation for doubly-connected linked List
Created     :   September 19, 2012
Notes       :

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

Licensed under the Oculus VR Rift SDK License Version 3.3 (the "License");
you may not use the Oculus VR Rift SDK except in compliance with the License,
which is provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-3.3

Unless required by applicable law or agreed to in writing, the Oculus VR SDK
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#ifndef OVR_List_h
#define OVR_List_h

#include "OVR_Types.h"

namespace OVR {

//-----------------------------------------------------------------------------------
// ***** ListNode
//
// Base class for the elements of the intrusive linked list.
// To store elements in the List do:
//
// class MyData : ListNode<MyData>
// {
//     . . .
// };

template <class T>
class ListNode {
 private:
  ListNode<T>* pPrev;
  ListNode<T>* pNext;

  template <class X, class B>
  friend class List;

#ifdef OVR_BUILD_DEBUG
  bool marker; // Is this a marker node (rather than an actual data node)?
#endif

 public:
#ifdef OVR_BUILD_DEBUG
  T* GetPrev() const {
    if (pPrev && !pPrev->marker) {
      return (T*)(pPrev);
    } else {
      OVR_FAIL_M("Unstable call to ListNode<>::GetPrev() without first checking List<>IsFirst()");
      return nullptr;
    }
  }
  T* GetNext() const {
    if (pNext && !pNext->marker) {
      return (T*)(pNext);
    } else {
      OVR_FAIL_M("Unstable call to ListNode<>::GetNext() without first checking List<>IsLast()");
      return nullptr;
    }
  }
#else
  T* GetPrev() const {
    return (T*)(pPrev);
  }
  T* GetNext() const {
    return (T*)(pNext);
  }
#endif

  ListNode() {
#ifdef OVR_BUILD_DEBUG
    marker = false; // Most nodes are data nodes, so that is the default.
#endif
    pPrev = nullptr;
    pNext = nullptr;
  }

  bool IsInList() {
    return (pNext != nullptr);
  }

  void RemoveNode() {
    pPrev->pNext = pNext;
    pNext->pPrev = pPrev;
    pPrev = nullptr;
    pNext = nullptr;
  }

  // Removes us from the list and inserts pnew there instead.
  void ReplaceNodeWith(T* pnew) {
    pPrev->pNext = pnew;
    pNext->pPrev = pnew;
    pnew->pPrev = pPrev;
    pnew->pNext = pNext;
    pPrev = nullptr;
    pNext = nullptr;
  }

  // Inserts the argument linked list node after us in the list.
  void InsertNodeAfter(T* p) {
    p->pPrev = pNext->pPrev; // this
    p->pNext = pNext;
    pNext->pPrev = p;
    pNext = p;
  }
  // Inserts the argument linked list node before us in the list.
  void InsertNodeBefore(T* p) {
    p->pNext = pNext->pPrev; // this
    p->pPrev = pPrev;
    pPrev->pNext = p;
    pPrev = p;
  }

  void Alloc_MoveTo(ListNode<T>* pdest) {
    pdest->pNext = pNext;
    pdest->pPrev = pPrev;
    pPrev->pNext = pdest;
    pNext->pPrev = pdest;
    pPrev = nullptr;
    pNext = nullptr;
  }
};

//------------------------------------------------------------------------
// ***** List
//
// Doubly linked intrusive list.
// The data type must be derived from ListNode.
//
// Adding:   PushFront(), PushBack().
// Removing: Remove() - the element must be in the list!
// Moving:   BringToFront(), SendToBack() - the element must be in the list!
//
// Iterating:
//    MyData* data = MyList.GetFirst();
//    while (!MyList.IsNull(data))
//    {
//        . . .
//        data = MyList.GetNext(data);
//    }
//
// Removing:
//    MyData* data = MyList.GetFirst();
//    while (!MyList.IsNull(data))
//    {
//        MyData* next = MyList.GetNext(data);
//        if (ToBeRemoved(data))
//             MyList.Remove(data);
//        data = next;
//    }
//

// List<> represents a doubly-linked list of T, where each T must derive
// from ListNode<B>. B specifies the base class that was directly
// derived from ListNode, and is only necessary if there is an intermediate
// inheritance chain.

template <class T, class B = T>
class List {
 public:
  typedef T ValueType;

  List() {
    Root.pNext = Root.pPrev = &Root;
#ifdef OVR_BUILD_DEBUG
    Root.marker = true; // This is a marker node.
#endif
  }

  void Clear() {
    Root.pNext = Root.pPrev = &Root;
  }

  size_t GetSize() const {
    size_t n = 0;

    for (const ListNode<B>* pNode = Root.pNext; pNode != &Root; pNode = pNode->pNext)
      ++n;

    return n;
  }

  const ValueType* GetFirst() const {
    return IsEmpty() ? nullptr : (const ValueType*)Root.pNext;
  }
  const ValueType* GetLast() const {
    return IsEmpty() ? nullptr : (const ValueType*)Root.pPrev;
  }
  ValueType* GetFirst() {
    return IsEmpty() ? nullptr : (ValueType*)Root.pNext;
  }
  ValueType* GetLast() {
    return IsEmpty() ? nullptr : (ValueType*)Root.pPrev;
  }

  // Determine if list is empty (i.e.) points to itself.
  // Go through void* access to avoid issues with strict-aliasing optimizing out the
  // access after RemoveNode(), etc.
  bool IsEmpty() const {
    return Root.pNext == &Root;
  }
  bool IsFirst(const ValueType* p) const {
    return p == Root.pNext;
  }
  bool IsLast(const ValueType* p) const {
    return p == Root.pPrev;
  }
  bool IsNull(const ListNode<B>* p) const {
    return p == nullptr || p == &Root;
  }

  inline const ValueType* GetPrev(const ValueType* p) const {
    return IsNull(p->pPrev) ? nullptr : (const ValueType*)p->pPrev;
  }
  inline const ValueType* GetNext(const ValueType* p) const {
    return IsNull(p->pNext) ? nullptr : (const ValueType*)p->pNext;
  }
  inline ValueType* GetPrev(ValueType* p) {
    return IsNull(p->pPrev) ? nullptr : (ValueType*)p->pPrev;
  }
  inline ValueType* GetNext(ValueType* p) {
    return IsNull(p->pNext) ? nullptr : (ValueType*)p->pNext;
  }

  void PushFront(ValueType* p) {
    p->pNext = Root.pNext;
    p->pPrev = &Root;
    Root.pNext->pPrev = p;
    Root.pNext = p;
  }

  void PushBack(ValueType* p) {
    p->pPrev = Root.pPrev;
    p->pNext = &Root;
    Root.pPrev->pNext = p;
    Root.pPrev = p;
  }

  static void Remove(ValueType* p) {
    p->pPrev->pNext = p->pNext;
    p->pNext->pPrev = p->pPrev;
    p->pPrev = nullptr;
    p->pNext = nullptr;
  }

  void BringToFront(ValueType* p) {
    Remove(p);
    PushFront(p);
  }

  void SendToBack(ValueType* p) {
    Remove(p);
    PushBack(p);
  }

  // Appends the contents of the argument list to the front of this list;
  // items are removed from the argument list.
  void PushListToFront(List<T>& src) {
    if (!src.IsEmpty()) {
      ValueType* pfirst = src.GetFirst();
      ValueType* plast = src.GetLast();
      src.Clear();
      plast->pNext = Root.pNext;
      pfirst->pPrev = &Root;
      Root.pNext->pPrev = plast;
      Root.pNext = pfirst;
    }
  }

  void PushListToBack(List<T>& src) {
    if (!src.IsEmpty()) {
      ValueType* pfirst = src.GetFirst();
      ValueType* plast = src.GetLast();
      src.Clear();
      plast->pNext = &Root;
      pfirst->pPrev = Root.pPrev;
      Root.pPrev->pNext = pfirst;
      Root.pPrev = plast;
    }
  }

  // Removes all source list items after (and including) the 'pfirst' node from the
  // source list and adds them to out list.
  void PushFollowingListItemsToFront(List<T>& src, ValueType* pfirst) {
    if (pfirst != &src.Root) {
      ValueType* plast = src.Root.pPrev;

      // Remove list remainder from source.
      pfirst->pPrev->pNext = &src.Root;
      src.Root.pPrev = pfirst->pPrev;
      // Add the rest of the items to list.
      plast->pNext = Root.pNext;
      pfirst->pPrev = &Root;
      Root.pNext->pPrev = plast;
      Root.pNext = pfirst;
    }
  }

  // Removes all source list items up to but NOT including the 'pend' node from the
  // source list and adds them to out list.
  void PushPrecedingListItemsToFront(List<T>& src, ValueType* ptail) {
    if (src.GetFirst() != ptail) {
      ValueType* pfirst = src.Root.pNext;
      ValueType* plast = ptail->pPrev;

      // Remove list remainder from source.
      ptail->pPrev = &src.Root;
      src.Root.pNext = ptail;

      // Add the rest of the items to list.
      plast->pNext = Root.pNext;
      pfirst->pPrev = &Root;
      Root.pNext->pPrev = plast;
      Root.pNext = pfirst;
    }
  }

  // Removes a range of source list items starting at 'pfirst' and up to, but not including 'pend',
  // and adds them to out list. Note that source items MUST already be in the list.
  void PushListItemsToFront(ValueType* pfirst, ValueType* pend) {
    if (pfirst != pend) {
      ValueType* plast = pend->pPrev;

      // Remove list remainder from source.
      pfirst->pPrev->pNext = pend;
      pend->pPrev = pfirst->pPrev;
      // Add the rest of the items to list.
      plast->pNext = Root.pNext;
      pfirst->pPrev = &Root;
      Root.pNext->pPrev = plast;
      Root.pNext = pfirst;
    }
  }

  void Alloc_MoveTo(List<T>* pdest) {
    if (IsEmpty())
      pdest->Clear();
    else {
      pdest->Root.pNext = Root.pNext;
      pdest->Root.pPrev = Root.pPrev;

      Root.pNext->pPrev = &pdest->Root;
      Root.pPrev->pNext = &pdest->Root;
    }
  }

 private:
  // Copying is prohibited
  List(const List<T>&);
  const List<T>& operator=(const List<T>&);

  ListNode<B> Root;
};

//------------------------------------------------------------------------
// ***** FreeListElements
//
// Remove all elements in the list and free them in the allocator

template <class List, class Allocator>
void FreeListElements(List& list, Allocator& allocator) {
  typename List::ValueType* self = list.GetFirst();
  while (!list.IsNull(self)) {
    typename List::ValueType* next = list.GetNext(self);
    allocator.Free(self);
    self = next;
  }
  list.Clear();
}

} // namespace OVR

#endif
