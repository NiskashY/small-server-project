#include <iostream>
#include <map>
#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*
ws = new WebSocket("ws://localhost:9001/");
ws.onmessage= ({data}) => console.log(data);

ws.send(JSON.stringify({"command": "PRIVATE_MSG", "user_id_to":11, "text":"Приветик братан"} ))
 */

// structure bound to each connection
struct UserData {
    int user_id;
    std::string name;
};

#pragma region globalVariables
const char *const kCommand = "command";
const char *const  kPrivateMsg = "private_msg";
const char *const  kPublicMsg = "public_msg";
const char *const  kStatus = "status";  // {command: STATUS, online:1/0, user_id: ..., name: ...}
const char *const  kSetName = "set_name";
const char *const kPublicChannel = "public_channel";
const char *const  kUserId = "user_id";
const char *const  kUserIdTo = "user_id_to";
const char *const  kUserIdFrom = "user_id_from";
const char *const kText = "text";
const char *const kName = "name";
const char *const kOnline = "online";

#pragma endregion

std::string GetStatus(UserData* data, const bool& online);

std::string GetStrId(const int& id);

void ShowLogUserMsgTransfer(const auto* user_data, const std::string& message);

void ProcessMessage(auto* ws, const auto& message);

int main() {
    int latest_user_id = 1;

    std::map<int, UserData*> all_users;

    uWS::App().get("/hello", [](auto *res, auto *req) {
        /* You can efficiently stream huge files too */
        res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("Hello HTTP!");
    }).ws<UserData>("/*", {
            /* Just a few of the available handlers */
            .open = [&latest_user_id, &all_users](auto *ws) {
                // connect to the server
                UserData* data = ws->getUserData();
                data->user_id = latest_user_id++;
                data->name = "unnamed";
                std::cout << "user connected: { ID = " << data->user_id  << " }" << '\n';
                ws->subscribe("user" + std::to_string(data->user_id));
                ws->publish(kPublicChannel, GetStatus(data, true));
                ws->subscribe(kPublicChannel);
                for (const auto& [key, value] : all_users) {
                    // uWS::OpCode::TEXT - change binary to txt
                    ws->send(GetStatus(value, true), uWS::OpCode::TEXT);
                }
                all_users[data->user_id] = data;
            },
            .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
                // send JSON-data to the server
                ws->send(message, opCode);
                ProcessMessage(ws, message);
            },
            .close = [&all_users](auto *ws, int code, std::string_view message) {
                // close connection
                UserData* data = ws->getUserData();
                ws->publish(kPublicChannel, GetStatus(data, false));
                ShowLogUserMsgTransfer(data, "disconnected");
                all_users.erase(data->user_id);
            }
    }).listen(9001, [](auto *listenSocket) {
        if (listenSocket) {
            std::cout << "URL http://localhost:" << 9001 << "/hello" << '\n';
            std::cout << "URL http://localhost:" << 9001 << '\n';
        }
    }).run();
    return 0;
}


std::string GetStatus(UserData *data, const bool &online) {
    json response;
    response[kCommand] = kStatus;
    response[kUserId] = data->user_id;
    response[kName] = data->name;
    response[kOnline] = online;
    return response.dump();
}

std::string GetStrId(const int &id) {
    return "{ ID " + std::to_string(id) + " }";
}

void ShowLogUserMsgTransfer(const auto *user_data, const std::string &message) {
    std::cout << "User " << GetStrId(user_data->user_id);
    std::cout << " sends message" << ' ' << message;
    std::cout << '\n';
}

void ProcessMessage(auto *ws, const auto &message) {
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
        ShowLogUserMsgTransfer(user_data, "to " + GetStrId(user_id));
    } else if (data[kCommand] == kPublicMsg) {
        // From-> {command: PUBLIC_MSG, test::HELLO}
        // To->   {command: PUBLIC_MSG, test::HELLO}
        json response;
        response[kCommand] = kPublicMsg;
        response[kUserIdFrom] = user_data->user_id;
        response[kText] = data[kText];
        response[kUserIdFrom] = user_data->user_id;
        ws->publish(kPublicChannel, response.dump());
        ShowLogUserMsgTransfer(user_data, "");
    } else if (data[kCommand] == kSetName) {
        std::string name = data[kName];
        user_data->name = name;
        ws->publish(kPublicChannel, GetStatus(user_data, true));
        ShowLogUserMsgTransfer(user_data, "set their name");
    }
}
