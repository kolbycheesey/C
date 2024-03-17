// C++ Hello World
#include <iostream>
int main() {
    std::cout << "Hello\nWorld\n";
}

// Variables
/*int main() {
    //type name = value;
    //type name;
    //name = value;
}*/

// Booleans
// int main() {
//   bool isOn = true;
//   bool isGameOver = false;

//   std::cout << isOn << std::endl;
//   std::cout << isGameOver << std::endl;

//   isGameOver = true;

//   std::cout << isGameOver << std::endl;
// }

// Integers
// int main() {
//   // int age = 28;

//   int health;
//   std::cout << health << std::endl;
//   health = 50;
//   std::cout << health << std::endl;
//   health = -50;
//   std::cout << health << std::endl;

//   uint age = 28;
//   std::cout << age << std::endl;
//   // uint nAge = -28;
//   // std::cout << nAge << std::endl;

//   long long longInt = 50;
//   std::cout << longInt << std::endl;
// }

// Doubles
// int main() {
//   double price = 19.99;

//   double balance;
//   std::cout << balance << std::endl;
//   balance = 100;
//   std::cout << balance << std::endl;
//   balance = -5.95;
//   std::cout << balance << std::endl;
// }

// Characters
// int main() {
//   char comma = ',';
//   std::cout << comma << std::endl;
//   char a = 'a';
//   std::cout << a << std::endl;
  
//   char one = '1';
//   std::cout << one << std::endl;
//   int numberOne = 1;
//   std::cout << numberOne << std::endl;
// }

// Strings
// int main() {
//   std::string name = "Zenva";

//   std::string mixedString = "1n. ";
//   std::cout << mixedString << std::endl;

//   int numberOne = 1;
//   char one = '1';
//   std::string stringOne = "1";
  
// }

// Constants
// int main() {
//   const std::string email = "email@email.com";

//   // email = "fasdfsf";
//   std::cout << email << std::endl;
// }

// Input via cin
// int main() {
//   // std::cout << "";
//   // std::string input1;
//   // std::string input2;
//   // std::cin >> input1 >> input2;
//   // std::cout << input1 << std::endl;
//   // std::cout << input2 << std::endl;

//   // int input;
//   // std::cin >> input;
//   // std::cout << input;

//   // 1. Prompt user for name
//   // 2. Take in and store user name
//   // 3. Print some welcome message
//   std::cout << "Enter your name:" << std::endl;
//   std::string name;
//   std::cin >> name;
//   std::cout << "Hello, " << name << ". Welcome to my program." << std::endl;
// }

// Arithmetic Operators
// int main() {
//   // + - * / %

//   int age = 28;
//   // int newAge = age + 1;
//   double incrementer = 1.0;
//   // age = age + 1;
//   age = age + incrementer;

//   // std::cout << age << std::endl;
//   // std::cout << newAge << std::endl;

//   int modulusAge = age % 5;
//   // std::cout << modulusAge << std::endl;

//   modulusAge = (age + 1) % 5;
//   // std::cout << modulusAge << std::endl;

//   std::string name = "Nimish";
//   std::cout << name + " Narang" << std::endl;
// }

// Assignment Operator
// int main() {
//   // =

//   int age = 28;
//   int newAge = 29;
//   newAge = age;
//   age = 29;

//   age = age + 1;
//   age += 1;

//   std::string name = "Nimish";
//   name += " Narang";
// }

// Increment and Decrement Operators
// int main() {
//   // ++ --

//   int age = 28;
//   // age = age + 1;
//   // age += 1;
//   // age++;
//   // std::cout << age << std::endl;
//   // age--;
//   // std::cout << age << std::endl;

//   int newAge1 = age++;
//   age = 28;
//   int newAge2 = ++age;
//   // std::cout << newAge1 << std::endl;
//   // std::cout << newAge2 << std::endl;

//   // 1. What is the end result?
//   int a = 0;
//   int result = a - 5 * 2 + 20 / 2;

//   // 2. Can we change the equation to get a positive value?

//   // 3. What is the end result?
//   int b = 10;
//   b += 2 * 7;

//   // 4. I want to spell my full name. What is wrong with this equation?
//   std::string firstName = "Nimish";
//   std::string lastName = "Narang";
//   std::string fullName = firstName + lastName;

//   // 5. What are the values of c and d?
//   int c = 1;
//   int d = c--;
// }

// Comparison Operators
// int main() {
//   // == != > >= < <=

//   int age = 28;
//   int minAge = 18;

//   bool isAdult = age >= minAge;
//   // std::cout << isAdult << std::endl;

//   std::string a = "a";
//   std::string b = "b";
//   bool results = a < b;
//   // std::cout << results << std::endl;

//   std::cout << (isAdult == true) << std::endl;
//   std::cout << isAdult << std::endl;
// }

// Logical Operators
// int main() {
//   // ! && ||

