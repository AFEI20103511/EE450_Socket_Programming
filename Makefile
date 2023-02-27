all: m c ee cs cl

m: serverM.cpp
	g++ -o serverM ./serverM.cpp

c: serverC.cpp
	g++ -o serverC ./serverC.cpp

ee: serverEE.cpp
	g++ -o serverEE ./serverEE.cpp

cs: serverCS.cpp
	g++ -o serverCS ./serverCS.cpp

cl: client.cpp
	g++ -o client ./client.cpp

clean:
	rm -f serverC serverCS serverEE serverM client