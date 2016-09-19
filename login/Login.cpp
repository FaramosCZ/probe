/*------------------------------------------------------------------------------
Projekt: Login screen
Author: Michal Schorm <mschorm@centrum.cz> <mschorm@redhat.com> <faramos.cz@gmail.com>
Date: 5/2016, 9/2016                                                

File content: class Login

Feature request: additional parameter to manage input file with encrypted credential
Feature request: additional parameter to encrypt input file with chosen method (one-way ciphers recomended)
-------------------------------------------------------------------------------*/
#include <iostream>    // cin, cout
#include <cstddef>     // work with NULL
#include <string>      // strings...
#include <fstream>     // work with files
#include <unistd.h>    // usleep(); pause();
#include <sys/ioctl.h> // ioctl()
#include <signal.h>    // signals

using namespace std;



//!  Class Login - basic "Login - Password" dialog box in CLI 
/*!
  restrict access to anything behind this program
  provide very simple file with logins and passwords combinations.
  Warning: This program will mask (block) many signals, in order to deny all other communication with shell than to this program. Shortcuts like "CTRL+C or CTRL+Z" won't work.
*/

#ifndef LOGIN_CLASS
#define LOGIN_CLASS

class Login
{
 private:
   //! Path to file, that contains pairs: Login - Password
   /*!
     File MUST contain:
     1) only plain text
     2) One combination cosists of three rows - Login, Password and a blank row
     3) nothing less, nothing more. File will be checked before use. Only characters from first half of ASCII >20 are acceptable.
   */
   string accounts_file;

   //! ASCII graphic login variables, contains number of rows & cols
   struct winsize screen, memory;
   
   int box_width;								// width of the box (in cols)
   int box_height;								// height of the box (in rows)
   int width_of_predefined_text;						// either "Login:" or "Password: "
   int line_text;								// row, from the beginning of box rows, where text starts (count starts from 0)
   int space_before_box, space_after_box, space_above_box, space_under_box;	// space to center the box
   string * buffer = NULL;							// buffer containing everything what will displayed
   char znak;									// helping variable for getting the credentials
   string input_login;								// buffer for login
   string input_password;							// buffer for password
   string input_password_hidden;						// buffer for displayed password (set of "***")
   
   string line_1 = "+--------------------------------+ ";			// Template for the box
   string line_2 = "|                                | ";			// |
   string line_3 = "| Login:                         | ";			// |
   string line_4 = "| Password:                      | ";			// |
   string line_5 = "|                                | ";			// |
   string line_6 = "+--------------------------------+ ";			// |__  

   //! Check if file with accounts has a valid format
   bool Check_file_integrity_accounts()
     {
      ifstream fd( (this->accounts_file).c_str() );
      if( fd.good() )	// if file exist
        {
         int counter = 0;
         string unused;
         while( getline(fd, unused) ) ++counter; 
           if( counter%3 == 0 ) return true;  
        }
      return false;   
     }

