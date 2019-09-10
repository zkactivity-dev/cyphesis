// AUTOGENERATED file, created by the tool generate_stub.py, don't edit!
// If you want to add your own functionality, instead edit the stubCommAsioListener_custom.h file.

#ifndef STUB_SERVER_COMMASIOLISTENER_H
#define STUB_SERVER_COMMASIOLISTENER_H

#include "server/CommAsioListener.h"
#include "stubCommAsioListener_custom.h"

#ifndef STUB_CommAsioListener_CommAsioListener
//#define STUB_CommAsioListener_CommAsioListener
  template <typename ProtocolT,typename ClientT>
   CommAsioListener<ProtocolT,ClientT>::CommAsioListener(std::function<std::shared_ptr<ClientT>()> clientCreator, std::function<void(ClientT&)> clientStarter, std::string serverName, boost::asio::io_context& ioService, const typename ProtocolT::endpoint& endpoint)
    : mFactories(nullptr)
  {
    
  }
#endif //STUB_CommAsioListener_CommAsioListener

#ifndef STUB_CommAsioListener_CommAsioListener_DTOR
//#define STUB_CommAsioListener_CommAsioListener_DTOR
  template <typename ProtocolT,typename ClientT>
   CommAsioListener<ProtocolT,ClientT>::~CommAsioListener()
  {
    
  }
#endif //STUB_CommAsioListener_CommAsioListener_DTOR

#ifndef STUB_CommAsioListener_startAccept
//#define STUB_CommAsioListener_startAccept
  template <typename ProtocolT,typename ClientT>
  void CommAsioListener<ProtocolT,ClientT>::startAccept()
  {
    
  }
#endif //STUB_CommAsioListener_startAccept


#endif