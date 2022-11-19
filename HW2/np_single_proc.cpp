#include<iostream>
#include<cstring>
#include<sstream>
#include<string>
#include<vector>
#include<cctype>
#include<map>
#include<pwd.h>
#include<errno.h>
#include<fcntl.h>
#include<netdb.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<strings.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define MAX_COUNT 1001
#define cmdTime first
#define processState second
#define f first
#define s second

#define SA struct sockaddr
#define	MAXLINE	15000
using namespace std;
struct processState{
	int numberedPipe[2] = {-1, -1};
	vector<pid_t> pids;
};

map<pair<int, int>, processState> userPipes;

struct usersInfo{
	char ip4[INET_ADDRSTRLEN];
	int port;
	string name = "(no name)";
	string cmd;
	map<int, processState> numberedPipes;
	map<string, string> env;
	int counter;
};

map<int, processState> numberedPipes;

int state, counter, numUsers = 0;
auto NPIN = STDIN_FILENO;
auto NPOUT = STDOUT_FILENO;
auto NPERR = STDERR_FILENO;

int	client[FD_SETSIZE];
map<int, usersInfo> usersData;

void handler(int signo){
	while(waitpid(-1, &state, WNOHANG) > 0);
}

void dup2Client(int fd){
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);
}

void who(int me){
	cout << "<ID>\t<nickname>\t<IP:port>\t<indicate me>\n";
	for(auto user : usersData){
		cout << user.f << '\t' << user.s.name << '\t' << user.s.ip4 << ':' << user.s.port;
		if(user.f == me){
			cout << '\t' << "<-me";
		}
		cout << '\n';
	}
}

void tell(int me, int i, string msg){
	if(client[i] == -1){
		cerr << "*** Error: user #" << i << " does not exist yet. ***" << '\n';
		return;
	}
	dup2Client(client[i]);
	cout << "*** " << usersData[me].name << " told you ***: " << msg << '\n';
	dup2Client(client[me]);
}

void yell(int me, string msg){
	for(auto user : usersData){
		dup2Client(client[user.f]);
		cout << "*** " << usersData[me].name << " yelled ***: " << msg << '\n';
	} 
	dup2Client(client[me]);
}

void name(int me, string name){
	for(auto user : usersData){
		if(name == user.s.name){
			cerr << "*** User '" << name << "' already exists. ***" << '\n';
			return;
		}
	}
	usersData[me].name = name;
	for(auto user : usersData){
		dup2Client(client[user.f]);
		cout << "*** User from " << usersData[me].ip4 << ":" << usersData[me].port << " is named '" << name << "'. ***" << '\n';
	}
	dup2Client(client[me]);
}

void splitOrdinaryPipe(vector<string> &dst, string &src){
	dst.clear();
	int begin = 0, cur = 0;
	for(; cur < src.size(); cur++){
		if(src[cur] == '|'){
			dst.push_back(src.substr(begin, cur - begin));
			begin = cur + 1;
		}else if(src[cur] == '>'){
			if(cur + 1 < src.size()){
				if(src[cur + 1] == '>'){
					dst.push_back(src.substr(begin, cur - begin));
					dst.push_back(">>" + src.substr(cur + 1));
					return;
				}else if(src[cur + 1] != ' '){
					continue;
				}
			}
			dst.push_back(src.substr(begin, cur - begin));
			dst.push_back("> " + src.substr(cur + 1));
			return;
		}
	}
	if(begin != src.size()) dst.push_back(src.substr(begin));
}

void splitArgument(vector<string> &dst, string &src){
	dst.clear();
	string arg;
	int begin = 0, charType = 0, X = 0; //0 otherwise 1 space 2 " 3 '
	if(src[0] == '>'){
		if(src.size() >= 2 && src[1] == '>'){
			dst.push_back(">>");
			begin = 2;
		}else if(src.size() >= 2 && src[1] == ' '){
			dst.push_back(">");
			begin = 1;
		}
	}
	for(; begin < src.size(); begin++){
		if(src[begin] == ' ' && X != 1){
			if(charType == 1){
				dst.push_back(arg);
				if(X == 3){
					X = 2;
				}
				if(arg == "tell"){
					X = 3;
				}
				if(arg == "yell"){
					X = 2;
				}
				arg.clear();
				charType = 0;
			}
		}else{
			if(X == 2) X = 1;
			if(charType == 0) charType = 1;
			arg.push_back(src[begin]);
		}
	}
	if(arg.size() != 0){
		dst.push_back(arg);
	} 
}

