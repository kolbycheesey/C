#include <iostream>
#include <Windows.h>

using namespace std;

void loadingBar()
{
    system("cls");
    system("color 9F");

    char a = 177, b = 219;

    cout << "\n\n\n\n\n\n\t\t\t\t\tStudent Data System";
    cout << "\n\n\n\t\t\t\t\tLoading...\n";
    cout << endl;
    cout << "\t\t\t\t\t";

    for(int i = 0; i < 26; i++)

    cout << a;

    cout << "\t\t\t\t\t";

    for(int i = 0; i < 26; i++)
    {
        cout << b;
        Sleep(200);
    }
}