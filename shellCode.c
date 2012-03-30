//***** YASH VADALIA *****/
//    201001015


// 	ASSUMPTIONS

/*
	-- If the input contains a combination of pipe and redirection operation the it is assumed that input redirection will always be first among the sub commands of the pipe and output redirections will always be the last

	-- While using !histn command the nth command is relaced int the place where !histn was there so history will not contain !histn command but the nth command. The same is reflected in pid all command

	-- In pipes all the sub commands of the pipe are taken to be seperate processes and thus are seperately displayed in pid all command

	-- some commands like cd hist pid (user defined) are not executed by forking and thus have the pid of the parent(main) process

*/

#include<signal.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/stat.h>

int size = 200;
int inpsize = 400;
int num = 0;
int shell_pid;
int *arglen;
char *ptr_home_dir;
char *** history;
char **process;
int *pid;
int num_process;
char **process_curr;
int *pid_curr;
int num_curr;

void hist1(char **,int);
void hist(char **,int);
void pidx(char **,int);
void cd(char **,int);
void other(char **,int,int);
void pipe_func(char **,int);
void redir_pipe_func(char **,int);
void redir_func(char **,int);
void whichone(char **,int);

void bg_handler(int para)
{
	int i;
	int status;
	pid_t childpid = wait(&status);
	if(childpid == -1)
		perror("Error");
	for(i=0; i<num_curr; i++)
		if(pid_curr[i] == childpid)
			break;
	if(i != num_curr)
	{
		if(WIFEXITED(status))
		{
			printf("process %s with pid %d exited normally\n",process_curr[i],pid_curr[i]);
			printf("press enter to continue\n");
		}
		else
		{
			printf("process %s with pid %d did not exit normally\n",process_curr[i],pid_curr[i]);
			printf("press enter to continue\n");
		}
		pid_curr[i] = 0;
		process_curr[i] = NULL;
	}
	return;
}

void cd(char **arg,int len)
{
	char tmp[size];
	int ret;
	if(len == 1)
		ret = chdir(ptr_home_dir);
	else if(arg[1][0] == '~' && arg[1][1]=='\0')
		ret = chdir(ptr_home_dir);
	else
	{
		if(arg[1][0] == '~')
		{
			strcpy(tmp,ptr_home_dir);
			strcat(tmp,&arg[1][1]);
			ret = chdir(tmp);
		}
		else
			ret = chdir(arg[1]);
	}
	if(ret == -1)
		perror("Error");
	else
	{
		strcpy(process[num_process],arg[0]);
		pid[num_process++] = shell_pid;
	}
}


void hist(char **arg,int len)
{
	if(len != 1)
	{
		fprintf(stderr,"Invalid Command\n");
		return;
	}
	int i = 0;
	int j = 0;
	int flag = 0;
	char temp[30];
	if(arg[0][4] == '\0')
	{
		for(i=0; i<num; i++)
		{
			for(j=0; j<arglen[i]; j++)
			{
				printf("%s ",history[i][j]);
			}
			printf("\n");
		}
	}
	else
	{
		for(i=4; i<strlen(arg[0]); i++)
		{
			if(arg[0][i]<'0' || arg[0][i]>'9')
			{
				flag = 1;
				break;
			}
			temp[j++] = arg[0][i];
		}
		temp[j] = '\0';
		if(flag == 1)
		{
			fprintf(stderr,"Invalid Command\n");
			return;
		}
		flag = atoi(temp);
		if(num < flag)
			flag = num;
		for(i=num-flag; i<num; i++)
		{
			for(j=0; j<arglen[i]; j++)
			{
				printf("%s ",history[i][j]);
			}
			printf("\n");
		}
	}
	strcpy(process[num_process],arg[0]);
	pid[num_process++] = shell_pid;
}

void hist1(char **arg,int len)
{
	if(len != 1)
	{
		fprintf(stderr,"Invalid Command\n");
		return;
	}
	char temp[50];
	char **fix;
	int i = 0;
	int j = 0;
	int flag = 0;
	for(i=5; i<strlen(arg[0]); i++)
	{
		if(arg[0][i]<'0' || arg[0][i]>'9')
		{
			flag = 1;
			break;
		}
		temp[j++] = arg[0][i];
	}
	temp[j] = '\0';
	if(flag == 1)
	{
		fprintf(stderr,"command not valid\n");
		return;
	}
	flag = atoi(temp);
	if(num < flag)
	{
		fprintf(stderr,"command not valid\n");
		return;
	}
	fix = (char **)calloc(size,sizeof(char *));
	for(i=0; i<size; i++)
		fix[i] = (char *)calloc(size,sizeof(char));
	for(i=0; i<arglen[flag-1]; i++)
	{
		strcpy(fix[i],history[flag-1][i]);
	}
	fix[i] = NULL;

	for(i=0; i<arglen[num-1]; i++)
		free(history[num-1][i]);

	history[num-1] = fix;
	arglen[num-1] = arglen[flag-1];
	arg = history[flag-1];

	whichone(arg,arglen[flag-1]);
}

