#include <iostream>
#include <nlohmann/json.hpp>
#include <uwebsockets/App.h>
#include <string>
#include <map>
#include "chatbot.h"

using namespace std;
using json = nlohmann::json;

const string COMMAND = "command";
const string PRIVATE_MSG = "private_msg";
const string SET_NAME = "set_name";
const string USER_ID = "user_id";
const string MESSAGE = "message";
const string USER_FROM = "user_from";
const string NAME = "user_name";
const string ONLINE = "online";
const string STATUS = "status";
const string BROADCAST = "broadcast";

struct PerSocketData {
    int user_id;
    string name = "NULL";
};

map<int, PerSocketData*> activeUsers;

typedef uWS::WebSocket<false,true, PerSocketData> UWEBSOCK;

string status (PerSocketData *data, bool b) {
  json request;
  request[COMMAND] = STATUS;
  request[NAME] = data->name;
  request[USER_ID] = data->user_id;
  request[ONLINE] = b;
  return request.dump();
}

void processMessage(UWEBSOCK *ws, string_view &message, map<int, PerSocketData*> &users, uWS::OpCode opCode) {
  json parsed = json::parse(message);
  PerSocketData *data = ws->getUserData();
  json response;
  string command = parsed[COMMAND];
  if (command == PRIVATE_MSG) {
    int user_id = parsed[USER_ID];
    string user_msg = parsed[MESSAGE];
    if (user_id == 1) {
      response[COMMAND] = PRIVATE_MSG;
      response[USER_FROM] = user_id;
      response[MESSAGE] = chat_bot(user_msg);
      ws->send(response.dump(), opCode, true);
    } else if (users.find(user_id) != users.end()) {
      response[COMMAND] = PRIVATE_MSG;
      response[USER_FROM] = data->user_id;
      response[MESSAGE] = user_msg;
      ws->publish("userN" + to_string(user_id), response.dump());
    } else {
      cout << "Error! There is no user with ID = " << user_id << "!" << endl;
      response[COMMAND] = PRIVATE_MSG;
      response[USER_FROM] = data->user_id;
      response[MESSAGE] = "User not found!";
      ws->send(response.dump(), opCode, true);
    }
  }
  if (command == SET_NAME) {
    string user_name = parsed[NAME];
    int pos = (int) user_name.find("::");
    if (pos == -1 && user_name.length() <= 255) {
      data->name = user_name;
      cout << "User № " << data->user_id << " set his name to " << data->name << endl;
      response[COMMAND] = PRIVATE_MSG;
      response[USER_FROM] = data->user_id;
      response[MESSAGE] = "Your name was changed to " + data->name;
      ws->send(response.dump(), opCode, true);
      ws->publish(BROADCAST, status(data, false));
    } else {
      cout << "This name is not allowed!" << endl;
      response[COMMAND] = PRIVATE_MSG;
      response[USER_FROM] = data->user_id;
      response[MESSAGE] = "Incorrect name!";
      ws->send(response.dump(), opCode, true);
    }
  }
}

int main() {
  int latest_id = 1;
  train();
  uWS::App().ws<PerSocketData>("/*", {
    .idleTimeout = 9996,
    .open = [&latest_id](auto *ws) {
      PerSocketData *data = ws->getUserData();
      data->user_id = ++latest_id;
      cout << "User № " << data->user_id << " entered the chat." << endl;
      ws->publish(BROADCAST, status(data, true));
      ws->subscribe("broadcast");
      ws->subscribe("userN" + to_string(data->user_id));
      for (auto entry: activeUsers) ws->send(status(entry.second, true), uWS::OpCode::TEXT);
      activeUsers[data->user_id] = data;
      cout << "Total users connected: " << activeUsers.size() << endl;
    },
    .message = [](auto *ws, string_view message, uWS::OpCode opCode) {
      PerSocketData *data = ws->getUserData();
      cout << "Message from ";
      if (data->name == "NULL") cout << "№ " << data->user_id;
      else cout << data->name;
      cout << ": " << message << endl;
      processMessage(ws, message, activeUsers, opCode);
    },
    .close = [](auto *ws, int code, std::string_view message) {
        PerSocketData *data = ws->getUserData();
        cout << "User № " << data->user_id << " exited the chat." << endl;
        ws->publish(BROADCAST, status(data, false));
        activeUsers.erase(data->user_id);
    }
  }).listen(9001, [](auto *listen_socket) {
      if (listen_socket) {
        cout << "Listening on port " << 9001 << ": " << endl;
        cout << "Chat-bot entered the chat! (User № 1)" << endl;
        greeting();
      }
  }).run();
  return 0;
}
