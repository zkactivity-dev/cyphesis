// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2001 Alistair Riddoch
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

// $Id$

#ifndef TOOLS_ADMIN_CLIENT_H
#define TOOLS_ADMIN_CLIENT_H

#include "common/AtlasStreamClient.h"

#include <Atlas/Objects/ObjectsFwd.h>

#include <set>

typedef std::multimap<std::string, std::pair<std::pair<std::string, std::string>, Atlas::Message::MapType> > RuleWaitList;

/// \brief Client class to encapsulate functionality of an administrative client
class AdminClient : public AtlasStreamClient
{
  private:
    /// \brief Flags used to track replies from the server
    bool error_flag, reply_flag, login_flag;
    /// \brief Password used to log into the server
    std::string password;
    /// \brief Account identifier returned after successful login
    std::string accountId;
    /// \brief Stored error message from the last received Error operation
    std::string m_errorMessage;
    /// \brief Store of rules which can't be uploaded until their parent has
    /// been uploaded
    RuleWaitList m_waitingRules;
    /// \brief List of names of rules already uploaded
    std::set<std::string> m_uploadedRules;

  protected:
    virtual void objectArrived(const Atlas::Objects::Root &);

    virtual void operation(const Atlas::Objects::Operation::RootOperation &);

    virtual void infoArrived(const Atlas::Objects::Operation::RootOperation &);
    virtual void errorArrived(const Atlas::Objects::Operation::RootOperation &);

    void waitForInfo();
    int checkRule(const std::string & id);
  public:
    AdminClient();
    ~AdminClient();

    int login();
    void getLogin();
    int uploadRule(const std::string & id, const std::string & set,
                   const Atlas::Message::MapType &);

    /// \brief Set the password used to log into the server
    void setPassword(const std::string & passwd) {
        password = passwd;
    }

    /// \brief Set the username used to log into the server
    void setUsername(const std::string & uname) {
        m_username = uname;
    }

    void report();
};

#endif // TOOLS_ADMIN_CLIENT_H