//   bool isGameOver = false;
//   bool isNotGameOver = !isGameOver;

//   // std::cout << isGameOver << std::endl;
//   // std::cout << isNotGameOver << std::endl;

//   int health = 10;
//   int lives = 0;
//   isGameOver = health <= 0 && lives <= 0;
//   // std::cout << isGameOver << std::endl;

//   bool result;  
//   // 1. What is the result?
//   int a = 1;
//   result = a - 1 != 0;

//   // 2. What is the result?
//   std::string y = "y";
//   std::string x = "x";
//   result = !(x > y);

//   // 3. What is the result?
//   bool hasKey = true;
//   bool isDoorLocked = true;
//   int movementPoints = 1;
//   result = (hasKey || !isDoorLocked) && movementPoints > 0;
//   std::cout << result << std::endl;
// }

// Ternary Operator
// int main() {
//   // ? :

//   bool isBirthday = false;
//   int age = isBirthday ? 29 : 28;

//   std::cout << age << std::endl;
// }

// If Statements
// int main() {
//   char trafficLight = 'g';
//   int speed = 30;

//   if (trafficLight == 'g') {
//     std::cout << "Go!\n";
//   } else if (trafficLight == 'r') {
//     std::cout << "Stop!\n";
//   } else if (trafficLight == 'y') {
//     if (speed >= 30) {
//       std::cout << "Speed up!\n";
//     } else {
//       std::cout << "Slow down!\n";
//     }
//   } else {
//     std::cout << "Unknown state\n";
//   }
// }

// Switch Statements
// int main() {
//   char trafficLight = 'b';
//   int speed = 30;

//   switch (trafficLight) {
//     case 'g':
//       std::cout << "Go!\n";
//       break;
//     case 'r':
//       std::cout << "Stop!\n";
//       break;
//     case 'y':
//       if (speed >= 30) {
//         std::cout << "Speed up!\n";
//       } else {
//         std::cout << "Slow down!\n";
//       }
//       break;
//     default:
//       std::cout << "Unknown state\n";
//   }
// }

// While Loops
// int main() {
//   int pos = 0;
//   int endPos = 5;
//   bool isGameOver = false;

//   // while (pos < endPos) {
//   //   pos++;
//   //   std::cout << "Current position is " << pos << std::endl;
//   // }

//   while (!isGameOver) {
//     pos++;
//     std::cout << "Current position is " << pos << std::endl;
    // if (pos >= endPos) {
    //   isGameOver = true;
    // }
//   }
//   std::cout << "You have reached the end!" << std::endl;

//   // do {
//   //   code
//   // } while (); 
// }

// For Loops
// int main() {

//   // for (int i = 0; i < 5; i++) {
//   //   std::cout << "Hello!\n";
//   // }

//   int pos = 0;
//   int endPos = 5;
//   for (pos = 1; pos <= endPos; pos++) {
//     std::cout << "Current position: " << pos << std::endl;
//   }
//   std::cout << "You have reached the end!" << std::endl;
// }

// Control Statements
// int main() {
//   int pos = 0;
//   int endPos = 10;
//   int itemPos = 2;
//   int enemyPos = 6;

//   while (pos < endPos) {
//     pos++;

//     if (pos == itemPos) {
//       std::cout << "Item found!" << std::endl;
//       continue;
//     } else if (pos == enemyPos) {
//       std::cout << "Enemy collision!" << std::endl;
//       break;
//     }
//     std::cout << "Empty space!" << std::endl;
//   }
// }

// Functions
// void printName() {
//   std::cout << "Nimish" << std::endl;
// }

// int main() {
//   printName();
// }


// Function Parameters
// void printName(std::string name) {
//   std::cout << name << std::endl;
// }

// void printFullName(std::string first, std::string last) {
//   std::string fullName = first + " " + last;
//   std::cout << "Hello " << fullName << "!\n";
// }

// int main() {  
//   // printName("Nimish");
//   // printName("Zenva");
//   // printName();
//   // std::string first = "Nimish";
//   printFullName("Nimish", "Narang");
// }


// Return Statements
// std::string getFullName(std::string firstName, std::string lastName) {
//   std::string fullName = firstName + " " + lastName;
//   return fullName;
// }

// int main() {
//   std::string fullName = getFullName("Nimish", "Narang");
//   std::cout << fullName << std::endl;

//   // std::string getFullName(std::string, std::string);
// }

// String Functions
// int main() {
//   std::string name = "Nimish";
  
//   // std::cout << name.size() << std::endl;
//   // std::cout << name.empty() << std::endl;
//   // std::cout << name.substr(0, 3) << std::endl;

//   // name.append(" Narang");
//   // std::cout << name << std::endl;

//   name = "Nimesh";
//   name.insert(3, "i");
//   name.erase(4, 1);
//   std::cout << name << std::endl;
// }