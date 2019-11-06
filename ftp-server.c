// Partly taken from https://www.geeksforgeeks.org/socket-programming-cc/

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 

#include <string.h> 
#include <pthread.h>
#include <dirent.h>

#include <sys/socket.h> 
#include <sys/types.h>
#include <netinet/in.h> 

void *worker (void * arg)
{
	int conn ;
	char buf[1024] ;

	char * data = 0x0, * orig = 0x0 ;
	int len = 0 ;
	int s ;

	conn = *((int *)arg) ; // 이 정수가 가리키는 곳 의 정수를 conn에 넣는다. 
	free(arg) ;
	
	printf("[WORKER IN]\n");
	while ( (s = recv(conn, buf, 1023, 0)) > 0 ) {
		buf[s] = 0x0 ;
		if (data == 0x0) {
			data = strdup(buf) ;
			len = s ;
		}
		else {
			data = realloc(data, len + s + 1) ;
			strncpy(data + len, buf, s) ;
			data[len + s] = 0x0 ;
			len += s ;
		}
	}
	//받은게 data에 저장이 되죠.
	printf(">%s\n", data) ;

	//받은 명령이 list면
	if (strcmp(data, "#list") == 0) {
		DIR * d ;
		d = opendir(".") ;
		if (d == 0x0) {
			perror("fail") ;
			exit(EXIT_FAILURE) ;
		}

		struct dirent * e ;
		//readdir == opendir
		for (e = readdir(d) ; e != 0x0 ; e = readdir(d)) {
			if (e->d_type == DT_REG) {
				//printf("%s\n", e->d_name) ;
				char * orig ;
				data = malloc(sizeof(char) * (strlen(e->d_name) + 2)) ;
				strcpy(data, e->d_name) ;
				strcat(data, "\n") ;
				len = strlen(data) ;
				orig = data ;
				printf("> %s", data) ;
				while (len > 0 && (s = send(conn, data, len, 0)) > 0) {
					data += s ;
					len -= s ;
				}
				free(orig) ;
			}
		}
	}
	// 받은 명령이 겟이면, data에 보내야 할 파일이름 있음. 
	else if(strcmp(data, "get")==0)/* get file */ {
		FILE *fp;
        char buffer[1024];
        fp = fopen(data, "rb");

        if(fp == NULL){
            printf("File not found\n");
            //return 1;
            return 0x0;
        }
        else{
            printf("Found file!!\n");

            //////read data into buffer
            bzero(buffer, 1024);
            int R;
            while((R = fread(buffer, sizeof(char), sizeof(buffer), fp)>0)){
                if(send(/*thread생성 시 파라미터로 new_socket보내줌 그거 conn으로 받음*/conn, buffer, R, 0)){
                    printf("ERROR\n");
                    exit(1);
                }
                bzero(buffer, 1024);
            }
            // 확인 ㄱ
            printf("Ok sent to client!! :)\n");
            close(conn);
            printf("[Server] Connection with Client closed. \n");
        }


	}

	shutdown(conn, SHUT_WR) ;
	return 0x0 ;
}

int main (int argc, char const *argv[]) 
{ 
	int listen_fd, new_socket ; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	
	if (argc != 2) {
		fprintf(stderr, "Wrong number of arguments") ;
		exit(EXIT_FAILURE) ;
    }

	char buffer[1024] = {0}; 

    printf("I'm server.\n");
	listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  {
		perror("socket failed : "); 
		exit(EXIT_FAILURE); 
	}
	
	//port number 분리해주기. 
	char first[1024];
	strcpy(first, argv[1]);

	char *ptr = strtok(first, ":");
	char args1[16]/*IP*/, args2[16]/*port#*/;
	for(int i=0; i<2; i++){
		if(i==0){
			strcpy(args1, ptr);
		}
		else if(i==1){
			strcpy(args2, ptr);
		}

		if(ptr==NULL) { break; }
		printf("%s\n", ptr);
		ptr = strtok(NULL, ":");
	}
	memset(&address, '0', sizeof(address)); 

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ;
	printf("[INADDR] %s\n", INADDR_ANY);
	address.sin_port = htons(atoi(args2)); 

	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : "); 
		exit(EXIT_FAILURE); 
	} 


    printf("It is bound\n");
	while (1) {
		printf("2\n");
		if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
			perror("listen failed : "); 
			exit(EXIT_FAILURE); 
		} 

		new_socket = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen) ;
		if (new_socket < 0) {
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 

        // accept해서 받아온 값을 new_socket에 넣어준다. 
		pthread_t worker_tid ;
		int * worker_arg = (int *) malloc(sizeof(int)) ;
		*worker_arg = new_socket ;

		pthread_create(&worker_tid, 0x0, worker, (void *) worker_arg) ;
	}
} 

