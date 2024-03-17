#include <iostream>
#include <regex>

//Prog2
/*int addNum(int first, int second)
{
    int sum = first + second;
    return sum;
}*/

//Prog1
int main(int argc, char **argv)
{
    /*int firstNumber = 12;
    int secondNumber = 9;

    int sum = firstNumber + secondNumber;

    std::cout << "The sum of the two numbers is : " << sum << std::endl;

    sum = addNum(firstNumber, secondNumber);
    std::cout << "The sum of the two numbers is : " << sum << std::endl;

    sum = addNum(34,7);
    std::cout << "The sum of the two numbers is : " << sum << std::endl;

    std::cout << "The sum of the two numbers 55 and 16 is : " << addNum(55,16) << std::endl;*/

    //prog3.exe
    //THE STRING IN WHICH THE SUBSTRING TO BE FOUND.
    std::string testString = "Find Something In This Test String";
    //std::vector<std::string> test = {"In","This"};

    //THE SUBSTRING TO BE FOUND.
    auto pattern{ "In This (Test|Example)" };

    //std::regex_constants::icase - TO IGNORE CASE.
    auto rx = std::regex{ pattern,std::regex_constants::icase };
    //auto rx = std::regex{ test[0],std::regex_constants::icase };

    //SEARCH THE STRING.
    bool isStrExists = std::regex_search(testString, rx);

    std::cout << "exists : " << isStrExists << std::endl;

    return 0;
}