void splitline(vector<vector<string>> &dst, string &src){
	dst.clear();
	vector<string> tmp;
	splitOrdinaryPipe(tmp, src);
	dst.resize(tmp.size());
	int i = 0;
	for(string cmd: tmp){
		splitArgument(dst[i], cmd);
		i++;
	}
}

char **stoc(vector<string> &src){ //vector<string> to char[][]
	int argc = src.size();
	char **argv = new char*[argc+1]; 
	for(int i = 0; i < argc; i++){
		argv[i] = new char[src[i].size()];
		strcpy(argv[i], src[i].c_str());
	}
	argv[argc] = new char;
	argv[argc] = NULL;
	return argv;
}

int buildInCommand(vector<string> &cmd, int me){
	if(cmd.size() < 1) return 1;
	if(cmd[0] == "exit"){
		return -1;
	}else if(cmd[0] == "printenv"){
		if(cmd.size() < 2 || getenv(cmd[1].c_str()) == NULL);
		else cout << getenv(cmd[1].c_str()) << '\n';
		return 1;
	}else if(cmd[0] == "setenv"){
		if(cmd.size() < 2);
		else if(cmd.size() < 3){
			usersData[me].env[cmd[1]] = "";
			setenv(cmd[1].c_str(), "", 1);
		} 	
		else{
			usersData[me].env[cmd[1]] = cmd[2];
			setenv(cmd[1].c_str(), cmd[2].c_str(), 1);
		}						
		return 1;
	}else if(cmd[0] == "who"){
		who(me);
		return 1;
	}else if(cmd[0] == "tell"){
		int recv = 0;
		if(cmd.size() < 2){
			cerr << "tell [id] [msg]\n";
			return 1;
		}
		sscanf(cmd[1].c_str(), "%d", &recv);
		if(cmd.size() < 3){
			cerr << "tell [id] [msg]\n";
			return 1;
		}
		tell(me, recv, cmd[2]);
		return 1;
	}else if(cmd[0] == "yell"){
		if(cmd.size() < 2){
			cerr << "yell [msg]\n";
			return 1;
		}
		yell(me, cmd[1]);
		return 1;
	}else if(cmd[0] == "name"){
		if(cmd.size() < 2){
			cerr << "name [new name]\n";
			return 1;
		}
		name(me, cmd[1]);
		return 1;
	}
	return 0;
}

