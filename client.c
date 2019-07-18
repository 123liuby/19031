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

#include "system.h"

void do_admin_query(int sockfd,MSG *msg)
{
	msg->msgtype=ADMIN_QUERY;
	send(sockfd,msg,sizeof(MSG),0);
	printf("staffno  usertype   name  passwd  age   phone	addr   work  date  level  salary\n");
	printf("=====================================================================================\n");
	while(1){
		recv(sockfd,msg,sizeof(MSG),0);
		if(strncmp(msg->recvmsg,"ADMIN_QUERY",11)==0){
			memset(msg->recvmsg,0,sizeof(msg->recvmsg));
			break;
		}
		printf("%d  %d  %s  %s %d %s %s  %s  %s  %d %lf \n",msg->info.no,msg->info.usertype,
				msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,
				msg->info.addr,msg->info.work,msg->info.date,msg->info.level,msg->info.salary);
	}
	return;
}

void do_admin_modification(int sockfd,MSG *msg)
{
	memset(msg->recvmsg,0,DATALEN);
	memset(msg->info.name,0,NAMELEN);
	memset(msg->username,0,NAMELEN);
	msg->msgtype = ADMIN_MODIFY;

	printf("*************************************************************************************\n");
	printf("1.no 2.usertype 3.name 4.passwd 5.age 6.phone 7.addr 8.work 9.date 10.level 11.salary\n");
	printf("*************************************************************************************\n");

	printf("请输入修改员工的姓名：");
	scanf("%s",msg->info.name);
	getchar();

	printf("请输入修改哪一项：");
	scanf("%d",&msg->flags);
	getchar();

	printf("请输入修改后的值：");
	scanf("%s",msg->recvmsg);
	getchar();

	if((send(sockfd,msg,sizeof(MSG),0)) <0 ){
		perror("faile to send ---- admin modify\n");
	}

	if((recv(sockfd,msg,sizeof(MSG),0)) <0 ){
		perror("faile to send ---- admin modify\n");
	}else{
		printf("%s\n",msg->recvmsg);
	}
	return ;
}

