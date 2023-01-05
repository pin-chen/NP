#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <thread>
#define f first
#define s second
#define ul unsigned long int

using boost::asio::ip::tcp;
using namespace std;

const string panel = "<!DOCTYPE html>\n<html lang=\"en\">\n  <head>\b    <title>NP Project 3 Panel</title>\n    <link\n      rel=\"stylesheet\"\n      href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"\n      integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"\n      crossorigin=\"anonymous\"\n    />\n    <link\n      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\n      rel=\"stylesheet\"\n    />\n    <link\n      rel=\"icon\"\n      type=\"image/png\"\n      href=\"https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png\"\n    />\n    <style>\n      * {\n        font-family: 'Source Code Pro', monospace;\n      }\n    </style>\n  </head>\n  <body class=\"bg-secondary pt-5\">\n    <form action=\"console.cgi\" method=\"GET\">\n      <table class=\"table mx-auto bg-light\" style=\"width: inherit\">\n        <thead class=\"thead-dark\">\n          <tr>\n            <th scope=\"col\">#</th>\n            <th scope=\"col\">Host</th>            <th scope=\"col\">Port</th>            <th scope=\"col\">Input File</th>\n          </tr>\n        </thead>\n        <tbody>\n          <tr>\n            <th scope=\"row\" class=\"align-middle\">Session 1</th>            <td>\n              <div class=\"input-group\">\n                <select name=\"h0\" class=\"custom-select\">\n                  <option></option><option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option><option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option><option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option><option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option><option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option><option value=\"nplinux6.cs.nctu.edu.tw\">nplinux6</option><option value=\"nplinux7.cs.nctu.edu.tw\">nplinux7</option><option value=\"nplinux8.cs.nctu.edu.tw\">nplinux8</option><option value=\"nplinux9.cs.nctu.edu.tw\">nplinux9</option><option value=\"nplinux10.cs.nctu.edu.tw\">nplinux10</option><option value=\"nplinux11.cs.nctu.edu.tw\">nplinux11</option><option value=\"nplinux12.cs.nctu.edu.tw\">nplinux12</option>\n                </select>\n                <div class=\"input-group-append\">\n                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>\n                </div>\n              </div>\n            </td>\n            <td>\n              <input name=\"p0\" type=\"text\" class=\"form-control\" size=\"5\" />\n            </td>\n            <td>\n              <select name=\"f0\" class=\"custom-select\">\n                <option></option>\n                <option value=\"t1.txt\">t1.txt</option><option value=\"t2.txt\">t2.txt</option><option value=\"t3.txt\">t3.txt</option><option value=\"t4.txt\">t4.txt</option><option value=\"t5.txt\">t5.txt</option>\n              </select>\n            </td>\n          </tr>\n          <tr>\n            <th scope=\"row\" class=\"align-middle\">Session 2</th>\n            <td>\n              <div class=\"input-group\">\n                <select name=\"h1\" class=\"custom-select\">\n                  <option></option><option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option><option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option><option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option><option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option><option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option><option value=\"nplinux6.cs.nctu.edu.tw\">nplinux6</option><option value=\"nplinux7.cs.nctu.edu.tw\">nplinux7</option><option value=\"nplinux8.cs.nctu.edu.tw\">nplinux8</option><option value=\"nplinux9.cs.nctu.edu.tw\">nplinux9</option><option value=\"nplinux10.cs.nctu.edu.tw\">nplinux10</option><option value=\"nplinux11.cs.nctu.edu.tw\">nplinux11</option><option value=\"nplinux12.cs.nctu.edu.tw\">nplinux12</option>\n                </select>\n                <div class=\"input-group-append\">\n                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>\n                </div>\n              </div>\n            </td>\n            <td>\n              <input name=\"p1\" type=\"text\" class=\"form-control\" size=\"5\" />\n            </td>\n            <td>\n              <select name=\"f1\" class=\"custom-select\">\n                <option></option>\n                <option value=\"t1.txt\">t1.txt</option><option value=\"t2.txt\">t2.txt</option><option value=\"t3.txt\">t3.txt</option><option value=\"t4.txt\">t4.txt</option><option value=\"t5.txt\">t5.txt</option>\n              </select>\n            </td>\n          </tr>\n          <tr>\n            <th scope=\"row\" class=\"align-middle\">Session 3</th>\n            <td>\n              <div class=\"input-group\">\n                <select name=\"h2\" class=\"custom-select\">\n                  <option></option><option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option><option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option><option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option><option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option><option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option><option value=\"nplinux6.cs.nctu.edu.tw\">nplinux6</option><option value=\"nplinux7.cs.nctu.edu.tw\">nplinux7</option><option value=\"nplinux8.cs.nctu.edu.tw\">nplinux8</option><option value=\"nplinux9.cs.nctu.edu.tw\">nplinux9</option><option value=\"nplinux10.cs.nctu.edu.tw\">nplinux10</option><option value=\"nplinux11.cs.nctu.edu.tw\">nplinux11</option><option value=\"nplinux12.cs.nctu.edu.tw\">nplinux12</option>\n                </select>\n                <div class=\"input-group-append\">\n                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>\n                </div>\n              </div>\n            </td>\n            <td>\n              <input name=\"p2\" type=\"text\" class=\"form-control\" size=\"5\" />\n            </td>\n            <td>\n              <select name=\"f2\" class=\"custom-select\">\n                <option></option>\n                <option value=\"t1.txt\">t1.txt</option><option value=\"t2.txt\">t2.txt</option><option value=\"t3.txt\">t3.txt</option><option value=\"t4.txt\">t4.txt</option><option value=\"t5.txt\">t5.txt</option>\n              </select>\n            </td>\n          </tr>\n          <tr>\n            <th scope=\"row\" class=\"align-middle\">Session 4</th>\n            <td>\n              <div class=\"input-group\">\n                <select name=\"h3\" class=\"custom-select\">\n                  <option></option><option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option><option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option><option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option><option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option><option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option><option value=\"nplinux6.cs.nctu.edu.tw\">nplinux6</option><option value=\"nplinux7.cs.nctu.edu.tw\">nplinux7</option><option value=\"nplinux8.cs.nctu.edu.tw\">nplinux8</option><option value=\"nplinux9.cs.nctu.edu.tw\">nplinux9</option><option value=\"nplinux10.cs.nctu.edu.tw\">nplinux10</option><option value=\"nplinux11.cs.nctu.edu.tw\">nplinux11</option><option value=\"nplinux12.cs.nctu.edu.tw\">nplinux12</option>\n                </select>\n                <div class=\"input-group-append\">\n                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>\n                </div>\n              </div>\n            </td>\n            <td>\n              <input name=\"p3\" type=\"text\" class=\"form-control\" size=\"5\" />\n            </td>\n            <td>\n              <select name=\"f3\" class=\"custom-select\">\n                <option></option>\n                <option value=\"t1.txt\">t1.txt</option><option value=\"t2.txt\">t2.txt</option><option value=\"t3.txt\">t3.txt</option><option value=\"t4.txt\">t4.txt</option><option value=\"t5.txt\">t5.txt</option>\n              </select>\n            </td>\n          </tr>\n          <tr>\n            <th scope=\"row\" class=\"align-middle\">Session 5</th>\n            <td>\n              <div class=\"input-group\">\n                <select name=\"h4\" class=\"custom-select\">\n                  <option></option><option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option><option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option><option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option><option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option><option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option><option value=\"nplinux6.cs.nctu.edu.tw\">nplinux6</option><option value=\"nplinux7.cs.nctu.edu.tw\">nplinux7</option><option value=\"nplinux8.cs.nctu.edu.tw\">nplinux8</option><option value=\"nplinux9.cs.nctu.edu.tw\">nplinux9</option><option value=\"nplinux10.cs.nctu.edu.tw\">nplinux10</option><option value=\"nplinux11.cs.nctu.edu.tw\">nplinux11</option><option value=\"nplinux12.cs.nctu.edu.tw\">nplinux12</option>\n                </select>\n                <div class=\"input-group-append\">\n                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>\n                </div>\n              </div>\n            </td>\n            <td>\n              <input name=\"p4\" type=\"text\" class=\"form-control\" size=\"5\" />\n            </td>\n            <td>\n              <select name=\"f4\" class=\"custom-select\">\n                <option></option>\n                <option value=\"t1.txt\">t1.txt</option><option value=\"t2.txt\">t2.txt</option><option value=\"t3.txt\">t3.txt</option><option value=\"t4.txt\">t4.txt</option><option value=\"t5.txt\">t5.txt</option>\n              </select>\n            </td>\n          </tr>\n          <tr>\n            <td colspan=\"3\"></td>\n            <td>\n              <button type=\"submit\" class=\"btn btn-info btn-block\">Run</button>\n            </td>\n          </tr>\n        </tbody>\n      </table>\n    </form>\n  </body>\n</html>";