void pidx(char **arg,int len)
{
	int i;
	int flag = 0;
	if(len == 1)
	{
		printf("command name: ./a.out process id: %d\n",shell_pid);
		flag = 1;
	}
	else if(strcmp(arg[1],"all") == 0)
	{   
		for(i=0; i<num_process; i++)
		{   
			printf("command name: %s process id: %d\n",process[i],pid[i]);
		}   
		flag = 1;
	}   
	else if(strcmp(arg[1],"current") == 0)
	{ 
		for(i=0; i<num_curr; i++)
		{   
			if(pid_curr[i] != 0)
			{   
				printf("command name: %s process id: %d\n",process_curr[i],pid_curr[i]);
			}   
		}   
		flag = 1;
	}   
	else
		fprintf(stderr,"command not found\n");
	if(flag == 1)
	{
		strcpy(process[num_process],arg[0]);
		pid[num_process++] = shell_pid;
	}
}

void other(char **arg,int len,int bg)
{
	int flag = 0,childpid;
	pid_t ret = vfork();
	if(ret < 0)
		fprintf(stderr,"forking failed\n");
	if(ret == 0)
	{
		if(bg == 1)
			childpid = getpid();
		if(arg[0] == NULL)
		{
			fprintf(stderr,"Invalid Command\n");
		}
		else
		{
			flag = execvp(arg[0],arg);
			if(errno == 2)
				fprintf(stderr,"Invalid Command\n");
			else
				perror("Error");
			exit(0);
		}
	}
	else if(ret > 0)
	{
		if(bg == 0)
		{
			signal(SIGCHLD,SIG_IGN);
			waitpid(ret,NULL,0);
		}
		else
		{
			signal(SIGCHLD,bg_handler);
		}
	}
	if(flag != -1)
	{
		if(bg == 1)
		{
			strcpy(process_curr[num_curr],arg[0]);
			pid_curr[num_curr++] = childpid;
		}
		strcpy(process[num_process],arg[0]);
		pid[num_process++] = ret;
	}
}

void pipe_func(char **arg,int len)
{

	int countpipe = 0;
	int i;
	int j;
	int k = 0;
	int *partlen;
	int temp1;
	int temp2;
	char ***var;
	partlen = (int *)calloc(size,sizeof(int));
	var=(char ***)calloc(size,sizeof(char **));
	for(i=0; i<size; i++)
		var[i] = (char **)calloc(size,sizeof(char *));
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
			var[i][j] = (char *)calloc(size,sizeof(char));
	for(i=0; i<len; i++)
	{
		if(strcmp(arg[i],"|") == 0)
		{
			var[countpipe][k] = NULL;
			partlen[countpipe++] = k;
			k = 0;
		}
		else
			strcpy(var[countpipe][k++],arg[i]);
	}
	var[countpipe][k] = NULL;
	partlen[countpipe] = k;

	int a[countpipe][2];
	for(i=0; i<countpipe; i++)
	{
		pipe(a[i]);
	}
	temp1 = dup(0);
	temp2 = dup(1);

	dup2(a[0][1],1);
	whichone(var[0],partlen[0]);
	for(i=1; i<countpipe; i++)
	{
		close(a[i-1][1]);
		dup2(a[i-1][0],0);
		dup2(a[i][1],1);
		whichone(var[i],partlen[i]);
	}
	close(a[countpipe-1][1]);
	dup2(a[countpipe-1][0],0);
	dup2(temp2,1);
	whichone(var[countpipe],partlen[countpipe]);

	for(i=0; i<countpipe; i++)
		close(a[i][0]);
	dup2(temp1,0);
	free(partlen);
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
			free(var[i][j]);
	for(i=0; i<size; i++)
		free(var[i]);
	free(var);
}

