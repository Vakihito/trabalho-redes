#include <iostream>
#include <stdio.h>
#include <string.h>
#include "chat.h"

using namespace std;

void print_name(string name, string color) {
    if (color.compare("red") == 0)
        cout << "\033[1;31m" << "[" << name << "]: " << "\033[0m";
    else if (color.compare("green") == 0)
        cout << "\033[1;32m" << "[" << name << "]: " << "\033[0m";
    else if (color.compare("yellow") == 0)
        cout << "\033[1;33m" << "[" << name << "]: " << "\033[0m";
    else if (color.compare("blue") == 0)
        cout << "\033[1;34m" << "[" << name << "]: " << "\033[0m";
    else if (color.compare("white") == 0)
        cout << "\033[1;314m" << "[" << name << "]: " << "\033[0m";
    else
        cout << "[" << name << "]: ";

    return;
}

void print_text(string text, string color, bool new_line) {
    if (color.compare("red") == 0) cout << "\033[1;31m" << text << "\033[0m";
    else if (color.compare("green") == 0) cout << "\033[1;32m" << text << "\033[0m";
    else if (color.compare("yellow") == 0) cout << "\033[1;33m" << text << "\033[0m";
    else if (color.compare("blue") == 0) cout << "\033[1;34m" << text << "\033[0m";
    else if (color.compare("white") == 0) cout << "\033[1;314m" << text << "\033[0m";
    else cout << text;

    if (new_line) cout << endl;

    return;
}

void concat_charA_to_str(string *str, char *char_array, unsigned int n) {
    for(unsigned int i = 0; i < n; i++) 
        (*str).push_back(char_array[i]);

    return;
}

void stoca(char *char_array, string str, unsigned int n) {
    for (unsigned int i = 0; i < n && i < str.length(); i++)
        char_array[i] = str[i];
    return;
}

char *str_to_charA(string str, int n) {
    char *char_array = new char[n];
    for (unsigned int i = 0; i < n and i < str.length(); i++)
        char_array[i] = str[i];
    char_array[n < str.length() ? n : str.length()] = '\0';
    return char_array;
}