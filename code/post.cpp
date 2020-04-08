#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <unistd.h>
#include <sys/ioctl.h>
#include <fstream>

using namespace std;

string post_cgi(string);

int main(void){
    int unread = 0;
    char buf[1000];
    
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
    return_string = post_cgi(string(buf));

    // return data
    printf("%s",return_string.c_str());
}


string post_cgi(string message){
    // 尋找連續兩個\r\n後面的資料
    string data = message.substr(message.find("\r\n\r\n")+4,message.find("\r\n\r\n")+4 - message.length());

    // 尋找data=的位子
    int data_pos = data.find("data=");
    if(data_pos == -1){
        return "No data";
    }

    // 取得data裡面的值
    string post_data = data.substr(data_pos+5,data_pos+5-data.length());

    // 讀檔案
    fstream file("file.txt");
    string content((istreambuf_iterator<char>(file)),
                    (istreambuf_iterator<char>()));

    // 寫入檔案
    file << post_data << '\n';
    file.close();

    return post_data;
}
