//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Mar 2, 2013
//
// Avoid the abomination that is std::algorithm on std::set
//
// WARNING (VS15): std::set::operator<(...) uses T::operator<(...) NOT std::less<T>() so if you have custom
// ordering and you want to compare sets, make sure you also define T::operator<(...) appropriately.

#ifndef FGSTDSET_HPP
#define FGSTDSET_HPP

#include "FgStdLibs.hpp"

template<class T>
std::ostream &
operator<<(std::ostream & os,const std::set<T> & v)
{
    os << "{";
    for (typename std::set<T>::const_iterator it=v.begin(); it != v.end(); ++it)
        os << *it << ",";
    return os << "}";
}

// Useful in functional contexts where 's' is already an expression:
template<class T>
inline
std::vector<T>
fgSetToVec(const std::set<T> & s)
{return std::vector<T>(s.begin(),s.end()); }

template<class T>
inline
std::set<T>
fgVecToSet(const std::vector<T> & v)
{return std::set<T>(v.begin(),v.end()); }

template<class T>
inline
bool
fgContains(const std::set<T> & s,const T & v)
{return (s.find(v) != s.end()); }

// Returns true if the intersect of s0 and s1 is non-empty. Loop is through s1 so prefer s0 for the larger set.
template<class T>
bool
fgContainsAny(const std::set<T> & s0,const std::set<T> & s1)
{
    for (auto it=s1.begin(); it != s1.end(); ++it)
        if (fgContains(s0,*it))
            return true;
    return false;
}

// Returns true if s0 contains all elements of s1. Loop is through s1 so prefer s0 for the larger set.
template<class T>
bool
fgContainsAll(const std::set<T> & s0,const std::set<T> & s1)
{
    for (auto it=s1.begin(); it != s1.end(); ++it)
        if (!fgContains(s0,*it))
            return false;
    return true;
}

template<class T>
void
fgUnion_(std::set<T> & s0,const std::set<T> & s1)
{s0.insert(s1.begin(),s1.end()); }

template<class T>
void
fgUnion_(std::set<T> & s0,const std::vector<T> & s1)
{s0.insert(s1.begin(),s1.end()); }

template<class T>
std::set<T>
fgUnion(const std::set<T> & s0,const std::set<T> & s1)
{
    // WTF is with set_union ...
    std::set<T>     ret = s0;
    ret.insert(s1.begin(),s1.end());
    return ret;
}

// std::set_intersection is stupidly complex.
// Loop is through s1 so prefer s0 for the larger set.
template<class T>
std::set<T>
fgIntersection(const std::set<T> & s0,const std::set<T> & s1)
{
    std::set<T>         ret;
    for (const T & s : s1)
        if (fgContains(s0,s))
            ret.insert(s);
    return ret;
}

template<class T>
std::set<T>
operator+(std::set<T> lhs,const std::set<T> & rhs)
{
    lhs.insert(rhs.begin(),rhs.end());
    return lhs;
}

// set_difference is ridiculously verbose:
template<class T>
std::set<T>
operator-(const std::set<T> & lhs,const std::set<T> & rhs)
{
    std::set<T>         ret;
    for (const T & l : lhs)
        if (!fgContains(rhs,l))
            ret.insert(l);
    return ret;
}

// plus is a nice short-hand for union:
template<class T>
void
operator+=(std::set<T> & l,const std::set<T> & r)
{l.insert(r.begin(),r.end()); }

template<class T>
void
operator-=(std::set<T> & lhs,const std::set<T> & rhs)
{
    for (const T & r : rhs) {
        auto it = lhs.find(r);
        if (it != lhs.end())
            lhs.erase(it);
    }
}

template<class T>
std::set<T>
fgInsert(const std::set<T> & s,const T & v)
{
    std::set<T>         ret = s;
    ret.insert(v);
    return ret;
}

#endif
