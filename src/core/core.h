#pragma once

// Generic swap template.
#ifndef SWAP
#define SWAP(m_x, m_y) swap_impl((m_x), (m_y))
template<class T>
inline void swap_impl(T& x, T& y)
{
    T t = x;
    x = y;
    y = t;
}
#endif// SWAP