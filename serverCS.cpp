/*****************************************************************************
 * Copyright Zheng Zhang, USC CSCI EE450
 *
 * EE server stores CS course information
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

#define SERVERPORT 22212
#define BUFFERSIZE 5120
using namespace std;

map<char, string> title;

int udpSock;
char* recvBuffer; 
char* sendBuffer;

// ServerM class
class Server
{
    // attributes
    map<string,vector<string> > csMap;
    
    struct sockaddr_in clientAddr;
    map<char,string> attrs;

    public:
        // constructor
        Server()
        {
            recvBuffer = (char*) malloc(sizeof(char) * BUFFERSIZE);
            sendBuffer = (char*) malloc(sizeof(char) * BUFFERSIZE);
        }

        // initial
        void initial()
        {   
            // boot up message
            cout << "The ServerCS is up and running using UDP on port " << SERVERPORT << "." << endl;
            title['1'] = "Credits";
            title['2'] = "Professor";
            title['3'] = "Days";
            title['4'] = "Course Name";
            readFile();
            // printMap(csMap);
            initialUDPSock();
            
        }

        // run
        void run()
        {
            // cout << "CS server is built up successfully!" << endl;
            while(true) {
                int clientAddrlen = sizeof(clientAddr);
                int len = recvfrom(udpSock, recvBuffer, BUFFERSIZE, 0, (struct sockaddr*)&clientAddr, (socklen_t *)&clientAddrlen);
                recvBuffer[len] = 0;
                // cout << "Recv data: " << recvBuffer << endl;

                // retrive catergory from map
                string message;

                // two types of checking
                if(recvBuffer[0] == '1') {
                    // cout << "Type 1 checking" << endl;
                    // when indicator is 1, it is to retrive attribute
                    message = type1(len);
                    // cout << "Checking user existence ---" << message << endl;

                } else {
                    // cout << "Type 2 checking" << endl;
                    // when indicator is 2, it is to retrive all attribute
                    message = type2(len);
                    cout << "The message is " << message << endl;
                }

                // store message to sendBuffer
                for(int i = 0; i < message.length(); i++) {
                    sendBuffer[i] = message[i];
                }
                sendBuffer[message.length()] = '\0';
                int sendBuffermessLen = message.length() + 1;
                // send back message
                sendto(udpSock, sendBuffer, sendBuffermessLen, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
                // message after sending the results to main server
                cout << "The ServerCS finished sending the response to the Main Server." << endl;
                // cout << "------------" << sendBuffer << endl;
            }
        }

        string type1(int len) {
            string course;
            char attr;
            int ptr = 2;
            while(ptr < len) {
                while(ptr < len && recvBuffer[ptr] != ',') {
                    course.push_back(recvBuffer[ptr++]);
                }
                break;
            }

            attr = recvBuffer[len-1];
            // cout << "Course and attr get!" << endl;
            // cout << "Course is " << course << endl;
            // cout << "Attr is " << attr << endl;
            // message after receiving the request from main server
            // cout << attr << endl;
            // cout << attrs['0'] << endl;
            // cout << attrs['1'] << endl;
            // cout << attrs['2'] << endl;
            // cout << attrs['3'] << endl;

            cout << "The ServerCS received a request from the Main Server about the " << title[attr] << " of " << course << "." << endl;
            // printMap(csMap);
            if(csMap.count(course) == 0) {
                // when course is not found
                cout << "Didn't find the course: " << course << "." << endl;
                return "0";
            }
            cout << "The course information has been found: The " << title[attr] << " of " << course << " is " << csMap[course][attr - '1'] << "." << endl;
            string reMess = "1," + csMap[course][attr - '1'];
            return reMess;
        }

        string type2(int len) {
            int ptr = 2;
            string course;
            while(ptr < len) {
                course.push_back(recvBuffer[ptr++]);
            }
            string message = course + ": ";
            vector<string> temp = csMap[course];
            cout << "Course is " << course << endl;
            for(int i = 0; i < int(temp.size()); i++) {
                message = message + temp[i] + ", ";
            }
            message = message.substr(0,message.length() - 1);
            cout << "Type2 message is " << message << endl;
            return message;
        }

        // helper function to print map
        void printMap(map<string,vector<string> > myMap)
        {
            map<string,vector<string> >::iterator it;
            for(it = myMap.begin(); it != myMap.end(); it++)
            {
                cout << it->first << ':';
                for(int i = 0; i < it->second.size(); i++) {
                    cout << it->second[i] << "===";
                }
                cout << endl;
            }
        }
    
    private:
        void readFile()
        {
            ifstream file;
            file.open("./cs.txt", ios::binary);
            string line;
            while (getline(file,line))
            {   
                string cur = line.c_str();
                if(cur[cur.length() - 1] == 13) {
                    cur = cur.substr(0, cur.length()-1);
                }
                // cur.pop_back();
                int delimiter = 0;
                while(cur[delimiter] != ',') {
                    delimiter++;
                }
                string key = cur.substr(0,delimiter);
                string val = cur.substr(delimiter+1, cur.length() - delimiter - 1);
                vector<string> items = getItems(val);
                // cout << "Key is ---------" << key << endl;
                // cout << "Val is ---------" << val << endl;
                csMap[key] = items;
            }    
            // store int and characters of a row of course info
            // attrs['0'] = "Credit";
            // attrs['1'] = "Professor";
            // attrs['2'] = "Days";
            // attrs['3'] = "CourseName";
            // printMap(csMap);
        }

        vector<string> getItems(string s) 
        {
            vector<string> re;
            int l = 0;
            int r = 0;
            while(r < s.length()) {
                if(s[r] != ',') {
                    r++;
                    continue;
                }
                re.push_back(s.substr(l, r-l));
                l = ++r;

            }
            re.push_back(s.substr(l, r-l));
            // for(int i = 0; i < re.size(); i++) {
            //     cout << re[i] << endl;
            // }
            return re;
        }

        void initialUDPSock() {
            // cout << "Initialize CS server UPD socket" << endl;
            udpSock = socket(AF_INET, SOCK_DGRAM, 0);
            if(udpSock < 0) {
                cout << "Credential UDP Socket to credential creation error" << endl;
                exit(0);
            }
            
            struct sockaddr_in addrCS;
            int addrLen = sizeof(addrCS);
            addrCS.sin_family = AF_INET;
            inet_aton(LOCAL_BIND_IP, &addrCS.sin_addr); 
            addrCS.sin_port = htons(SERVERPORT);
            if(bind(udpSock, (sockaddr *)&addrCS, addrLen) < 0) {
                cout << "Binding with updC failed" << endl;
                exit(0);
            }
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
    Server cs;
    cs.initial();
    cs.run();

    return 0;
}