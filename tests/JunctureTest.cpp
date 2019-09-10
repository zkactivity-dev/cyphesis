// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2010 Alistair Riddoch
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif

#include "server/Juncture.h"

#include "server/CommPeer.h"
#include "server/Connection.h"
#include "server/Peer.h"
#include "server/ServerRouting.h"
#include "common/CommAsioClient_impl.h"

#include "common/operations/Connect.h"
#include "common/CommSocket.h"

#include <Atlas/Objects/Operation.h>

#include <cassert>


class BaseWorld;

Atlas::Objects::Factories factories;
class StubSocket : public CommSocket
{
  public:
    StubSocket(boost::asio::io_context& io_context) : CommSocket(io_context)
    {
    }

    virtual void disconnect()
    {
    }

    virtual int flush()
    {
        return 0;
    }

};

// Test class to expose protected methods.
class TestJuncture : public Juncture
{
  public:
    TestJuncture(Connection * c = 0) : Juncture(c, "1", 1) { }

    void test_onPeerLost() { onPeerLost(); }
    void test_onPeerReplied(const Operation & op) { onPeerReplied(op); }

    void test_addPeer(Peer * p) { m_peer = p; }
//    void test_addSocket(CommPeer * cp) { m_socket = cp; }
};

int stub_CommPeer_connect_return = 0;

int main()
{
    {
        Juncture * j = new Juncture(0, "1", 1);

        delete j;
    }

    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;
        Operation op;
        j->operation(op, res);

        delete j;
    }

    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;
        Atlas::Objects::Operation::Login op;
        j->operation(op, res);

        delete j;
    }

    // Login op, no args
    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;
        Operation op;
        j->LoginOperation(op, res);

        delete j;
    }

    // Login op, empty arg
    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;

        Operation op;
        Atlas::Objects::Root arg;
        op->setArgs1(arg);
        
        j->LoginOperation(op, res);

        delete j;
    }

    // Login op, username in arg
    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;

        Operation op;
        Atlas::Objects::Root arg;
        arg->setAttr("username", "69e362c6-03a4-11e0-9608-001ec93e7c08");
        op->setArgs1(arg);
        
        j->LoginOperation(op, res);

        delete j;
    }

    // Login op, bad username in arg
    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;

        Operation op;
        Atlas::Objects::Root arg;
        arg->setAttr("username", 0x69e362c6);
        op->setArgs1(arg);
        
        j->LoginOperation(op, res);

        delete j;
    }

    // Login op, username & password in arg
    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;

        Operation op;
        Atlas::Objects::Root arg;
        arg->setAttr("username", "69e362c6-03a4-11e0-9608-001ec93e7c08");
        arg->setAttr("password", "a12a2f3a-03a4-11e0-8379-001ec93e7c08");
        op->setArgs1(arg);
        
        j->LoginOperation(op, res);

        delete j;
    }

    // Login op, username & bad password in arg
    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;

        Operation op;
        Atlas::Objects::Root arg;
        arg->setAttr("username", "69e362c6-03a4-11e0-9608-001ec93e7c08");
        arg->setAttr("password", 0x12a2f3aL);
        op->setArgs1(arg);
        
        j->LoginOperation(op, res);

        delete j;
    }

    // Login op, username & password in arg, connected
    {
        ServerRouting sr(*(BaseWorld*)0, "", "", "2", 2, "3", 3);
        TestJuncture * j = new TestJuncture(0);
        boost::asio::io_context io_context;
        CommPeer * cp = new CommPeer("", io_context, factories);
        j->test_addPeer(new Peer(*cp, sr, "", 6767, "4", 4));

        OpVector res;

        Operation op;
        Atlas::Objects::Root arg;
        arg->setAttr("username", "69e362c6-03a4-11e0-9608-001ec93e7c08");
        arg->setAttr("password", "a12a2f3a-03a4-11e0-8379-001ec93e7c08");
        op->setArgs1(arg);
        
        j->LoginOperation(op, res);

        delete j;
    }

    // Login op, username & password in arg, connected already authenticating
    {
        ServerRouting sr(*(BaseWorld*)0, "", "", "2", 2, "3", 3);

        TestJuncture * j = new TestJuncture(0);

        boost::asio::io_context io_context;
        CommPeer * cp = new CommPeer("", io_context, factories);
        Peer * p = new Peer(*cp, sr, "", 6767, "4", 4);
        j->test_addPeer(p);

        p->setAuthState(PEER_AUTHENTICATING);

        OpVector res;

        Operation op;
        Atlas::Objects::Root arg;
        arg->setAttr("username", "69e362c6-03a4-11e0-9608-001ec93e7c08");
        arg->setAttr("password", "a12a2f3a-03a4-11e0-8379-001ec93e7c08");
        op->setArgs1(arg);
        
        j->LoginOperation(op, res);

        delete j;
    }

    // Login op, username & password in arg, connected, with serialno
    {
        ServerRouting sr(*(BaseWorld*)0, "", "", "2", 2, "3", 3);

        TestJuncture * j = new TestJuncture(0);

        boost::asio::io_context io_context;
        CommPeer * cp = new CommPeer("", io_context, factories);
        j->test_addPeer(new Peer(*cp, sr, "", 6767, "4", 4));

        OpVector res;

        Operation op;
        Atlas::Objects::Root arg;
        arg->setAttr("username", "69e362c6-03a4-11e0-9608-001ec93e7c08");
        arg->setAttr("password", "a12a2f3a-03a4-11e0-8379-001ec93e7c08");
        op->setArgs1(arg);
        op->setSerialno(0x6dc5b5eaL);
        
        j->LoginOperation(op, res);

        delete j;
    }

    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;
        Operation op;
        j->OtherOperation(op, res);

        delete j;
    }

    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;
        Atlas::Objects::Operation::Connect op;
        j->OtherOperation(op, res);

        delete j;
    }

    // Connect op, no args
    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;
        Operation op;
        j->customConnectOperation(op, res);

        delete j;
    }

    // Connect op, no args, already connected
    {
        TestJuncture * j = new TestJuncture(0);

        j->test_addPeer(new Peer(*(CommPeer*)0, *(ServerRouting*)0, "", 6767, "4", 4));

        OpVector res;
        Operation op;
        j->customConnectOperation(op, res);

        delete j;
    }

    // Connect op, hostname in arg
    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;
        Operation op;
        Atlas::Objects::Root arg;
        arg->setAttr("hostname", "3752ca4a-03a9-11e0-bd8a-001ec93e7c08");

        op->setArgs1(arg);

        j->customConnectOperation(op, res);

        delete j;
    }

    // Connect op, bad hostname in arg
    {
        Juncture * j = new Juncture(0, "1", 1);

        OpVector res;
        Operation op;
        Atlas::Objects::Root arg;
        arg->setAttr("hostname", 0x3752ca4aL);

        op->setArgs1(arg);

        j->customConnectOperation(op, res);

        delete j;
    }

    // Connect op, hostname and port in arg, connected this end
    {
        ServerRouting sr(*(BaseWorld*)0, "", "", "2", 2, "3", 3);
        boost::asio::io_context io_context;
        CommSocket * cc = new StubSocket(io_context);
        Connection * c = new Connection(*cc, sr, "", "4", 4);

        Juncture * j = new Juncture(c, "1", 1);

        OpVector res;
        Operation op;
        Atlas::Objects::Root arg;
        arg->setAttr("hostname", "3752ca4a-03a9-11e0-bd8a-001ec93e7c08");
        arg->setAttr("port", 0x03a9);

        op->setArgs1(arg);

        j->customConnectOperation(op, res);

        delete j;
    }

    // Connect op, hostname and port in arg, connected this end, connect fails
    {
        stub_CommPeer_connect_return = -1;

        ServerRouting sr(*(BaseWorld*)0, "", "", "2", 2, "3", 3);
        boost::asio::io_context io_context;
        CommSocket * cc = new StubSocket(io_context);
        Connection * c = new Connection(*cc, sr, "", "4", 4);

        Juncture * j = new Juncture(c, "1", 1);

        OpVector res;
        Operation op;
        Atlas::Objects::Root arg;
        arg->setAttr("hostname", "3752ca4a-03a9-11e0-bd8a-001ec93e7c08");
        arg->setAttr("port", 0x03a9);

        op->setArgs1(arg);

        j->customConnectOperation(op, res);

        stub_CommPeer_connect_return = 0;

        delete j;
    }

    // Teleport unconnected
    {
        TestJuncture * j = new TestJuncture(0);

        j->teleportEntity(0);

        delete j;
    }

    // Teleport connected
    {
        TestJuncture * j = new TestJuncture(0);

        j->test_addPeer(new Peer(*(CommPeer*)0, *(ServerRouting*)0, "", 6767, "4", 4));
        j->teleportEntity(0);

        delete j;
    }

    {
        TestJuncture * j = new TestJuncture(0);

        j->test_onPeerLost();

        delete j;
    }

    // Peer replied, unconnected this end
    {
        TestJuncture * j = new TestJuncture(0);

        Operation op;
        j->test_onPeerReplied(op);

        delete j;
    }

    // Peer replied, connected this end
    {
        Connection * c = new Connection(*(CommSocket*)0,
                                        *(ServerRouting*)0, "", "4", 4);

        TestJuncture * j = new TestJuncture(c);

        Operation op;
        j->test_onPeerReplied(op);

        delete j;
    }

    return 0;
}

