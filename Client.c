/* Gil Yaakov
*  Date: 14/12/2020
*  File name: Client.c
*  Description: This program receives as input pid of 
*               calculation server and sends to the server
*               an operator (+,-,*,/) and two numbers -
*               the server sends back the answer of the 
*               calculation operation on the two numbers
*               with the operator. The program prints the answer.
*/

#include <stdio.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#define SIZE 50 /*I assumed the size of the result won't be more than 50 bytes*/

void sig_read_client(int num){
                     
                     int pid,status;
                     /***********CREAT THE NAME OF THE FILE to_client_xxxx.txt**********/
                     char str[11] = "to_client_";
                     int mypid = getpid();
                     char client_pid_c[6];
                     sprintf(client_pid_c, "%d", mypid);
                     strcat(str,client_pid_c);
                     strcat(str,".txt");
                     /******************************************************************/
                     if(pid=fork()==0){ /*Creat a procces*/
                        int fd_to_client = open(str,O_RDONLY); /*Open fd for to_client_xxxx.txt*/
                        if(fd_to_client==-1){
                           perror("open");
                           exit(1);
                          }
                        char to_client_buf[SIZE+1]; /*Buffer for the content of to_client_xxxx.txt*/       
                        int to_client_read = read(fd_to_client, to_client_buf, SIZE+1); /*Read the result*/
                        if(to_client_read==-1){
                           perror("read");
                           exit(1);
                           }
                        close(fd_to_client);
                        char *result = strtok(to_client_buf, ",");
                        printf("My pid is: %d and my result is: %s\n", mypid, result); /*Print the result to the screen*/
                        if(pid=fork()==0){ /*Crear grandson*/
                          execl("/bin/rm","rm",str,NULL); /*Delete the temp file to_client_xxxx.txt*/
                          }
                        waitpid(pid,&status,NULL);
                        }
                     else{
                          waitpid(pid,&status,NULL);
                         }
                     exit(0);
                     }

int main(int argc,char* argv[]){
        
         signal(SIGUSR1, sig_read_client); /*Signal registeration*/
        /**********PARAMETERS CHECKS**********/        
        if(argc<5){
           write(1, "ERROR: there aren't enough parameters\n",39);
           exit(1);  
           }
        if(argc>5){
           write(1, "ERROR: there are to much parameters\n",37);
           exit(1);
           }
        char P3 = *argv[3];
        if((P3!='1')&&(P3!='2')&&(P3!='3')&&(P3!='4')){
           write(1, "ERROR: there isn't such operation\n", 35);
           exit(1);
           }
        int j;
        long val;
        char *next;
        /*Procces each argument*/
        for(j = 1; j < argc; j++){
            val = strtol(argv[j],&next,10);
            /*Check for empty string and characters left after conversion*/
            if((next == argv[j]) || (*next != '\0')){
               printf("'%s' is not valid\n", argv[j]);
               exit(1);
               }
            }
        /*Check if we divide by zero*/
        if((P3 == '4')&&(*argv[4]=='0')){
           write(1,"ERROR: divid by zero\n", 22);
           exit(1);
           }
        /*************************************/
        int temp;
        sscanf(argv[1],"%d", &temp); /*Casting from string to int*/
        pid_t srv_pid = temp; /*Server pid*/
        pid_t client_pid = getpid(); /*Client pid*/        
        /*******CHECK IF to_srv EXSIST********/
        int i;
        int success_flag = 0;
        srand(time(0));
        int wait_time;
        int fd_to_srv;
        for(i=0;i<10;++i)
        {
            if((fd_to_srv = open("to_srv.txt", O_RDWR|O_CREAT|O_APPEND|O_EXCL, 0666))== -1){ /*If it exsist*/
                wait_time = rand() % 5;
                wait_time += 1;
                sleep(wait_time);
               }
            else{ /*If it not exsist*/
                success_flag = 1;
                break;
                }
        }
        if(success_flag == 0){
           write(1,"ERROR: server is busy\n", 23);
           close(fd_to_srv);
           exit(1); 
           }
        /****OPEN to_srv.txt & WRITE TO IT****/
        int pid,status;
        if(pid=fork()==0){ /*Creat a procces*/
           dup2(fd_to_srv,1);
           close(fd_to_srv);
           printf("%d", client_pid);
           printf(",");
           printf("%s", argv[2]);
           printf(",");
           printf("%c", *argv[3]);
           printf(",");
           printf("%s", argv[4]);
           printf(",");  
           }
        else{
             waitpid(pid,&status,NULL); /*Waiting for the son to finish*/
             close(fd_to_srv);
             if(kill(srv_pid, SIGUSR2)==-1){ /*Send a signal to to server we are done*/
                printf("ERROR: There is no such pid\n");
                if(pid=fork()==0){
                   execl("/bin/rm","rm","to_srv.txt",NULL); /*Delete the file*/
                   }
                waitpid(pid,&status,NULL);
                exit(1);
                }
             pause(); /*Waiting to answer from the server*/
             }
        exit(0);
}

