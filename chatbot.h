#pragma once
#include <iostream>
#include <regex>
#include <algorithm>
#include <map>
#include <string>
#include <fstream>

using namespace std;

string toLower (string text) {
  transform(text.begin(), text.end(), text.begin(), ::tolower);
  return text;
}

int loadPhrases(map<string, string> &database) {
  ifstream phrases("../database.txt");
  string line;
  string delimiter = " $ ";
  int len = 0;
  if (phrases.is_open()) {
    while (getline(phrases, line)) {
      int pos = (int) line.find(delimiter);
      string question = line.substr(0, pos);
      string answer = line.substr(pos + delimiter.length());
      database.insert(make_pair(question, answer));
      len++;
    }
    phrases.close();
  } else cout << "Error! Cannot access database." << endl;
  return len;
}

void savePhrases() {
  ofstream phrases("../database.txt", ios::app);
  if (phrases.is_open()) {
    cout << "Starting learning mode..." << endl;
    string line;
    while (line != "stop") {
      cout << "Enter question and answer: ";
      string delimiter = " - ";
      getline(cin, line);
      if (line == "stop") break;
      int pos = (int) line.find(delimiter);
      string question = line.substr(0, pos);
      string answer = line.substr(pos + delimiter.length());
      question = toLower(question);
      if (question[pos - 1] == '?') question = question.substr(0, pos - 1);
      delimiter = " $ ";
      phrases << question << delimiter << answer << endl;
    }
    cout << "Saving queries..." << endl;
    phrases.close();
  } else cout << "Error! Cannot access database." << endl;
}

void bot (const string& msg) {
  cout << "Bot: " << msg << endl;
}

string chat_bot (const string& question) {
  map<string, string> database;
  string query = toLower(question);
  string answer;
  int phraseCount = loadPhrases(database);
  bool found = false;
  for (const auto &entry: database) {
    regex pattern(".*" + entry.first + ".*");
    if (regex_match(query, pattern)) {
      found = true;
      answer = entry.second;
    }
  }
  if (!found || phraseCount == 0) answer = "Sorry, I can't answer that.";
  bot(answer);
  return answer;
}

void greeting() {
  map<string, string> database;
  int phraseCount = loadPhrases(database);
  bot("Hello, I'm a chat-bot! I can answer " + to_string(phraseCount) + " questions.");
}

void train() {
  string command;
  cout << "Do you want to train the bot first?" << endl;
  cout << "Yes/No: ";
  cin >> command;
  cin.ignore();
  command = toLower(command);
  if (command == "yes") savePhrases();
}
