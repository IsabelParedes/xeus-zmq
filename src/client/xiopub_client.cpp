/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <iostream>

#include "xiopub_client.hpp"
#include "xclient_zmq_impl.hpp"
#include "../common/xmiddleware_impl.hpp"
#include "../common/xzmq_serializer.hpp"

namespace xeus
{

    xiopub_client::xiopub_client(zmq::context_t& context,
                                 const xeus::xconfiguration& config)
        : m_iopub(context, zmq::socket_type::sub)
        , m_controller(context, zmq::socket_type::rep)
    {
        m_iopub.connect(get_end_point(config.m_transport, config.m_ip, config.m_iopub_port));
        init_socket(m_controller, get_controller_end_point("iopub"));
    }

    xiopub_client::~xiopub_client()
    {
    }

    std::size_t xiopub_client::iopub_queue_size() const
    {
        std::lock_guard<std::mutex> guard(m_queue_mutex);
        return m_message_queue.size();
    }

    std::optional<xmessage> xiopub_client::pop_iopub_message()
    {
        std::lock_guard<std::mutex> guard(m_queue_mutex);
        if (!m_message_queue.empty())
        {
            xmessage msg = std::move(m_message_queue.back());
            m_message_queue.pop();
            return msg;
        } else {
            return std::nullopt;
        }
    }

    void xiopub_client::run()
    {
        zmq::pollitem_t items[] = {
            { m_iopub, 0, ZMQ_POLLIN, 0 }
        };

        while (true)
        {
            zmq::poll(&items[0], 1, std::chrono::milliseconds(-1));

            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_iopub);
                try
                {
                    xmessage msg = p_client_impl->deserialize(wire_msg);
                    {
                        std::lock_guard<std::mutex> guard(m_queue_mutex);
                        m_message_queue.push(std::move(msg));
                    }
                    p_client_impl->notify_shell_listener(std::move(msg));
                }
                catch(std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                }
            }
        }
    }
}