void redir_func(char **arg,int len)
{
	int i;
	int k = 0;
	int temp1;
	int temp2;
	int fd1 = 0;
	int fd2 = 0;
	char **temp;
	char input[200];
	char output[200];
	input[0] = '\0';
	output[0] = '\0';
	temp = (char **)calloc(size,sizeof(char *));
	for(i=0; i<size; i++)
		temp[i] = (char *)calloc(size,sizeof(char));

	for(i=0; i<len; i++)
	{
		if(arg[i] == NULL)
			continue;
		else if(strcmp(arg[i],"<") == 0)
		{
			if(len <= i+1)
			{
				fprintf(stderr,"Ivalid Command : unexpected token newline\n");
				return;
			}
			strcpy(input,arg[i+1]);
		}
		else if(strcmp(arg[i],">") == 0)
		{
			if(len <= i+1)
			{
				fprintf(stderr,"Ivalid Command : unexpected token newline\n");
				return;
			}
			strcpy(output,arg[i+1]);
		}
	}
	for(i=0; i<len; i++)
	{
		if(arg[i][0] == '<' || arg[i][0] == '>')
		{
			i++;
			continue;
		}
		if(arg[i] != NULL)
			strcpy(temp[k++],arg[i]);
	}
	temp[k] = NULL;
	if(input[0] != '\0')
		fd1 = open(input,O_RDONLY);
	if(output[0] != '\0')
		fd2 = open(output,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR);
	if(fd1<0 || fd2<0)
	{
		perror("Error");
		return;
	}
	temp1 = dup(0);
	temp2 = dup(1);
	if(fd1 > 0)
		dup2(fd1,0);
	if(fd2 > 0)
		dup2(fd2,1);
	whichone(temp,k);
	dup2(temp1,0);
	dup2(temp2,1);
	if(fd1 > 0)
		close(fd1);
	if(fd2 > 0)
		close(fd2);
	for(i=0; i<size; i++)
		free(temp[i]);
	free(temp);
}

void redir_pipe_func(char **arg,int len)
{
	int countpipe=0;
	int i;
	int j;
	int k = 0;
	int *partlen;
	int temp1;
	int temp2;
	int fd1 = 0;
	int fd2 = 0;
	char ***var;
	char input[50];
	char output[50];
	input[0]='\0';
	output[0]='\0';
	var = (char ***)calloc(size,sizeof(char **));
	for(i=0; i<size; i++)
		var[i] = (char **)calloc(size,sizeof(char *));
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
			var[i][j] = (char *)calloc(size,sizeof(char));
	partlen = (int *)calloc(size,sizeof(int));
	
	for(i=0; i<len; i++)
	{
		if(arg[i]==NULL)
			continue;
		else if(!strcmp(arg[i],">"))
			strcpy(output,arg[i+1]);
		else if(!strcmp(arg[i],"<"))
			strcpy(input,arg[i+1]);
	}
	if(input[0] != '\0')
		fd1=open(input,O_RDONLY);
	if(output[0] != '\0')
		fd2=open(output,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR);
	if(fd1 < 0 || fd2 < 0)
	{
		perror("ERROR:");
		return;
	}

	for(i=0; i<len; i++)
	{
		if(arg[i] == NULL)
			continue;
		else if(strcmp(arg[i],"<") == 0 || strcmp(arg[i],">") == 0)
		{
			i++;
			continue;
		}
		else if(strcmp(arg[i],"|") == 0)
		{
			var[countpipe][k] = NULL;
			partlen[countpipe++] = k;
			k = 0;
		}
		else
			strcpy(var[countpipe][k++],arg[i]);
	}
	var[countpipe][k] = NULL;
	partlen[countpipe] = k;

	int a[countpipe][2];
	for(i=0; i<countpipe; i++)
	{
		pipe(a[i]);
	}
	temp1 = dup(0);
	temp2 = dup(1);
	if(fd1 != 0)
		dup2(fd1,0);
	else
		dup2(temp1,0);

	dup2(a[0][1],1);
	whichone(var[0],partlen[0]);
	for(i=1; i<countpipe; i++)
	{
		close(a[i-1][1]);
		dup2(a[i-1][0],0);
		dup2(a[i][1],1);
		whichone(var[i],partlen[i]);
	}
	dup2(a[countpipe-1][0],0);
	if(fd2 != 0)
		dup2(fd2,1);
	else
		dup2(temp2,1);
	close(a[countpipe-1][1]);
	whichone(var[countpipe],partlen[countpipe]);
	for(i=0; i<countpipe; i++)
	{
		close(a[i][0]);
	}
	dup2(temp1,0);
	dup2(temp2,1);
	free(partlen);
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
			free(var[i][j]);
	for(i=0; i<size; i++)
		free(var[i]);
	free(var);

}
void whichone(char **arg,int len)
{
	int i;
	int bg = 0;
	int pipe = 0;
	int redir = 0;
	if(arg[0] == NULL)
	{
		fprintf(stderr,"Invalid Command\n");
	}
	for(i=0; i<len; i++)
		if((strcmp(arg[i],"<") == 0) || (strcmp(arg[i],">") == 0))
			redir = 1;
	for(i=0; i<len; i++)
		if(strcmp(arg[i],"|") == 0)
			pipe = 1;

	if(arg[len-1][strlen(arg[len-1])-1] == '&')
	{
		bg = 1;
		if(arg[len-1][0] == '&')
			arg[len-1] = NULL;
		else
			arg[len-1][strlen(arg[len-1])-1] = '\0';
	}
	if(pipe && !redir)
	{
		pipe_func(arg,len);
	}
	else if(!pipe && redir)
	{
		redir_func(arg,len);
	}
	else if(pipe && redir)
	{
		redir_pipe_func(arg,len);
	}
	else if(strcmp(arg[0],"cd") == 0)
	{
		cd(arg,len);
	}
	else if(strcmp(arg[0],"pid") == 0)
	{
		pidx(arg,len);
	}
	else if(arg[0][0]=='h' && arg[0][1]=='i' && arg[0][2]=='s' && arg[0][3]=='t')
	{
		hist(arg,len);
	}
	else if(arg[0][0]=='!' && arg[0][1]=='h' && arg[0][2]=='i' && arg[0][3]=='s' && arg[0][4]=='t')
	{
		hist1(arg,len);
	}
	else
	{
		other(arg,len,bg);
	}
}

