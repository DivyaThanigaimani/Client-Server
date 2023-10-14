//Sec Id:2
//Std Name:Susmitha Chandramohan Std Id:110100921
//Std Name:Divya Thanigaimani Std Id:110097685
//Grp Id:15
#include <stdio.h>// Provides functions for input and output operations.
#include <stdlib.h>//
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>//Contains definitions for functions that deal with IP address conversion and manipulation.
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <zlib.h>//Contains declarations for functions and data types related to the zlib compression library.

#define PORT 8888 //represents the port number the program will use to connect to the server.
#define MAX_CMD 65535

int commandsValidation(char *command);
void unZipTarFile();

int main(int argCount, char const *argValue[]) {
    int clientSock = 0, serverResRead;
    struct sockaddr_in serv_addr;
    char command[1024] = {0};
    char buffer[1024] = {0};

    // create client socket
    // the parameter AF_INET indicates that the socket will use the IPv4 address family. This means that the socket will be configured to work with IPv4 addresses for communication.
    //SOCK_STREAM socket type, it indicates that the socket will use TCP for communication. -reliable, ordered, and connection-oriented data exchange,
    if ((clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // set server structure
    serv_addr.sin_family = AF_INET;
    //convert from  host byte order to network byte order,to maintain consistency across diff platforms
    serv_addr.sin_port = htons(PORT);

    // transform IP
    //string fromart to binary=>inet_ptron client socket is attempting to connect to a server running on the same machine as the client
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // link to server
    if (connect(clientSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    for(;;)
    {
        // get the command
        printf("Enter command: ");
        fgets(command, 100, stdin);
        command[strcspn(command, "\n")] = '\0';
        if(strcmp(command, "quit") == 0)
        {
            break;
        }

        int isCommandValid = commandsValidation(command);
        if(isCommandValid == -1)
        {
            continue;
        }

        // send command to server
        send(clientSock, command, strlen(command), 0);
        printf("command sent: %s\n", command);

        // receive and print
        printf("Here\n");
        char *commandCopy = (char *)malloc(1024*sizeof(char));
        strcpy(commandCopy, command);
        char *commandName = strtok(commandCopy, " ");
        if(strcmp(commandName, "filesrch") == 0)
        {
            printf("filesrch\n");
            serverResRead = read(clientSock, buffer, 1024);
            puts(buffer);
        }
        else
        {
            printf("Start\n");
            FILE *localFilePointer;
            localFilePointer = fopen("temp.tar.gz", "wb");
            while ((serverResRead = read(clientSock, buffer, 1024)) > 0)
            {
         
                printf("in loop: %d\n", serverResRead);
                int writeBytes = 0;
                writeBytes = fwrite(buffer, sizeof(char), serverResRead, localFilePointer);

                printf("Here %d\n", writeBytes);
                memset(buffer, 0, sizeof(buffer)); //clears the buffer for next iteration
                if(serverResRead<1024)
                {
                    break;
                }
            }
            printf("Total Bytes Read %d\n", serverResRead);
            fclose(localFilePointer);
        }

        char *splitCommand = (char *)malloc(1024*sizeof(char));
        char *lastArgument = (char *)malloc(1024*sizeof(char));
        strcpy(commandCopy, command);
        printf("The command is: %s\n", commandCopy);
        splitCommand = strtok(commandCopy, " ");
        for(int incVar=0;(splitCommand = strtok(NULL, " "));incVar++)
        {
            strcpy(lastArgument, splitCommand);
            printf("Here %s\n", splitCommand);
        }
        puts(lastArgument);
        if(strcmp(lastArgument, "-u") == 0)
        {
            printf("Unzip\n");
            unZipTarFile();
        }
        printf("\n");
    }

    return 0;
}

int commandsValidation(char *command)
{
    char *commandCopy = (char *)malloc(1024*sizeof(char));
    strcpy(commandCopy, command);

    char * mainCommand = strtok(commandCopy, " ");
    printf("%s\n", mainCommand);
    if(strcmp(mainCommand, "filesrch") == 0)
    {
        int incVar=0;
        for(incVar=0; (mainCommand = strtok(NULL, " ")); incVar++)
        {

        }
        if(incVar>1)
        {
            printf("Too many arguments. The input form is: filesrch filename\n");
            return -1;
        }
        else if(incVar<1)
        {
            printf("Please enter the file name.\n");
            return -1;
        }
        return 1;
    }
    else if(strcmp(mainCommand, "tarfgetz") == 0)
    {
        int incVar=0;
        char *lastArgument = (char *)malloc(1024*sizeof(char));
        char *size1 = (char *)malloc(1024*sizeof(char));
        char *size2 = (char *)malloc(1024*sizeof(char));
        for(incVar=0; (mainCommand = strtok(NULL, " ")); incVar++)
        {
            puts(mainCommand);
            strcpy(lastArgument, mainCommand);
            if(incVar == 0)
            {
                size1 = lastArgument;
            }
            if(incVar == 1)
            {
                size2 = lastArgument;
            }
        }
        printf("%d\n", incVar);
        if(atoi(size1)>atoi(size2))
        {
            printf("minium size should less than maxium size.\n");
            return -1;
        }
        if(strcmp(lastArgument, "-u") == 0)
        {
            if(incVar>3)
            {
                printf("Too many arguments. The input form is: tarfgetz size1 size2 <-u>\n");
                return -1;
            }
            else if(incVar<3)
            {
                printf("Please enter the size range.\n");
                return -1;
            }
        }
        else if(incVar>2)
        {
            printf("Too many arguments. The input form is: tarfgetz size1 size2 <-u>\n");
            return -1;
        }
        else if(incVar<2)
        {
            printf("Please enter the size range.\n");
            return -1;
        }
        return 1;
    }
    else if(strcmp(mainCommand, "getdirf") == 0)
    {
        int incVar=0;
        char *lastArgument = (char *)malloc(1024*sizeof(char));
        char *date1 = (char *)malloc(1024*sizeof(char));
        char *date2 = (char *)malloc(1024*sizeof(char));
        for(incVar=0; (mainCommand = strtok(NULL, " ")); incVar++)
        {
            printf("%s\n", mainCommand);
            strcpy(lastArgument, mainCommand);
            if(incVar == 0)
            {
                date1 = lastArgument;
            }
            if(incVar == 1)
            {
                date2 = lastArgument;
            }
        }
        printf("%s\n", lastArgument);
        if(strcmp(date1, date2)>0)
        {
            printf("Date 1 should be less than Date 2.\n");
            return -1;
        }
        if(strcmp(lastArgument, "-u") == 0)
        {
            if(incVar>3)
            {
                printf("Too many arguments. The input form is: getdirf date1 date2 <-u>\n");
                return -1;
            }
            else if(incVar<3)
            {
                printf("Please enter the date range.\n");
                return -1;
            }
        }
        
        else if(incVar>2)
        {
            printf("Too many arguments. helloThe input form is: getdirf date1 date2 <-u>\n");
            return -1;
        }
        else if(incVar<2)
        {
            printf("Please enter the date range.\n");
            return -1;
        }
        return 1;
    }
    else if(strcmp(mainCommand, "fgets") == 0)
    {
        int incVar=0;
        char * lastArgument = (char *)malloc(1024*sizeof(char));
        for(incVar=0; (mainCommand = strtok(NULL, " ")); incVar++)
        {
            strcpy(lastArgument, mainCommand);
        }
        if(strcmp(lastArgument, "-u") == 0)
        {
            if(incVar>5)
            {
                printf("Too many arguments. The input form is: fgets file1 file2 file3 file4 <-u >\n");
                return -1;
            }
            else if(incVar<2)
            {
                printf("Please enter the file name.\n");
                return -1;
            }
        }
        else if(incVar>4)
        {
            printf("Too many arguments. The input form is: fgets file1 file2 file3 file4 <-u >\n");
            return -1;
        }
        else if(incVar<1)
        {
            printf("Please enter the file name.\n");
            return -1;
        }
        return 1;
    }
    else if(strcmp(mainCommand, "targzf") == 0)
    {
        int incVar=0;
        char * lastArgument = (char *)malloc(1024*sizeof(char));
        for(incVar=0; (mainCommand = strtok(NULL, " ")); incVar++)
        {
            strcpy(lastArgument, mainCommand);
        }
        if(strcmp(lastArgument, "-u") == 0)
        {
            if(incVar>5)
            {
                printf("Too many arguments. The input form is: targzf <extension list> <-u> //up to 4 different file types\n");
                return -1;
            }
            else if(incVar<2)
            {
                printf("Please enter the file type.\n");
                return -1;
            }
        }
        else if(incVar>4)
        {
            printf("Too many arguments. The input form is: targzf <extension list> <-u> //up to 4 different file types\n");
            return -1;
        }
        else if(incVar<1)
        {
            printf("Please enter the file type.\n");
            return -1;
        }
        return 1;
    }
    else
    {
        printf("Invliad input, please enter angain.\n");
        return -1;
    }
}

void unZipTarFile()
{
    const char *filename = "temp.tar.gz";
    char cmd[MAX_CMD];
    //-xzf flags indicate that the command is extracting (-x) while dealing with a compressed file (-z for gzip compression) and a filename is specified using %s
    snprintf(cmd, MAX_CMD, "tar -xzf %s", filename); //decompress the tar file
    FILE *localFilePointer = popen(cmd, "r");
    if (!localFilePointer) {
        printf("Failed to execute command\n");
        return;
    }
    pclose(localFilePointer);
    return;
}