void admin_menu(int sockfd,MSG *msg)
{
	int n;
	while(1)
	{
		printf("*************************************************************\n");
		printf("* 1：查询  2：修改 3：添加用户  4：删除用户  5：查询历史记录*\n");
		printf("* 6：返回上一层												*\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();
		switch(n)
		{
		case 1:
			do_admin_query(sockfd,msg);
			break;
		case 2:
			do_admin_modification(sockfd,msg);
			break;
		case 6:
			msg->msgtype = QUIT;
			send(sockfd, msg, sizeof(MSG), 0);
			do_login(sockfd);
		default:
			printf("您输入有误，请重新输入！\n");
		}
	}
}

void do_user_query(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = USER_QUERY;
	msg->usertype = USER;
	msg->flags = 0;
	printf("username:%s\n",msg->info.name);
	if(send(sockfd,msg,sizeof(MSG),0) < 0){
		perror("failed to send ---- do_user_query\n");
	}
	if(recv(sockfd,msg,sizeof(MSG),0) < 0){
		perror("failed to receive from server ---- do_user_query\n");
	}
	if(msg->flags == 1){
		printf("no:%d,type:%d,name:%s,pwd:%s,age:%d,phone:%s,addr:%s,work:%s,date;%s,level:%d,salary:%lf\n",\
				msg->info.no, msg->info.usertype, msg->info.name, msg->info.passwd, msg->info.age,\
				msg->info.phone, msg->info.addr, msg->info.work, msg->info.date,\
				msg->info.level, msg->info.salary);
	}
}

void user_menu(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int n;
	while(1)
	{
		printf("*************************************************************\n");
		printf("**********  1：查询   2：修改	3：返回上一层	 ************\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			do_user_query(sockfd,msg);
			break;
		case 2:
			do_user_modification(sockfd,msg);
			break;
		case 3:
			msg->msgtype = QUIT;
			send(sockfd, msg, sizeof(MSG), 0);
			do_login(sockfd);
		default:
			printf("您输入有误，请输入数字\n");
			break;
		}
	}
}

void do_user_modification(int sockfd,MSG *msg)
{
	msg->msgtype = USER_MODIFY;
	msg->usertype = USER;
	msg->flags = 0;
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	printf("please input your staffnum:\n");
	scanf("%d",&(msg->info.no));
	getchar();
	printf("please select which one you want to moddify:\n");
	printf("================================================\n");
	printf("--1.password--2.age--3.phone--4.addr--\n");
	printf("================================================\n");
	int num;
	scanf("%d",&num);
	getchar();

	switch(num){
	case 1:
		printf("please input new psw:\n");
		scanf("%s",msg->info.passwd);
		getchar();
		msg->flags = 1;
		if((send(sockfd,msg,sizeof(MSG),0)) < 0){
			perror("failed to send pwd----user update\n");
		}
		if(recv(sockfd,msg,sizeof(MSG),0) < 0){
			perror("failed reveive from serve --- user update\n");
		}
		if(msg->flags == 0){
			printf("update %s----new pwd:%s\n",msg->recvmsg,msg->info.passwd);
		}
		break;
	case 2:
		printf("please input new age:\n");
		scanf("%d",&(msg->info.age));
		getchar();
		msg->flags = 2;
		if((send(sockfd,msg,sizeof(MSG),0)) < 0){
			perror("failed to send age----user update\n");
		}
		if(recv(sockfd,msg,sizeof(MSG),0) < 0){
			perror("failed reveive from serve --- user update\n");
		}
		if(msg->flags == 0){
			printf("update %s----new age:%d\n",msg->recvmsg,msg->info.age);
		}
		break;
	case 3:
		printf("please input new phonenum:\n");
		scanf("%s",msg->info.phone);
		getchar();
		msg->flags = 3;
		if((send(sockfd,msg,sizeof(MSG),0)) < 0){
			perror("failed to send phone----user update\n");
		}
		if(recv(sockfd,msg,sizeof(MSG),0) < 0){
			perror("failed reveive from serve --- user update\n");
		}
		if(msg->flags == 0){
			printf("update %s----new phone:%s\n",msg->recvmsg,msg->info.phone);
		}
		break;
	case 4:
		printf("please input new addr:\n");
		scanf("%s",msg->info.addr);
		getchar();
		msg->flags = 4;
		if((send(sockfd,msg,sizeof(MSG),0)) < 0){
			perror("failed to send addr----user update\n");
		}
		if(recv(sockfd,msg,sizeof(MSG),0) < 0){
			perror("failed reveive from serve --- user update\n");
		}
		if(msg->flags == 0){
			printf("update %s----new addr:%s\n",msg->recvmsg,msg->info.addr);
		}
		break;
	default:
		printf("input error\n");
	}
}

int admin_or_user_login(int sockfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);

	memset(msg->username, 0, NAMELEN);
	printf("请输入用户名：");
	scanf("%s",msg->username);
	getchar();

	memset(msg->passwd, 0, DATALEN);
	printf("请输入密码（6位）");
	scanf("%s",msg->passwd);
	getchar();

	send(sockfd, msg, sizeof(MSG), 0);
	recv(sockfd, msg, sizeof(MSG), 0);
	printf("msg->recvmsg :%s\n",msg->recvmsg);

	if(strncmp(msg->recvmsg, "OK", 2) == 0){
		if(msg->usertype == ADMIN){
			printf("亲爱的管理员，欢迎您登陆员工管理系统！\n");
			admin_menu(sockfd,msg);
		}else if(msg->usertype == USER){
			printf("亲爱的用户，欢迎您登陆员工管理系统！\n");
			user_menu(sockfd,msg);
		}
	}else{	
		printf("登陆失败！%s\n", msg->recvmsg);
		return -1;
	}
	return 0;
}


int do_login(int sockfd)
{	
	int n;
	MSG msg;

	while(1){
		printf("*************************************************************\n");
		printf("********  1：管理员模式    2：普通用户模式    3：退出********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			msg.msgtype  = ADMIN_LOGIN;
			msg.usertype = ADMIN;
			break;
		case 2:
			msg.msgtype =  USER_LOGIN;
			msg.usertype = USER;
			break;
		case 3:
			msg.msgtype = QUIT;
			if(send(sockfd, &msg, sizeof(MSG), 0)<0)
			{
				perror("do_login send");
				return -1;
			}
			close(sockfd);
			exit(0);
		default:
			printf("您的输入有误，请重新输入\n"); 
		}
		admin_or_user_login(sockfd,&msg);
	}
}

int main(int argc, const char *argv[])
{
	int sockfd;
	int acceptfd;
	ssize_t recvbytes,sendbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);

	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("sockfd :%d.\n",sockfd); 

	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;

	serveraddr.sin_port   = htons(5001);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sockfd,(const struct sockaddr *)&serveraddr,addrlen) == -1){
		perror("connect failed.\n");
		exit(-1);
	}

	do_login(sockfd);
	close(sockfd);

	return 0;
}


