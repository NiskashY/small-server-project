#include <iostream>
#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// structure bound to each connection
struct UserData {
    int user_id;
    std::string name;
};

#pragma region globalVariables
const char *const kCommand = "command";
const char *const  kPrivateMsg = "PRIVATE_MSG";
const char *const  kPublicMsg = "PUBLIC_MSG";
const char *const  kUserIdTo = "user_id_to";
const char *const  kUserIdFrom = "user_id_from";
const char *const kText = "text";
#pragma endregion

void ShowId(const int& id) {
    std::cout << "{ ID " << id << " }";
}

void ShowLogUserMsgTransfer(const auto* user_data, const int& user_id) {
    std::cout << "User "; ShowId(user_data->user_id);
    std::cout << " sends message to User "; ShowId(user_id);
    std::cout << '\n';
}

void ProcessMessage(auto* ws, const auto& message) {
    json data = json::parse(message);
    UserData* user_data = ws->getUserData();

    if (data[kCommand] == kPrivateMsg) {
        int user_id = data[kUserIdTo];
        std::string text = data[kText];

        json response;
        response[kCommand] = kPrivateMsg;
        response[kUserIdFrom] = user_data->user_id;
        response[kText] = text;
        ws->publish("user" + std::to_string(user_id), response.dump());

        ShowLogUserMsgTransfer(user_data, user_id);
    } else if (data[kCommand] == kPublicMsg) {
        int user_id = data[kUserIdTo];
        std::string text = data[kText];

        json response;
        response[kCommand] = kPublicMsg;
        response[kUserIdFrom] = user_data->user_id;
        response[kText] = text;
        ws->publish("user" + std::to_string(user_id), response.dump());

        ShowLogUserMsgTransfer(user_data, user_id);
    }

}

int main() {
    int latest_user_id = 10;

    uWS::App().get("/hello", [](auto *res, auto *req) {
        /* You can efficiently stream huge files too */
        res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("Hello HTTP!");
    }).ws<UserData>("/*", {
            /* Just a few of the available handlers */
            .open = [&latest_user_id](auto *ws) {
                // connect to the server
                UserData* data = ws->getUserData();
                data->user_id = latest_user_id++;
                std::cout << "user connected: { ID = " << data->user_id  << " }" << '\n';
                ws->subscribe("user" + std::to_string(data->user_id));
            },
            .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
                // send JSON-data to the server
                ws->send(message, opCode);
                ProcessMessage(ws, message);
            },
            .close = [](auto *ws, int code, std::string_view message) {
                // close connection
            }
    }).listen(9001, [](auto *listenSocket) {
        if (listenSocket) {
            std::cout << "URL http://localhost:" << 9001 << "/hello" << '\n';
            std::cout << "URL http://localhost:" << 9001 << '\n';
        }
    }).run();
    return 0;
}


