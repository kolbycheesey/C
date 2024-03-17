#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <mutex>
#include <thread>
#include <condition_variable>

std::mutex mtx;
std::condition_variable waitWriting;

bool WRITING = false;
const int THREAD = 3;

/*  Assumption is being made that the input file will only ever be input.txt
    and output will only ever be output.txt not allowing the user to input
    at this time.

    Compiled using: g++ -Wall -Werror -std=c++17 main.cpp
*/
void operations(std::ifstream& input, std::ofstream& output){

    std::string line[THREAD];
    int readCnt = 0;
    int writeCnt = 0;
    std::unique_lock<std::mutex> lock(mtx);
    while(!input.eof()){
        std::getline(input, line[readCnt]);
        std::reverse(line[readCnt].begin(), line[readCnt].end());
        readCnt = (readCnt + 1) % THREAD;

        while(WRITING)
            waitWriting.wait(lock);
        WRITING = true;

        output << line[writeCnt++] << "\n";
        writeCnt = writeCnt % THREAD;
        WRITING = false;
    }

}

int main()
{
    std::ifstream   file;
                    file.open("input.txt");
    std::ofstream   out;
                    out.open("output.txt");
    std::thread     threads[THREAD];

    for(int i = 0; i < THREAD; i++){
        threads[i] = std::thread(operations, ref(file), ref(out));
    }

    for(int i = 0; i < THREAD; i++){
        threads[i].join();
    }

    if(file.is_open())
        file.close();
    if(out.is_open())
        out.close();
}
