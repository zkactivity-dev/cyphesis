// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#include "Database.h"

#include "log.h"
#include "debug.h"
#include "globals.h"
#include "stringstream.h"

#include <Atlas/Message/Encoder.h>
#include <Atlas/Codecs/XML.h>

#include <varconf/Config.h>

#include <iostream>

using Atlas::Message::Object;

typedef Atlas::Codecs::XML Serialiser;

static const bool debug_flag = false;

Database * Database::m_instance = NULL;

Database::Database() : m_rule_db("rules"),
                       m_connection(NULL)
{
    
}

bool Database::tuplesOk()
{
    bool status = false;

    PGresult * res;
    while ((res = PQgetResult(m_connection)) != NULL) {
        if (PQresultStatus(res) == PGRES_TUPLES_OK) {
            status = true;
        }
        PQclear(res);
    };
    return status;
}

bool Database::commandOk()
{
    bool status = false;

    PGresult * res;
    while ((res = PQgetResult(m_connection)) != NULL) {
        if (PQresultStatus(res) == PGRES_COMMAND_OK) {
            status = true;
        }
        PQclear(res);
    };
    return status;
}

bool Database::initConnection(bool createDatabase)
{
    std::stringstream conninfos;
    if (global_conf->findItem("cyphesis", "dbserver")) {
        conninfos << "host=" << std::string(global_conf->getItem("cyphesis", "dbserver")) << " ";
    }

    std::string dbname = "cyphesis";
    if (global_conf->findItem("cyphesis", "dbname")) {
        dbname = std::string(global_conf->getItem("cyphesis", "dbname"));
    }
    conninfos << "dbname=" << dbname << " ";

    if (global_conf->findItem("cyphesis", "dbuser")) {
        conninfos << "user=" << std::string(global_conf->getItem("cyphesis", "dbuser")) << " ";
    }

    if (global_conf->findItem("cyphesis", "dbpasswd")) {
        conninfos << "password=" << std::string(global_conf->getItem("cyphesis", "dbpasswd")) << " ";
    }

    const std::string cinfo = conninfos.str();

    if (createDatabase) {
        // Currently not able to create the database
    }

    m_connection = PQconnectdb(cinfo.c_str());

    if ((m_connection == NULL) || (PQstatus(m_connection) != CONNECTION_OK)) {
        log(ERROR, "Database connection failed");
        return false;
    }

    return true;
}

bool Database::initRule(bool createTables)
{
    int status = 0;
    status = PQsendQuery(m_connection, "SELECT * FROM rules WHERE id = 'test' AND contents = 'test';");
    if (!status) {
        reportError();
        return false;
    }
    
    if (!tuplesOk()) {
        debug(std::cout << "Rule table does not exist"
                        << std::endl << std::flush;);
        if (createTables) {
            status = PQsendQuery(m_connection, "CREATE TABLE rules ( id varchar(80) PRIMARY KEY, contents text );");
            if (!status) {
                reportError();
                return false;
            }
            if (!commandOk()) {
                log(ERROR, "Error creating rules table in database");
                reportError();
                return false;
            }
        } else {
            log(ERROR, "Server table does not exist in database");
            return false;
        }
    }
    return true;
}

void Database::shutdownConnection()
{
    PQfinish(m_connection);
}

Database * Database::instance()
{
    if (m_instance == NULL) {
        m_instance = new Database();
    }
    return m_instance;
}

bool Database::decodeObject(const std::string & data,
                            Atlas::Message::Object::MapType &o)
{
    if (data.empty()) {
        return true;
    }

    std::stringstream str(data, std::ios::in);

    Serialiser codec(str, &m_d);
    Atlas::Message::Encoder enc(&codec);

    // Clear the decoder
    m_d.get();

    codec.Poll();

    if (!m_d.check()) {
        log(WARNING, "Database entry does not appear to be decodable");
        return false;
    }
    
    o = m_d.get();
    return true;
}

bool Database::encodeObject(const Atlas::Message::Object::MapType & o,
                            std::string & data)
{
    std::stringstream str;

    Serialiser codec(str, &m_d);
    Atlas::Message::Encoder enc(&codec);

    codec.StreamBegin();
    enc.StreamMessage(o);
    codec.StreamEnd();

    data = str.str();
    return true;
}

