/*****************************************************************************
 * Copyright Zheng Zhang, USC CSCI EE450
 *
 * Main server is the middle-man communicating with front-end users and 
 * back-end servers(Auth, CS Dept, EE Dept)
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

#define BUFFER_SIZE 5120

#define LOCAL_BIND_IP HOMEWORK_FINAL_IP
#define SERVER_C_IP HOMEWORK_FINAL_IP
#define SERVER_EE_IP HOMEWORK_FINAL_IP
#define SERVER_CS_IP HOMEWORK_FINAL_IP


#define VIRTUAL_LOCAL_IP "192.168.1.52"
#define MY_DEBUGGER_IP "192.168.1.75"
#define HOMEWORK_FINAL_IP "127.0.0.1"


#define TCPPORT 25212
#define UDPPORT 24212
#define SERVERCPORT 21212
#define CSPORT 22212
#define EEPORT 23212

using namespace std;
int tcpSock;
int udp;
char* commonBuffer;


// ServerM class
class ServerM
{
    
    int client;
    
    string clientInput;
    int backendServer;
    struct sockaddr_in addrM;
    struct sockaddr_in addrCS;
    struct sockaddr_in addrEE;
    struct sockaddr_in addrCred;
    string username;
    string unencryp_username;

    public:
        // constructor
        ServerM()
        {
            commonBuffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
        }

        // initial
        void initial()
        {
            // booting up message
            cout << "The main server is up and running." << endl;
            initialTCPSock();
            // cout << "initial TCP successfully" << endl;
            initialUDPSockC();
            // cout << "initial UDP successfully" << endl; 
        }

        // run
        void run()
        {
            // receive client requet
            while(true) {
                // receive TCP data
                int len = recv(client, commonBuffer,BUFFER_SIZE,0);
                if(len == 0) {
                    continue;
                }
                cout << "Len is ------" << len << endl;
                commonBuffer[len] = '\0';
                // after receiving the username and password from the client

                if(commonBuffer[0] == '0') {
                    vector<string> encryp = getEncryption(len);  
                    processCredData(encryp);
                } else {
                    // cout << "Expected: Retrive courses information" << endl;
                    processQueryData(len);
                    // cout << "Finished current query" << endl;
                }  
                
                
            }
        }

    private:
        void initialTCPSock() 
        {
            
            // instantiate sockets
            tcpSock = socket(AF_INET, SOCK_STREAM, 0);
              
            if(tcpSock < 0 ) {
                cout << "TCP Socket creation error" << endl;
                exit(0);
            }
               
            // bind sockets with ip and port
            struct sockaddr_in addrTCP;
            int addrLen = sizeof(addrTCP);
            addrTCP.sin_family = AF_INET;
            // addrTCP.sin_addr.s_addr = inet_addr("127.0.0.1");
            inet_aton(LOCAL_BIND_IP, &addrTCP.sin_addr);
            // inet_aton("192.168.1.52", &addrTCP.sin_addr);
            addrTCP.sin_port = htons(25212);
            //cout << "hello3" << endl; 
            if(bind(tcpSock, (sockaddr *)&addrTCP, addrLen) < 0) 
            {
                cout << "Main server binding failed" << endl;
                exit(0);
            }
            //cout << "hello3" << endl; 
            // listening
            listen(tcpSock, 10);
            client = accept(tcpSock, (sockaddr *)&addrTCP, (socklen_t *)&addrLen);
            // cout << "hello3" << endl; 
        }

        
        void processCredData(vector<string> encryp) {
            // send UDP to backend servers
            string message = encryp[0] + "," + encryp[1];
            for(int i = 0; i < message.length(); i++) {
                commonBuffer[i] = message[i];
            }
            username = encryp[0];
            // cout << "The encripted username and password: " << encryp[0] << "," <<encryp[1] << endl;
            cout << "The main server received the authentication for " << unencryp_username << " using TCP overport " << TCPPORT << "." << endl;
            // send data to serverC
            //message = "0," + message;
            // cout << "Ready to send message " << message << " to serverC" << endl;
            sendto(udp, commonBuffer, message.length(), 0, (struct sockaddr*)&addrCred, sizeof(addrCred));
            cout << "The main server sent an authentication request to serverC." << endl;
            // receive feedback from serverC
            int addrLen = sizeof(addrM);
            int CredResultLen = recvfrom(udp, commonBuffer, 1, 0, (struct sockaddr*)&addrM, (socklen_t*)&addrLen);
            cout << "The main server received the result of the authentication request from ServerC using UDP over port " << SERVERCPORT << "." << endl;
            commonBuffer[CredResultLen] = 0;
            // cout <<"ServerC replies: " <<  commonBuffer << endl;
            /*
            // send feedback to client
            string alert;
            if(commonBuffer[0] == '0') {
                alert = "0";
            } else {
                alert = "1";
            }
            */
            send(client, commonBuffer,CredResultLen, 0);
            cout << "The main server sent the authentication result to the client." << endl;
        }

        vector<string> getEncryption(int len) {
            int ptr = 0;
            ptr += 2;
            
            string unencryp_password;
            //cout << "The len of commonbuffer is: " << len << endl;
            unencryp_username.clear();
            while(commonBuffer[ptr] != ','){
                unencryp_username.push_back(commonBuffer[ptr]);
                ptr++;
            }
            ptr += 1;
            unencryp_password.clear();
            while(ptr < len) {
                unencryp_password.push_back(commonBuffer[ptr++]);
            }
            vector<string> re;
            string encryp_username = encryp(unencryp_username);
            string encryp_password = encryp(unencryp_password);
            // cout << "The encryp username is " << encryp_username << endl;
            // cout << "The encryp password is " << encryp_password << endl;
            re.push_back(encryp_username);
            re.push_back(encryp_password);
            return re;
        }

        string encryp(string s) {
            string re;
            for(int i = 0; i < s.length(); i++) {
                char cur = s.at(i);
                //cout << "The char is: " << cur << endl;
                if('0' <= cur && cur <= '9') {
                    if(cur + 4 > '9') {
                        re.push_back(cur + 4 - 10);
                        //cout << "The char is: " << cur + 4 - 10 << endl;
                    } else {
                        re.push_back(cur + 4);
                        //cout << "The char is: " << cur + 4 << endl;
                    }
                } else if(('A' <= cur && cur <= 'Z')) {
                    if(cur + 4 > 'Z') {
                        re.push_back(cur + 4 - 26);
                        //cout << "The char is: " << cur + 4 - 26 << endl;
                    } else {
                        re.push_back(cur + 4);
                        //cout << "The char is: " << cur + 4 << endl;
                    }
                } else if(('a' <= cur && cur <= 'z')) {
                    if(cur + 4 > 'z') {
                        re.push_back(cur + 4 - 26);
                        //cout << "The char is: " << cur + 4 - 26 << endl;
                    } else {
                        re.push_back(cur + 4);
                        //cout << "The char is: " << cur + 4 << endl;
                    }
                } else {
                    re.push_back(cur);
                    //cout << "The char is: " << cur << endl;
                }
            }
            return re;
        }

        void initialUDPSockC() {
            // serverM udp socket creation
            udp = socket(AF_INET, SOCK_DGRAM, 0);
            if(udp < 0) {
                cout << "UDP Socket creation error" << endl;
                exit(0);
            }
            
            // initialize addrM
            addrM.sin_family = AF_INET;
            inet_aton(LOCAL_BIND_IP, &addrM.sin_addr);
            addrM.sin_port = htons(UDPPORT);
            
            // bind upd with its addr
            if(bind(udp, (sockaddr *)&addrM, sizeof(addrM)) < 0) {
                cout << "Binding with updC failed" << endl;
                exit(0);
            }

            // initialize credential server udp addr
            addrCred.sin_family = AF_INET;
            inet_aton(SERVER_C_IP, &addrCred.sin_addr);
            addrCred.sin_port = htons(SERVERCPORT);

            // initialize CS server udp addr
            addrCS.sin_family = AF_INET;
            inet_aton(SERVER_CS_IP, &addrCS.sin_addr);
            addrCS.sin_port = htons(22212);

            // initialize EE server udp addr
            addrEE.sin_family = AF_INET;
            inet_aton(SERVER_EE_IP, &addrEE.sin_addr);
            addrEE.sin_port = htons(23212);

        }

        void processQueryData(int len) {
            // when query type is 1, it is to retrieve character of single course
            // int len = recv(client, commonBuffer,BUFFER_SIZE,0);

            // cout << "The commonBuffer indicator is " << commonBuffer[0] << endl;
            if(commonBuffer[0] == '1') {
                // check
                processQuery1(len);
            } else {
            // when query type is 2, it is to retrievel all info for all courses
                processQuery2(len);
            }
            // send message back to client
            cout << "The main server sent the query information to the client." << endl;
        }

        void processQuery1(int len) {
            // get course number and attribute
            string course;
            string attr;
            // cout << "I'm parsing course query" << endl;
            for(int i = 2; i < len; i++) {
                string temp;
                while(i < len && commonBuffer[i] != ',') {
                    temp.push_back(commonBuffer[i++]);
                }
                if(i != len) {
                    course = temp;
                    temp.clear();
                } else {
                    attr = temp;
                }
            }   
            cout << "The main server received from " << unencryp_username << " to query course " << course << " about " << attr << " using TCP over port " << TCPPORT << "." << endl;
            char charAttr;
            if(attr == "Credit") {
                charAttr = '1';
            } else if (attr == "Professor") {
                charAttr = '2';
            } else if(attr == "Days") {
                charAttr = '3';
            } else {
                charAttr = '4';
            }
            string message = "1," + course + "," + charAttr;
            for(int i = 0; i < message.length(); i++) {
                commonBuffer[i] = message[i];
            }
            // cout << "The message sent to backend server is " << message << endl;
            // send message to backend server
            int feedbackMessLen;
            if(course[0] == 'C' && course[1] == 'S') {
                // send message to CS
                sendto(udp, commonBuffer, message.length(), 0, (struct sockaddr*)&addrCS,sizeof(addrCS));
                cout << "The main server sent a request to serverCS." << endl;
                // receive feedback from server CS
                int addrLen = sizeof(addrM);
                feedbackMessLen = recvfrom(udp, commonBuffer, 1024, 0, (struct sockaddr*)&addrM, (socklen_t*)&addrLen);
                // cout << "The received len is " << feedbackMessLen << endl;
                cout << "The main server received the response from serverCS using UDP over port " << CSPORT << endl;
                // cout << "The feedback info from serverCS is " << commonBuffer << endl;    
            } else if(course[0] == 'E' && course[1] == 'E') {
                // send message to EE
                // cout << "The message is " << message << endl;
                sendto(udp, commonBuffer, message.length(), 0, (struct sockaddr*)&addrEE,sizeof(addrEE));
                cout << "The main server sent a request to serverEE." << endl;
                // receive feedback from server EE
                int addrLen = sizeof(addrM);
                feedbackMessLen = recvfrom(udp, commonBuffer, 1024, 0, (struct sockaddr*)&addrM, (socklen_t*)&addrLen);
                cout << "The main server received the response from serverEE using UDP over port " << EEPORT << endl;
                // cout << "The feedback info from serverEE is " << commonBuffer << endl;
            }  else {
                commonBuffer[0] = '0';
                feedbackMessLen = 1;
            }
            
            commonBuffer[feedbackMessLen] = 0;
            // cout << commonBuffer << endl;


            if(commonBuffer[0] == '2') {
                string mess;
                for(int i = 2; i < feedbackMessLen; i++) {
                    mess.push_back(commonBuffer[i]);
                }
                send(client, mess.c_str(), mess.length(), 0);
            } else {
                
                send(client, commonBuffer, feedbackMessLen, 0);
            }

            // cout << commonBuffer << endl;
        }

        void processQuery2(int len) {
            int ptr = 2;
            while(ptr < len) {
                string course;
                while(ptr < len && commonBuffer[ptr] != ',') {
                    course.push_back(commonBuffer[ptr++]);
                } 
                ptr++;
                processCourseQ(course);
                course.clear();
            }
        }

        void processCourseQ(string course) {
            if(course[0] == 'C' && course[1] == 'S') {
                string message = "2," + course;
                // send message to cs server
                sendto(udp, commonBuffer, message.length(), 0, (struct sockaddr*)&addrCS,sizeof(addrCS));
                // receive feedback from server CS
                int addrLen = sizeof(addrM);
                recvfrom(udp, commonBuffer, 1, 0, (struct sockaddr*)&addrM, (socklen_t*)&addrLen);
                cout << "The feedback info from serverCS is " << commonBuffer << endl;
            } else {
                string message = "2," + course;
                // send message to ee server
                sendto(udp, commonBuffer, message.length(), 0, (struct sockaddr*)&addrEE,sizeof(addrEE));
                // receive feedback from serverC
                int addrLen = sizeof(addrM);
                recvfrom(udp, commonBuffer, 1, 0, (struct sockaddr*)&addrM, (socklen_t*)&addrLen);
                cout << "The feedback info from serverEE is " << commonBuffer << endl;
            }
        }
};

void hd(int signum) {
    // cout << "I caught ctrl + c" << endl;
    free(commonBuffer);
    close(tcpSock);
    close(udp);

    exit(0);
}

int main()
{
    signal(SIGINT, &hd);
    // instantiate an main server object and start
    ServerM m;
    m.initial();
    m.run();

    return 0;
}