   //! ASCII graphic layout computing
   void Resize(bool force)
     {
      sigset_t waitset;		// for passive wait while resize exception
      int sig;			// for passive wait while resize exception

      ioctl(STDOUT_FILENO, TIOCGWINSZ, &(this->memory) );    										// get new value of screen ROWS/COLS to the structure "memory".
      if( force==0 && this->screen.ws_row == this->memory.ws_row && this->screen.ws_col == this->memory.ws_col ){return;}		// screen was not resized actually, no need to re-calculate AND it was not force resize
      screen = memory;      														// get new value of screen ROWS/COLS to the structure "screen".

      this->box_width = this->line_1.length();		// length of one line
      this->box_height = 6;				// this box consists of 6 rows
      this->width_of_predefined_text = 12;		// ??
      this->line_text = 2;				// ??

      while(this->box_width > this->screen.ws_col+1 || this->box_height > this->screen.ws_row  )	// if the box is bigger than the window, hide it and wait for resize
        {
         system("clear");

	 sigemptyset( &waitset );		// passive wait while resize exception
	 sigaddset( &waitset, SIGWINCH );	// waiting for RESIZE signal
	 sigwait( &waitset, &sig );		// 

         ioctl(STDOUT_FILENO, TIOCGWINSZ, &(this->screen) );	// update size of the window
        }	

      this->space_before_box = this->space_after_box = (this->screen.ws_col-this->box_width)/2;		// center horizontally
      this->space_after_box += (this->screen.ws_col-this->box_width)%2;					// |__
      this->space_under_box = this->space_above_box = (this->screen.ws_row-this->box_height)/2;		// center vertically ?? "-3"
      this->space_above_box += (this->screen.ws_row-this->box_height)%2;				// |__
      
      if(this->buffer != NULL) this->buffer->clear();
      this->buffer = new string(this->screen.ws_col*(this->screen.ws_row-1), ' ');  			// fill buffer with spaces
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+0)+this->space_before_box), this->line_1.length(), this->line_1);  // and draw the box                                                                     
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+1)+this->space_before_box), this->line_2.length(), this->line_2);
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+2)+this->space_before_box), this->line_3.length(), this->line_3);
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+3)+this->space_before_box), this->line_4.length(), this->line_4);
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+4)+this->space_before_box), this->line_5.length(), this->line_5);
      this->buffer->replace( (this->screen.ws_col*(this->space_above_box+5)+this->space_before_box), this->line_6.length(), this->line_6);        

      system("clear");system("clear");			// 2x clear to completely hide rest of the box in case of scrolling 
      cout << *(this->buffer) << endl;
     }

   //! Signal blocking
   void Signal()
     {
      struct sigaction act;
      act.sa_handler = this->static_myHandler;
      sigemptyset(&act.sa_mask);
      act.sa_flags = 0;
      sigaction(SIGINT, &act, 0);  	// CTRL+C
      sigaction(SIGTERM, &act, 0); 	// CTRL+Z
      sigaction(SIGQUIT, &act, 0);
      sigaction(SIGHUP, &act, 0);
      sigaction(SIGTSTP, &act, 0);
      sigaction(SIGWINCH, &act, 0); 	// resize window
     }

   //! Signal handling - no action
   static void static_myHandler(int signum)
    {
     if(signum){}
     return;
    }



 public:
   //! Constructor
   Login(string accounts_file)
     {
      this->accounts_file = accounts_file; 
      if( ! this->Check_file_integrity_accounts() ) { fprintf(stderr, "\nFile containing accounts informations has not valid form\n"); exit(2); }

      input_login = "";
      input_password = "";
      input_password_hidden = "";
      
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &(this->screen) );    	// get screen ROWS/COLS to the structure "screen".
      this->Resize(1);						// force initial Resize()

      this->Signal();						// block all keyboard signals - give user no chance to escape from login screen to the system
     }

   //! Check if given combination of Login and Password exists
   bool Check_credentials(string login, string password)
     {
      ifstream fd(this->accounts_file.c_str());
      if( !fd.good() ){ cout << "\nNelze otevřít soubor\n"; return false; }
      
      string buffer;
      
      while( getline(fd, buffer) )
        {
         getline(fd, buffer);
         if(buffer == login) { getline(fd, buffer); if(buffer == password) return true; }
         else getline(fd, buffer);
        } 
      return false;
     }

   //! testing login - password shown !
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
  
   //! Login with centered dialog box in CLI
   void Login_ascii_graphic()
     {
      while( 1 )
        {         
          while( 1 ) // Get input for "Login: " field
            {         
             this->Resize(1);
             // Move "cursor" position and rewrite line with "Login: "
	     printf("\n\033[%d;%dH %s", this->space_above_box+this->line_text, this->space_before_box+this->width_of_predefined_text, this->input_login.c_str());
             fflush(stdout); // without fflush data will remain in stdout buffer
             
             if( this->input_login.length()==20 ) break; // check length overflow
             
             system("stty raw"); 	// This will change some input behaviour, so you don't need to press ENTER after every getchar
             this->znak = getchar();
             system("stty cooked"); 	// and now take it back to normal
      
             if( this->znak>=20 && this->znak<=126) { this->input_login += this->znak; }	// Add character only if it is some basic printable char
             else if(this->znak==13 || this->znak==10) break;                             	// check for ENTER
            }
          
          if( this->input_login.length()>=20 ){this->input_login.clear(); this->Resize(1); continue;}  
          
          while( 1 ) // Get input for "Password: " field
            {         
             this->Resize(0);
	     // Now move "cursor" position and rewrite lines with "Login: " and "Password: "
             printf("\n\033[%d;%dH %s  ", this->space_above_box+this->line_text, this->space_before_box+this->width_of_predefined_text, this->input_login.c_str());		// two spaces at the end will overwrite "^M" char for newline
             printf("\n\033[%d;%dH %s", this->space_above_box+this->line_text+1, this->space_before_box+this->width_of_predefined_text, this->input_password_hidden.c_str());
             fflush(stdout); // without fflush data will remain in stdout buffer
      
             if( this->input_password.length()==20 ) break; // check length overflow
      
             system("stty raw"); 	// This will change some input behaviour, so you don't need to press ENTER after every getchar
             this->znak = getchar(); 
             system("stty cooked"); 	// and now take it back to normal
             
             if( this->znak>=20 && this->znak<=126) { this->input_password += this->znak; this->input_password_hidden += '*';} // Add character only if it is some basic printable char and print aditional "*" to "Password: " field   
             else if(this->znak==13 || this->znak==10) break;	// check for ENTER                        
            }

         // overwrite displayed "Enter" character behind password with 2 spaces behind string
         printf("\n\033[%d;%dH %s  ", this->space_above_box+this->line_text+1, this->space_before_box+this->width_of_predefined_text, this->input_password_hidden.c_str());
         fflush(stdout); // without fflush data Will remain include stdout buffer
         
         if( this->input_password.length()>=20 ){this->input_login.clear(); this->input_password.clear(); this->input_password_hidden.clear(); continue;} 
         if( this->Check_credentials(this->input_login, this->input_password) ) break;		// If credentials are correct, quit login loop
         
	 // Empty buffers               
         this->input_login.clear();
         this->input_password.clear();         
         this->input_password_hidden.clear();
         
	 // Print some info about situation
         printf("\n\033[%d;%dH %s", this->space_above_box+this->line_text+6, this->space_before_box+8, "ACCESS DENIED !!!\n");
         usleep(1000*1000*2);			// defense against brute force attack (And for user to read status message)

        }
      system("clear");				// clear screen after successful login     
     }     

};
#endif