bool Database::getObject(const std::string & table, const std::string & key,
                         Atlas::Message::Object::MapType & o)
{
    debug(std::cout << "Database::getObject() " << table << "." << key
                    << std::endl << std::flush;);
    std::string query = std::string("SELECT * FROM ") + table + " WHERE id = '" + key + "';";

    int status = PQsendQuery(m_connection, query.c_str());
    if (!status) {
        reportError();
        return false;
    }

    PGresult * res;
    if ((res = PQgetResult(m_connection)) == NULL) {
        debug(std::cout << "Error accessing " << key << " in " << table
                        << " table" << std::endl << std::flush;);
        return false;
    }
    if ((PQntuples(res) < 1) || (PQnfields(res) < 2)) {
        debug(std::cout << "No entry for " << key << " in " << table
                        << " table" << std::endl << std::flush;);
        PQclear(res);
        while ((res = PQgetResult(m_connection)) != NULL) {
            PQclear(res);
        }
        return false;
    }
    const char * data = PQgetvalue(res, 0, 1);
    debug(std::cout << "Got record " << key << " from database, value " << data
                    << std::endl << std::flush;);

    bool ret = decodeObject(data, o);
    PQclear(res);

    while ((res = PQgetResult(m_connection)) != NULL) {
        PQclear(res);
        log(ERROR, "Extra database result to simple query.");
    };

    return ret;
}

bool Database::putObject(const std::string & table,
                         const std::string & key,
                         const Atlas::Message::Object::MapType & o)
{
    debug(std::cout << "Database::putObject() " << table << "." << key
                    << std::endl << std::flush;);
    std::stringstream str;

    Serialiser codec(str, &m_d);
    Atlas::Message::Encoder enc(&codec);

    codec.StreamBegin();
    enc.StreamMessage(o);
    codec.StreamEnd();

    debug(std::cout << "Encoded to: " << str.str().c_str() << " "
               << str.str().size() << std::endl << std::flush;);
    std::string query = std::string("INSERT INTO ") + table + " VALUES ('" + key + "', '" + str.str() + "');";
    int status = PQsendQuery(m_connection, query.c_str());
    if (!status) {
        reportError();
        return false;
    }
    if (!commandOk()) {
        debug(std::cerr << "Failed to insert item " << key << " into " << table
                        << " table" << std::endl << std::flush;);
        return false;
    }
    return true;
}

bool Database::updateObject(const std::string & table,
                            const std::string & key,
                            const Atlas::Message::Object::MapType & o)
{
    debug(std::cout << "Database::updateObject() " << table << "." << key
                    << std::endl << std::flush;);
    std::stringstream str;

    Serialiser codec(str, &m_d);
    Atlas::Message::Encoder enc(&codec);

    codec.StreamBegin();
    enc.StreamMessage(o);
    codec.StreamEnd();

    std::string query = std::string("UPDATE ") + table + " SET contents = '" +
                        str.str() + "' WHERE id='" + key + "';";
    int status = PQsendQuery(m_connection, query.c_str());
    if (!status) {
        reportError();
        return false;
    }
    if (!commandOk()) {
        debug(std::cerr << "Failed to update item " << key << " into " << table
                        << " table" << std::endl << std::flush;);
        return false;
    }
    return true;
}

bool Database::delObject(const std::string & table, const std::string & key)
{
#if 0
    Dbt key, data;

    key.set_data((void*)keystr);
    key.set_size(strlen(keystr) + 1);

    int err;
    if ((err = db.del(NULL, &key, 0)) != 0) {
        debug(cout << "db.del.ERROR! " << err << endl << flush;);
        return false;
    }
    return true;
#endif
    return true;
}

bool Database::getTable(const std::string & table, Object::MapType &o)
{
    std::string query = std::string("SELECT * FROM ") + table + ";";

    int status = PQsendQuery(m_connection, query.c_str());

    if (!status) {
        reportError();
        return false;
    }

    PGresult * res;
    if ((res = PQgetResult(m_connection)) == NULL) {
        debug(std::cout << "Error accessing " << table
                        << " table" << std::endl << std::flush;);
        return false;
    }
    int results = PQntuples(res);
    if ((results < 1) || (PQnfields(res) < 2)) {
        debug(std::cout << "No entries in " << table
                        << " table" << std::endl << std::flush;);
        PQclear(res);
        while ((res = PQgetResult(m_connection)) != NULL) {
            PQclear(res);
        }
        return false;
    }
    // const char * data = PQgetvalue(res, 0, 1);
    Object::MapType t;
    for(int i = 0; i < results; i++) {
        const char * key = PQgetvalue(res, i, 0);
        const char * data = PQgetvalue(res, i, 1);
        debug(std::cout << "Got record " << key << " from database, value "
                   << data << std::endl << std::flush;);
    
        if (decodeObject(data, t)) {
            o[key] = t;
        }
        
    }
    PQclear(res);

    while ((res = PQgetResult(m_connection)) != NULL) {
        PQclear(res);
        log(ERROR, "Extra database result to simple query.");
    };

    return true;
}

