# Mike's Windows RSA TCP Client / Server

## Description
Client and Server applications which uses C++, TCP sockets, Winsock, The RSA Algorithm with Cipher Block Chaining, to encrypt messages sent from the Client to the Server. This works on a single computer over the local host but the IP and port configuration can be changed to work over a local network. It is IPV6 and IPV4 compliant.

## Table of Contents
- [Installation](#installation)
- [Usage](#usage)
- [License](#license)

## Requirements
Windows 10 installation is required.

g++ 12.2.0 compiler is required.

MinGW is also required.

## Installation
Open the "secure_server" folder: 

To compile the code, run the compileServer batch file and press enter within the cmd window when complete.
If you want to later delete the .exe and .o files you can simply do this by running the clean batch file within the "secure_server" directory.

Open the "secure_client" folder:

To compile the code, run the compileClient batch file and press enter within the cmd window when complete.
If you want to later delete the .exe and .o files you can simply do this by running the clean batch file within the "secure_client" directory.

## Usage

To run the server open the Windows command line and cd into the directory of "secure_server", then type secure_server, allow it network access when the firewall requests and it will be ready to listen for the client!.


To run the server open the Windows command line and cd into the directory of "secure_client", then type secure_client, allow it network access when the firewall requests and it will run the client application and send to the server.

Now you can encrypted messages from the client to the server and the server will decrypt them!


## Contact

Michael Kemp - michaelandrewkemp99@gmail.com

## Acknowledgments

Initial code was from Dr Napoleon Reyes Ph.D. as part of a University Assignment.