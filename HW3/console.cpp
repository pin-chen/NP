#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#define f first
#define s second
#define ul unsigned long int

using boost::asio::ip::tcp;
using namespace std;

struct shellInfo{
	string host;
	string port;
	string cmdFile;
};

map<string, string> env;
map<ul, shellInfo> shellServer;
vector<string> envSet = {"REQUEST_METHOD", "REQUEST_URI", "QUERY_STRING", "SERVER_PROTOCOL", "HTTP_HOST", "SERVER_ADDR", "SERVER_PORT", "REMOTE_ADDR", "REMOTE_PORT"};

class shellClient : public std::enable_shared_from_this<shellClient>{
	public:
		shellClient(boost::asio::io_context& io_context, ul index) : resolver(io_context), socket_(io_context), index(index){}

		void start(){
			do_resolve();
		}

	private:
		void do_resolve(){
			auto self(shared_from_this());
			resolver.async_resolve(
				shellServer[index].host, shellServer[index].port,
				[this, self](boost::system::error_code ec, tcp::resolver::results_type result){
					if(!ec){
						memset(data_, '\0', sizeof(data_));
						endpoint_ = result;
						do_connect();
					}else{
						cerr << "resolv" << '\n';
						socket_.close();
					}
				}
			);
		}
		
		void do_connect(){
			auto self(shared_from_this());
			boost::asio::async_connect(
					socket_, endpoint_,
					[this, self](boost::system::error_code ec, tcp::endpoint ed){
					if(!ec){
						memset(data_, '\0', sizeof(data_));
						in.open("./test_case/" + shellServer[index].cmdFile);
						if(!in.is_open()){
							cout << shellServer[index].cmdFile << " open fail\n";
							socket_.close();
						}
						do_read();
					}else{
						cerr << "connect" << '\n';
						socket_.close();
					}
				}
			);
		}
	
		void do_read(){
			auto self(shared_from_this());
			socket_.async_read_some(
				boost::asio::buffer(data_, max_length), 
				[this, self](boost::system::error_code ec, std::size_t length){
					if (!ec){
						if(length == 0) return;
						data_[length] = '\0';
						string msg = string(data_);
						memset(data_, '\0', sizeof(data_));
						output_message(msg);
						if(msg.find("% ") != string::npos){
							do_write();
						}else{
							do_read();
						}
					}else{
						cerr << "read" << '\n';
						socket_.close();
					}
				}
			);
		}

		void do_write(){
			auto self(shared_from_this());
			string cmd;
			getline(in, cmd);
			cmd.push_back('\n');
			output_command(cmd);
			boost::asio::async_write(
				socket_, boost::asio::buffer(cmd, cmd.size()),
				[this, self](boost::system::error_code ec, std::size_t length){
					if (!ec){
						do_read();
					}
				}
			);
		}
		
		void output_message(string content){
			escape(content);
			cout << "<script>document.getElementById('s" << index << "').innerHTML += '" << content << "';</script>\n" << flush;
		}
		
		void output_command(string content){
			escape(content);
			cout << "<script>document.getElementById('s" << index << "').innerHTML += '<b>" << content << "</b>';</script>\n" << flush;
		}
		
		void escape(string &src){
			boost::replace_all(src, "&", "&amp;");
			boost::replace_all(src, "\r", "");
			boost::replace_all(src, "\n", "&NewLine;");
			boost::replace_all(src, "\'", "&apos;");
			boost::replace_all(src, "\"", "&quot;");
			boost::replace_all(src, "<", "&lt;");
			boost::replace_all(src, ">", "&gt;");
		}
		tcp::resolver resolver;
		tcp::socket socket_;
		ul index;
		tcp::resolver::results_type endpoint_;
		ifstream in;
		enum { max_length = 40960 };
		char data_[max_length];
};

void getEnv(){
	for(auto e : envSet){
		env[e] = string(getenv(e.c_str()));
	}
}

void splitArgument(vector<string> &dst, string &src){
	dst.clear();
	string arg;
	unsigned int cur = 0, charType = 0;
	for(; cur < src.size(); cur++){
		if(src[cur] == '='){
			if(charType == 1){
				dst.push_back(arg);
				arg.clear();
				charType = 0;
			}
		}else if(src[cur] == '\r'){
			continue;
		}else{
			if(charType == 0) charType = 1;
			arg.push_back(src[cur]);
		}
	}
	if(arg.size() != 0){
		dst.push_back(arg);
	}
}