bool Database::clearTable(const std::string & table)
{
    std::string query = std::string("DELETE FROM ") + table + ";";
    int status = PQsendQuery(m_connection, query.c_str());
    if (!status) {
        reportError();
        return false;
    }
    if (!commandOk()) {
        debug(std::cout << "Error clearing " << table
                        << " table" << std::endl << std::flush;);
        reportError();
        return false;
    }
    return true;
}

void Database::reportError()
{
    std::string msg = std::string("DATABASE ERROR: ") +
                      PQerrorMessage(m_connection);
    log(ERROR, msg.c_str());
}

const DatabaseResult Database::runSimpleSelectQuery(const std::string & query)
{
    debug(std::cout << "QUERY: " << query << std::endl << std::flush;);
    int status = PQsendQuery(m_connection, query.c_str());
    if (!status) {
        log(ERROR, "Database query error.");
        reportError();
        return DatabaseResult(0);
    }
    debug(std::cout << "done" << std::endl << std::flush;);
    PGresult * res;
    if ((res = PQgetResult(m_connection)) == NULL) {
        log(ERROR, "Error selecting.");
        reportError();
        debug(std::cout << "Row query didn't work"
                        << std::endl << std::flush;);
        return DatabaseResult(0);
    }
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        log(ERROR, "Error selecting row.");
        std::cout << "QUERY: " << query << std::endl << std::flush;
        reportError();
        PQclear(res);
        res = 0;
    }
    PGresult * nres;
    while ((nres = PQgetResult(m_connection)) != NULL) {
        PQclear(nres);
        log(ERROR, "Extra database result to simple query.");
    };
    return DatabaseResult(res);
}

bool Database::runCommandQuery(const std::string & query)
{
    int status = PQsendQuery(m_connection, query.c_str());
    if (!status) {
        log(ERROR, "Database query error.");
        reportError();
        return false;
    }
    if (!commandOk()) {
        log(ERROR, "Error running command query row.");
        log(NOTICE, query.c_str());
        reportError();
        debug(std::cout << "Row query didn't work"
                        << std::endl << std::flush;);
    } else {
        debug(std::cout << "Query worked" << std::endl << std::flush;);
        return true;
    }
    return false;
}

bool Database::registerRelation(const std::string & name, RelationType kind)
{
    std::string query = "SELECT * FROM ";
    query += name;
    query += " WHERE id = 0 AND ";
    query += name;
    query += " = 0;";
    std::string createquery = "CREATE TABLE ";
    createquery += name;
    if ((kind == ManyToOne) || (kind == OneToOne)) {
        createquery += " (id integer PRIMARY KEY, ";
    } else {
        createquery += " (id integer, ";
    }
    createquery += name;
    createquery += " integer";
    if ((kind == OneToOne) || (kind == OneToMany)) {
        createquery += " UNIQUE);";
    } else {
        createquery += ");";
    }


    debug(std::cout << "QUERY: " << query << std::endl << std::flush;);
    int status = PQsendQuery(m_connection, query.c_str());
    if (!status) {
        log(ERROR, "Database query error.");
        reportError();
        return false;
    }
    if (!tuplesOk()) {
        debug(reportError(););
        debug(std::cout << "Table does not yet exist"
                        << std::endl << std::flush;);
    } else {
        debug(std::cout << "Table exists" << std::endl << std::flush;);
        return true;
    }

    debug(std::cout << "CREATE QUERY: " << createquery
                    << std::endl << std::flush;);
    if (!runCommandQuery(createquery)) {
        return false;
    }
    if ((kind == ManyToOne) || (kind == OneToOne)) {
        return true;
    } else {
        std::string indexQuery = "CREATE INDEX ";
        indexQuery += name;
        indexQuery += "_id_idx ON ";
        indexQuery += name;
        indexQuery += " (id);";
        return runCommandQuery(indexQuery);
    }
}

