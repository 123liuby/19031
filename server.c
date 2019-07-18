#include<stdio.h>
#include <sys/types.h>      
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>

#include "system.h"

sqlite3 *db; 

int process_user_or_admin_login_request(int acceptfd,MSG *msg)
{
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;

	msg->info.usertype =  msg->usertype;
	strcpy(msg->info.name,msg->username);
	strcpy(msg->info.passwd,msg->passwd);

	printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{
		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
		}
	}
	return 0;	
}

int process_admin_query_request(int acceptfd,MSG *msg)
{
	char sql[DATALEN]={0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	int i;
	sprintf(sql,"select * from usrinfo;");
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg)!=SQLITE_OK)	{
			printf("---****----%s.\n",errmsg);		
	}else{
		int num=ncolumn;
		for(i=0;i<nrow;i++){
			msg->info.no=atoi(result[num]);
			msg->info.usertype=atoi(result[num+1]);
			strcpy(msg->info.name,(char *)result[num+2]);
			strcpy(msg->info.passwd,(char *)result[num+3]);
			msg->info.age=atoi(result[num+4]);
			strcpy(msg->info.phone,(char *)result[num+5]);
			strcpy(msg->info.addr,(char *)result[num+6]);
			strcpy(msg->info.work,(char *)result[num+7]);
			strcpy(msg->info.date,(char *)result[num+8]);
			msg->info.level=atoi(result[num+9]);
			msg->info.salary=atoi(result[num+10]);
			send(acceptfd,msg,sizeof(MSG),0);
			num=num+11;
		}
	}
		strcpy(msg->recvmsg,"ADMIN_QUERY");
		send(acceptfd,msg,sizeof(MSG),0);
		memset(msg->recvmsg,0,sizeof(msg->recvmsg));
}

int process_user_query_request(int acceptfd,MSG *msg)
{
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;

	sprintf(sql,"select * from usrinfo where name = '%s' and passwd = '%s';"\
			,msg->info.name,msg->info.passwd);

	int callback(void * para,int f_num,char ** f_value, char **f_name){
		msg->info.no = atoi(f_value[0]);
		msg->info.usertype = atoi(f_value[1]);
		strcpy(msg->info.name,f_value[2]);
		strcpy(msg->info.passwd ,f_value[3]);
		msg->info.age = atoi(f_value[4]);
		strcpy(msg->info.phone , f_value[5]);
		strcpy(msg->info.addr , f_value[6]);
		strcpy(msg->info.work , f_value[7]);
		strcpy(msg->info.date ,f_value[8]);
		msg->info.level = atoi(f_value[9]);
		msg->info.salary = atoi(f_value[10]);
	}
	if((sqlite3_exec(db,sql,callback,"select",&errmsg))!= SQLITE_OK){		
		printf("%s --failed to select table ---- USER_QUERY",errmsg);
	}
	msg->flags = 1;

	if((send(acceptfd,msg,sizeof(MSG),0)) < 0){
		printf("server failed to send ---USER_QUERY\n");
	}
}

int process_admin_modify_request(int acceptfd,MSG *msg)
{
	char *errmsg;
	char temp[10] = {0};
	printf("------%s-------%d\n",__func__,__LINE__);
	char sql[DATALEN] = {0};
	printf("modify user: %s\n",msg->info.name);
	switch(msg->flags)
	{
	case 1:
		strcpy(temp,"staffno");
		break;
	case 2:
		strcpy(temp,"usertype");
		break;
	case 3:
		strcpy(temp,"name");
		break;
	case 4:
		strcpy(temp,"passwd");
		break;
	case 5:
		strcpy(temp,"age");
		break;
	case 6:
		strcpy(temp,"phone");
		break;
	case 7:
		strcpy(temp,"addr");
		break;
	case 8:
		strcpy(temp,"work");
		break;
	case 9:
		strcpy(temp,"date");
		break;
	case 10:
		strcpy(temp,"level");
		break;
	case 11:
		strcpy(temp,"salary");
		break;
	default:
		printf("input error");
	}
	sprintf(sql,"update usrinfo set '%s' = '%s' where name = '%s';",temp,msg->recvmsg,msg->info.name);

	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
			printf("update failed,%s\n",errmsg);
	strcpy(msg->recvmsg,"modify ok");
	send(acceptfd,msg,sizeof(MSG),0);

	return 0;
}

int process_user_modify_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN]={0};
	char *errmsg;
	switch(msg->flags){
	case 1 :
		sprintf(sql,"update usrinfo set passwd = '%s' where staffno = %d;",msg->info.passwd, msg->info.no);
		printf("sql:%s\n",sql);
		break;
	case 2:
		sprintf(sql,"update usrinfo set age = %d where staffno = %d;",msg->info.age, msg->info.no);
		printf("sql:%s\n",sql);
		break;
	case 3:
		sprintf(sql,"update usrinfo set phone = '%s' where staffno = %d;",msg->info.phone, msg->info.no);
		printf("sql:%s\n",sql);
		break;
	case 4:
		sprintf(sql,"update usrinfo set addr = '%s' where staffno = %d;",msg->info.addr, msg->info.no);
		printf("sql:%s\n",sql);
		break;
	}

	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		printf("%s\n",errmsg);
		return -1;
	}else{
		printf("update success\n");
		strcpy(msg->recvmsg,"OK");
		msg->flags = 0;
	}
	if(send(acceptfd,msg,sizeof(MSG),0) <0){
		perror("server send failed\n");
		return -1;
	}
	return 0;
}

