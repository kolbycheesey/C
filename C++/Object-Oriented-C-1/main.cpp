// #include <iostream>


// Fields
// class GameCharacter {
//   public:
//     std::string name;
//     int health;
//     int damage;
// } character1;

// int main() {  
//   character1.name = "Nimish";
//   character1.health = 100;
//   character1.damage = 10;

//   GameCharacter character2;
//   character2.name = "Zenva";
//   character2.health = 150;
//   character2.damage = 20;

//   std::cout << character1.name << std::endl;
// }


// Constructors Parts 1 and 2
// #include <iostream>

// class GameCharacter {
//   public:
//     std::string name;
//     int maxHealth;
//     int currentHealth;
//     int damage;

//     GameCharacter(std::string, int, int, int);
//     GameCharacter(std::string, int, int);
// };

// GameCharacter::GameCharacter(std::string _name, int _maxHealth, int _currentHealth, int _damage) {
//   name = _name;
//   maxHealth = _maxHealth;
//   currentHealth = _currentHealth;
//   damage = _damage;
// }

// // GameCharacter::GameCharacter(std::string _name, int _health, int _damage) {
// //   name = _name;
// //   maxHealth = _health;
// //   currentHealth = _health;
// //   damage = _damage;
// // }

// GameCharacter::GameCharacter(std::string _name, int _health, int _damage): name(_name), maxHealth(_health), damage(_damage) {
//   currentHealth = 0;
// }

// int main() {  
//   GameCharacter character1 = GameCharacter("Nimish", 100, 100, 10);

//   GameCharacter character2 = GameCharacter("Zenva", 150, 20);

//   // std::cout << character2.maxHealth << std::endl;

//   // GameCharacter c ("Zenva", 150, 20);
//   // GameCharacter c = "Zenva";
//   // GameCharacter c {"Zenva", 150, 20};
//   // GameCharacter c = {"Zenva", 150, 20};
// }


// Methods
// class GameCharacter {
//   public:
//     std::string name;
//     int maxHealth;
//     int currentHealth;
//     int damage;
//     bool isAlive;

//     GameCharacter(std::string, int, int, int);
//     void takeDamage(int);
//     void heal(int);
// };

// GameCharacter::GameCharacter(std::string _name, int _maxHealth, int _currentHealth, int _damage) {
//   name = _name;
//   maxHealth = _maxHealth;
//   currentHealth = _currentHealth;
//   damage = _damage;
//   isAlive = true;
// }

// void GameCharacter::takeDamage(int amount) {
//   currentHealth -= amount;
//   if (currentHealth <= 0) {
//     currentHealth = 0;
//     isAlive = false;
//   }
// }

// void GameCharacter::heal(int amount) {
//   currentHealth += amount;
//   if (currentHealth > maxHealth) {
//     currentHealth = maxHealth;
//   }
// }

// int main() {  
//   GameCharacter character1 = GameCharacter("Nimish", 100, 100, 10);
//   character1.takeDamage(20);
//   character1.heal(20);

//   std::cout << character1.currentHealth << std::endl;
//   std::cout << character1.isAlive << std::endl;
// }


// Static Members
// class GameCharacter {
//   public:
//     static std::string type;

//     std::string name;

//     GameCharacter(std::string);
//     void changeName(std::string);

//     static void changeType(std::string);
// };

// GameCharacter::GameCharacter(std::string _name) {
//   name = _name;
// }

// std::string GameCharacter::type = "Non player character";

// void GameCharacter::changeName(std::string newName) {
//   name = newName;
// }

// void GameCharacter::changeType(std::string newType) {
//   type = newType;
// }

// int main() {  
//   GameCharacter character1 = GameCharacter("Nimish"); 
//   GameCharacter character2 = GameCharacter("Zenva"); 

//   GameCharacter::changeType("New type");

//   std::cout << character1.type << std::endl;
//   std::cout << character2.type << std::endl;

//   // character1.type = "afsdf";

//   // std::cout << character1.type << std::endl;
//   // std::cout << character2.type << std::endl;

//   // std::cout << character1.type << std::endl;
//   // std::cout << GameCharacter::type << std::endl;
// }


// Class Pointers
// void attack(GameCharacter *attacker, GameCharacter * defender) {
//   int damage = attacker->damage;
//   defender->takeDamage(damage);
// }

// int main() {  
//   GameCharacter character1 = GameCharacter("Nimish", 100, 100, 10);

//   GameCharacter * characterPtr = &character1;
//   characterPtr->damage = 15;
//   characterPtr->takeDamage(20);
//   // std::cout << character1.currentHealth << std::endl;
//   // std::cout << character1.damage << std::endl;

//   GameCharacter character2 = GameCharacter("Zenva", 150, 150, 20);
//   attack(&character2, &character1);
//   std::cout << character1.currentHealth << std::endl;
// }