const DatabaseResult Database::selectRelation(const std::string & name,
                                              const std::string & id)
{
    std::string query = "SELECT ";
    query += name;
    query += " FROM ";
    query += name;
    query += " WHERE id = ";
    query += id;
    query += ";";

    debug(std::cout << "Selecting on id = " << id << " ... " << std::flush;);

    return runSimpleSelectQuery(query);
}

bool Database::createRelationRow(const std::string & name,
                                 const std::string & id,
                                 const std::string & other)
{
    std::string query = "INSERT INTO ";
    query += name;
    query += " (id, ";
    query += name;
    query += ") VALUES (";
    query += id;
    query += ", ";
    query += other;
    query += ");";

    return runCommandQuery(query);
}

bool Database::removeRelationRow(const std::string & name,
                                 const std::string & id)
{
    std::string query = "DELETE FROM ";
    query += name;
    query += " WHERE id=";
    query += id;
    query += ";";

    return runCommandQuery(query);
}

bool Database::removeRelationRowByOther(const std::string & name,
                                        const std::string & other)
{
    std::string query = "DELETE FROM ";
    query += name;
    query += " WHERE ";
    query += name;
    query += "=";
    query += other;
    query += ";";

    return runCommandQuery(query);
}

bool Database::registerSimpleTable(const std::string & name,
                                   const Atlas::Message::Object::MapType & row)
{
    if (row.empty()) {
        log(ERROR, "Attempt to create empty database table");
    }
    // Check whether the table exists
    std::string query = "SELECT * FROM ";
    std::string createquery = "CREATE TABLE ";
    query += name;
    createquery += name;
    query += " WHERE id = 0";
    createquery += " (id integer UNIQUE PRIMARY KEY";
    Atlas::Message::Object::MapType::const_iterator I = row.begin();
    for(; I != row.end(); ++I) {
        query += " AND ";
        createquery += ", ";
        const std::string & column = I->first;
        query += column;
        createquery += column;
        const Atlas::Message::Object & type = I->second;
        if (type.IsString()) {
            query += " LIKE 'foo'";
            int size = type.AsString().size();
            if (size == 0) {
                createquery += " text";
            } else {
                char buf[32];
                snprintf(buf, 32, "%d", size);
                createquery += " varchar(";
                createquery += buf;
                createquery += ")";
            }
        } else if (type.IsInt()) {
            query += " = 1";
            createquery += " integer";
        } else if (type.IsFloat()) {
            query += " = 1.0";
            createquery += " float";
        } else {
            log(ERROR, "Illegal column type in database entity row");
        }
    }
    query += ";";

    debug(std::cout << "QUERY: " << query << std::endl << std::flush;);
    int status = PQsendQuery(m_connection, query.c_str());
    if (!status) {
        log(ERROR, "Database query error.");
        reportError();
        return false;
    }
    if (!tuplesOk()) {
        debug(reportError(););
        debug(std::cout << "Table does not yet exist"
                        << std::endl << std::flush;);
    } else {
        debug(std::cout << "Table exists" << std::endl << std::flush;);
        return true;
    }

    createquery += ");";
    debug(std::cout << "CREATE QUERY: " << createquery
                    << std::endl << std::flush;);
    return runCommandQuery(createquery);
}

const DatabaseResult Database::selectSimpleRow(const std::string & id,
                                               const std::string & name)
{
    std::string query = "SELECT * FROM ";
    query += name;
    query += " WHERE id = ";
    query += id;
    query += ";";

    debug(std::cout << "Selecting on id = " << id << " ... " << std::flush;);

    return runSimpleSelectQuery(query);
}

const DatabaseResult Database::selectSimpleRowBy(const std::string & name,
                                                 const std::string & column,
                                                 const std::string & value)
{
    std::string query = "SELECT * FROM ";
    query += name;
    query += " WHERE ";
    query += column;
    query += " = ";
    query += value;
    query += ";";

    debug(std::cout << "Selecting on " << column << " = " << value
                    << " ... " << std::flush;);

    return runSimpleSelectQuery(query);
}

bool Database::createSimpleRow(const std::string & name,
                               const std::string & id,
                               const std::string & columns,
                               const std::string & values)
{
    std::string query = "INSERT INTO ";
    query += name;
    query += " ( id, ";
    query += columns;
    query += " ) VALUES ( ";
    query += id;
    query += ", ";
    query += values;
    query += ");";

    return runCommandQuery(query);
}

