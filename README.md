# socket-chatserver

## Description
A little chatroom written in C with sockets! 
One can host the server on a certain port of their machine, 
and other users on different machines can join the room hosted by that server.

## IMPORTANT
I made this to learn about sockets, practice some C, and generally for fun. 
In case you want to try this (for some godforsaken reason), only try it with friends or multiple tabs on your local machine.
I've gotten what I wanted out of this project for now, so I'm not going to bother with stuff like malicious user input and extensive error handling.
Just stick to the rules, and it should work fine.

## Usage
Host the chatroom: `./myserver <port>`

Join the chatroom: `./myclient <IP> <port>`

The client's IP argument should be the machine IP, and the port should be the same as the port on which the server is running. 

## Makefile
If you make changes to any files and want to try them out then just run `make`. You can run `make clean` to delete the executables.

## Known Issues
These are just a few issues I know are still there. If there's any new ones you find and care about, you can make an issue or comment or PR or whatever floats your boat.

- user can just send empty newlines in chat
- no moderation filter (womp womp)
- weird behavior when user input has newlines (like pasting a paragraph)
