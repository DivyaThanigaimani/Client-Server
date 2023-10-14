//Sec Id:2
//Std Name:Susmitha Chandramohan Std Id:110100921
//Std Name:Divya Thanigaimani Std Id:110097685
//Grp Id:15

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>

#define PORT 8888
#define MAX_PATH 1024
#define MIRROR_PORT 8800

void processclient(int new_socket);
char *filesrch(char *dirName, char *targetFile);
void backupSize(const char *root_dir, int minSize, int maxSize);
void filesrchDate(char *dirPath, char *startTime, char *endTime);
void filesrchName(char *dirName, char *targetFile[], int num);
void filesrchType(char *dirName, char *targetFile[], int num);
void mirrorHandler(int new_socket);
int commandCheck(char *command);

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int clientCheck = 0; //keep track of the number of clients connected
    // create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set the server structure
    //reuse of the same address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; //connect to any available network
    address.sin_port = htons(PORT);

    // bind the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // listening for incoming connection
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // waiting for connect
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Client connected.\n");

        // handle requirement
        pid_t pid;
        pid = fork();
        int mirrorCheck = (clientCheck/6)%2;
        if(pid != 0)
        {
            close(new_socket);
            clientCheck++;
        }
        else
        {
            if(mirrorCheck == 1)
            {
                printf("go Mirror.\n");
                mirrorHandler(new_socket);
                exit(0);
            }
            printf("go Server.\n");
            processclient(new_socket);
            exit(0);
        }
    }

    return 0;
}

void processclient(int new_socket) {
    char command[1024] = {0};
    char buffer[1024] = {0};

    while (1) {
        // receive
        memset(command, 0, 1024);
        int valread = read(new_socket, command, 1024);
        if (commandCheck(command) == 0) {
            printf("Client disconnected.\n");
            break;
        }

        printf("Filename received: %s\n", command);

        if(commandCheck(command) == 1)
        {
            //printf("Here\n");
            char *name = strtok(command, " ");
            //printf("01 %s\n", name);
            name = strtok(NULL, " ");// continues tokennization to get the file name
            printf("02 %s\n", name);
            /*printf("02e %s02\n", name);
            if(strcmp(name, "EB.txt") == 0)
            {
                printf("here\n");
            }*/
            char *receive = filesrch(getenv("HOME"), name);
            puts(receive);
            send(new_socket, receive, strlen(receive), 0);
        }
        else if(commandCheck(command) == 2)
        {
            char *size = strtok(command, " ");
            puts(size);
            char *minSize = (char *)malloc(1024*sizeof(char));
            char *maxSize = (char *)malloc(1024*sizeof(char));
            size = strtok(NULL, " ");
            puts(size);
            strcpy(minSize, size);
            puts(size);
            size = strtok(NULL, " ");
            strcpy(maxSize, size);
            printf("01 %s\n", minSize);
            printf("02 %s\n", maxSize);
            backupSize(getenv("HOME"), atoi(minSize), atoi(maxSize));
            system("gzip temp.tar");

            FILE *fp;
            fp = fopen("temp.tar.gz", "rb");
            if (fp)
            {
                size_t bytesRead = 0;
                while ((bytesRead = fread(buffer, 1, 1024, fp)) > 0) {
                    printf("Here %d\n", bytesRead);
                    send(new_socket, buffer, bytesRead, 0);
                }
                //send(new_socket, "END_OF_TRANSMISSION", strlen("END_OF_TRANSMISSION"), 0);
                printf("Send end\n");
                fclose(fp);
            }
        }
        else if(commandCheck(command) == 3)
        {
            //printf("Here\n");
            char *date = strtok(command, " ");
            puts(date);
            char *startDate = (char *)malloc(1024*sizeof(char));
            char *endDate = (char *)malloc(1024*sizeof(char));
            date = strtok(NULL, " ");
            puts(date);
            strcpy(startDate, date);
            puts(date);
            date = strtok(NULL, " ");
            strcpy(endDate, date);
            printf("01 %s\n", startDate);
            printf("02 %s\n", endDate);
            filesrchDate(getenv("HOME"), startDate, endDate);
            system("gzip temp.tar");

            FILE *fp;
            fp = fopen("temp.tar.gz", "rb");
            if (fp)
            {
                size_t bytesRead = 0;
                while ((bytesRead = fread(buffer, 1, 1024, fp)) > 0) {
                    send(new_socket, buffer, bytesRead, 0);
                }
                printf("Send end\n");
                fclose(fp);
            }
        }
        else if(commandCheck(command) == 4)
        {
            //printf("Here\n");
            char *name = strtok(command, " ");
            char *nameList[4];
            int i=0;
            for(i=0; (name = strtok(NULL, " ")); i++)
            {
                nameList[i] = (char *)malloc(1024*sizeof(char));
                strcpy(nameList[i], name);
                printf("File is: %s to %s\n", name, nameList[i]);
            }
            if(i>4)
            {
                printf("Too many fils.\n");
                return ;
            }
            printf("num is: %d\n", i);
            //printf("01 %s\n", name);
            //name = strtok(NULL, " ");
            /*printf("02 %s\n", name);
            printf("02e %s02\n", name);
            if(strcmp(name, "EB.txt") == 0)
            {
                printf("here\n");
            }*/
            filesrchName(getenv("HOME"), nameList, i);
            system("gzip temp.tar");

            FILE *fp;
            fp = fopen("temp.tar.gz", "rb");
            if (fp)
            {
                size_t bytesRead = 0;
                while ((bytesRead = fread(buffer, 1, 1024, fp)) > 0) {
                    send(new_socket, buffer, bytesRead, 0);
                }
                printf("Send end\n");
                fclose(fp);
            }
        }
        else if(commandCheck(command) == 5)
        {
            //printf("Here\n");
            char *type = strtok(command, " ");
            char *typeList[4];
            int i=0;
            for(i=0; (type = strtok(NULL, " ")); i++)
            {
                typeList[i] = (char *)malloc(1024*sizeof(char));
                strcpy(typeList[i], type);
                printf("File is: %s to %s\n", type, typeList[i]);
            }
            if(i>4)
            {
                printf("Too many fils.\n");
                return;
            }
            printf("num is: %d\n", i);
            //printf("01 %s\n", name);
            //name = strtok(NULL, " ");
            /*printf("02 %s\n", name);
            printf("02e %s02\n", name);
            if(strcmp(name, "EB.txt") == 0)
            {
                printf("here\n");
            }*/
            filesrchType(getenv("HOME"), typeList, i);
            system("gzip temp.tar");

            FILE *fp;
            fp = fopen("temp.tar.gz", "rb");
            if (fp)
            {
                size_t bytesRead = 0;
                while ((bytesRead = fread(buffer, 1, 1024, fp)) > 0) {
                    send(new_socket, buffer, bytesRead, 0);
                }
                printf("Send end\n");
                fclose(fp);
            }
        }
        else if(commandCheck(command) == 0)
        {
            break;
        }
    }

    close(new_socket);
}

