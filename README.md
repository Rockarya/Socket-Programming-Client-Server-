1)How to execute my code:-

	->To run the server file use these 2 commands:-
	
		$ gcc -o server server.c
		
		$ ./server

	->To run the client file use these 2 commands:-
	
		$ gcc -o client client.c
		
		$ ./client

	->Please note that you have to run the server file first and after that run client file. 
	
	->This way client file will get connected to the server file else it won't be get connected.

2)Key points to note in my code:-

	->Implemented basic functionalities asked in the assignment.
	->Implemented space bar and tab seperated commands (means i have tokenized commands by both space and tab). 
	->Appending a file if it already exists in client's directory.
	->Showing progress of file downloading(upto 2 decimal in percentage).
	->Done basic error handling in code.(Both on client and server side)
	->Also wrote some acknowledgement statements on both client and server side.
	->Added some sleep statments for synchronization
