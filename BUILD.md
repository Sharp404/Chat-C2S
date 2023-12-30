# Chat C2S in C 
> by Sharp - 1st Year of Master's in Computer Security

## Requirements :

* Linux Environment
* C Environment with gcc compiler

## How to build the program ?

1. Open the .tar.gz archive in CLI :
    ```tar -xzvf chat_c2s.tar.gz```

2. Go on the directory where the program is via CLI :
    ```cd {your_environment}/chat_c2s/src/```

Don't forget to compile the files of this program before running !
3. C Compiler :
    ```gcc server.c handle_client.c -o server -lpthread```

4. Run the program and start playing! :
    On the server side :
    ```./server```
    
    On the client side :
    You can use one of this command ```telnet localhost <port>``` or ```nc -l -p <port>```

5. [Optionnal] If the sockets are not connected, here is the command to see if sockets are already used :
    ```netstat -an | grep <port>```

6. [Optionnal] It is possible to test if the program has errors by running Valgrind with these commands :
    ```gcc -g serverChat.c handle_clientChat.c -o serverChat```
    ```valgrind --leak-check=full --track-origins=yes ./serverChat```
