#pragma once
#include <nlohmann/json.hpp>
#include <uwebsockets/App.h>
#include "chatbot.h"
#include "literals.h"

using namespace std;
using json = nlohmann::json;


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
      response[NAME] = BOT;
      response[MESSAGE] = chat_bot(user_msg);
      ws->send(response.dump(), opCode, true);
    } else if (users.find(user_id) != users.end()) {
      response[COMMAND] = PRIVATE_MSG;
      response[USER_FROM] = data->user_id;
      if (data->name != "NULL") response[NAME] = data->name;
      response[MESSAGE] = user_msg;
      ws->publish("userN" + to_string(user_id), response.dump());
    } else {
      cout << "Error! There is no user with ID = " << user_id << "!" << endl;
      response[COMMAND] = NOTIFICATION;
      response[USER_FROM] = 0;
      response[NAME] = SERVER;
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
      response[COMMAND] = NOTIFICATION;
      response[USER_FROM] = 0;
      response[NAME] = SERVER;
      response[MESSAGE] = "Your name was changed to " + data->name;
      ws->send(response.dump(), opCode, true);
      ws->publish(BROADCAST, status(data, false));
    } else {
      cout << "This name is not allowed!" << endl;
      response[COMMAND] = NOTIFICATION;
      response[USER_FROM] = 0;
      response[NAME] = SERVER;
      response[MESSAGE] = "Incorrect name!";
      ws->send(response.dump(), opCode, true);
    }
  }
  if (command == TRAIN) {
    string line = parsed[QUERY];
    if (savePhrase(line)) {
      response[COMMAND] = PRIVATE_MSG;
      response[USER_FROM] = 1;
      response[NAME] = BOT;
      response[MESSAGE] = "Thanks for teaching me!";
    } else {
      response[COMMAND] = NOTIFICATION;
      response[USER_FROM] = 0;
      response[NAME] = SERVER;
      response[MESSAGE] = "Error! Cannot access bot's database.";
    }
    ws->send(response.dump(), opCode, true);
  }
}
