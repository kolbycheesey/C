#include <iostream>
using namespace std;

bool GAMEOVER;
const int width = 20;
const int height = 20;
int x,y, fruitX, fruitY, score;


void Setup(){
    GAMEOVER = false;
}

void Draw(){

}

void Input(){

}

void Logic(){

}

int main()
{
    Setup();
    while(!GAMEOVER){
        Draw();
        Input();
        Logic();
        //Sleep(10);
    }

    return 0;
}