struct shellInfo{
	string host;
	string port;
	string cmdFile;
};

void send_(tcp::socket &socket, const string &message) {
	boost::asio::write( socket, boost::asio::buffer(message) );
}

class shellClient : public std::enable_shared_from_this<shellClient>{
	public:
		shellClient(tcp::socket &socket, map<ul, shellInfo> shellServer, boost::asio::io_context& io_context, ul index) : outSocket(socket), shellServer(shellServer), resolver(io_context), socket_(io_context), index(index){}

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
			string out = "<script>document.getElementById('s" + to_string(index) + "').innerHTML += '" + content + "';</script>\n";
			send_(outSocket, out);
		}
		
		void output_command(string content){
			escape(content);
			string out = "<script>document.getElementById('s" + to_string(index) + "').innerHTML += '<b>" + content + "</b>';</script>\n";
			send_(outSocket, out);
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
		tcp::socket &outSocket;
		map<ul, shellInfo> shellServer;
		int fd;
		tcp::resolver resolver;
		tcp::socket socket_;
		ul index;
		tcp::resolver::results_type endpoint_;
		ifstream in;
		enum { max_length = 40960 };
		char data_[max_length];
};

void console_splitArgument(vector<string> &dst, string &src){
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

void console_splitline(vector<vector<string>> &dst, string src){
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
		console_splitArgument(dst[i], cmd);
		i++;
	}
}

