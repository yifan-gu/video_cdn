#include <stdio.h>
#include <stdlib.h>

#include <logger.h>
#include <proxy.h>

Proxy proxy;

int main(int argc, char const* argv[])
{
  init_log(NULL);

  proxy.alpha = atof(argv[2]);

  logger(LOG_DEBUG, "Connecting to video server...");
  proxy_conn_server(argv[7]);
  logger(LOG_DEBUG, "Connected video server successfully!");

  logger(LOG_DEBUG, "Proxy starts listening");
  proxy_start_listen(argv[4], argv[3]);

  return 0;
}

int proxy_conn_server(const char * server_ip){
  
}

int proxy_start_listen(const char *local_ip, const char *port){
  
}
