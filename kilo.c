#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>



struct termios orig_termios;

void disableRawMode(void){
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&orig_termios);





    }



void enableRawmode(void){
    
    tcgetattr(STDIN_FILENO,&orig_termios);
    atexit(disableRawMode);
    struct termios raw=orig_termios;


    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN  );

    tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw);
                

    }

int main(void){
    enableRawmode();




    char c; //we create a variable c
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
        if(iscntrl(c)){
            printf("%d\n",c);
        


            }else{
                printf("%d('%c'\n)",c,c);



                }

        
        

                    
        };
        
        return 0;


    }