bool Database::registerEntityIdGenerator()
{
    int status = PQsendQuery(m_connection, "SELECT * FROM entity_ent_id_seq;");
    if (!status) {
        log(ERROR, "Database query error.");
        reportError();
        return false;
    }
    if (!tuplesOk()) {
        debug(reportError(););
        debug(std::cout << "Sequence does not yet exist"
                        << std::endl << std::flush;);
    } else {
        debug(std::cout << "Sequence exists" << std::endl << std::flush;);
        return true;
    }
    return runCommandQuery("CREATE SEQUENCE entity_ent_id_seq;");
}

bool Database::getEntityId(std::string & id)
{
    int status = PQsendQuery(m_connection,
                             "SELECT nextval('entity_ent_id_seq');");
    if (!status) {
        log(ERROR, "Database query error.");
        reportError();
        return false;
    }
    PGresult * res;
    if ((res = PQgetResult(m_connection)) == NULL) {
        log(ERROR, "Error getting new ID.");
        reportError();
        return false;
    }
    const char * cid = PQgetvalue(res, 0, 0);
    id = cid;
    PQclear(res);
    while ((res = PQgetResult(m_connection)) != NULL) {
        PQclear(res);
        log(ERROR, "Extra database result to simple query.");
    };
    return true;
}

bool Database::registerEntityTable(const std::string & classname,
                                   const Atlas::Message::Object::MapType & row,
                                   const std::string & parent)
// TODO
// row probably needs to be richer to provide a more detailed, and possibly
// ordered description of each the columns required.
{
    if (entityTables.find(classname) != entityTables.end()) {
        log(ERROR, "Attempt to register entity table already registered.");
        debug(std::cerr << "Table for class " << classname
                        << " already registered." << std::endl << std::flush;);
        return false;
    }
    if (!parent.empty()) {
        if (entityTables.empty()) {
            log(ERROR, "Registering non-root entity table when no root registered.");
            debug(std::cerr << "Table for class " << classname
                            << " cannot be non-root."
                            << std::endl << std::flush;);
            return false;
        }
        if (entityTables.find(parent) == entityTables.end()) {
            log(ERROR, "Registering entity table with non existant parent.");
            debug(std::cerr << "Table for class " << classname
                            << " cannot have non-existant parent " << parent
                            << std::endl << std::flush;);
            return false;
        }
    } else if (!entityTables.empty()) {
        log(ERROR, "Attempt to create root entity class table when one already registered.");
        debug(std::cerr << "Table for class " << classname
                        << " cannot be root." << std::endl << std::flush;);
        return false;
    } else {
        if (!registerEntityIdGenerator()) {
            log(ERROR, "Faled to register Id generator in database.");
        }
    }
    // At this point we know the table request make sense.
    entityTables[classname] = parent;
    const std::string tablename = classname + "_ent";
    // Check whether the table exists
    std::string query = "SELECT * FROM ";
    std::string createquery = "CREATE TABLE ";
    query += tablename;
    createquery += tablename;
    if (!row.empty()) {
        query += " WHERE ";
    }
    createquery += " (";
    if (parent.empty()) {
        createquery += "id integer UNIQUE PRIMARY KEY, ";
    }
    Atlas::Message::Object::MapType::const_iterator I = row.begin();
    for(; I != row.end(); ++I) {
        if (I != row.begin()) {
            query += " AND ";
            createquery += ", ";
        }
        const std::string & column = I->first;
        query += column;
        createquery += column;
        const Atlas::Message::Object & type = I->second;
        if (type.IsString()) {
            query += " LIKE 'foo'";
            int size = type.AsString().size();
            if (size == 0) {
                createquery += " text";
            } else {
                char buf[32];
                snprintf(buf, 32, "%d", size);
                createquery += " varchar(";
                createquery += buf;
                createquery += ")";
            }
        } else if (type.IsInt()) {
            if (type.AsInt() == 0xb001) {
                query += " = 't'";
                createquery += " boolean";
            } else {
                query += " = 1";
                createquery += " integer";
            }
        } else if (type.IsFloat()) {
            query += " = 1.0";
            createquery += " float";
        } else {
            log(ERROR, "Illegal column type in database entity row");
        }
    }
    query += ";";

    debug(std::cout << "QUERY: " << query << std::endl << std::flush;);
    int status = PQsendQuery(m_connection, query.c_str());
    if (!status) {
        log(ERROR, "Database query error.");
        reportError();
        return false;
    }
    if (!tuplesOk()) {
        debug(reportError(););
        debug(std::cout << "Table does not yet exist"
                        << std::endl << std::flush;);
    } else {
        debug(std::cout << "Table exists" << std::endl << std::flush;);
        return true;
    }
    // create table
    createquery += ")";
    if (!parent.empty()) {
        createquery += " INHERITS (";
        createquery += parent;
        createquery += "_ent)";
    }
    createquery += ";";
    debug(std::cout << "CREATE QUERY: " << createquery
                    << std::endl << std::flush;);

    return runCommandQuery(createquery);
}