// Inheritance Parts 1 and 2
// class GameCharacter {
//   public:
//     static std::string type;

//     std::string name;
//     int currentHealth;
//     int damage;
//     bool isAlive;

//     GameCharacter(std::string, int, int);
//     void takeDamage(int);

//     static void printType();
// };

// GameCharacter::GameCharacter(std::string _name, int _currentHealth, int _damage) {
//   name = _name;
//   currentHealth = _currentHealth;
//   damage = _damage;
//   isAlive = true;
// }

// std::string GameCharacter::type = "Non player character";

// void GameCharacter::takeDamage(int amount) {
//   currentHealth -= amount;
//   if (currentHealth <= 0) {
//     currentHealth = 0;
//     isAlive = false;
//   }
// }

// void GameCharacter::printType() {
//   std::cout << "Game character has the type of " << type << std::endl;
// }

// class Player: public GameCharacter {
//   public:
//     static std::string type;

//     int maxHealth;

//     Player(std::string, int, int, int);

//     void heal(int);
// };

// Player::Player(std::string _name, int _currentHealth, int _maxHealth, int _damage): GameCharacter(_name, _currentHealth, _damage) {
//   maxHealth = _maxHealth;
// }

// std::string Player::type = "Player character";

// void Player::heal(int amount) {
//   currentHealth += amount;
//   if (currentHealth > maxHealth) {
//     currentHealth = maxHealth;
//   }
// }

// int main() {  
//   GameCharacter npc = GameCharacter("Non player character", 100, 10);
//   npc.takeDamage(10);
//   // npc.heal(10);
//   // std::cout << npc.currentHealth << std::endl;
  

//   Player player = Player("Player", 120, 150, 20);
//   player.takeDamage(10);
//   player.heal(20);
//   // std::cout << player.currentHealth << std::endl;

//   // player.type = "Player character";
//   // std::cout << player.type << std::endl;
//   // std::cout << npc.type << std::endl;
//   player.printType();
// }


// Polymorphism
// class Player: public GameCharacter {
//   public:
//     static std::string type;

//     int maxHealth;
//     int armour;

//     Player(std::string, int, int, int);

//     void heal(int);
//     void takeDamage(int);
// };

// Player::Player(std::string _name, int _currentHealth, int _maxHealth, int _damage): GameCharacter(_name, _currentHealth, _damage) {
//   maxHealth = _maxHealth;
//   armour = 5;
// }

// std::string Player::type = "Player character";

// void Player::heal(int amount) {
//   currentHealth += amount;
//   if (currentHealth > maxHealth) {
//     currentHealth = maxHealth;
//   }
// }

// void Player::takeDamage(int amount) {
//   if (amount - armour < 0) {
//     return;
//   } 
//   currentHealth -= (amount - armour);
//   if (currentHealth <= 0) {
//     currentHealth = 0;
//     isAlive = false;
//   }
// }

// // void attack(GameCharacter *c1, GameCharacter *c2) {
// //   c1->armour
// // }

// int main() {  
//   GameCharacter npc = GameCharacter("Non player character", 100, 10);
//   npc.takeDamage(10);
//   // std::cout << npc.currentHealth << std::endl;
  
//   Player player = Player("Player", 100, 100, 20);
//   player.takeDamage(10);
//   // std::cout << player.currentHealth << std::endl;

//   GameCharacter player2 = Player("Player 2", 100, 100, 10);
//   player2.takeDamage(10);
//   // std::cout << player2.currentHealth << std::endl;
//   // Player npc2 = GameCharacter("", 1, 2);

//   GameCharacter *gcPtr = &player;
//   // gcPtr->armour;
// }


// Access Control
// class GameCharacter {
//   protected:
//     int currentHealth;
//     int maxHealth;
//   public:
//     GameCharacter(int);
//     int getHealth();
//     void takeDamage(int);
// };

// GameCharacter::GameCharacter(int _currentHealth) {
//   currentHealth = _currentHealth;
//   maxHealth = _currentHealth;
// }

// int GameCharacter::getHealth() {
//   return currentHealth;
// }

// void GameCharacter::takeDamage(int amount) {
//   currentHealth -= amount;
// }

// class Player: public GameCharacter {
//   public:
//     Player(int);

//     void heal(int);
// };

// Player::Player(int _currentHealth): GameCharacter(_currentHealth) { }

// void Player::heal(int amount) {
//   currentHealth += amount;
//   if (currentHealth > maxHealth) {
//     currentHealth = maxHealth;
//   }
// }

// int main() {  
//   GameCharacter npc = GameCharacter(100);
//   npc.takeDamage(50);
//   std::cout << npc.getHealth() << std::endl;

//   Player player = Player(100);
//   // player.currentHealth = 200;
//   // std::cout << player.currentHealth << std::endl;
// }