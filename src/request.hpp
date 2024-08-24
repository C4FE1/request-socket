#include "includers.hpp"

int CreateConnectSocket(const std::string hostname, int port){
	// Criação do socket do tipo SOCK_STREAM para usar o protocolo TCP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd<0){
		//Na linha de cima AF_INET identifica qual tipo de endereço nesse caso IPv4
		std::cerr << "Nao foi possivel criar o socket." << std::endl;
		exit(1);
	}
	
	std::cout << "Socket Criado com sucesso." << std::endl;

	// Transformação do endereço para a requisição
	struct hostent* server = gethostbyname(hostname.c_str());
	if (server== nullptr){
		std::cerr << "Nao foi possivel achar o host." << std::endl;
		close(sockfd);
		exit(1);
	}

	std::cout << "Host foi acessado com sucesso." << std::endl;

	// Criação de uma struct para organizar e acessar informaçãoes sobre a porta
	struct sockaddr_in server_addr;
    	memset(&server_addr, 0, sizeof(server_addr)); // Limpar a estrutura
    	server_addr.sin_family = AF_INET;
    	server_addr.sin_port = htons(port);
    	memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

	if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        	std::cerr << "Nao foi possivel conectar ao socket" << std::endl;
        	close(sockfd);
        	exit(1);
    	}
	std::cout << "Socket Conectado com sucesso." << std::endl;

	return sockfd;
}

// Inicia a biblioteca OPENSSL
void InitializeOpenSSL() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// Cria o contexto ssl
SSL_CTX* CreateContext() {
    	const SSL_METHOD* method = TLS_client_method();
    	SSL_CTX* ctx = SSL_CTX_new(method);
	if (!ctx) {
        	ERR_print_errors_fp(stderr);
		exit(1);
    	}
	return ctx;
}

// Cria um objeto ssl (Certificado)
SSL* CreateSSL(SSL_CTX* ctx, int sockfd) {
	SSL* ssl = SSL_new(ctx);
	if (!ssl) {
		ERR_print_errors_fp(stderr);
        	exit(1);
    	}
    	SSL_set_fd(ssl, sockfd);
    	return ssl;
}

// Conecta ao servidor usando o cerficado ssl/tls
void ConnectSSL(SSL* ssl) {
	if (SSL_connect(ssl) <= 0) {
		ERR_print_errors_fp(stderr);
        	exit(1);
   	}
}

// Esta função recebe o certificado ssl/tls e faz a requisição ao endereço associado const std::string request
void AskHttpRequest(SSL* ssl,const std::string& Http,const std::string& path){
	const std::string request = 
		"GET "+ path + " HTTP/1.1\r\n"
        	"Host: " + Http + "\r\n"
        	"Connection: close\r\n"
        	"\r\n";

	// Lida com a criptografia para requisição em sites seguros (HTTPS)
	if (SSL_write(ssl, request.c_str(), request.length()) <= 0) {
        	ERR_print_errors_fp(stderr);
        	exit(1);
    	}
}

// Esta função recebe a resposta da requisição feita ao endereço associado const std::string request
void ListenHttpResponse(SSL* ssl, const std::string& path){
	char buffer[4096];

	int bytes_recebidos;

	size_t pos = path.rfind('/');

	const std::string filename = path.substr(pos+1);
	
	std::ofstream file(filename, std::ios::binary);

    	if (!file.is_open()) {
        	std::cerr << "Erro ao tentar abrir arquivo para escrita: " << filename << std::endl;
        	return;
    	}

	bool header_ended = false;
	std::string header_buffer;

	while ((bytes_recebidos = SSL_read(ssl, buffer, sizeof(buffer) - 1)) > 0) {
        	buffer[bytes_recebidos] = '\0';
		if (!header_ended) {
            		std::string response_part(buffer);
            		size_t header_end_pos = response_part.find("\r\n\r\n");

            		if (header_end_pos != std::string::npos) {
				header_end_pos += 4;
                		file.write(response_part.substr(header_end_pos).c_str(), response_part.length() - header_end_pos);
                		header_ended = true;
            		
			} else {
                		header_buffer += response_part;
            		}

        	} else {
            		file.write(buffer, bytes_recebidos);
        	}	
    	}
    	if (bytes_recebidos < 0) {
        	ERR_print_errors_fp(stderr);
    	}
	file.close();
}

// Fecha o socket e finaliza a comunicação SSL/TLS
void cleanup(SSL* ssl, SSL_CTX* ctx, int sockfd) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    EVP_cleanup();
}

// Une todas as funções acima para fazer URL Request 
void URL_Request(const std::string& hostname , const std::string& path ){
	InitializeOpenSSL();
	SSL_CTX* ctx = CreateContext();
    	int sockfd = CreateConnectSocket(hostname, 443); // HTTPS port 443
    	SSL* ssl = CreateSSL(ctx, sockfd);
    	ConnectSSL(ssl);
    	AskHttpRequest(ssl, hostname, path);
	ListenHttpResponse(ssl,path);
	cleanup(ssl, ctx, sockfd);
}
