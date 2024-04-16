/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "xeus-zmq/xloop.hpp"

namespace xeus
{
    template <typename T>
    xloop_wrapper<T>::xloop_wrapper(const T *loop) : m_loop(loop)
    {
    }

    template <typename T>
    void xloop_wrapper<T>::run() const
    {
        m_loop->run();
    }
}