void execute(vector<vector<string>> &cmd, int pipeTime, string &fullcmd, int me){
	int numOrdinaryPipe = cmd.size() - 1, fd = -1;
	int idOfUserPipeIn = 0, idOfUserPipeOut = 0;
	bool userIN = false, userOUT = false;
	//file redirection check
	bool fileRedirection = cmd[numOrdinaryPipe][0] == ">";
	bool demo = cmd[numOrdinaryPipe][0] == ">>";
	
	if(fileRedirection || demo) numOrdinaryPipe--;
	//memory for pipe
	int ordinaryPipe[numOrdinaryPipe * 2];
	memset(ordinaryPipe, -1, sizeof(ordinaryPipe));
	pid_t pid;
	for(int i = 0; i < numOrdinaryPipe + 1; i++){
		//
		bool IN = false, OUT = false, userPipeExsits = false;
		//user pipe
		int argc = cmd[i].size();
		if(argc > 1){
			if(cmd[i][argc - 1][0] == '>'){
				sscanf(cmd[i][argc - 1].substr(1).c_str(), "%d", &idOfUserPipeOut);
				OUT = true;
			}else if(cmd[i][argc - 1][0] == '<'){
				sscanf(cmd[i][argc - 1].substr(1).c_str(), "%d", &idOfUserPipeIn);
				IN = true;
			}
			if(argc > 2){
				if(cmd[i][argc - 2][0] == '>'){
					sscanf(cmd[i][argc - 2].substr(1).c_str(), "%d", &idOfUserPipeOut);
					OUT = true;
				}else if(cmd[i][argc - 2][0] == '<'){
					sscanf(cmd[i][argc - 2].substr(1).c_str(), "%d", &idOfUserPipeIn);
					IN = true;
				}
			}
		}
		if(IN && idOfUserPipeIn != 0){
			cmd[i].pop_back();
			if(client[idOfUserPipeIn] == -1){
				cerr << "*** Error: user #" << idOfUserPipeIn << " does not exist yet. ***" << '\n';
			}else if(userPipes.find({idOfUserPipeIn, me}) == userPipes.end()){
				cerr << "*** Error: the pipe #" << idOfUserPipeIn << "->#" << me << " does not exist yet. ***" << '\n';
			}else{
				for(auto user : usersData){
					dup2Client(client[user.f]);
					cout << "*** " << usersData[me].name << " (#" << me << ") just received from " << usersData[idOfUserPipeIn].name << " (#" << idOfUserPipeIn << ") by '" << fullcmd << "' ***" << '\n';
				}
				dup2Client(client[me]);
				userIN = true;
			}
		}
		if(OUT && idOfUserPipeOut != 0){
			cmd[i].pop_back();
			if(client[idOfUserPipeOut] == -1){
				cerr << "*** Error: user #" << idOfUserPipeOut << " does not exist yet. ***" << '\n';
			}else if(userPipes.find({me, idOfUserPipeOut}) != userPipes.end()){
				cerr << "*** Error: the pipe #" << me << "->#" << idOfUserPipeOut << " already exists. ***" << '\n';
				userPipeExsits = true;
			}else{
				for(auto user : usersData){
					dup2Client(client[user.f]);
					cout << "*** " << usersData[me].name << " (#" << me << ") just piped '" << fullcmd << "' to " << usersData[idOfUserPipeOut].name << " (#" << idOfUserPipeOut << ") ***" << '\n';
				}
				dup2Client(client[me]);
				pipe(userPipes[{me, idOfUserPipeOut}].numberedPipe);
				userOUT = true;
			}
		}
again_fork:
		//construct pipe
		if(i < numOrdinaryPipe) pipe(ordinaryPipe + i * 2);
		while(waitpid(-1, &state, WNOHANG) > 0);
		//fork
		pid = fork();
		if(pid < 0){ //fork error
			usleep(1000);
			goto again_fork;
		}else if(pid == 0){ //child
			//user pipe
			bool A = false, B = false;
			if(IN && idOfUserPipeIn != 0){
				if(client[idOfUserPipeIn] == -1){
					dup2(open("/dev/null", O_RDONLY, 0), STDIN_FILENO);
				}else if(userPipes.find({idOfUserPipeIn, me}) == userPipes.end()){
					dup2(open("/dev/null", O_RDONLY, 0), STDIN_FILENO);
				}else{
					A = true;
					dup2(userPipes[{idOfUserPipeIn, me}].numberedPipe[0], STDIN_FILENO);
				}
			}
			if(OUT && idOfUserPipeOut != 0){
				if(client[idOfUserPipeOut] == -1){
					dup2(open("/dev/null", O_RDWR, 0), STDOUT_FILENO);
				}else if(userPipeExsits){
					dup2(open("/dev/null", O_RDWR, 0), STDOUT_FILENO);
				}else{
					B = true;
					dup2(userPipes[{me, idOfUserPipeOut}].numberedPipe[1], STDOUT_FILENO);
				}
			}
			//dup2
			if(!A){
				if(i != 0)
					dup2(ordinaryPipe[(i - 1) * 2 ], STDIN_FILENO);
				else if(numberedPipes.find(counter) != numberedPipes.end())
					dup2(numberedPipes[counter].numberedPipe[0], STDIN_FILENO);
			}
			if(!B){
				if(i != numOrdinaryPipe){
					dup2(ordinaryPipe[i * 2 + 1], STDOUT_FILENO);
				}else if(fileRedirection){
					fd = open(cmd[numOrdinaryPipe + 1][1].c_str(), O_TRUNC | O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
					dup2(fd, STDOUT_FILENO);
				}else if(demo){
					fd = open(cmd[numOrdinaryPipe + 1][2].c_str(), O_RDWR | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
					dup2(fd, STDOUT_FILENO);
				}else{
					dup2(NPOUT, STDOUT_FILENO);
					dup2(NPERR, STDERR_FILENO);
				}

			}
			//close
			for(int j = 0; j < (i + 1) * 2 && j < numOrdinaryPipe * 2; j++) close(ordinaryPipe[j]);
			for(auto user : usersData){
				close(client[user.f]);
				for(auto process : user.s.numberedPipes){
					close(process.processState.numberedPipe[0]);
					close(process.processState.numberedPipe[1]);
				} 
			}
			for(auto process : userPipes){
				close(process.processState.numberedPipe[0]);
				close(process.processState.numberedPipe[1]);
			}
			close(3);
			close(4);
			close(5);
			close(6);
			for(auto j : client) close(j);
			//exec
			char **argv = stoc(cmd[i]);
			int e = execvp(argv[0], argv);
			if(e == -1){
				cerr << "Unknown command: [" << &cmd[i][0][0] << "].\n";
				exit(1);
			}
			exit(0);
		}else{ //parent
			while(waitpid(-1, &state, WNOHANG) > 0);
			//close ordinary pipe
			if(i != 0){
				close(ordinaryPipe[(i - 1) * 2]);
				close(ordinaryPipe[(i - 1) * 2 + 1]);
			}
			//close and wait numbered pipe
			if(numberedPipes.find(counter) != numberedPipes.end()){
				close(numberedPipes[counter].numberedPipe[0]);
				close(numberedPipes[counter].numberedPipe[1]);
				for(auto pid : numberedPipes[counter].pids) waitpid(pid, &state, 0);
				numberedPipes.erase(counter);
			}
			//close and wait user pipe
			if(userIN){
				close(userPipes[{idOfUserPipeIn, me}].numberedPipe[0]);
				close(userPipes[{idOfUserPipeIn, me}].numberedPipe[1]);
				for(auto pid : userPipes[{idOfUserPipeIn, me}].pids){
					waitpid(pid, &state, 0);
				}
				userPipes.erase({idOfUserPipeIn, me});
			}
		}
	}
	//wait child or load pid
	if(userOUT) 				userPipes[{me, idOfUserPipeOut}].pids.push_back(pid);
	else if(pipeTime == -1) 	waitpid(pid, &state, 0);
	else 						numberedPipes[pipeTime].pids.push_back(pid);
	while(waitpid(-1, &state, WNOHANG) > 0);
	//close fd
	if(fileRedirection || demo) close(fd);
	//init fd
	NPOUT = STDOUT_FILENO;
	NPERR = STDERR_FILENO;
}

void dealNumberedPipes(string &line, int me){ //split, execute, load pipe and pid
	int begin = 0, num = 0, pipeTime, cur, newCur;
	for(cur = 0; cur < line.size(); cur++){
		if(line[cur] == '|' || line[cur] == '!'){
			for(newCur = cur + 1; newCur < line.size() && isdigit(line[newCur]); newCur++){
				num *= 10;
				num += (int) line[newCur] - '0';
			}
			if(num == 0) continue;
			counter = (counter + 1) % MAX_COUNT;
			pipeTime = ((counter + num) % MAX_COUNT);
			if(numberedPipes.find(pipeTime) == numberedPipes.end()){
				pipe(numberedPipes[pipeTime].numberedPipe);
			}
			NPOUT = numberedPipes[pipeTime].numberedPipe[1];
			if(line[cur] == '!') NPERR = numberedPipes[pipeTime].numberedPipe[1];
			vector<vector<string>> cmd;
			string tline = line.substr(begin, cur - begin);
			splitline(cmd, tline);
			execute(cmd, pipeTime, line, me);
			begin = newCur;
			cur = newCur - 1;
			num = 0;
		}
	}
	line = line.substr(begin);
}

void init(int i){
	clearenv();
	for(auto env : usersData[i].env){
		setenv(env.f.c_str(), env.s.c_str(), 1);
	}
	counter = usersData[i].counter;
	numberedPipes = usersData[i].numberedPipes;
}


void server(int port){
	//socket
	int connectfd, listenfd, val = 1, len = sizeof(val), n;
	struct sockaddr_in serverAddress, clientAddress;
	char buf[MAXLINE];
	socklen_t clientLen;
	//fd select
	fd_set rset, wset, eset, allset;
	int i, maxi, maxfd, nready, currentfd;
	//loop
	string in;
	vector<vector<string>> cmd;
	//socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd == -1){
		perror("socket");
		exit(1);
	}
	//serverAddress
	bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);
	//setsockopt
	n = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &val, len);
	if(n == -1){
		perror("setsockopt(REUSEADDR)");
		exit(1);
	}
	n = setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &val, len);
	if(n == -1){
		perror("setsockopt(REUSEPORT)");
		exit(1);
	}
	//bind
	n = bind(listenfd, (SA*) &serverAddress, sizeof(serverAddress));
	if(n == -1){
		perror("bind");
		exit(1);
	}
	//
	usersData.clear();
	//listen
	n = listen(listenfd, 3);
	if(n == -1){
		perror("listen");
		exit(1);
	}
	//FD
	maxfd = 50;
	maxi = -1;
	for(i = 0; i < FD_SETSIZE; i++) client[i] = -1;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	//child signal
	signal(SIGCHLD, handler);
	//backup STD
	dup2(0, 4);
	dup2(1, 5);
	dup2(2, 6);
	for(;;){
		rset = allset;
//again_select:
		nready = select(maxfd+1, &rset, NULL, NULL, (struct timeval *)0);
		if(nready == -1){
			//perror("select");
			if(errno == EINTR) continue;
			//if(errno == EINTR) goto again_select;
			perror("select");
			continue;
		}
		if(FD_ISSET(listenfd, &rset)){
			clientLen = sizeof(clientAddress);
			connectfd = accept(listenfd, (SA *) &clientAddress, &clientLen);
			for(i = 1; i < FD_SETSIZE; i++){
				if(client[i] < 0){
					client[i] = connectfd;
					break;
				}
			}
			if(i == FD_SETSIZE){
				perror("Too many clients");
				exit(1);
			}
			
			//user count
			numUsers++;
			//load ip port
			inet_ntop(AF_INET, &(clientAddress.sin_addr), usersData[i].ip4, INET_ADDRSTRLEN);
			usersData[i].port = ntohs(clientAddress.sin_port);
			usersData[i].env["PATH"] = "bin:.";
			usersData[i].counter = 0;
			usersData[i].numberedPipes.clear();
			//
			init(i);
			//dup2 client
			dup2Client(client[i]);
			cout << "****************************************\n";
			cout << "** Welcome to the information server. **\n";
			cout << "****************************************\n";
			for(auto user : usersData){
				dup2Client(client[user.f]);
				cout << "*** User '" << usersData[i].name << "' entered from " << usersData[i].ip4 << ":" << usersData[i].port << ". ***" << '\n';
			}
			dup2Client(client[i]);
			cout << "% ";
			dup2(4, 0);
			dup2(5, 1);
			dup2(6, 2);
			//
			FD_SET(connectfd, &allset);
			if(connectfd > maxfd) maxfd = connectfd;
			if(i > maxi) maxi = i;
			if(--nready <= 0) continue;
		}
		for(i = 1; i <= maxi; i++){
			if( (currentfd = client[i]) < 0 ) continue;
			if(FD_ISSET(currentfd, &rset)){
				init(i);
				dup2Client(client[i]);
again_read:
				n = read(0, buf, MAXLINE);
				if(n == -1){
					if(errno == EINTR) goto again_read;
					perror("read");
					continue;
				}else if(n == 0){
					dup2(4, 0);
					dup2(5, 1);
					dup2(6, 2);
					close(client[i]);
					FD_CLR(client[i], &allset);
					client[i] = -1;
					for(auto user : usersData){
						if(client[i] == -1) continue;
						userPipes.erase({user.f, i});
						userPipes.erase({i, user.f});
						dup2Client(client[user.f]);
						cout << "*** User '" << usersData[i].name << "' left. ***" << '\n';
					}
					usersData.erase(i);
					numUsers--;
				}else{
					int e;
					for(int j = 0; j < n; j++){
						if(buf[j] == '\r') continue;
						if(buf[j] != '\n'){
							usersData[i].cmd.push_back(buf[j]);
							continue;
						}
						string line = usersData[i].cmd;
						usersData[i].cmd.clear();
						vector<string> bicmd;
						splitArgument(bicmd, line);
						e = buildInCommand(bicmd, i);
						if(e == 1){
							cout << "% ";
							continue;
						}else if(e == -1){
							break;
						}
						dealNumberedPipes(line, i);
						splitline(cmd, line);
						if(cmd.size() <= 0){
							cout << "% ";
							continue;
						}
						counter = (counter + 1) % MAX_COUNT;
						execute(cmd, -1, line, i);
						cout << "% ";
					}
					usersData[i].numberedPipes = numberedPipes;
					usersData[i].counter = counter;
					if(e == -1){
						dup2(4, 0);
						dup2(5, 1);
						dup2(6, 2);
						close(client[i]);
						FD_CLR(client[i], &allset);
						client[i] = -1;
						for(auto user : usersData){
							if(client[user.f] == -1) continue;
							userPipes.erase({user.f, i});
							userPipes.erase({i, user.f});
							dup2Client(client[user.f]);
							cout << "*** User '" << usersData[i].name << "' left. ***" << '\n';
						}
						usersData.erase(i);
						numUsers--;
					}
				}
				//STD callback
				dup2(4, 0);
				dup2(5, 1);
				dup2(6, 2);
				//
				if(--nready <= 0) break; 
			}
		}
		
	}
}

int main(int argc, char**argv){
	std::cout.setf(std::ios::unitbuf);
	std::cerr.setf(std::ios::unitbuf);
	int port = 11111;
	if(argc > 1){
		sscanf(argv[1], "%d", &port);
	}
	if(port < 1024 || port > 65535){
		cerr << "Error port!";
		exit(1);
	}
	server(port);
	return 0;
}