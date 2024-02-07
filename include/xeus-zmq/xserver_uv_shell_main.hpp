/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SERVER_UV_SHELL_MAIN_HPP
#define XEUS_SERVER_UV_SHELL_MAIN_HPP

#include <atomic>

#include "zmq_addon.hpp"

#include "xeus/xserver.hpp"
#include "xeus/xkernel_configuration.hpp"
#include "xeus/xeus_context.hpp"

#include "xeus-zmq.hpp"
#include "xthread.hpp"

namespace xeus
{
    class xauthentication;
    class xcontrol_uv;
    class xheartbeat;
    class xpublisher;
    class xshell_uv;
    class xcontext;

    class XEUS_ZMQ_API xserver_uv_shell_main : public xserver
    {
    public:

        using controller_ptr = std::unique_ptr<xcontrol_uv>;
        using heartbeat_ptr = std::unique_ptr<xheartbeat>;
        using publisher_ptr = std::unique_ptr<xpublisher>;
        using shell_ptr = std::unique_ptr<xshell_uv>;

        xserver_uv_shell_main(zmq::context_t& context,
                          const xconfiguration& config,
                          nl::json::error_handler_t eh);

        ~xserver_uv_shell_main() override;

        // The xcontrol_uv object needs to call this method
        using xserver::notify_control_listener;
        // The xshell_uv object needs to call these methods
        using xserver::notify_shell_listener;
        using xserver::notify_stdin_listener;

        zmq::multipart_t notify_internal_listener(zmq::multipart_t& wire_msg);
        void notify_control_stopped();

        xmessage deserialize(zmq::multipart_t& wire_msg) const;

        zmq::multipart_t serialize_iopub(xpub_message&& msg);

    protected:

        xcontrol_messenger& get_control_messenger_impl() override;

        void send_shell_impl(xmessage msg) override;
        void send_control_impl(xmessage msg) override;
        void send_stdin_impl(xmessage msg) override;
        void publish_impl(xpub_message msg, channel c) override;

        void start_impl(xpub_message msg) override;
        void abort_queue_impl(const listener& l, long polling_interval) override;
        void stop_impl() override;
        void update_config_impl(xconfiguration& config) const override;

        void start_control_thread();
        void start_heartbeat_thread();
        void start_publisher_thread();
        void start_shell_thread();

        xcontrol_uv& get_controller();
        xshell_uv& get_shell();

        bool is_control_stopped() const;

    private:

        void start_server(zmq::multipart_t& wire_msg);

        using authentication_ptr = std::unique_ptr<xauthentication>;
        authentication_ptr p_auth;

        controller_ptr p_controller;
        heartbeat_ptr p_heartbeat;
        publisher_ptr p_publisher;
        shell_ptr p_shell;

        xthread m_control_thread;
        xthread m_hb_thread;
        xthread m_iopub_thread;
        xthread m_shell_thread;

        nl::json::error_handler_t m_error_handler;

        std::atomic<bool> m_control_stopped;
    };

    XEUS_ZMQ_API
    std::unique_ptr<xserver> make_xserver_uv_shell_main(
        xcontext& context,
        const xconfiguration& config,
        nl::json::error_handler_t eh = nl::json::error_handler_t::strict);

} // namespace xeus

#endif // XEUS_SERVER_UV_SHELL_MAIN_HPP
