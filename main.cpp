#include "process.h"

using namespace std;

int main() {
  int latest_id = 1;
  train();
  PerSocketData server = {0, SERVER};
  PerSocketData bot = {1, BOT};
  activeUsers.insert(make_pair(0, &server));
  activeUsers.insert(make_pair(1, &bot));
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
