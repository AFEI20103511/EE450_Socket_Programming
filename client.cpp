/*****************************************************************************
 * Copyright Zheng Zhang, USC CSCI EE450
 *
 * Client server is the place where "students" sign in and loop up courses info
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
#include <cstring>
#include <unistd.h>
#include <signal.h>

#define BUFFER_SIZE 5120

#define SERVER_M_IP HOMEWORK_FINAL_IP

#define VIRTUAL_LOCAL_IP "192.168.1.52"
#define MY_DEBUGGER_IP "192.168.1.75"
#define HOMEWORK_FINAL_IP "127.0.0.1"

bool mySignal = true;
char* commonBuffer;
int sockfd;

using namespace std;

// ServerM class
class Client
{
    
    string username;
    string password;
    
    int numberOfQuery;
    int serverM;
    int bufferPtr;
    bool tcpConnected;
    struct sockaddr_in client;

    public:
        // constructor
        Client()
        {
            commonBuffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
        }

        // initial TCP socket
        void initial()
        {
            cout << "The client is up and running." << endl;
            bufferPtr = 0;
            numberOfQuery = 0;
            tcpConnected = false;

            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            // error message when socket creation failed
            if(sockfd < 0) {
                cout << "Socket creation failed." << endl;
                exit(0);
            }
            // assign socket attributes
            struct sockaddr_in serverAddr;
            serverAddr.sin_family = AF_INET;
            // inet_aton("192.168.1.128", &serverAddr.sin_addr);
            inet_aton(SERVER_M_IP, &serverAddr.sin_addr);
            //inet_aton("192.168.1.75", &serverAddr.sin_addr);
            serverAddr.sin_port = htons(25212);
            int addrLen = sizeof(serverAddr);

            // create local address info
            socklen_t clientsz = sizeof(client);
            
            if(connect(sockfd, (sockaddr *)&serverAddr, sizeof(sockaddr)) < 0) {
                cout << "John's Connection Failed" << endl;
                exit(0);
            }
            // connection to TCP server is successful
            getsockname(sockfd, (struct sockaddr *) &client, &clientsz);
 
        }

        // run
        void run()
        {   
            int times = 0;
            while(!tcpConnected) {
                if(!tcpConnected && times == 3) {
                    cout << "Authentication Failed for 3 attempts. Client will shut down." << endl;
                    exit(1);
                }
                // credential verification
                tcpConnected = credential();
                times++;
                if(!tcpConnected) {
                    printf("Attempts remaining:%d\n", 3 - times);
                }
                

                // cout << "The credential alert messge is: " << commonBuffer[0] << endl;
            }
            // queries
            cin.ignore(1024, '\n');
            while(mySignal) {
                processQuery();
                cin.ignore(1024, '\n');
            }
        }

        ~Client() {
            free(commonBuffer);
            close(sockfd);
        }

    private:
        bool credential() {
            // ask user for input
            cout << "Please enter the username: ";
            cin >> username;
            cout << "Please enter the password: ";
            cin >> password;
            // credential data uses 0 to represent, 2nd part is username, 3rd part is password
            string cred = "0," + username + "," + password;
            // send credential data to main server
            // cout << "The credential message is " << cred << endl;
            send(sockfd, cred.c_str(), cred.length(), 0);
            cout << username << " sent an authentication request to the main server." << endl;
            numberOfQuery = 1;
            // receive credential feedback from main server
            int len = recv(sockfd, commonBuffer,BUFFER_SIZE,0);
            // if received message is 1, means successful
            char* strusername = (char *)alloca(username.size() + 1);
            memcpy(strusername, username.c_str(), username.size() + 1);
            // cout << "Cred result recv buffer is:" << commonBuffer[0]<< endl;

            if(commonBuffer[0] == '2') {
                // if both are passed
                printf("%s received the result of authentication using TCP over port %u. Authentication is successful\n", strusername, ntohs(client.sin_port));
                return true;
            }
            else if(commonBuffer[0] == '1')
            {
                printf("%s received the result of authentication using TCP over port %u. Authentication failed: Password does not match\n", strusername, ntohs(client.sin_port));
                return false;
            }
            else
            {
                //0
                char* mess = commonBuffer + 2;
                printf("%s received the result of authentication using TCP over port %u. Authentication failed: Username Does not exist\n", strusername, ntohs(client.sin_port));
                return false;
            }
            
        }

        void processQuery() {
            cout << "Please enter the course code to query:" << endl;
            string courses;
            // cin.ignore(1024, '\n');
            getline(cin, courses, '\n');
            // cout << "The input courses are " << courses << endl;
            
            vector<string> queries = parse(courses);
            // cout << "The number of courses to check is " << queries.size() << endl;
            string courseCode;
            for(int i = 0; i < queries.size(); i++) {
                courseCode = courseCode + queries[i] + ",";
            }
            courseCode = courseCode.substr(0, courseCode.length() - 1);
            string attr;
            if(int(queries.size()) == 1) {
                cout << "Please enter the category (Credit / Professor / Days / CourseName):" << endl;
                while(true) {
                    cin >> attr;
                    if(attr == "Credit" || attr == "Professor" ||attr == "Days" ||attr == "CourseName") {
                        break;
                    }
                    cout << "Wrong category, please re-enter:" << endl;
                }
                
                string message = "1," + courseCode + "," + attr;
                // cout << "The course you'd like to check is " << courses <<" and attr you'd like to check is " << attr << endl;
                send(sockfd, message.c_str(), message.length(), 0);
                cout << username << " sent a request to the main server." << endl;
            } else {
                string message = "2," + courseCode;
                send(sockfd, message.c_str(), message.length(), 0);
                cout << username << " sent a request with multiple CourseCode to the main server." << endl;
            }

            // receive feeback
            int len = recv(sockfd, commonBuffer,BUFFER_SIZE,0);
            printf("The client received theresponse from the Main server using TCP over port %u.\n",  ntohs(client.sin_port));
            
            commonBuffer[len] = 0;

            // cout << "Recv query result : "<<commonBuffer << endl;

            if(queries.size() > 1)
                printf("The client received the response from the Main server using TCP over port %u.\n", ntohs(client.sin_port));

            // decides which type of query feedback is 
            if(commonBuffer[0] == '0') {
                // when one course query is not found
                cout << "Didn't find the course: " << courseCode << "." << endl;
                cout << "-----Start a new request-----" << endl;
            } else if(commonBuffer[0] == '1') {
                // query for one course
                string feedback;
                for(int i = 2; i < len; i++) {
                    feedback.push_back(commonBuffer[i]);
                }
                cout << "The " << attr << " of " << courseCode << " is " << feedback << endl;
                cout << endl;
                cout << "-----Start a new request-----" << endl;
            } else {
                // query for multiple courses feedback
                cout << "CourseCode: Credits, Professor, Days, Course Name" << endl;
                string feedback;
                for(int i = 2; i < len; i++) {
                    feedback.push_back(commonBuffer[i]);
                }
                cout << feedback << endl;
                feedback.clear();
                for(int i = 1; i < queries.size(); i++) {
                    int tempLen = recv(sockfd, commonBuffer,BUFFER_SIZE,0);
                    for(int i = 2; i < len; i++) {                    
                        feedback.push_back(commonBuffer[i]);
                    }
                    cout << feedback << endl;
                }
                cout << endl;
                cout << "-----Start a new request-----" << endl;
            }

        }

        vector<string> parse(string courses) {
            vector<string> re;
            int ptr = 0;
            while(ptr < courses.length()) {
                string course;
                while(ptr <courses.length() && courses[ptr] != ' ') {
                    course.push_back(courses[ptr]);
                    ptr++;
                }
                ptr++;
                re.push_back(course);
                course.clear();
            }
            // for(int i =0; i < re.size(); i++) {
            //     cout << re[i] << endl;
            // }
            return re; 
        }

};

void hd(int signum) {
    mySignal = false;
    // cout << "I caught ctrl + c" << endl;
    free(commonBuffer);
    close(sockfd);
    exit(0);
}

int main()
{
    signal(SIGINT, &hd);
    // instantiate an main server object and start
    Client c;
    
    // connect to main server
    c.initial();
    c.run();

    return 0;
}
