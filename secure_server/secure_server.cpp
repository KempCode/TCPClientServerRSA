//////////////////////////////////////////////////////////////
// TCP SECURE SERVER GCC (IPV6 ready)
//
//
// References: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738520(v=vs.85).aspx
//             http://long.ccaba.upc.edu/long/045Guidelines/eva/ipv6.html#daytimeServer6
//
//////////////////////////////////////////////////////////////


#define USE_IPV6 false  //if set to false, IPv4 addressing scheme will be used; you need to set this to true to 
												//enable IPv6 later on.  The assignment will be marked using IPv6!

#if defined __unix__ || defined __APPLE__
  #include <unistd.h>
  #include <errno.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h> //used by getnameinfo()
  #include <iostream>

#elif defined __WIN32__
  #include <winsock2.h>
  #include <ws2tcpip.h> //required by getaddrinfo() and special constants
  #include <stdlib.h>
  #include <stdio.h>
  #include <iostream>
  #include <vector>
  #include <cfenv>
  #include <cmath>
  #include <cstring>
  #include <string>
  #include <sstream>
  #define WSVERS MAKEWORD(2,2) /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
                    //The high-order byte specifies the minor version number; 
                    //the low-order byte specifies the major version number.
  WSADATA wsadata; //Create a WSADATA object called wsadata. 
#endif


#define DEFAULT_PORT "1234" 

#define BUFFER_SIZE 500
#define RBUFFER_SIZE 500

using namespace std;
/////////////////////////////////////////////////////////////////////
//GLOBALS
unsigned long long dCA_D = 3243525;
unsigned long long dCA_N = 8672701;

unsigned long long rsaCBC_E = 65537;
unsigned long long rsaCBC_D = 309649;
unsigned long long rsaCBC_N = 388627;



void printBuffer(const char *header, char *buffer){
	cout << "------" << header << "------" << endl;
	for(unsigned int i=0; i < strlen(buffer); i++){
		if(buffer[i] == '\r'){
		   cout << "buffer[" << i << "]=\\r" << endl;	
		} else if(buffer[i] == '\n'){
		   cout << "buffer[" << i << "]=\\n" << endl;	
		} else {   
		   cout << "buffer[" << i << "]=" << buffer[i] << endl;
		}
	}
	cout << "---" << endl;
}

unsigned long long repeat_square(int x,int e,int n); //integer data type only, can be upgraded to  unsigned long long
//////////////////////////////////////////////////////////////
//Note: integer data type only, can be upgraded to  unsigned long long
unsigned long long repeat_square(unsigned long long x, unsigned long long e, unsigned long long n) {
  unsigned long long y=1;//initialize y to 1, very important
  while (e>0) {
    if ((e%2)==0) {
      x=(x*x)%n;
      e=e/2;
    }
    else {
      y=(x*y)%n;
      e=e-1;
    }
  }
  return y; //the result is stored in y
}



struct myMessage{
   char messageRBuffer[RBUFFER_SIZE]; 
   int byteSize;
};

void receiveMessage(SOCKET ns, myMessage* thisMessage){
   int bytes = 0;
   char localRBuffer[RBUFFER_SIZE];
   int randx=0;

   while(1){
      while (1) {
           // cout << bytes;
            bytes = recv(ns, &localRBuffer[randx], 1, 0);

            if ((bytes < 0) || (bytes == 0)) break;
					 
            if (localRBuffer[randx] == '\n') { /*end on a LF, Note: LF is equal to one character*/  
               localRBuffer[randx] = '\0';
               break;
            }
            if (localRBuffer[randx] != '\r') randx++; /*ignore CRs*/
         }
			
         if ((bytes < 0) || (bytes == 0)) break;

         strcpy(thisMessage->messageRBuffer, localRBuffer);
         thisMessage->byteSize = bytes;
         break;
   }
}


void sendMessage(SOCKET ns, myMessage* thisMessage, string messageSend){
	char localSBuffer[200];

	//String To Send.
	std::string z = messageSend;
	const int zLength = z.length();
    char* charz_array = new char[zLength + 1];
    strcpy(charz_array, z.c_str());
    snprintf(localSBuffer, 200, "%s", charz_array);
	strcat(localSBuffer,"\r\n");


	strcpy(thisMessage->messageRBuffer, localSBuffer);
	thisMessage->byteSize = send(ns, localSBuffer, strlen(localSBuffer),0);
	// TESTING ERROR CHECKING
	if (thisMessage->byteSize == SOCKET_ERROR) {
	         printf("send failed\n");
    		 WSACleanup();
	      	 exit(1);
	      }
}




/////////////////////////////////////////////////////////////////////