int process_client_quit_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	close(acceptfd);
	printf("Client QUIT.\n");
	return 0;
}

void historyinfo_insert(MSG * msg,char * words){

	time_t now;
	struct tm *tm_now;
	char date[128]={'0'};
	time(&now);

	tm_now=localtime(&now);
	sprintf(date,"%d-%d-%d %d:%d:%d",tm_now->tm_year+1900,tm_now->tm_mon+1,tm_now->tm_mday,\
			tm_now->tm_hour,tm_now->tm_min,tm_now->tm_sec);
	char * errmsg;
	char sql[DATALEN]={0};
	sprintf(sql,"insert into historyinfo values ('%s','%s','%s');",\
			date, msg->username, words);
	if((sqlite3_exec(db,sql,NULL,NULL,&errmsg))!=SQLITE_OK){
		printf("history failed -- %s\n",errmsg);
	}
	return ;
}

int process_client_request(int acceptfd,MSG *msg)
{
	switch(msg->msgtype)
	{
	case USER_LOGIN:
	case ADMIN_LOGIN:
		process_user_or_admin_login_request(acceptfd,msg);
		historyinfo_insert(msg,"LOGIN");
		break;
	case ADMIN_QUERY:
		process_admin_query_request(acceptfd,msg);
		historyinfo_insert(msg,"ADMIN_QUERY");
		break;
	case USER_QUERY:
		process_user_query_request(acceptfd,msg);
		historyinfo_insert(msg,"USER_QUERY");
		break;
	case ADMIN_MODIFY:
		process_admin_modify_request(acceptfd,msg);
		historyinfo_insert(msg,"ADMIN_MODIFY");
		break;
	case USER_MODIFY:
		process_user_modify_request(acceptfd,msg);
		historyinfo_insert(msg,"USER_MODIFY");
		break;
	case QUIT:
		process_client_quit_request(acceptfd,msg);
		break;
	default:
		break;
	}
}

int main(int argc, const char *argv[])
{
	//socket->填充->绑定->监听->等待连接->数据交互->关闭 
	int sockfd;
	int acceptfd;
	ssize_t recvbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);

	MSG msg;

	char *errmsg;

	if(sqlite3_open(STAFF_DATABASE,&db) != SQLITE_OK){
		printf("%s.\n",sqlite3_errmsg(db));
	}else{
		printf("the database open success.\n");
	}

	if(sqlite3_exec(db,"create table usrinfo(staffno integer,usertype integer,name text,passwd text,age integer,phone text,addr text,work text,date text,level integer,salary REAL);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create usrinfo table success.\n");
	}

	if(sqlite3_exec(db,"create table historyinfo(time text,name text,words text);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{ 
		printf("create historyinfo table success.\n");
	}

	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1){
			perror("socket failed.\n");
				exit(-1);
	}
	printf("sockfd :%d.\n",sockfd); 

	int b_reuse = 1;
	setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof (int));

	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port   = htons(5001);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(bind(sockfd, (const struct sockaddr *)&serveraddr,addrlen) == -1){
		printf("bind failed.\n");
		exit(-1);
	}

	if(listen(sockfd,10) == -1){
		printf("listen failed.\n");
		exit(-1);
	}

	fd_set readfds,tempfds;
	FD_ZERO(&readfds);
	FD_ZERO(&tempfds);
	FD_SET(sockfd,&readfds);
	int nfds = sockfd;
	int retval;
	int i = 0;
	
	while(1){
		tempfds = readfds;
		retval =select(nfds + 1, &tempfds, NULL,NULL,NULL);
		for(i = 0;i < nfds + 1; i ++){
			if(FD_ISSET(i,&tempfds)){
				if(i == sockfd){
					acceptfd = accept(sockfd,(struct sockaddr *)&clientaddr,&cli_len);
					if(acceptfd == -1){
						printf("acceptfd failed.\n");
						exit(-1);
					}
					printf("ip : %s.\n",inet_ntoa(clientaddr.sin_addr));
					FD_SET(acceptfd,&readfds);
					nfds = nfds > acceptfd ? nfds : acceptfd;
				}else{
					recvbytes = recv(i,&msg,sizeof(msg),0);
					printf("msg.type :%#x.\n",msg.msgtype);
					if(recvbytes == -1){
						printf("recv failed.\n");
						continue;
					}else if(recvbytes == 0){
						printf("peer shutdown.\n");
						close(i);
						FD_CLR(i, &readfds);               
					}else{
						process_client_request(i,&msg);
					}
				}
			}
		}
	}
	close(sockfd);

	return 0;
}


								
