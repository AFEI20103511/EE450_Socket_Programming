/*****************************************************************************
 * Copyright Zheng Zhang, USC CSCI EE450
 *
 * Credential sever store user encrypted username and password
 *****************************************************************************/

#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>

#define LOCAL_BIND_IP HOMEWORK_FINAL_IP


#define VIRTUAL_LOCAL_IP "192.168.1.52"
#define MY_DEBUGGER_IP "192.168.1.75"
#define HOMEWORK_FINAL_IP "127.0.0.1"

#define SERVERPORT 21212
#define BUFFERSIZE 5120
using namespace std;

int udpSock;
char* recvBuffer; 
char* sendBuffer;

// ServerC class
class ServerC
{
    
    public:
        // attributes
        map<string,string> credMap;
        struct sockaddr_in clientAddr;

        // constructor
        ServerC()
        {
            recvBuffer = (char*) malloc(sizeof(char) * BUFFERSIZE);
            sendBuffer = (char*) malloc(sizeof(char) * BUFFERSIZE);

        }

        // initial
        void initial()
        {
            readFile();
            //Map(credMap);
            initialUDPSock();
        }

        // run
        void run()
        {   
            while(true) {
                int clientAddrlen = sizeof(clientAddr);
                //cout << "I'm here1" << endl;
                int len = recvfrom(udpSock, recvBuffer, BUFFERSIZE, 0, (struct sockaddr*)&clientAddr, (socklen_t *)&clientAddrlen);
                // received an authentication request from main server
                cout << "The ServerC received an authentication request from the Main Server." << endl;

                int result = credentialChecking(len);
                //cout << "Result is " << result << endl;
                if(result == 2) {
                    // send 2 as feedback
                    // cout << "PASS" << endl;
                    sendBuffer[0] = '2';   
                } else if(result == 1) {
                    // send 1 as feedback
                    // cout << "FAIL_PASS_NO_MATCH" << endl;
                    sendBuffer[0] = '1';
                }  else {
                    // send 0 as feedback
                    // cout << "FAIL_NO_USER," << endl;
                    sendBuffer[0] = '0';
                }
                //cout << "The checking result is " << sendBuffer << endl;
                sendto(udpSock, sendBuffer, 1, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
                // message after sending a response to Main server
                cout << "The ServerC finished sending the response to the Main Server." << endl;
            }
        }

        // helper function to print map
        void printMap(map<string,string> myMap)
        {
            map<string,string>::iterator it;
            for(it = myMap.begin(); it != myMap.end(); it++)
            {
                cout << it->first
                     << ':'
                     << it->second
                     << endl;
            }
        }
    
    private:
        void readFile()
        {
            ifstream file;
            file.open("./cred.txt", ios::binary);
            string line;
            while (getline(file,line))
            {   
                string cur = line.c_str();
                int delimiter = 0;
                while(cur[delimiter] != ',') {
                    delimiter++;
                }
                string key = cur.substr(0,delimiter);
                string val = cur.substr(delimiter+1, cur.length() - delimiter - 1);
                
                if(!val.empty() && val[val.size()-1] == 13) {
                    //val.pop_back();
                    val = val.substr(0, val.size()-1);
                }
                // cout << key << endl;
                // cout << val << endl;
                credMap[key] = val;
            }    
            // printMap(credMap);    
        }

        void initialUDPSock() {
            // cout << "Initialize Credential server UPD socket" << endl;
            // boot up message
            cout << "The ServerC is up and running using UDP on port " << SERVERPORT << "." << endl;

            udpSock = socket(AF_INET, SOCK_DGRAM, 0);
            if(udpSock < 0) {
                cout << "Credential UDP Socket to credential creation error" << endl;
                exit(0);
            }
            
            struct sockaddr_in addrC;
            int addrLen = sizeof(addrC);
            addrC.sin_family = AF_INET;
            inet_aton(LOCAL_BIND_IP, &addrC.sin_addr); 
            addrC.sin_port = htons(SERVERPORT);
            if(bind(udpSock, (sockaddr *)&addrC, addrLen) < 0) {
                cout << "Binding with updC failed" << endl;
                exit(0);
            }
        }
        
        int credentialChecking(int len) {
            int ptr = 0;
            string encryp_username;
            string encryp_password;
            while(recvBuffer[ptr] != ','){
                encryp_username.push_back(recvBuffer[ptr]);
                ptr++;
            }
            ptr += 1;
            while(ptr < len) {
                encryp_password.push_back(recvBuffer[ptr++]);
            }
            // cout << "The encryp_username is: " << encryp_username << endl;
            // cout << "The encryp_password is: " << encryp_password << endl;  

            // three types of results
            if(credMap.count(encryp_username) == 0) {
                //cout << "No username" << endl;
                return 0;
            } else if(credMap.count(encryp_username) == 1 && credMap[encryp_username] != encryp_password) {
                //cout << "Password not correct." << endl;
                return 1;
            } 
            //cout << "All matched" << endl;
            return 2;
        }
};

void hd(int signum) {
    // cout << "I caught ctrl + c" << endl;
    free(sendBuffer);
    free(recvBuffer);
    close(udpSock);
    exit(0);
}

int main()
{
    signal(SIGINT, &hd);
    // instantiate an main server object and start
    ServerC c;
    c.initial();
    c.run();
    
    return 0;
}