//*******************************************************************
//MAIN
//*******************************************************************
int main(int argc, char *argv[]) {
	
//********************************************************************
// INITIALIZATION of the SOCKET library
//********************************************************************
   
	struct sockaddr_storage clientAddress; //IPV6
	
	char clientHost[NI_MAXHOST]; 
	char clientService[NI_MAXSERV];
	
   


    char send_buffer[BUFFER_SIZE],receive_buffer[RBUFFER_SIZE];
    int n,bytes,addrlen;
	char portNum[NI_MAXSERV];
	// char username[80];
	// char passwd[80];
		
   //memset(&localaddr,0,sizeof(localaddr));


#if defined __unix__ || defined __APPLE__
   int s,ns;

#elif defined _WIN32

	SOCKET s,ns;

//********************************************************************
// WSSTARTUP
/*	All processes (applications or DLLs) that call Winsock functions must 
	initialize the use of the Windows Sockets DLL before making other Winsock 
	functions calls. 
	This also makes certain that Winsock is supported on the system.
*/
//********************************************************************
   int err;
	
   err = WSAStartup(WSVERS, &wsadata);
   if (err != 0) {
      WSACleanup();
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
      printf("WSAStartup failed with error: %d\n", err);
		exit(1);
   }

	
//********************************************************************
/* Confirm that the WinSock DLL supports 2.2.        */
/* Note that if the DLL supports versions greater    */
/* than 2.2 in addition to 2.2, it will still return */
/* 2.2 in wVersion since that is the version we      */
/* requested.                                        */
//********************************************************************

    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        exit(1);
    }
    else{
		  printf("\n\n<<<TCP SERVER>>>\n"); 
		  printf("\nThe Winsock 2.2 dll was initialised.\n");
	 }
	 
#endif

//********************************************************************
// set the socket address structure.
//
//********************************************************************
struct addrinfo *result = NULL;
struct addrinfo hints;
int iResult;


//********************************************************************
// STEP#0 - Specify server address information and socket properties
//********************************************************************

	 
//ZeroMemory(&hints, sizeof (hints)); //alternatively, for Windows only
memset(&hints, 0, sizeof(struct addrinfo));

if(USE_IPV6){
   hints.ai_family = AF_INET6;  
}	 else { //IPV4
   hints.ai_family = AF_INET;
}	 

hints.ai_socktype = SOCK_STREAM;
hints.ai_protocol = IPPROTO_TCP;
hints.ai_flags = AI_PASSIVE; // For wildcard IP address 
                             //setting the AI_PASSIVE flag indicates the caller intends to use 
									  //the returned socket address structure in a call to the bind function. 

// Resolve the local address and port to be used by the server
if(argc==2){	 
	 iResult = getaddrinfo(NULL, argv[1], &hints, &result); //converts human-readable text strings representing hostnames or IP addresses 
	                                                        //into a dynamically allocated linked list of struct addrinfo structures
																			  //IPV4 & IPV6-compliant
	 sprintf(portNum,"%s", argv[1]);
	 printf("\nargv[1] = %s\n", argv[1]); 	
} else {
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result); //converts human-readable text strings representing hostnames or IP addresses 
	                                                             //into a dynamically allocated linked list of struct addrinfo structures
																				    //IPV4 & IPV6-compliant
	sprintf(portNum,"%s", DEFAULT_PORT);
	printf("\nUsing DEFAULT_PORT = %s\n", portNum); 
}

#if defined __unix__ || defined __APPLE__

	if (iResult != 0) {
	    printf("getaddrinfo failed: %d\n", iResult);
	    
	    return 1;
	}	 
#elif defined _WIN32

	if (iResult != 0) {
	    printf("getaddrinfo failed: %d\n", iResult);

	    WSACleanup();
	    return 1;
	}	 
#endif

//********************************************************************
// STEP#1 - Create welcome SOCKET
//********************************************************************

#if defined __unix__ || defined __APPLE__
  s = -1;
#elif defined _WIN32
  s = INVALID_SOCKET; //socket for listening
#endif
// Create a SOCKET for the server to listen for client connections

s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

#if defined __unix__ || defined __APPLE__

if (s < 0) {
    printf("Error at socket()");
    freeaddrinfo(result);    
    exit(1);//return 1;
}

#elif defined _WIN32

//check for errors in socket allocation
if (s == INVALID_SOCKET) {
    printf("Error at socket(): %d\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    exit(1);//return 1;
}
#endif
//********************************************************************

	
//********************************************************************
//STEP#2 - BIND the welcome socket
//********************************************************************

// bind the TCP welcome socket to the local address of the machine and port number
    iResult = bind( s, result->ai_addr, (int)result->ai_addrlen);

#if defined __unix__ || defined __APPLE__ 
	if (iResult != 0) {
        printf("bind failed with error");
        freeaddrinfo(result);
		 
        close(s);
        
        return 1;
    }

#elif defined _WIN32 

    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
		 
        closesocket(s);
        WSACleanup();
        return 1;
    }
