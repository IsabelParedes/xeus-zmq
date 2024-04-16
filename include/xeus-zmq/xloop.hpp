/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XLOOP_HPP
#define XLOOP_HPP

#include "xeus-zmq.hpp"

namespace xeus
{
    class XEUS_ZMQ_API xloop_interface
    {
    public:

        virtual ~xloop_interface() = default;
        virtual void run() const = 0;

    };

    template <typename T>
    class XEUS_ZMQ_API xloop_wrapper : public xloop_interface
    {
    public:

        xloop_wrapper(const T *loop);
        void run() const override;

    private:

        const T *m_loop;
    };

}

#endif