// stubs

#include "common/log.h"

#include <Atlas/Negotiate.h>
#include "stubs/common/stubLink.h"

namespace Atlas { namespace Objects { namespace Operation {

int CONNECT_NO = 500;

} } }
#include "stubs/server/stubServerRouting.h"
#include "stubs/common/stubInheritance.h"

CommPeer::CommPeer(const std::string & n, boost::asio::io_context& svr, Atlas::Objects::Factories& factories) :
        CommAsioClient<boost::asio::ip::tcp> (n, svr, factories), m_auth_timer(svr)
{
}

CommPeer::~CommPeer()
{
}

void CommPeer::setup(Link * connection)
{
}

void CommPeer::connect(boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> const&)
{
}

int CommSocket::flush()
{
    return 0;
}

Peer::Peer(CommSocket & client,
           ServerRouting & svr,
           const std::string & addr,
           int port,
           const std::string & id, long iid) :
      Link(client, id, iid),
      m_state(PEER_INIT),
      m_server(svr)
{
}

Peer::~Peer()
{
}

PeerAuthState Peer::getAuthState()
{
    return m_state;
}

void Peer::setAuthState(PeerAuthState state)
{
    m_state = state;
}

void Peer::externalOperation(const Operation & op, Link &)
{
}

void Peer::operation(const Operation &op, OpVector &res)
{
}

int Peer::teleportEntity(const LocatedEntity * ent)
{
    return 0;
}

void Peer::cleanTeleports()
{
}

#include "stubs/server/stubConnection.h"
#include "stubs/server/stubConnectableRouter.h"
#include "stubs/common/stubRouter.h"

void log(LogLevel lvl, const std::string & msg)
{
}

#include <common/Shaker.h>

Shaker::Shaker()
{
}

std::string Shaker::generateSalt(size_t length)
{
    return "";
}
