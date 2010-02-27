#include "serial.h"

int     tcp_listen (int);
int handle_client (int);
void    cut_crlf (char *);
ssize_t readline (int, void *, size_t);
char   *get_line (FILE *);
FILE   *open_file (char *);
int	ignore_headers(int);


void *
ajax_socket (void *pport)
{
    int     cli, srv;
	int		port;
	int		status;
    int     clisize;
	pid_t	pid;
	port = (int) pport;
	
    struct sockaddr_in cliaddr;
	
	
    srv = tcp_listen (port);
	
    for (;;)
    {
		// pthread_t th_client;
		
		clisize = sizeof (cliaddr);
		if ((cli = accept (srv, (struct sockaddr *) &cliaddr, (socklen_t *) &clisize)) == -1)
			continue;
				
		if ((pid = fork()) == 0) {
			close(srv);
			handle_client(cli);
			exit(0);
		}
		
		waitpid(pid, &status, 0);
		// pthread throws strange errors after a while...
		// pthread_create( &th_client, NULL, handle_client, (void *) cli);

		close (cli);
    }
}

int
tcp_listen (int port)
{
	
    int     on = 1;
    int     sock;
	
    struct sockaddr_in servaddr;
	
    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
		perror ("socket() failed");
		exit (-1);
    }
	
    setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
	
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons (port);
	
    if (bind (sock, (struct sockaddr*) &servaddr, sizeof (servaddr)) == -1)
    {
		perror ("bind() failed");
		exit (-1);
    }
	
    if (listen (sock, 3) == -1)
    {
		perror ("listen() failed");
		exit (-1);
    }
	
    return sock;
}

int
handle_client (int connfd)
{
    char    recv_buf[1024];
    FILE   *fd;
    char   *line;
	
    if (readline (connfd, recv_buf, sizeof (recv_buf)) <= 0)
		return -1;
	
    cut_crlf (recv_buf);
	
    if (!strcmp (recv_buf, "GET / HTTP/1.1"))
    {
		printf ("GET ");
		// read and ignore all http headers
		if (ignore_headers(connfd) == -1)
			return 0;
		
		// send the html file
		fd = open_file ("ajax.html");
		
		printf ("ajax.html\n");
		while ((line = get_line (fd)) != NULL)
		{
			send (connfd, line, strlen (line), 0);
		}
    }
	
    else if (strstr (recv_buf, "GET /speed.png"))
    {
		int file_fd;
		int ret;
		static char buffer[1024+1]; /* static so zero filled */

		// read and ignore all http headers
		if (ignore_headers(connfd) == -1)
			return 0;
		
		if(( file_fd = open("speed.png", O_RDONLY)) == -1)
		{
			perror("couldnt open png file\n");
			return -1;
		}
		write (connfd, "HTTP/1.0 200 OK\r\nContent-Type: image/png\r\n\r\n",
			  strlen("HTTP/1.0 200 OK\r\nContent-Type: image/png\r\n\r\n"));
		
		while ( (ret = read(file_fd, buffer, 1024)) > 0 ) {
			
			(void)write(connfd, buffer, ret);
			
		}
    }	
    else if (strstr (recv_buf, "GET /con_km.png"))
    {
		int file_fd;
		int ret;
		static char buffer[1024+1]; /* static so zero filled */
		
		// read and ignore all http headers
		if (ignore_headers(connfd) == -1)
			return 0;
		
		if(( file_fd = open("con_km.png", O_RDONLY)) == -1)
		{
			perror("couldnt open png file\n");
			return -1;
		}
		
		write (connfd, "HTTP/1.0 200 OK\r\nContent-Type: image/png\r\n\r\n",
			   strlen("HTTP/1.0 200 OK\r\nContent-Type: image/png\r\n\r\n"));
		
		while ( (ret = read(file_fd, buffer, 1024)) > 0 ) {
			
			(void)write(connfd, buffer, ret);
			
		}
    }	
	
    else if (!strcmp (recv_buf, "POST /update_con_km HTTP/1.1"))
    {
		// read and ignore headers
		if (ignore_headers(connfd) == -1)
			return 0;
		
		char    buf[256];
		snprintf (buf, sizeof (buf), "%.02f", con_km);
		send (connfd, buf, strlen (buf), 0);
    }
    else if (!strcmp (recv_buf, "POST /update_speed HTTP/1.1"))
    {
		// read and ignore headers
		if (ignore_headers(connfd) == -1)
			return 0;
		
		char    buf[256];
		snprintf (buf, sizeof (buf), "%.01f", speed);
		send (connfd, buf, strlen (buf), 0);
    }
    else if (!strcmp (recv_buf, "POST /update_rpm HTTP/1.1"))
    {
		// read and ignore headers
		if (ignore_headers(connfd) == -1)
			return 0;
		
		char    buf[256];
		snprintf (buf, sizeof (buf), "%.00f", rpm);
		send (connfd, buf, strlen (buf), 0);
    }	
    else if (!strcmp (recv_buf, "POST /update_con_h HTTP/1.1"))
    {
		// read and ignore headers
		if (ignore_headers(connfd) == -1)
			return 0;
		
		char    buf[256];
		snprintf (buf, sizeof (buf), "%.02f", con_h);
		send (connfd, buf, strlen (buf), 0);
    }	
    else if (!strcmp (recv_buf, "POST /update_load HTTP/1.1"))
    {
		// read and ignore headers
		if (ignore_headers(connfd) == -1)
			return 0;
		
		char    buf[256];
		snprintf (buf, sizeof (buf), "%.00f", load);
		send (connfd, buf, strlen (buf), 0);
    }	
    else if (!strcmp (recv_buf, "POST /update_temp1 HTTP/1.1"))
    {
		// read and ignore headers
		if (ignore_headers(connfd) == -1)
			return 0;
		
		char    buf[256];
		snprintf (buf, sizeof (buf), "%.01f", temp1);
		send (connfd, buf, strlen (buf), 0);
    }	
    else if (!strcmp (recv_buf, "POST /update_temp2 HTTP/1.1"))
    {
		// read and ignore headers
		if (ignore_headers(connfd) == -1)
			return 0;
		
		char    buf[256];
		snprintf (buf, sizeof (buf), "%.01f", temp2);
		send (connfd, buf, strlen (buf), 0);
    }	
    else if (!strcmp (recv_buf, "POST /update_voltage HTTP/1.1"))
    {
		// read and ignore headers
		if (ignore_headers(connfd) == -1)
			return 0;
		
		char    buf[256];
		snprintf (buf, sizeof (buf), "%.02f", voltage);
		send (connfd, buf, strlen (buf), 0);
    }	
    else
		printf ("got something else (%s)\n", recv_buf);

	
    return 0;
}

