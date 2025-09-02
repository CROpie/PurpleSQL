#include <iostream>
#include <string>
#include "minisocket.hpp"
#include "dbconn.hpp"
#include "json.hpp"

using json = nlohmann::json;

minisocket::Server server;
dbConn::Connector dbConnector("127.0.0.1", "12345");
int db_fd;

void onMessage(int client_fd, const std::string& sql) {
    if (sql.empty()) return;

    json response = dbConnector.queryDB(db_fd, sql.c_str());

    server.sendFrame(client_fd, response.dump(2));
}

int main() {

    // make TCP connection to DB
    db_fd = dbConnector.connectToDb();

    // make websocket connection to frontend
    server.init("9047", &onMessage, true);
    std::cout << "Server started on " << "9047" << std::endl;

    server.run();

    dbConnector.closeConnection(db_fd);

    return 0;

}

/*
    "CREATE TABLE testTable (id INT, isDeleted BOOL, message VARCHAR(255));";
    "INSERT INTO testTable (id, isDeleted, message) VALUES (1, false, 'first message'), (2, true, 'second message');"
    "SELECT * FROM testTable;";
*/