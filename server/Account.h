// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000,2001 Alistair Riddoch

#ifndef SERVER_ACCOUNT_H
#define SERVER_ACCOUNT_H

#include "common/OOGThing.h"

class Connection;

class Account : public OOGThing {
  private:
    typedef std::map<std::string, SigC::Connection *> ConMap;

    EntityDict m_charactersDict;
    ConMap m_destroyedConnections;

    Entity * addCharacter(const std::string &,
                          const Atlas::Message::Object::MapType &);
  protected:
    void characterDestroyed(std::string);

    virtual OpVector characterError(const Create &, const Atlas::Message::Object::MapType &) const = 0;

  public:
    Connection * m_connection;
    std::string m_username;
    std::string m_password;

    Account(Connection * conn, const std::string & username,
                               const std::string & passwd,
                               const std::string & id);
    virtual ~Account();

    virtual const char * getType() const;

    virtual void addToObject(Atlas::Message::Object::MapType &) const;
    virtual OpVector LogoutOperation(const Logout & op);
    virtual OpVector CreateOperation(const Create & op);
    virtual OpVector ImaginaryOperation(const Imaginary & op);
    virtual OpVector TalkOperation(const Talk & op);
    virtual OpVector LookOperation(const Look & op);

    void addCharacter(Entity *);

    const EntityDict & getCharacters() const {
        return m_charactersDict;
    }
};

#endif // SERVER_ACCOUNT_H
