#include "request.hpp"

void help(){
	std::cout << "Uso: hget <URL sem https://>" << std::endl;
}


int main(int argc,char* argv[]){
	if ( argc < 2 ){
		std::cerr << "Argumentos insuficientes." << std::endl;
		help();
		return -1;
	}
	const std::string url = argv[1];
    	size_t pos = url.find('/');
    	std::string hostname = url.substr(0, pos);
    	std::string path = (pos != std::string::npos) ? url.substr(pos) : "/";
	std::cout << "Requistar : " << path << std::endl;
	std::cout << "No endereço : " << hostname << std::endl;
	URL_Request(hostname,path);

	return 0;
}