#endif    
	 
	 freeaddrinfo(result); //free the memory allocated by the getaddrinfo 
	                       //function for the server's address, as it is 
	                       //no longer needed
//********************************************************************
	 
/*
   if (bind(s,(struct sockaddr *)(&localaddr),sizeof(localaddr)) == SOCKET_ERROR) {
      printf("Bind failed!\n");
   }
*/
	
//********************************************************************
//STEP#3 - LISTEN on welcome socket for any incoming connection
//********************************************************************
#if defined __unix__ || defined __APPLE__ 
	if (listen( s, SOMAXCONN ) < 0 ) {
     printf( "Listen failed with error\n");
     close(s);
     
     exit(1);
   } 

#elif defined _WIN32 	 
	if (listen( s, SOMAXCONN ) == SOCKET_ERROR ) {
     printf( "Listen failed with error: %d\n", WSAGetLastError() );
     closesocket(s);
     WSACleanup();
     exit(1);
   } 
#endif   
	
//*******************************************************************
//INFINITE LOOP
//********************************************************************
while (1) {  //main loop
	  printf("\n<<<SERVER>>> is listening at PORT: %s\n", portNum);
      addrlen = sizeof(clientAddress); //IPv4 & IPv6-compliant
		
//********************************************************************
//NEW SOCKET newsocket = accept
//********************************************************************
#if defined __unix__ || defined __APPLE__ 
      ns = -1;
#elif defined _WIN32       
	   ns = INVALID_SOCKET;
#endif	   

		//Accept a client socket
		//ns = accept(s, NULL, NULL);

//********************************************************************	
// STEP#4 - Accept a client connection.  
//	accept() blocks the iteration, and causes the program to wait.  
//	Once an incoming client is detected, it returns a new socket ns
// exclusively for the client.  
// It also extracts the client's IP address and Port number and stores
// it in a structure.
//********************************************************************

#if defined __unix__ || defined __APPLE__ 
	ns = accept(s,(struct sockaddr *)(&clientAddress),(socklen_t*)&addrlen); //IPV4 & IPV6-compliant

	if (ns < 0) {
		 printf("accept failed\n");
		 close(s);
		 
		 return 1;
	}
#elif defined _WIN32 
	ns = accept(s,(struct sockaddr *)(&clientAddress),&addrlen); //IPV4 & IPV6-compliant
	if (ns == INVALID_SOCKET) {
		 printf("accept failed: %d\n", WSAGetLastError());
		 closesocket(s);
		 WSACleanup();
		 return 1;
	}
#endif

	
	  printf("\nA <<<CLIENT>>> has been accepted.\n");
		
		
	  memset(clientHost, 0, sizeof(clientHost));
   	  memset(clientService, 0, sizeof(clientService));
      getnameinfo((struct sockaddr *)&clientAddress, addrlen,
                    clientHost, sizeof(clientHost),
                    clientService, sizeof(clientService),
                    NI_NUMERICHOST);
		
      printf("\nConnected to <<<Client>>> with IP address:%s, at Port:%s\n",clientHost, clientService);
		      
		
//********************************************************************		
//Communicate with the Client
//********************************************************************
	  printf("\n--------------------------------------------\n");
	  printf("the <<<SERVER>>> is waiting to receive messages.\n");

   
   //TESTING*****************************************
   //print secure server's Public key (e,n)
   cout << "Secure server's Public key (e,n): " << rsaCBC_E << ", " << rsaCBC_N << endl;
   //print secure server's Private key (d,n)
   cout << "Secure server's Private key (d,n): " << rsaCBC_D << ", " << rsaCBC_N << endl;


   //Encrypt each number e and n 1 at a time...
   long long dceToSend = repeat_square(rsaCBC_E, dCA_D, dCA_N);
   long long dcnToSend = repeat_square(rsaCBC_N, dCA_D, dCA_N);
   string dCA_EN = "";
   dCA_EN += to_string(dceToSend);
   dCA_EN += ",";
   dCA_EN += to_string(dcnToSend);
   //send the dCA(e, n) to the client.
   myMessage dCAENSend;
   sendMessage(ns, &dCAENSend, dCA_EN);

   //print Certificate issued by a Certification Authority (CA) dCA(e, n)
   cout << "Just sent dCA(e, n):" << dCA_EN << endl;

   //RECEIVE the ACK 226 public key recvd
   myMessage ackMessage1;
   receiveMessage(ns, &ackMessage1);
   if (ackMessage1.byteSize == SOCKET_ERROR) break;

   if(strncmp(ackMessage1.messageRBuffer, "ACK 226 public key recvd", 23) == 0){
      cout << "Received: ACK 226 public key recvd" << endl;
   }else{
      cout << "ACK 226 NOT RECEIVED" << endl;
   }   

   //Get ready to receive e(NONCE)
   myMessage RNONCE_encrypted;
   receiveMessage(ns, &RNONCE_encrypted);
   if (RNONCE_encrypted.byteSize == SOCKET_ERROR) break;
   //Display original NONCE
   cout << "The encrypted e(NONCE) received is:" << RNONCE_encrypted.messageRBuffer << endl;
   //Decrypt d(e(NONCE))
   unsigned long long RNONCE_encrypted_num = stoull(RNONCE_encrypted.messageRBuffer);
   unsigned long long NONCE_decrypted;
   NONCE_decrypted = repeat_square(RNONCE_encrypted_num, rsaCBC_D, rsaCBC_N);
   //Display decrypted NONCE
   cout << "The decrypted d(e(NONCE)) received is: " << NONCE_decrypted << endl;


   //Send ACK 220 nonce OK
   myMessage sendACKNonce;
   sendMessage(ns, &sendACKNonce, "ACK 220 nonce OK");
   cout << "Just sent ACK 220 nonce OK" << endl;


   cout << "****************************************\n";

      while (1) {
         n = 0;
//********************************************************************
//RECEIVE one command (delimited by \r\n)
//********************************************************************
         while (1) {
            bytes = recv(ns, &receive_buffer[n], 1, 0);

            if ((bytes < 0) || (bytes == 0)) break;
					 
            if (receive_buffer[n] == '\n') { /*end on a LF, Note: LF is equal to one character*/  
               receive_buffer[n] = '\0';
               break;
            }
            if (receive_buffer[n] != '\r') n++; /*ignore CRs*/
         }
			
         if ((bytes < 0) || (bytes == 0)) break;
         sprintf(send_buffer, "Message:'%s' - There are %d bytes of information\r\n", receive_buffer, n);

//********************************************************************
//PROCESS REQUEST
//********************************************************************		
         cout << "The received encrypted message was: " << receive_buffer << endl;
         //printBuffer("RECEIVE_BUFFER", receive_buffer);
         string receivedString = receive_buffer;
         std::vector<int> encryptedNumbersReceived;
         std::vector<int> decryptedReceipt;

         //Adding received encrypted integers into the vector.
         stringstream ss (receivedString);
         string numberi;
         char del = ' ';
         while (!ss.eof()){
               std::getline(ss, numberi, del);
               encryptedNumbersReceived.push_back(stoi(numberi));
         }


         //Now Decrypt all of the data using RSA CBC and Nonce

         unsigned long long localNONCE = NONCE_decrypted;
         for (unsigned long long i: encryptedNumbersReceived){
            unsigned long long temp;
            unsigned long long last_nonce = i;
            temp = repeat_square(i, rsaCBC_D, rsaCBC_N);
            temp = temp ^ localNONCE;
            localNONCE = last_nonce;

            decryptedReceipt.push_back(temp);
         }


         //Convert vector into string...
         string finalDecryption = "";
         for(int i: decryptedReceipt){
            finalDecryption += char(i);
         }
         cout << "After decryption, the message found is: " << finalDecryption << endl << endl;
         


//********************************************************************
//SEND
//********************************************************************         
		//bytes = send(ns, send_buffer, strlen(send_buffer), 0);
		 //printf("echo decrypted message back to client --> %s\n",finalDecryption);
		 //printBuffer("SEND_BUFFER", send_buffer);

#if defined __unix__ || defined __APPLE__ 
		if (bytes < 0) break;
#elif defined _WIN32 		 
        if (bytes == SOCKET_ERROR) break;
#endif         
			
      }
//********************************************************************
//CLOSE SOCKET
//********************************************************************
	  

#if defined __unix__ || defined __APPLE__ 
      int iResult = shutdown(ns, SHUT_WR);
	  if (iResult < 0) {
         printf("shutdown failed with error\n");
         close(ns);
         
         exit(1);
      }
      close(ns);

#elif defined _WIN32 
      int iResult = shutdown(ns, SD_SEND);
      if (iResult == SOCKET_ERROR) {
         printf("shutdown failed with error: %d\n", WSAGetLastError());
         closesocket(ns);
         WSACleanup();
         exit(1);
      }	

      closesocket(ns);
#endif      
//***********************************************************************
				
		
      printf("\ndisconnected from << Client >> with IP address:%s, Port:%s\n",clientHost, clientService);
   	  printf("=============================================");
		
} //main loop
//***********************************************************************
#if defined __unix__ || defined __APPLE__ 
	close(s);
#elif defined _WIN32 
	closesocket(s);
	WSACleanup(); /* call WSACleanup when done using the Winsock dll */
#endif
   
   return 0;
}