int main()
{
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);

	char host_name[size];
	char *user_name;
	char home_dir[size];
	char curr_dir[size];
	char *ptr_curr_dir;
	int i;
	int j;

	user_name = getenv("USER");
	gethostname(host_name,size);
	ptr_home_dir = getcwd(home_dir,size);
	shell_pid = getpid();

	history = (char ***)calloc(size,sizeof(char **));
	for(i=0; i<size; i++)
		history[i] = (char **)calloc(size,sizeof(char *));
	for(i=0; i<size; i++)
		for(j=0; j<size; j++)
			history[i][j] = (char *)calloc(size,sizeof(char));
	arglen = (int *)calloc(size,sizeof(int));

	process = (char **)calloc(size,sizeof(char *));
	for(i=0; i<size; i++)
		process[i] = (char *)calloc(size,sizeof(char));
	pid = (int*)calloc(size,sizeof(int));
	process_curr = (char **)calloc(size,sizeof(char *));
	for(i=0; i<size; i++)
		process_curr[i] = (char *)calloc(size,sizeof(char));
	pid_curr = (int*)calloc(size,sizeof(int));
	num_process = 0;
	num_curr = 0;

	while(1)
	{
		char inp[inpsize];
		char temp[inpsize];
		char *delim = NULL;
		char **arg;
		int len = 0;

		ptr_curr_dir = getcwd(curr_dir,size);
		printf("%s@%s:",user_name,host_name);
		if(strstr(ptr_curr_dir,ptr_home_dir) == NULL)
			printf("%s",ptr_curr_dir);
		else
			printf("~%s",ptr_curr_dir + strlen(ptr_home_dir));
		printf(">");
		inp[0] = '\0';	temp[0] = '\0';
		arg = (char **)calloc(size,sizeof(char *));
		for(i=0; i<size; i++)
			arg[i] = (char *)calloc(size,sizeof(char));


		while(scanf("%[^\n]",inp) == EOF) {}
		getchar();
		inp[strlen(inp)] = '\0';
		strcpy(temp,inp);
		temp[strlen(temp)] = '\0';
		if(strcmp(inp,"quit") == 0)
			break;

		delim = strtok(inp," \t\n|<>");
		if(delim == NULL)
			continue;
		while(delim != NULL)
		{
			strcpy(arg[len++],delim);
			char *po = temp + (int)(delim-inp) + strlen(delim);
			while(*po==' ' || *po=='\t' || *po=='\n' || *po=='<' || *po=='>' || *po=='|')
			{
				if(*po=='<' || *po=='>' || *po=='|')
				{
					arg[len][0] = *po;
					arg[len++][1] = '\0';
				}
				po = po + 1;
			}
			delim = strtok(NULL," \t\n<>|");
		}
		arg[len] = NULL;
		history[num] = arg;
		arglen[num++] = len;
		whichone(arg,len);
		signal(SIGCHLD,bg_handler);
	}
	for(i=0;i<size;i++)
		free(process[i]);
	free(process);
	free(pid);
	for(i=0;i<size;i++)
		free(process_curr[i]);
	free(process_curr);
	free(pid_curr);
	for(i=0;i<size;i++)
		for(j=0;j<size;j++)
			free(history[i][j]);
	for(i=0;i<size;i++)
		free(history[i]);


	printf("\n");
	return 0;
}
