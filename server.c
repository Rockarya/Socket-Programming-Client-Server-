#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 8000

const int N=3e4;		//Max allowed size of buffer is around 32k 
int main()
{
    int server_fd,fd, new_socket,i,l,total_bytes,count,current_size,offset,f;
    struct sockaddr_in address;  
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    


    printf("Client is connected to the server!\n\n");
    while(1)
    {
        char command[10005] = {'\0'},*token=NULL,str[100][1005];
        read(new_socket,command,sizeof(command));
        token=strtok(command," \t");
        token=strtok(NULL," \t");       //first word will be definetly "get" so need to store it in str string

        l=0;
        while(token!=NULL)
        {
            strcpy(str[l],token);
            l++;
            token=strtok(NULL," \t");
        }

        //transfering each file now
        for(i=0;i<l;i++)
        {
            sleep(1);           //These sleeps are just for synchronization purpose
            char msg[1005]={'\0'},ack[105]={'\0'};
            fd=open(str[i],O_RDONLY,00400);

            if(fd<0)
            {
                strcpy(ack,"NO");
                send(new_socket,ack,strlen(ack),0);     //sending acknowledgement statement about existence of file

                sprintf(msg,"File %s doesn't exits in Server's directory :-(\n",str[i]);

                //Just a feedback statement
                printf("File %s requested by client doesn't exist in Server's directory :-o\n\n",str[i]);
            }
            else
            {
                strcpy(ack,"YES");
                send(new_socket,ack,strlen(ack),0);     //sending acknowledgement statement about existence of file

                char dmsg[10005]={'\0'};
                sprintf(dmsg,"Downloading started of file %s",str[i]);
                send(new_socket,dmsg,strlen(dmsg),0);
                printf("%s\n",dmsg);
                
                offset=lseek(fd, 0, SEEK_END);    //finding the size of the file

                char offstr[105]={'\0'};
                sprintf(offstr,"%d",offset);
                send(new_socket,offstr,strlen(offstr),0);

                total_bytes=offset;
                count=0;
                current_size=0;
                lseek(fd,0,SEEK_SET);      //By this you will get to starting of the file.
                f=1;
                while(offset>0 && f==1)      
                { 
                    f=0;
                    char buffer[1000005]={'\0'};
                    if(offset<N)
                    {
                        read(fd,buffer,offset);
                        send(new_socket,buffer,strlen(buffer),0);
                        current_size+=offset;
                        offset=0;
                    }
                    else
                    {
                        offset-=N;
                        read(fd,buffer,N);
                        send(new_socket,buffer,strlen(buffer),0);
                        current_size+=N;
                        count++;
                        lseek(fd,count*N,SEEK_SET);	//Changing the pointer of the file after reading first N bytes of file(DO NOT USE SEEK_CUR!!!)
                    }

                    char new_ack[105]={'\0'};       
                    read(new_socket,new_ack,sizeof(ack));       //These msgs are just to synchronize reading and writing from client and server side
                    if(strcmp(new_ack,"YES")==0)
                    {
                        f=1;
                    }

                    char progress[105]={'\0'},ch='%';
                    long double percentage;
                    percentage=(long double)current_size/total_bytes;
                    percentage*=100.0;
                    sprintf(progress,"%0.2Lf%c",percentage,ch);  

                    printf("%s\r",progress);        // /r is carriage return
                    fflush(stdout);    
                }

                sprintf(msg,"File %s downloaded succesfully in Client's directory :-)\n",str[i]);
                printf("\n%s\n",msg);
            }
            //sending acknowledgement statement about file
            sleep(0.1);
            send(new_socket,msg,strlen(msg),0);
            close(fd);
        }
    }
    return 0;
}
