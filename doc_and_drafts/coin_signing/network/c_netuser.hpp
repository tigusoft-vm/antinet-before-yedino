#ifndef C_NETUSER_H
#define C_NETUSER_H

#include "../libs01.hpp"
#include "../c_user.hpp"
#include "c_TCPasync.hpp"

using namespace boost::asio;

class c_netuser final : public c_user {
  public:
    c_netuser (const std::string &username,
               const std::string &host,
               unsigned short server_port = 30000,
               unsigned short local_port = 30000);
    c_netuser (c_user &&user,
               const std::string &host,
               unsigned short server_port = 30000,
               unsigned short local_port = 30000);

    void set_target(const std::string &host, unsigned short server_port);
    unsigned short get_server_port();
    unsigned short get_local_port();

    /// Setting new target is optional
    void send_token_bynet ();
    void send_token_bynet (const std::string &host, unsigned short server_port);

    ed_key get_public_key();


    virtual ~c_netuser ();

  private:
//    friend class boost::serialization::access;

//    template <typename Archive>
//    void serialize (Archive &ar, const unsigned int version) {
//        UNUSED(version);
//        ar & boost::serialization::base_object<c_user>(*this);
//        ar & server_port;
//    }

    c_TCPasync m_TCPasync;
    std::map<protocol,c_TCPcommand> m_TCPcommands;	// TODO vector --> map
    std::atomic<bool> m_stop_thread;
    std::thread m_thread;

    void send_contract();
    void check_inboxes ();
    void recieve_coin ();
    void recieve_contract ();
};

#endif // C_NETUSER_H