#include "amqp_consumer.hpp"

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include <components/log/log.hpp>

using namespace components;

amqp_consumer::amqp_consumer(const std::string& url) : _url(url) {
    if (_url.scheme() != "amqp") {
        throw std::runtime_error("URL scheme must be: amqp, provided: " + std::string{_url.scheme()});
    }
}

const char* amqp_consumer::get_host() const {
    if (_url.host().size()) {
        return std::string{ _url.host()}.c_str();
    }
    return "localhost";
}

int amqp_consumer::get_port() const {
    std::string port{_url.port()};
    if (port.size()) {
        return stoi(port);
    }
    return 5672;
}

void amqp_consumer::start_loop() {
    amqp_socket_t* socket = NULL;
    amqp_connection_state_t conn;
    int error = 0;

    conn = amqp_new_connection();
    socket = amqp_tcp_socket_new(conn);
    if (!socket) {
        throw std::runtime_error("Cannot create socket");
    }

    error = amqp_socket_open(socket, get_host(), get_port());
    if (error) {
        throw std::runtime_error("Cannot listen on " + std::string(get_host()) + ":" + std::to_string(get_port()));
    }

    // TODO binding to queue

    get_logger().info("Listening for queue");
    get_logger().info(std::string{_url.user()});
    get_logger().info(std::string{_url.password()});

    while (true) {
        amqp_rpc_reply_t ret;
        amqp_envelope_t envelope;

        amqp_maybe_release_buffers(conn);
        ret = amqp_consume_message(conn, &envelope, NULL, 0);
        if (ret.reply_type != AMQP_RESPONSE_NORMAL) {
            // TODO close and log
            break;
        }

        // TODO process message - call task (how pass task context here?)

        amqp_destroy_envelope(&envelope);
    }
}