void splitline(vector<vector<string>> &dst, string src){
	dst.clear();
	vector<string> tmp;
	unsigned int begin = 0, cur = 0;
	for(; cur < src.size(); cur++){
		if(src[cur] == '&'){
			tmp.push_back(src.substr(begin, cur - begin));
			begin = cur + 1;
		}
	}
	if(begin != src.size()) tmp.push_back(src.substr(begin));
	dst.resize(tmp.size());
	int i = 0;
	for(string cmd : tmp){
		splitArgument(dst[i], cmd);
		i++;
	}
}

void getShellServerInfo(){
	vector<vector<string>> query;
	splitline(query, env["QUERY_STRING"]);
	for(ul i = 0; i < query.size(); i++){
		if(query[i].size() != 2) continue;
		char type = query[i][0][0];
		ul index = query[i][0][1] - '0';
		switch(type){
			case 'h':{
				shellServer[index].host = query[i][1];
				break;
			}
			case 'p':{
				shellServer[index].port = query[i][1];
				break;
			}
			case 'f':{
				shellServer[index].cmdFile = query[i][1];
				break;
			}
		}
	}
}

void http(){
	cout << "<!DOCTYPE html>" 																							<< '\n';
	cout << "<html lang=\"en\">" 																						<< '\n';
	cout << "  <head>" 																									<< '\n';
	cout << "    <meta charset=\"UTF-8\" />" 																			<< '\n';
	cout << "    <title>NP Project 3 Sample Console</title>" 															<< '\n';
	cout << "    <link" 																								<< '\n';
	cout << "      rel=\"stylesheet\"" 																					<< '\n';
	cout << "      href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"" 					<< '\n';
	cout << "      integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"" 				<< '\n';
	cout << "      crossorigin=\"anonymous\"" 																			<< '\n';
	cout << "    />" 																									<< '\n';
	cout << "    <link" 																								<< '\n';
	cout << "      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"" 									<< '\n';
	cout << "      rel=\"stylesheet\"" 																					<< '\n';
	cout << "    />" 																									<< '\n';
	cout << "    <link" 																								<< '\n';
	cout << "      rel=\"icon\"" 																						<< '\n';
	cout << "      type=\"image/png\"" 																					<< '\n';
	cout << "      href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"" 			<< '\n';
	cout << "    />" 																									<< '\n';
	cout << "    <style>" 																								<< '\n';
	cout << "      * {" 																								<< '\n';
	cout << "        font-family: 'Source Code Pro', monospace;" 														<< '\n';
	cout << "        font-size: 1rem !important;" 																		<< '\n';
	cout << "      }" 																									<< '\n';
	cout << "      body {" 																								<< '\n';
	cout << "        background-color: #212529;" 																		<< '\n';
	cout << "      }" 																									<< '\n';
	cout << "      pre {" 																								<< '\n';
	cout << "        color: #cccccc;" 																					<< '\n';
	cout << "      }" 																									<< '\n';
	cout << "      b {" 																								<< '\n';
	cout << "        color: #01b468;" 																					<< '\n';
	cout << "      }" 																									<< '\n';
	cout << "    </style>" 																								<< '\n';
	cout << "  </head>" 																								<< '\n';
	cout << "  <body>" 																									<< '\n';
	cout << "    <table class=\"table table-dark table-bordered\">" 													<< '\n';
	cout << "      <thead>" 																							<< '\n';
	cout << "        <tr>" 																								<< '\n';
	for(ul i = 0; i < shellServer.size(); i++){
		cout << "          <th scope=\"col\">" << shellServer[i].host << ":" << shellServer[i].port << "</th>"			<< '\n';
	}
	cout << "        </tr>" 																							<< '\n';
	cout << "      </thead>" 																							<< '\n';
	cout << "      <tbody>" 																							<< '\n';
	cout << "        <tr>" 																								<< '\n';
	for(ul i = 0; i < shellServer.size(); i++){
		cout << "          <td><pre id=\"s" << i << "\" class=\"mb-0\"></pre></td>" 									<< '\n';
	}
	cout << "        </tr>" 																							<< '\n';
	cout << "      </tbody>" 																							<< '\n';
	cout << "    </table>" 																								<< '\n';
	cout << "  </body>" 																								<< '\n';
	cout << "</html>" 																									<< '\n';
	cout << flush;
}

int main(int argc, char* argv[]){
	try{
		cout << "Content-type: text/html\r\n\r\n" << flush;
		getEnv();
		getShellServerInfo();
		http();
		boost::asio::io_context io_context;
		for(ul i = 0; i < shellServer.size(); i++){
			std::make_shared<shellClient>(io_context, i)->start();
		}
		io_context.run();
	}catch (std::exception& e){
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}