bool Database::createEntityRow(const std::string & classname,
                               const std::string & id,
                               const std::string & columns,
                               const std::string & values)
{
    TableDict::const_iterator I = entityTables.find(classname);
    if (I == entityTables.end()) {
        log(ERROR, "Attempt to insert into entity table not registered.");
        return false;
    }
    std::string query = "INSERT INTO ";
    query += classname;
    query += "_ent ( id, ";
    query += columns;
    query += " ) VALUES ( ";
    query += id;
    query += ", ";
    query += values;
    query += ");";
    debug(std::cout << "QUERY: " << query << std::endl << std::flush;);

    return runCommandQuery(query);
}

bool Database::updateEntityRow(const std::string & classname,
                               const std::string & id,
                               const std::string & columns)
{
    if (columns.empty()) {
        log(WARNING, "Update query passed to database with no columns.");
        return false;
    }
    TableDict::const_iterator I = entityTables.find(classname);
    if (I == entityTables.end()) {
        log(ERROR, "Attempt to update entity table not registered.");
        return false;
    }
    std::string query = "UPDATE ";
    query += classname;
    query += "_ent SET ";
    query += columns;
    query += " WHERE id='";
    query += id;
    query += "';";
    debug(std::cout << "QUERY: " << query << std::endl << std::flush;);

    return runCommandQuery(query);
}

bool Database::removeEntityRow(const std::string & classname,
                               const std::string & id)
{
    TableDict::const_iterator I = entityTables.find(classname);
    if (I == entityTables.end()) {
        log(ERROR, "Attempt to remove from entity table not registered.");
        return false;
    }
    std::string query = "DELETE FROM ";
    query += classname;
    query += "_ent WHERE id='";
    query += id;
    query += "';";
    debug(std::cout << "QUERY: " << query << std::endl << std::flush;);

    return runCommandQuery(query);
}

const DatabaseResult Database::selectEntityRow(const std::string & id,
                                               const std::string & classname)
{
    std::string table = (classname == "" ? "entity" : classname);
    TableDict::const_iterator I = entityTables.find(classname);
    if (I == entityTables.end()) {
        log(ERROR, "Attempt to select from entity table not registered.");
        return DatabaseResult(0);
    }
    std::string query = "SELECT * FROM ";
    query += table;
    query += "_ent WHERE id='";
    query += id;
    query += "';";

    debug(std::cout << "Selecting on id = " << id << " ... " << std::flush;);

    return runSimpleSelectQuery(query);
}

const DatabaseResult Database::selectOnlyByLoc(const std::string & loc,
                                               const std::string & classname)
{
    TableDict::const_iterator I = entityTables.find(classname);
    if (I == entityTables.end()) {
        log(ERROR, "Attempt to select from entity table not registered.");
        return DatabaseResult(0);
    }

    std::string query = "SELECT * FROM ONLY ";
    query += classname;
    query += "_ent WHERE loc";
    if (loc.empty()) {
        query += " is null;";
    } else {
        query += "=";
        query += loc;
        query += ";";
    }


    return runSimpleSelectQuery(query);
}

const DatabaseResult Database::selectClassByLoc(const std::string & loc)
{
    std::string query = "SELECT id, class FROM entity_ent WHERE loc";
    if (loc.empty()) {
        query += " is null;";
    } else {
        query += "=";
        query += loc;
        query += ";";
    }

    debug(std::cout << "Selecting on loc = " << loc << " ... " << std::flush;);

    return runSimpleSelectQuery(query);
}

const char * DatabaseResult::field(const char * column, int row) const
{
    int col_num = PQfnumber(m_res, column);
    if (col_num == -1) {
        return "";
    }
    return PQgetvalue(m_res, row, col_num);
}

const char * DatabaseResult::const_iterator::column(const char * column) const
{
    int col_num = PQfnumber(m_dr.m_res, column);
    if (col_num == -1) {
        return "";
    }
    return PQgetvalue(m_dr.m_res, m_row, col_num);
}
