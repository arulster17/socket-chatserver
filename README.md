# socket-chatserver

## Description
A little chatroom written in C with sockets! 
One can host the server on a certain port of their machine, 
and other users on different machines can join the room hosted by that server.

## IMPORTANT
I made this to learn about sockets, practice some C, and generally for fun. Some of the code may be a mess, but I've added comments throughout that explain what certain code is doing. If you are really curious and the comments aren't clear, I would be glad to explain.

In case you want to try this (for some godforsaken reason), only try it with friends or multiple tabs on your local machine.
I've done what I wanted to with this project for now, so I'm not going to bother with stuff like malicious user input and extensive error handling.
There's always a non-zero chance with C and user input that fishy stuff can happen, so don't risk it.
I don't think there's any way for user input to cause major problems, but I'm not 100% sure. 

## Usage
Host the chatroom: `./myserver <port>`

Join the chatroom: `./myclient <IP> <port>`

The client's IP argument should be the machine IP, and the port should be the same as the port on which the server is running. 

## Makefile
If you make changes to any files and want to try them out then just run `make`. You can run `make clean` to delete the executables.

## Known Issues
These are just a few issues I know are still there. If there's any new ones you find and care about, you can make an issue or comment or PR or whatever floats your boat.

- user can just send empty newlines in chat to spam
- no moderation filter (womp womp)
- weird behavior when user input has newlines (like pasting a paragraph)

### Thanks for reading!