char *filesrch(char *dirName, char *targetFile)
{
    /*if(a < 5)
    {
        a++;
        printf("%s\n", targetFile);
    }*/
    DIR *dir = opendir(dirName);
    if (dir == NULL) {
        perror("opendir");
        return "Error";
    }
    //int check = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // skip "." and ".." FOLDER & FILE traverse
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || entry->d_name[0]=='.')
        {
            continue;
        }

        // structure path
        char path[1024];
        sprintf(path, "%s/%s", dirName, entry->d_name);//copies the direct info to path

        struct stat sb;
        if (lstat(path, &sb) == -1)//path status stored in sb
        {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(sb.st_mode))
        {
            //printf("Found directory: %s\n", path);
            // recursion
            char *result = filesrch(path, targetFile);
            if (result != NULL)
            {
                closedir(dir);
                return result;
            }
        }
        else
        {
            //printf("Found file: %s\n", path);
            if(strcmp(targetFile, entry->d_name) == 0)
            {
                char *fileInfo = (char *)malloc(1024*sizeof(char));
                struct stat sb;
                stat(path, &sb);
                strcpy(fileInfo, "Find ");
                strcat(fileInfo, entry->d_name);
                strcat(fileInfo, " at ");
                strcat(fileInfo, path);
                strcat(fileInfo, "\n");
                strcat(fileInfo, "The size of the file is: ");
                char num[1024];
                sprintf(num, "%d", sb.st_size);
                strcat(fileInfo, num);
                strcat(fileInfo, " Bytes.\n");
                strcat(fileInfo, "Created at: ");
                strcat(fileInfo, ctime(&sb.st_ctime));
                //strcat(fileInfo, );
                printf("Find %s at %s.\n", entry->d_name, path);
                printf("The size of the file is: %d.\n", sb.st_size);
                printf("Created at: %s", ctime(&sb.st_ctime));
                printf("The strinf is:\n%s\n", fileInfo);
                //check++;
                return fileInfo;
            }
        }
    }
    //if(check == 1)
        //printf("%d\n", check);
    closedir(dir);
    return NULL;
}

