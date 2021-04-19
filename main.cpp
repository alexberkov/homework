#include <iostream>
#include <fstream>

using namespace std;

int main() {
    string word, keyword;
    cout << "Enter keyword: ";
    cin >> keyword;
    int co = 0;
    ifstream words;
    words.open("/Users/alexberkov/CLionProjects/22-1/words.txt");
    while (!words.eof()) {
        words >> word;
        if (word == keyword) co++;
    }
    cout << "Number of matches: " << co;
    words.close();
}