// read and ignore headers
int
ignore_headers(int fd)
{
	char recv_buf[1024];
	
	do
	{
		if (readline (fd, recv_buf, sizeof (recv_buf)) <= 0)
			return -1;
		
		cut_crlf (recv_buf);
		
	} while (strcmp (recv_buf, ""));
		
	return 0;
}

void
cut_crlf (char *stuff)
{
	
    char   *p;
	
    p = strchr (stuff, '\r');
    if (p)
		*p = '\0';
	
    p = strchr (stuff, '\n');
    if (p)
		*p = '\0';
}

ssize_t
readline (int fd, void *vptr, size_t maxlen)
{
	
    ssize_t n, rc;
    char    c, *ptr;
	
    ptr = vptr;
	
    for (n = 1; n < maxlen; n++)
    {
		
	again:
		if ((rc = read (fd, &c, 1)) == 1)
		{
			*ptr++ = c;
			if (c == '\n')
				break;
		}
		else if (rc == 0)
		{
			if (n == 1)
				return 0;
			else
				break;
		}
		else
		{
			if (errno == EINTR)
				goto again;
			return -1;
		}
    }
	
    *ptr = 0;
    return n;
}


char   *
get_line (FILE * fz)
{
    char    buffy[1024];
    char   *p;
	
    if (fgets (buffy, 1024, fz) == NULL)
		return NULL;
	
    // cut_crlf(buffy);
    p = buffy;
	
    return p;
}



FILE   *
open_file (char *file)
{
	
    FILE   *fd;
	
    if ((fd = fopen (file, "r")) == NULL)
    {
		perror ("could not read html file");
		exit (-1);
    }
	
    return fd;
}