/*------------------------------------------------------------------------------
Projekt: Login screen
Author: Michal Schorm
Date: 5/2016                                                

File content: class Login
-------------------------------------------------------------------------------*/
#include <iostream>   // cin, cout
#include <cstddef>    // work with NULL
#include <string>     // strings...
#include <fstream>    // work with files
#include <unistd.h>   // usleep();
#include <sys/ioctl.h> // ioctl()
#include <signal.h>    // signals

using namespace std;



//!  Class Login - for logging in any time
/*!


 
*/
#ifndef LOGIN_CLASS
#define LOGIN_CLASS
class Login
{
 private:
   //! Contains pairs: Login - Password
   /*!
   File MUST contain:
   1) only plain text
   2) One combination cosists of three rows - Login, Password and a blank row
   3) nothing less, nothing more. File will be checked before use. Only characters from first half of ASCII >20 are acceptable.
   */
   
   //! Name of file containing logins and passwords
   string accounts_file;

   //! Graphic login variables
   struct winsize screen, memory;
   
   int box_width;
   int box_height;             
   int width_of_predefined_text;
   int line_text;
   int space_before_box, space_after_box, space_above_box, space_under_box;
   string * buffer = NULL;
   char znak;
   string input_login;
   string input_password;
   string input_password_hidden;
   
   string line_1 = "+--------------------------------+ ";     
   string line_2 = "|                                | ";
   string line_3 = "| Login:                         | ";
   string line_4 = "| Password:                      | ";
   string line_5 = "|                                | ";
   string line_6 = "+--------------------------------+ ";  
   
   //! Check if file with accounts is valid
   bool Check_file_integrity_accounts()
     {
      ifstream fd( (this->accounts_file).c_str() );
      if( fd.good() )
        {
         int counter = 0;
         string unused;
         while( getline(fd, unused) ) ++counter; 
         if( counter%3 == 0 ) return true;  
        }
      return false;   
     }

