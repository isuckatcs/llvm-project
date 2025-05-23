// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___CXX03___ALGORITHM_ADJACENT_FIND_H
#define _LIBCPP___CXX03___ALGORITHM_ADJACENT_FIND_H

#include <__cxx03/__algorithm/comp.h>
#include <__cxx03/__algorithm/iterator_operations.h>
#include <__cxx03/__config>
#include <__cxx03/__iterator/iterator_traits.h>
#include <__cxx03/__utility/move.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__cxx03/__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

template <class _Iter, class _Sent, class _BinaryPredicate>
_LIBCPP_NODISCARD _LIBCPP_HIDE_FROM_ABI _Iter __adjacent_find(_Iter __first, _Sent __last, _BinaryPredicate&& __pred) {
  if (__first == __last)
    return __first;
  _Iter __i = __first;
  while (++__i != __last) {
    if (__pred(*__first, *__i))
      return __first;
    __first = __i;
  }
  return __i;
}

template <class _ForwardIterator, class _BinaryPredicate>
_LIBCPP_NODISCARD inline _LIBCPP_HIDE_FROM_ABI _ForwardIterator
adjacent_find(_ForwardIterator __first, _ForwardIterator __last, _BinaryPredicate __pred) {
  return std::__adjacent_find(std::move(__first), std::move(__last), __pred);
}

template <class _ForwardIterator>
_LIBCPP_NODISCARD inline _LIBCPP_HIDE_FROM_ABI _ForwardIterator
adjacent_find(_ForwardIterator __first, _ForwardIterator __last) {
  return std::adjacent_find(std::move(__first), std::move(__last), __equal_to());
}

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___CXX03___ALGORITHM_ADJACENT_FIND_H
