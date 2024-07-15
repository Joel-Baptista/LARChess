#include "utils.h"

bool isupper(char c) {
    return c >= 'A' && c <= 'Z';
}

int find_nth(std::string s, char c, int n){
    int count = 0;
    for (int i=0; i<s.length(); i++){
        if(s[i] == c){
            count++;
        }
        if (count>=n){
            return i;
        }
    }
    return -1;
}