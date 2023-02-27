/*****************************************************************************
 * Copyright Zheng Zhang, USC CSCI EE450
 *
 * This file is for test individual functions
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

#define BUFFER_SIZE 5120


#define LOCALIPVIRTUAL "192.168.1.52"
#define HWIP "127.0.0.1"
#define DST_IP MINE_DEBUGGER
#define MINE_DEBUGGER "192.168.1.75"
#define MY_IP "192.168.1.75"

using namespace std;


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
                // cout << course << endl;
                re.push_back(course);
                course.clear();
            }
            for(int i =0; i < re.size(); i++) {
                cout << re[i] << endl;
            }
            return re; 
        }

int main(int argc, char const *argv[])
{
    /* code */
    string i = "EE450 CS561";
    parse(i);

}
