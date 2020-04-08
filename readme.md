# CPP_Web_Server_With_CGI
>> 網路安全 Homework1
>> B10507004 四電機四甲 游照臨

## 建置環境
- OS: macOS Catalina 10.15.4
- Darwin Kernel Version 19.4.0
- Compiler: Apple clang version 11.0.3 (clang-1103.0.32.29)


## 使用說明
- 在終端機輸入 `./Host`，即可開始運行
![](https://i.imgur.com/KzcMF5a.png)


## 重要程式碼說明
(細節說明皆於程式碼註解中)
### 主程式
#### initial_socket
```C++
void initial_socket(){
    sock = socket(AF_INET,SOCK_STREAM,0); // Create socket
    /*
        AF_INET = ipv4 tcp or udp
        SOCK_STREAM = tcp
        type = 預設都０
    */


    if(sock == -1){
        cout<<"Socket create error!"<<endl;
    }


    bzero(&Server_addr,sizeof(Server_addr)); // 先清0這ㄍ struct
    Server_addr.sin_family = AF_INET; // ipv4
    Server_addr.sin_port = htons(Server_port); // hton: Host To Network Short integer (小印地安轉大印地安)
    Server_addr.sin_addr.s_addr = 0; // Any ip 



    bind(sock, (struct sockaddr *)&Server_addr, sizeof(Server_addr));
    /*
        sock = sock status code
        (struct sockaddr *) &Server_addr = 把 Server_addr 轉成 sockaddr ㄉ指標
        socklen = sizeof(Server_addr)
    */


    int list;
    list = listen(sock, 100);
    /*
        sock
        幾個人能連入server
    */

    if(list == -1){
        cout<<"Listen error!"<<endl;
    }


    bzero(&Client_addr,sizeof(Client_addr)); // 先清0這ㄍ struct
    Client_addr_len = sizeof(Client_addr); // 定義Client address的Length

    int alive = 1;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char *) &alive,sizeof alive);
    /*
        sock：將要被設定或者獲取選項的套接字。
        level：選項所在的協議層。
        optname：需要訪問的選項名。
        optval：對於getsockopt()，指向返回選項值的緩衝。對於setsockopt()，指向包含新選項值的緩衝。
        optlen：對於getsockopt()，作為入口引數時，選項值的最大長度。作為出口引數時，選項值的實際長度。對於setsockopt()，現選項的長度。
    */

    cout<<"Server start at http://0.0.0.0:"<<Server_port<<endl;
    cout<<"Get CGI http://0.0.0.0:"<<Server_port<<"/get.cgi"<<endl;
    cout<<"Post CGI http://0.0.0.0:"<<Server_port<<"/post.cgi"<<endl;
    cout<< endl <<"Press Ctrl+C to terminate the server"<<endl;

}
```
- 透過socket 建立 ipv4的 TCP Socket Server
- 設定Server的Address、Port、可接受的ip等

- 使用bind 功能把Socket給綁定
- 最後開啟Socket

#### create_process
```C++
string create_process(string path,string data,string cgi_name){
      string return_data;
      int cgiInput[2];
      int cgiOutput[2];
      int status;
      char inputData[1000];
      sprintf(inputData,"%s",data.c_str());
      pid_t cpid;
      char c;
      /* Use pipe to create a data channel betweeen two process
         'cgiInput'  handle  data from 'host' to 'CGI'
         'cgiOutput' handle data from 'CGI' to 'host'*/
      if(pipe(cgiInput)<0){
            perror("pipe");
            exit(EXIT_FAILURE);
      }
      if(pipe(cgiOutput)<0){
            perror("pipe");
            exit(EXIT_FAILURE);
      }


      /* Creates a new process to execute cgi program */
      cpid = fork();
      if(cpid < 0){
            perror("fork");
            exit(EXIT_FAILURE);
      }
      

      /*child process*/
      if(cpid == 0){
            //close unused fd
            close(cgiOutput[0]);
            close(cgiInput[1]);


            //redirect the output from stdout to cgiOutput
            //把stdout的寫入 導到cgiOutput
            dup2(cgiOutput[1],STDOUT_FILENO);

            //redirect the input from stdin to cgiInput
            //把stdin的讀取 導到cgiInput
            dup2(cgiInput[0], STDIN_FILENO); 
            
            //after redirect we don't need the old fd 
            close(cgiOutput[1]);
            close(cgiInput[0]);

            /* execute cgi program
               the stdout of CGI program is redirect to cgiOutput
               the stdin  of CGI program is redirect to cgiInput
            */

            // 執行CGI
            execlp((string("./") + cgi_name).c_str(),(string("./") + cgi_name).c_str(),NULL);
            exit(0);
      }

    else{        
        //關閉舊的fd
        close(cgiOutput[1]);
        close(cgiInput[0]); 

        // 把資料Write到cgi
        write(cgiInput[1], inputData, strlen(inputData));

        // 把cgi吐回來資料塞回string
        while (read(cgiOutput[0], &c, 1) > 0){
                return_data += c;
        }

        close(cgiOutput[0]);
        close(cgiInput[1]);
        waitpid(cpid, &status, 0);
    }
    return return_data;
}
```
- 將資料與需要呼叫的cgi的讀入
- 透過fork的方式開啟Child process執行cgi程式
- 並透過pipe重導向stdout將資料回傳給Parent Process
- 運行完後將結果回傳string

#### response200
```C++
void response200(int acc,string message){
    // 回應HTTP Request 200 的Header 加上 message

    string response = "HTTP/1.1 200 OK\nContent-Length: ";
        response.append(to_string(message.length()));
        response.append("\nContent-Type: text/html\nConnection: close\n\n");
        response.append(message);

        send(acc,response.c_str(),response.length(),0);
}
```
- 使用標準的HTTP格式將資料串接
- 並且透過send的方式透過socket送出

### CGI
#### get cgi
```C++
nt main(void){
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
```
- 透過ioctl等待stdin(由Parent process發出的資料)
- 透過C++ 的ifstream將資料轉為String並且輸出

#### post cgi
```C++
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
```
- 解析http輸入的資料，尋找data的值
- 並且將data的值寫入file.txt

## 設計架構與功能說明
### 系統架構圖
![](https://i.imgur.com/G9UKTma.jpg)

### 功能說明
#### GET method
- get.cgi
    - 讀取系統檔案路徑中的file.txt檔案，並直接回傳http200與文字檔案資料
- 非get.cgi
    - 回傳http200 `meow_meow ?__?`
    
#### POST method
- post.cgi
    - 使用`x-www-form-urlencoded`方式接收資料，資料為data欄位
    - 將data欄位的內容寫入系統檔案路徑中的file.txt
    - 回傳http200 file.txt的內容
- 非post.cgi
    - 回傳http200 `meow_meow ?__?`
    
## 成果截圖
![](https://i.imgur.com/KzcMF5a.png)

### 使用POSTMAN測試
- POST
![](https://i.imgur.com/Darn3a9.png)

- GET
![](https://i.imgur.com/QPd6hOD.png)

### 使用Firefox測試

- post.cgi (使用HackBar)
![](https://i.imgur.com/GOD40y4.png)

- 非get.cgi
![](https://i.imgur.com/ukn1gAZ.png)

- get.cgi
![](https://i.imgur.com/smg3CFU.png)
![](https://i.imgur.com/du8mNf7.png)

## 困難與心得
在這次的作業中，我學會了透過C++搭配Socket自製一個Websocket。在實作的過程中，我覺得最困難的部分並不是網路連線的部分，而是對於cgi呼叫所需要使用到的fork指令。

在這個作業之前，我從來沒有使用過關於fork、pipe、修改stdio等指令，對於這些指令十分陌生，感謝助教在Github上提供了範例，讓我學會了藉由subprocess呼叫execlp來執行其他binary，並且與之傳遞資料的方法。

其次是對於socket實作server也有了基礎的認識，包含了需要使用的各種function，例如accept、recv、listen、send......。C相較於Python還是屬於比較低階的語言，寫久了Python也開始對C越來越陌生，但C的執行效率在現在還是不可被取代的！謝謝老師與助教安排這次作業，讓我對於C、對於sub process、對於網路，都有了更深的認識！