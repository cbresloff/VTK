 /*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSMPThreadLocalObject.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMPThreadLocalObject - Thread local storage for VTK objects.
// .SECTION Description
// This class essentially does the same thing as vtkSMPThreadLocal with
// 2 additional functions:
// - Local() allocates an object of the template argument type using ::New
// - The destructor calls Delete() on all objects created with Local().
//
// .SECTION Warning
// There is absolutely no guarantee to the order in which the local objects
// will be stored and hence the order in which they will be traversed when
// using iterators. You should not even assume that two vtkSMPThreadLocal
// populated in the same parallel section will be populated in the same
// order. For example, consider the following
// \verbatim
// vtkSMPThreadLocal<int> Foo;
// vtkSMPThreadLocal<int> Bar;
// class AFunctor
// {
//    void Initialize() const
//    {
//      int& foo = Foo.Local();
//      int& bar = Bar.Local();
//      foo = random();
//      bar = foo;
//    }
//
//    void operator()(vtkIdType, vtkIdType)
//    {}
//    void Finalize()
//    {}
// };
//
// AFunctor functor;
// vtkSMPTools::For(0, 100000, functor);
//
// vtkSMPThreadLocal<int>::iterator itr1 = Foo.begin();
// vtkSMPThreadLocal<int>::iterator itr2 = Bar.begin();
// while (itr1 != Foo.end())
// {
//   assert(*itr1 == *itr2);
//   ++itr1; ++itr2;
// }
// \endverbatim
//
// It is possible and likely that the assert() will fail using the TBB
// backend. So if you need to store values related to each other and
// iterate over them together, use a struct or class to group them together
// and use a thread local of that class.
//
// .SECTION See Also
// vtkSMPThreadLocal

#ifndef __vtkSMPThreadLocalObject_h
#define __vtkSMPThreadLocalObject_h

#include "vtkSMPThreadLocal.h"

template <typename T>
class vtkSMPThreadLocalObject
{
  typedef vtkSMPThreadLocal<T*> TLS;
  typedef typename vtkSMPThreadLocal<T*>::iterator TLSIter;

  // Hide the copy constructor for now and assignment
  // operator for now.
  vtkSMPThreadLocalObject(const vtkSMPThreadLocalObject&);
  void operator=(const vtkSMPThreadLocalObject&);

public:
  // Description:
  // Default constructor.
  vtkSMPThreadLocalObject() : Internal(0)
    {
    }

  virtual ~vtkSMPThreadLocalObject()
    {
      iterator iter = this->begin();
      while (iter != this->end())
        {
        if (*iter)
          {
          (*iter)->Delete();
          }
        ++iter;
        }
    }

  // Description:
  // Returns an object local to the current thread.
  // This object is allocated with ::New() and will
  // be deleted in the destructor of vtkSMPThreadLocalObject.
  T*& Local()
    {
      T*& vtkobject = this->Internal.Local();
      if (!vtkobject)
        {
        vtkobject = T::New();
        }
      return vtkobject;
    }

  // Description:
  // Subset of the standard iterator API.
  // The most common design patter is to use iterators in a sequential
  // code block and to use only the thread local objects in parallel
  // code blocks.
  class iterator
  {
  public:
    iterator& operator++()
      {
        ++this->Iter;
        return *this;
      }

    bool operator!=(const iterator& other)
      {
        return this->Iter != other.Iter;
      }

    T*& operator*()
      {
        return *this->Iter;
      }

  private:
    TLSIter Iter;

    friend class vtkSMPThreadLocalObject<T>;
  };

  iterator begin()
    {
      iterator iter;
      iter.Iter = this->Internal.begin();
      return iter;
    };

  iterator end()
    {
      iterator iter;
      iter.Iter = this->Internal.end();
      return iter;
    }

private:
  TLS Internal;
};

#endif
// VTK-HeaderTest-Exclude: vtkSMPThreadLocalObject.h