#include <iostream>
#include "./Login.cpp"

using namespace std;



int main()
{
 string accounts_filename("./accounts.txt"); // define path to file with all accounts informations
 Login login(accounts_filename);             // create instance of class Login
 login.Login_ascii_graphic();                // call method Login_ascii_graphic(); When it ends, it guaranteed that right credentials were used.
   
   
 return 0;
}