void backupSize(const char *root_dir, int minSize, int maxSize) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char path[MAX_PATH];
    int file_size;

    dir = opendir(root_dir);
    if (dir == NULL) {
        fprintf(stderr, "Error opening directory %s\n", root_dir);
        exit(1);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || entry->d_name[0]=='.')
        {
            continue;
        }

        snprintf(path, MAX_PATH, "%s/%s", root_dir, entry->d_name);

        if (lstat(path, &st) == -1) {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            printf("Found directory: %s\n", path);
            backupSize(path, minSize, maxSize);
            //tempCheck = num;
        } else if (S_ISREG(st.st_mode)) {
            file_size = st.st_size;
            if (file_size >= minSize && file_size <= maxSize) {
                printf("Found file: %s, size: %d bytes\n", path, file_size);
                // add file to tar file
                char *tar_cmd = (char *)malloc(MAX_PATH*sizeof(char));
                sprintf(tar_cmd, "tar -rvf temp.tar %s", path);
                system(tar_cmd);
                free(tar_cmd);
            }
        }
    }

    closedir(dir);
}

void filesrchDate(char *dirPath, char *startTime, char *endTime)
{
    DIR *dir = opendir(dirPath);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || entry->d_name[0]=='.')
        {
            continue;
        }

            //printf("Here\n");
        // construct the full path of the file
        char filePath[1024];
        sprintf(filePath, "%s/%s", dirPath, entry->d_name);

        struct stat sb;
        if (lstat(filePath, &sb) == -1)
        {
            perror("lstat");
            continue;
        }

        // if the file is in the range
        struct tm *createTime = localtime(&sb.st_ctime);
        char createTimeStr[80];
        strftime(createTimeStr, 80, "%Y-%m-%d", createTime);
        //puts(createTimeStr);
        if (S_ISDIR(sb.st_mode))
        {
            // recursion the child
            filesrchDate(filePath, startTime, endTime);
        }
        else if (strcmp(createTimeStr, startTime) >= 0 && strcmp(createTimeStr, endTime) <= 0)
        {
            printf("as %s\n", filePath);
            char *tar_cmd = (char *)malloc(1024*sizeof(char));
            sprintf(tar_cmd, "tar -rvf temp.tar %s", filePath);
            system(tar_cmd);
            free(tar_cmd);
        }


    }

    closedir(dir);
}

void filesrchName(char *dirName, char *targetFile[], int num)
{
    /*if(a < 5)
    {
        a++;
        printf("%s\n", targetFile);
    }*/
    int found = 0;
    DIR *dir = opendir(dirName);
    if (dir == NULL) {
        perror("opendir");
        return;
    }
    //int check = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || entry->d_name[0]=='.')
        {
            continue;
        }

        // structure path
        char path[1024];
        sprintf(path, "%s/%s", dirName, entry->d_name);

        struct stat sb;
        if (lstat(path, &sb) == -1)
        {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(sb.st_mode))
        {
            //printf("Found directory: %s\n", path);
            // recursion
            filesrchName(path, targetFile, num);
        }
        else
        {
            //printf("Found file: %s\n", path);
            for(int i=0; i<num; i++)
            {
                if(strcmp(targetFile[i], entry->d_name) == 0)
                {
                    struct stat sb;
                    stat(path, &sb);

                    //strcat(fileInfo, );

                    printf("Find %s at %s.\n", entry->d_name, path);
                    printf("The size of the file is: %d.\n", sb.st_size);
                    printf("Created at: %s", ctime(&sb.st_ctime));

                    char *tar_cmd = (char *)malloc(1024*sizeof(char));
                    sprintf(tar_cmd, "tar -rvf temp.tar %s", path);
                    system(tar_cmd);
                    free(tar_cmd);
                    //check++;
                }
            }

        }
    }
    //if(check == 1)
        //printf("%d\n", check);
    closedir(dir);
}

