/*
** Copyright (c) 2002  Hughes Technologies Pty Ltd.  All rights
** reserved.
**
** Terms under which this software may be used or copied are
** provided in the  specific license associated with this product.
**
** Hughes Technologies disclaims all warranties with regard to this
** software, including all implied warranties of merchantability and
** fitness, in no event shall Hughes Technologies be liable for any
** special, indirect or consequential damages or any damages whatsoever
** resulting from loss of use, data or profits, whether in an action of
** contract, negligence or other tortious action, arising out of or in
** connection with the use or performance of this software.
**
**
** $Id: test_httpd.c,v 1.4 2002/03/04 03:53:43 bambi Exp $
**
*/


#include <stdio.h>
#include "httpd.h"


/*
** This is a static page of HTML.  It is loaded into the content
** tree using httpdAddStaticContent( ).
*/
#define test1_html "<HTML><BODY>This is just a test</BODY>"


/*
** Below are 2 dynamic pages, each generated by a C function.  The first
** is a simple page that offers a little dynamic info (the process ID)
** and the setups up a test link and a simple form.
** 
** The second page processes the form.  As you can see, you can access
** the form data from within your C code by accessing the symbol table
** using httpdGetVariableByName() (and other similar functions).  You
** can also include variables in the string passed to httpdOutput( ) and
** they will be expanded automatically.
*/
void index_html(server)
	httpd	*server;
{
	httpdPrintf(server,
	    "Welcome to the httpd server running in process number %d<P>\n",
	    getpid());
	httpdPrintf(server,
	    "Click <A HREF=/test1.html>here</A> to view a test page<P>\n");
	httpdPrintf(server, "<P><FORM ACTION=test2.html METHOD=POST>\n");
	httpdPrintf(server, "Enter your name <INPUT NAME=name SIZE=10>\n");
	httpdPrintf(server, "<INPUT TYPE=SUBMIT VALUE=Click!><P></FORM>\n");
	return;
}

void test2_html(server)
	httpd	*server;
{
	httpVar	*variable;

	/*
	** Grab the symbol table entry to see if the variable exists
	*/
	variable = httpdGetVariableByName(server, "name");
	if (variable == NULL)
	{
		httpdPrintf(server,"Missing form data!");
		return;
	}

	/*
	** Use httpdOutput() rather than httpdPrintf() so that the variable
	** embedded in the text is expanded automatically
	*/
	httpdOutput(server,"Hello $name");
}



int main(argc, argv)
	int	argc;
	char	*argv[];
{
	httpd	*server;

	/*
	** Create a server and setup our logging
	*/
	server = httpdCreate(NULL,80);
	if (server == NULL)
	{
		perror("Can't create server");
		exit(1);
	}
	httpdSetAccessLog(server, stdout);
	httpdSetErrorLog(server, stdout);

	/*
	** Setup some content for the server
	*/
	httpdAddCContent(server,"/", "index.html", HTTP_TRUE, 
		NULL, index_html);
	httpdAddCContent(server,"/", "test2.html", HTTP_FALSE, 
		NULL, test2_html);
	httpdAddStaticContent(server, "/", "test1.html", HTTP_FALSE,
		NULL, test1_html);

	/*
	** Go into our service loop
	*/
	while(1 == 1)
	{
		if (httpdGetConnection(server) < 0)
			continue;
		if(httpdReadRequest(server) < 0)
		{
			httpdEndRequest(server);
			continue;
		}
		httpdProcessRequest(server);
		httpdEndRequest(server);
	}
}