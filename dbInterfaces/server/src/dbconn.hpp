#pragma once

#include <string>
#include <iostream>
#include <netdb.h> 
#include <sys/socket.h>
#include <unistd.h>

#include "json.hpp"
using json = nlohmann::json;

/*
    Basic connection package for C++ to my DB
    Make connection to DB
    Send SQL string
    Print the response
    Close connection

    Future enhancements:
        Connection Pool (keep pool of open connections, rather than open->close a single one)
        Persistent DB connection
*/

namespace dbConn {

class Connector {

    public:
        // Constructor
        Connector(const char* address, const char* port)
            : address(address), port(port) {}

        int connectToDb() {

            struct addrinfo hints{}, *res;
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;

            // input address::port of database
            if (getaddrinfo(address, port, &hints, &res) != 0) {
                throw std::runtime_error("getaddrinfo error");
            };
            
            // create a socket to identify this instance of the incoming connection
            int db_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

            int yes = 1;
            setsockopt(db_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

            // 'actual' handshake - if succeed, have a live TCP connection
            if (connect(db_fd, res->ai_addr, res->ai_addrlen) != 0) {
                throw std::runtime_error("connection error");
            }
            return db_fd;
        }

        json queryDB(int db_fd, const std::string& sql) {

            json response;

            // send needs a pointer to raw bytes (const void*) and a length (size_t)
            // sql.data() is pointer to the sql char* array. size() is chars in the string
            int bytes = send(db_fd, sql.data(), sql.size(), 0);
            std::cout << "bytes sent: " << bytes << std::endl;

            char buffer[1024];
            int bytes_read = recv(db_fd, buffer, sizeof(buffer) - 1, 0);

            if (bytes_read == 0) {
                response["error"] = "no bytes with recv";
                return response;
            }

            buffer[bytes_read] = '\0';

            std::cout << "return value: " << buffer << std::endl;

            return json::parse(buffer);
        }

        void closeConnection(int db_fd) {
            close(db_fd);
        }

        json connectQueryClose(const std::string& sql) {
            int db_fd = connectToDb();
            json response = queryDB(db_fd, sql);
            closeConnection(db_fd);
            return response;
        }

    private:
        const char* address;
        const char* port;

};
}