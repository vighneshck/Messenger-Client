# Messenger
  ---------

This is a messaging application implemented using Socket Programming. This chat app is meant to serve as an excellent and efficient way to chat between two peers, one following the server architecture, and the other with the client architecture. 

How to run
----------

Open the terminal in 2 tabs. On the first tab (to run server), type:

Compile: gcc server.c -o server -lpthread -w

Run: ./server <server_TCP_port_number> <server_UDP_port_number> <client_UDP_port_number>

e.g.: ./server 5000 6000 7000

On the second tab (to run client), type:

Compile: gcc client.c -o client -lpthread -w

Run: ./client <ip_address> <server_TCP_port_number> <client_UDP_port_number> <server_UDP_port_number>

e.g.: ./client 127.0.0.1 5000 7000 6000

Features Implemented
--------------------

One of the peers is built using the server architecture consisting of functions including socket(), bind(), listen(), accept(), and send()/recv(), while the other has a client architecture consisting of socket(), connect() and send()/recv().

While performing accept(), a new socket is created by the server, which now communicates with the client using an existing thread. A new thread is created within both server AND client which handles receiving on both ends, while sending is done through the initial thread itself.

Both peers can message each other using TCP sockets, and also send files to and fro using the TCP stream AND UDP datagram too.



