#include "board.h"
#include "utils.h"
#include <iostream>


Board::Board(){
    fen = start_fen;
}

void Board::show(){
    char c;
    // Utilize only the fen's board part
    std::string board_fen = fen.substr(0, fen.find_first_of(' ')); 

    for(int i=0; i<board_fen.length(); i++){       
        c = board_fen[i];
        if (std::isalpha(c)){
            std::cout  << " " << c << " ";
        }else if(std::isdigit(c)){   
            for (int j=0; j<int(c) - 48;j++){
                std::cout << " . "; 
            }
        }else if(c=='/'){
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
};

char Board::turn(){
    char player = fen[fen.find_first_of(' ') + 1];
    return player;

}

bool Board::castling_rights(char player, char side){
    // Second and third occurance of space delimit castling rights
    int idx1 = find_nth(fen, ' ', 2);
    int idx2 = find_nth(fen, ' ', 3);

    std::string castling_fen = fen.substr(idx1 +1, idx2 - idx1 - 1); 
    
    for(int i=0; i < castling_fen.length(); i++){
        if (tolower(player) == 'w' && toupper(side) == castling_fen[i]){
            return true;
        }else if(tolower(player) == 'b' && tolower(side) == castling_fen[i]){
            return true;
        }
    }
    return false;
}