   //! Graphic layout computing
   void Resize()
     {
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &(this->memory) );    // get screen ROWS/COLS to the structure "memory".
      if( this->screen.ws_row == this->memory.ws_row && this->screen.ws_col == this->memory.ws_col ){return;}
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &(this->screen) );    // get screen ROWS/COLS to the structure "screen".

      this->box_width = this->line_1.length();
      this->box_height = 6;
      this->width_of_predefined_text = 12;
      this->line_text = 2;

      this->space_before_box = this->space_after_box = (this->screen.ws_col-this->box_width)/2;
      this->space_after_box += (this->screen.ws_col-this->box_width)%2;
      this->space_under_box = this->space_above_box = (this->screen.ws_row-this->box_height-3)/2;
      this->space_above_box += (this->screen.ws_row-this->box_height)%2;
      
      if(this->buffer != NULL)this->buffer->clear();
      this->buffer = new string(this->screen.ws_col*(this->screen.ws_row-1), ' ');  // place spaces inside
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+0)+this->space_before_box), this->line_1.length(), this->line_1);  // and draw the box                                                                     
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+1)+this->space_before_box), this->line_2.length(), this->line_2);
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+2)+this->space_before_box), this->line_3.length(), this->line_3);
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+3)+this->space_before_box), this->line_4.length(), this->line_4);
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+4)+this->space_before_box), this->line_5.length(), this->line_5);
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+5)+this->space_before_box), this->line_6.length(), this->line_6);        

      system("clear");
      system("clear");
      cout << *(this->buffer) << endl;
     }

   //! Signal blocking
   void Signal()
     {
      struct sigaction act;
      act.sa_handler = this->static_myHandler;
      sigemptyset(&act.sa_mask);
      act.sa_flags = 0;
      sigaction(SIGINT, &act, 0);  // CTRL+C
      sigaction(SIGTERM, &act, 0); // CTRL+Z
      sigaction(SIGQUIT, &act, 0);
      sigaction(SIGHUP, &act, 0);
      sigaction(SIGTSTP, &act, 0);
      sigaction(SIGWINCH, &act, 0); // resize window
     }

   //! Signal handling - no action
   static void static_myHandler(int signum)
    {
     if(signum){}
    }



 public:
   //! Constructor
   Login(string accounts_file)
     {
      this->accounts_file = accounts_file; 
      if( ! this->Check_file_integrity_accounts() ) { fprintf(stderr, "\nFile containing accounts informations is not valid\n"); exit(2); }

      input_login = "";
      input_password = "";
      input_password_hidden = "";
      
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &(this->screen) );    // get screen ROWS/COLS to the structure "screen".
      this->screen.ws_row = -1;
      this->screen.ws_col = -1;
      this->Resize();
      
      this->Signal();
     }

   //! Check if given combination of Login and Password exists
   bool Check_credentials(string login, string password)
     {
      ifstream fd(this->accounts_file.c_str());
      if( !fd.good() ){ cout << "\nNelze otevřít soubor\n"; return false; }
      
      string buffer;
      bool match;
      
      while( getline(fd, buffer) )
        {
         match=false;
         
         getline(fd, buffer);
         if(buffer == login) match=true;
         
         getline(fd, buffer);
         if(buffer == password) match=true;
         else match=false;
         
         if( match == true) return true;
        } 
      return false;
     }

   //! login 1
   void Login_classic()
     {
      string login;
      string password;
      bool match = false;

      do{
         system("clear");
         system("clear");
         
         cout << "Login: ";
         getline(cin, login);
         cout << "Password: ";
         getline(cin, password);
         
         match = this->Check_credentials(login, password); 

         if(match!=true)
           {
            cout << "\n\nINVALID LOGIN OR PASSWORD\n";
            usleep(1000*1000*2);
            continue;
           }
         else break; 
        } while (1);
     }
  
   //! login 2
   void Login_ascii_graphic()
     {
      while( 1 )
        {         
          while( 1 )
            {         
             this->Resize();
             printf("\n\033[%d;%dH %s", this->space_above_box+this->line_text, this->space_before_box+this->width_of_predefined_text, this->input_login.c_str());
             fflush(stdout); // without fflush data Will remain include stdout buffer
             
             if( this->input_login.length()==20 ) break;
             
             system("stty raw"); 
             this->znak = getchar(); 
             system("stty cooked"); 
      
             if( this->znak>=20 && this->znak<=126) { this->input_login += this->znak; }
             else if(this->znak==13 || this->znak==10) break;                             
            }
          
          if( this->input_login.length()==20 ){this->input_login.clear();this->screen.ws_row = -1;this->screen.ws_col = -1;continue;}  
          
          while( 1 )
            {         
             this->Resize();
             printf("\n\033[%d;%dH %s  ", this->space_above_box+this->line_text, this->space_before_box+this->width_of_predefined_text, this->input_login.c_str());
             printf("\n\033[%d;%dH %s", this->space_above_box+this->line_text+1, this->space_before_box+this->width_of_predefined_text, this->input_password_hidden.c_str());
             fflush(stdout); // without fflush data Will remain include stdout buffer
      
             if( this->input_password.length()==20 ) break;
      
             system("stty raw"); 
             this->znak = getchar(); 
             system("stty cooked"); 
             
             if(this->znak==13 || this->znak==10) break;
             else if( this->znak>=20) { this->input_password += this->znak; this->input_password_hidden += '*';}                            
            }
         
         if( this->input_password.length()==20 ){this->input_login.clear();this->input_password.clear();this->input_password_hidden.clear();this->screen.ws_row = -1;this->screen.ws_col = -1;continue;} 
         if( this->Check_credentials(this->input_login, this->input_password) ) break;
         
         // overwrite displayed "Enter" character behind password with 2 spaces behind string
         printf("\n\033[%d;%dH %s  ", this->space_above_box+this->line_text+1, this->space_before_box+this->width_of_predefined_text, this->input_password_hidden.c_str());
         fflush(stdout); // without fflush data Will remain include stdout buffer
         
         this->screen.ws_row = -1;
         this->screen.ws_col = -1;
               
         this->input_login.clear();
         this->input_password.clear();         
         this->input_password_hidden.clear();
         
         printf("\n\033[%d;%dH %s", this->space_above_box+this->line_text+6, this->space_before_box+8, "ACCESS DENIED !!!\n");
         usleep(1000*1000*2);
            
        }
      system("clear");     
     }     

};
#endif
