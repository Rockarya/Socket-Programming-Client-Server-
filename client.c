// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define PORT 8000

int main()
{
    struct sockaddr_in address;
    int sock = 0, valread,i,l,fd,current_size,total_bytes,k;
    struct sockaddr_in serv_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a 
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("Connection Failed \n");
        return -1;
    }



    printf("Connected to the server :-)\n\n");
    while(1)
    {
        printf("client> ");
        char *command=NULL,*token=NULL,**str=NULL,string[10005]={'\0'};
        size_t comsize=10005;            //It’s an unsigned integral type which is used to represent the size of objects in bytes
        command=(char *)malloc(comsize*sizeof(char));
        getline(&command,&comsize,stdin);
        command[strlen(command) -1]='\0';
        strcpy(string,command);
        token=strtok(command," \t");       //By writing " \t" delimiter it will tokenize with both space bar and tab

        if(strcmp("get",token)==0)
        {
            send(sock,string,strlen(string),0);     //sending the whole string to server(So that server knows what files it has to download)

            token=strtok(NULL," \t");      //starting loop after get token
            l=0;
            str=(char **)malloc(100*sizeof(char));
            while(token!=NULL)              //running loop for all the file names
            {
                str[l]=(char *)malloc(1000*sizeof(char));
                strcpy(str[l],token);
                l++;
                token=strtok(NULL," \t");
            }

            //downloading files here
            for(i=0;i<l;i++)
            {
                sleep(1);       //These sleeps are just for synchronization purpose 
                char ack[105]={'\0'};
                read(sock,ack,sizeof(ack));
                if(strcmp(ack,"YES")==0)
                {
                    char dmsg[10005]={'\0'};
                    read(sock,dmsg,sizeof(dmsg));       //download acknowledgement msg from the server
                    printf("%s\n",dmsg);

                    fd=open(str[i],O_RDWR , 00400 | 00200);    

                    if(fd>0)
                    {
                        printf("File %s already exists! Appending it now!\n",str[i]);
                        fd=open(str[i],O_RDWR | O_APPEND , 00400 | 00200);         //Appending a file if it already exists
                    }
                    else
                    {
                        fd=open(str[i],O_RDWR | O_CREAT, 00400 | 00200);    
                    }
      

                    char offstr[105]={'\0'};
                    read(sock,offstr,sizeof(offstr));
                    total_bytes=atoi(offstr);
                    if(total_bytes!=0)
                    {
                        current_size=0;   
                        lseek(fd,0,SEEK_SET);      //By this you will get to starting of the file. 
                        while(current_size!=total_bytes)
                        {
                            char buffer[100005]={'\0'};
                            read(sock,buffer,sizeof(buffer));

                            write(fd,buffer,strlen(buffer));
                            current_size+=strlen(buffer);

                            send(sock,ack,strlen(string),0);

                            char progress[105]={'\0'},ch='%';
                            long double percentage;
                            percentage=(long double)current_size/total_bytes;
                            percentage*=100.0;
                            sprintf(progress,"%0.2Lf%c",percentage,ch); 
                            printf("%s\r",progress);        // /r is carriage return
                            fflush(stdout);     
                        }
                    }
                    else
                    {
                        char ch='%';
                        printf("100.00%c",ch);
                    }
                    printf("\n");
                    close(fd);
                }
                char msg[10005]={'\0'};
                read(sock,msg,sizeof(msg));
                printf("%s\n",msg);
            }
        }
        else if(strcmp("exit",token)==0)
        {
            printf("Disconnected from the server :-o\n");
            break;
        }
        else 
        {
            printf("Error in your command!\n");
        }
    }
    return 0;
}