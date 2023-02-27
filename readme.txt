Student info:
Zheng Zhang
6539807212

What I have done:
I have finished non-optional part, and it works with no issues.

Code files:
client.cpp - client program to input username, password and queries, and display results
serverM - middle server between client and backend servers(credential/CS/EE servers), pass info to the right backend server
and return feedback to the client
serverC - for credential checking
serverCS - for CS course query answer retrival
serverEE - for EE course query answer retrival

Format of all message exchanges:
# client
    - to serverM
    credential communication:
        '0' - credential checking
        ig. "0,username,password"
    query to CS/EE dept:
        '1' - query 
        ig. "1,EE450,Professor"

# credential server
    - to serverM
        '0' - no such user
        '1' - incorrect password
        '2' - pass

# CS/EE server
    - to serverM
        '0' - no course found
        '1' - result found
        ig. "1,Ali" (for query EE450,Professor)

# serverM
    - to credential server
        '0' - credential checking
        ig. "0,encrypt_username,encrypt_password"
    
    - to CS/EE server
        ig. "1,CS,category#" (catergory# is different depends on selection)


Be aware:
    1. when input coursecode for course query, it cannot be empty(which means that you cannot just press "enter" button as coursecode)
       
    2. when input coursecode, there cannot exist space, ig "EE450 Credit" is wrong.(The input with space was original for multiply courses
        queries, but not finished)
    
     correct way is to input at least one char before "enter", ig. "C", "EE", "EE450", "EE10000",ect.

    3. if you've terminated a connected client, you should also restart serverM serverC serverCS serverEE to work with a new opened client(no need to compile again)

    4. the input file path of credential, CS and EE are harded in the program, please use the same name of the given tested file

No, I wrote all codes by myself.