void getShellServerInfo(map<ul, shellInfo> &shellServer, map<string, string> &env){
	vector<vector<string>> query;
	console_splitline(query, env["QUERY_STRING"]);
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

string http(map<ul, shellInfo> &shellServer){
	string out = "";
	out += "<!DOCTYPE html>\n"																				;
	out += "<html lang=\"en\">\n"																			;
	out += " <head>\n"																						;
	out += "    <meta charset=\"UTF-8\" />\n"																;
	out += "    <title>NP Project 3 Sample Console</title>\n"												;
	out += "    <link\n"																					;
	out += "      rel=\"stylesheet\"\n"																		;
	out += "      href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"\n"		;
	out += "      integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"\n"	;
	out += "      crossorigin=\"anonymous\"\n"																;
	out += "    />\n"																						;
	out += "    <link\n"																					;
	out += "      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\n"						;
	out += "      rel=\"stylesheet\"\n"																		;
	out += "    />\n"																						;
	out += "    <link\n"																					;
	out += "      rel=\"icon\"\n"																			;
	out += "      type=\"image/png\"\n"																		;
	out += "      href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"\n";
	out += "    />\n"																						;
	out += "    <style>\n"																					;
	out += "      * {\n"																					;
	out += "        font-family: 'Source Code Pro', monospace;\n"											;
	out += "        font-size: 1rem !important;\n"															;
	out += "      }\n"																						;
	out += "      body {\n"																					;
	out += "        background-color: #212529;\n"															;
	out += "      }\n"																						;
	out += "      pre {\n"																					;
	out += "        color: #cccccc;\n"																		;
	out += "      }\n"																						;
	out += "      b {\n"																					;
	out += "        color: #01b468;\n"																		;
	out += "      }\n"																						;
	out += "    </style>\n"																					;
	out += "  </head>\n"																					;
	out += "  <body>\n"																						;
	out += "    <table class=\"table table-dark table-bordered\">\n"										;
	out += "      <thead>\n"																				;
	out += "        <tr>\n"																					;
	for(ul i = 0; i < shellServer.size(); i++){
		out += "          <th scope=\"col\">" + shellServer[i].host + ":" + shellServer[i].port+ "</th>"	;
	}
	out += "        </tr>\n"																				;
	out += "      </thead>\n"																				;
	out += "      <tbody>\n"																				;
	out += "        <tr>\n"																					;
	for(ul i = 0; i < shellServer.size(); i++){
		out += "          <td><pre id=\"s" + to_string(i) + "\" class=\"mb-0\"></pre></td>\n"				;
	}
	out += "        </tr>\n"																				;
	out += "      </tbody>\n"																				;
	out += "    </table>\n"																					;
	out += "  </body>\n"																					;
	out += "</html>\n"																						;
	return out;
}

void console(tcp::socket &socket_, map<string, string> &env){
	try{
		map<ul, shellInfo> shellServer;
		getShellServerInfo(shellServer, env);
		send_(socket_, http(shellServer));
		boost::asio::io_context io_context;
		for(ul i = 0; i < shellServer.size(); i++){
			std::make_shared<shellClient>(socket_ ,shellServer, io_context, i)->start();
		}
		io_context.run();
		socket_.close();
	}catch (std::exception& e){
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

class session : public std::enable_shared_from_this<session>{
	public:
		session(tcp::socket socket) : socket_(std::move(socket)){}

		void start(){
			do_read();
		}

	private:
		void do_read(){
			auto self(shared_from_this());
			socket_.async_read_some(
				boost::asio::buffer(data_, max_length), 
				[this, self](boost::system::error_code ec, std::size_t length){
					if (!ec){
						data_[length] = '\0';
						fillEnv(string(data_));
						memset(data_, '\0', sizeof(data_));
						string header = "HTTP/1.1 200 OK\r\nServer: http_server\r\nContent-type: text/html\r\n\r\n";
						if(path == "/panel.cgi"){
							do_write(header);
							do_write(panel);
							socket_.close();
						}else if(path == "/console.cgi"){
							do_write(header);
							std::thread t(console,  std::ref(socket_), std::ref(env));
							t.detach();
						}else{
							header = "HTTP/1.1 404 NOT FOUND\r\n\r\n";
							do_write(header);
							//socket_.close();
						}
						cout << "do_read()";
						do_read();
					}
				}
			);
		}
		
		void do_write(string out){
			auto self(shared_from_this());
			boost::asio::async_write(
				socket_, boost::asio::buffer(out, out.size()),
				[this, self](boost::system::error_code ec, std::size_t length){
					if (!ec){
						
					}
				}
			);
		}
		
		void cleanEnv(){
			env["REQUEST_METHOD"] = "";
			env["REQUEST_URI"] = "";
			env["QUERY_STRING"] = "";
			env["SERVER_PROTOCOL"] = "";
			env["HTTP_HOST"] = "";
			env["SERVER_ADDR"] = "";
			env["SERVER_PORT"] = "";
			env["REMOTE_ADDR"] = "";
			env["REMOTE_PORT"] = "";
		}

		void splitArgument(vector<string> &dst, string &src){
			dst.clear();
			string arg;
			unsigned int cur = 0, charType = 0;
			for(; cur < src.size(); cur++){
				if(src[cur] == ' '){
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
				if(src[cur] == '\n'){
					tmp.push_back(src.substr(begin, cur - begin));
					begin = cur + 1;
				}
			}
			dst.resize(tmp.size());
			int i = 0;
			for(string cmd : tmp){
				splitArgument(dst[i], cmd);
				i++;
			}
		}

		char **stoc(vector<string> src){ //vector<string> to char[][]
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

		void fillEnv(string src){
			vector<vector<string>> cmd;
			splitline(cmd, src);
			if(cmd.size() < 2){
				cerr << "cmd.size() < 2\n";
				return;
			}
			env["REQUEST_METHOD"] = cmd[0][0];
			env["REQUEST_URI"] = cmd[0][1];
			int i = cmd[0][1].find('?');
			if(i == -1){
				env["QUERY_STRING"] = "";
				path = cmd[0][1];
			}else{
				env["QUERY_STRING"] = cmd[0][1].substr(i + 1);
				path = cmd[0][1].substr(0, i);
			}
			env["SERVER_PROTOCOL"] = cmd[0][2];
			env["HTTP_HOST"] = cmd[1][1];
			env["SERVER_ADDR"] = socket_.local_endpoint().address().to_string();
			env["SERVER_PORT"] = to_string(socket_.local_endpoint().port());
			env["REMOTE_ADDR"] = socket_.remote_endpoint().address().to_string();
			env["REMOTE_PORT"] = to_string(socket_.remote_endpoint().port());
		}
		tcp::socket socket_;
		map<string, string> env;
		string path;
		int state;
		enum { max_length = 4096 };
		char data_[max_length];
};

class server{
	public:
		server(boost::asio::io_context& io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)){
			do_accept();
		}

	private:
		void do_accept(){
			acceptor_.async_accept(
				[this](boost::system::error_code ec, tcp::socket socket){
					if (!ec){
						std::make_shared<session>(std::move(socket))->start();
					}
					do_accept();
				}
			);
		}
		tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]){
	try{
		if (argc != 2){
			std::cerr << "Usage: async_tcp_echo_server <port>\n";
			return 1;
		}
		boost::asio::io_context io_context;
		server s(io_context, std::atoi(argv[1]));
		io_context.run();
	}catch (std::exception& e){
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}