void filesrchType(char *dirName, char *targetFile[], int num)
{
    /*if(a < 5)
    {
        a++;
        printf("%s\n", targetFile);
    }*/
    int found = 0;
    DIR *dir = opendir(dirName);
    if (dir == NULL) {
        perror("opendir");
        return;
    }
    //int check = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || entry->d_name[0]=='.')
        {
            continue;
        }

        // structure path
        char path[1024];
        sprintf(path, "%s/%s", dirName, entry->d_name);

        struct stat sb;
        if (lstat(path, &sb) == -1)
        {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(sb.st_mode))
        {
            //printf("Found directory: %s\n", path);
            // recursion
            filesrchType(path, targetFile, num);
        }
        else
        {
            //printf("Found file: %s\n", path);
            for(int i=0; i<num; i++)
            {
                //char *dot
                char *dot;
                if((dot = strrchr(entry->d_name, '.')) == NULL)//check for extension,if no exten,go to other file.
                {
                    continue;
                }
                printf("%s ", path);
                if(strcmp(dot, targetFile[i]) == 0)
                {
                    struct stat sb;
                    stat(path, &sb);

                    //strcat(fileInfo, );

                    printf("\nFind %s at %s.\n", entry->d_name, path);
                    printf("The size of the file is: %d.\n", sb.st_size);
                    printf("Created at: %s", ctime(&sb.st_ctime));

                    char *tar_cmd = (char *)malloc(2048*sizeof(char));
                    sprintf(tar_cmd, "tar -rvf temp.tar %s", path);//append the file in the path to temp.tar
                    system(tar_cmd);//execute the command
                    free(tar_cmd);
                    break;
                    //check++;
                }
                //free(dot);
            }

        }
    }
    //if(check == 1)
        //printf("%d\n", check);
    closedir(dir);
}

int commandCheck(char *command)
{
	char *copy = (char *)malloc(1024*sizeof(char));
	strcpy(copy, command);
	char *mainCommand = strtok(copy, " ");
	if(strcmp(mainCommand, "filesrch") == 0)
	{
        //printf("filesrch\n");
		return 1;
	}
	if(strcmp(mainCommand, "tarfgetz") == 0)
	{
        printf("tarfgetz\n");
		return 2;
	}
    if(strcmp(mainCommand, "getdirf") == 0)
	{
        printf("getdirf\n");
		return 3;
	}
    if(strcmp(mainCommand, "fgets") == 0)
	{
        printf("fgets\n");
		return 4;
	}
    if(strcmp(mainCommand, "targzf") == 0)
	{
        printf("targzf\n");
		return 5;
	}
    if(strcmp(mainCommand, "quit") == 0)
    {
        printf("quit\n");
        return 0;
    }
    return -1;
}

void mirrorHandler(int new_socket)
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char command[1024] = {0};
    char buffer[1024] = {0};

    // create client socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    // set server structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(MIRROR_PORT);

    // transform IP
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    // link to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return;
    }

    for(;;)
    {
        //check if the socket is closed
        fd_set read_fds;
        struct timeval timeout;
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        int sel = select(sock + 1, &read_fds, NULL, NULL, &timeout);
        if (sel == -1)
        {
            perror("Select");
        }
        else if(sel == 0)
        {
            printf("socket closed\n");
            //break;
        }

        // get the commandmemset(command, 0, 1024);
        int valread = read(new_socket, command, 1024);
        if (commandCheck(command) == 0)
        {
            printf("Client disconnected.\n");
            break;
        }
        if(strcmp(command, "quit") == 0)
        {
            break;
        }

        int check = commandCheck(command);
        if(check == -1)
        {
            continue;
        }

        // send command to server
        send(sock, command, strlen(command), 0);
        printf("command sent: %s\n", command);

        // receive and print
        printf("Here\n");
        char *commandCopy = (char *)malloc(1024*sizeof(char));
        strcpy(commandCopy, command);
        char *mainC = strtok(commandCopy, " ");
        if(strcmp(mainC, "filesrch") == 0)
        {
            printf("filesrch\n");
            valread = read(sock, buffer, 1024);
            puts(buffer);

            send(new_socket, buffer, strlen(buffer), 0);
        }
        else
        {
            printf("Start\n");
            FILE *fp;
            fp = fopen("temp.tar.gz", "wb");
            while ((valread = read(sock, buffer, 1024)) > 0)
            {

                /*int flags = fcntl(sock, F_GETFL, 0);
                flags |= O_NONBLOCK;
                fcntl(sock, F_SETFL, flags);*/

                printf("in loop: %d\n", valread);
                int a = 0;
                //a = fwrite(buffer, sizeof(char), valread, fp);
                send(new_socket, buffer, valread, 0);
                printf("Here %d\n", a);
                memset(buffer, 0, sizeof(buffer));
                if(valread<1024)
                {
                    break;
                }
            }
            printf("end loop colcul %d\n", valread);
            fclose(fp);
        }
        printf("\n");
        memset(command, 0, sizeof(command));
    }

    return;
}
