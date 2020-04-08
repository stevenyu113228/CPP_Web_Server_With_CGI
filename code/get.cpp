#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <unistd.h>
#include <sys/ioctl.h>
#include <fstream>

using namespace std;

string get_text_cgi();

int main(void){
    int unread = 0;
    char buf[10];
    
    // wait for stdin
    while(unread<1){
        if(ioctl(STDIN_FILENO,FIONREAD,&unread)){
            perror("ioctl");
            exit(EXIT_FAILURE);
        }
    }

    // read from stdin fd
    read(STDIN_FILENO,buf,unread);

    // prepare string for response
    string return_string;
    return_string = get_text_cgi();
    
    printf("%s",return_string.c_str());
}

string get_text_cgi(){
    // read file
    ifstream ifs("file.txt");
    string content((istreambuf_iterator<char>(ifs)),
                    (istreambuf_iterator<char>()));
    return content;
}