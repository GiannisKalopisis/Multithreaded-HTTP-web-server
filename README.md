# Multithreaded-HTTP-web-server
A multithreaded HTTP Client/Server and a web crawler.


## Bash Script
The bash script webcreator.sh creates W sites with P websites each, in which W and P is given as parameters during execution. It also creates for each website a set of `f = (p / 2) + 1 internal links` to pages of the same site other than itself and a set of `q = (w / 2) + 1 external links` to other sites. It places these as it creates the website for each m lines, where m is a random number with 1000 <m <2000. Finally *it shows if all pages have inbound links*. An indicative run is: 
```
./webcreator.sh root_dir text_file W P
```
where :
  - ***root_dir*** is the directory where the sites will be saved
  - ***text_file*** is the text to read
  - ***W*** is the number of sites
  - ***P*** the number of pages in each site
  
## HTTP Web Server
This part is an HTTP server written in C. One possible execution is:
  
```
./myhttpd -p serving_port -c command_port -t num_of_threads -d root_dir
```
where:
  - **service_port**: is the port at which the server listens to return web pages.
  - **command_port**: is the port where the server listens to give instructions.
  - **num_of_threads**: is the number of threads the server will create in order to handle incoming requests. These threads are all created together at the beginning and are located in a thread pool and from there the server reuses them. In the case of a thread shut down the server starts a new one.
  - **root_dir**: is the directory that contains all the web sites created by webcreator.sh.
  
An example of running server is:
```
./myhttpd -p 8080 -c 9090 -t 4 -d root_dir/  
```
  
At the service port, the server receives HTTP/1.1 requests. The server listens to specific port for form requests:
```
GET /site0/page0_1244.html HTTP/1.1
User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)
Host: www.tutorialspoint.com
Accept-Language: en-us
Accept-Encoding: gzip, deflate
Connection: Keep-Alive
[blank line here]
```

When it receives such a request, the server assigns it to one of the threads in its thread pool. The thread's job is to return the root_dir/site0/page0_1244.html file back to the one requesting it. The Connection: header ignores it and closes the connection regardless of whether it says keep-alive or close.

The server works with the threads as follows: the server accepts a connection to the socket and places the corresponding file descriptor in a buffer from which the threads are read. Each thread is responsible for a file descriptor from which to read the GET request and return the response, that is the content of the web page (or a different response as below if the file does not exist or does not have the appropriate permissions thread).

An indicative request from the browser to the server is: `http://127.0.0.1:8080/site0/page0_8552.html`.

### File exists
If ***the requested file exists***, and the server is allowed to read it, then a format response is returned (the length is in bytes only for the size of the content, ie without the header):

```
HTTP/1.1 200 OK
Date: Mon, 27 May 2018 12:28:53 GMT
Server: myhttpd/1.0.0 (Ubuntu64)
Content-Length: 8873
Content-Type: text/html
Connection: Closed
[blank line]
[content of the requested file here... e.g. <html>hello one two ...</html>]
```


### File does not exist
If the ***requested file does not exist*** then the server must return a form response:
```
HTTP/1.1 404 Not Found
Date: Mon, 27 May 2018 12:28:53 GMT
Server: myhttpd/1.0.0 (Ubuntu64)
Content-Length: 124
Content-Type: text/html
Connection: Closed
[blank line]
<html>Sorry dude, couldn’t find this file.</html>
```

### No permissions
If the ***server has no permissions*** for the file then the answer is as follows:
```
HTTP/1.1 403 Forbidden
Date: Mon, 27 May 2018 12:28:53 GMT
Server: myhttpd/1.0.0 (Ubuntu64)
Content-Length: 124
Content-Type: text/html
Connection: Closed
[blank line]
<html>Trying to access this file but don’t think I can make it.</html>
```


### Server Commands
At the command port, the server listens to and receives the following simple commands (1 word each) that are executed directly by the server without having to be assigned a thread:

  - **STATS**: the server responds by how long it runs, how many pages it has returned, and the total number of bytes (ie `http://127.0.0.1:8080/STATS`).
  - **SHUTDOWN**: The server stops serving additional requests, releases any memory (shared or not) it has allocated, and stops executing (ie `http://127.0.0.1:8080/SHUTDOWN`).


## Web Crawler
This part is a web crawler written in C. The job of the crawler is to download all the web sites from the web server by downloading pages, analyzing them and finding links within the pages that should follow retrospectively. The crawler runs as follows:

```
./mycrawler -h host_or_IP -p port -c command_port -t num_of_threads -d save_dir starting_URL
```

where:
  - **host_or_ip**: is the name of the machine or IP on which the server is running
  - **port**: is the port the server is listening to
  - **command_port**: is the port the crawler listens to to give instructions
  - **num_of_threads**: is the number of threads the crawler runs. If a thread ends the crawler must start a new one.
  - **save_dir**: is the directory in which the crawler will save the downloaded pages. Essentially after the crawler runs (and if all server pages are accessible with a link) save_dir should be an exact copy of root_dir.
  - **starting_URL**: is the URL from which the crawler starts
  
An example of running crawler is:
```
./mycrawler -h 127.0.0.1 -p 8080 -c 7070 -t 5 -d save_dir http://localhost:8080/site0/page0_777.html  
```  
  
### Crawler Workflow
The crawler works as follows:
**1.** Getting started, it creates a thread pool with the corresponding threads. The threads are reused. It also creates a queue to store the links found so far and puts starting_URL in that queue.
**2.** One of the threads takes the URL from the queue and requests it from the server. The URL is in the format (for example) http://linux01.di.uoa.gr:8080/site1/page0_1234.html.
**3.** After downloading the file saves it to the corresponding directory / file inside save_dir.
**4.** Analyzes the file that has just been downloaded and finds other links that queue it.
**5.** Repeat the procedure from 2 with all threads running until there are no more links in the queue.


### Crawler Commands
At the command port the crawler listens and receives the following simple commands (1 word each) that are executed directly by the crawler without having to be assigned a thread:

  - **STATS**: crawler responds by how long it runs, how many pages it has collected and the total number of bytes (ie `http://127.0.0.1:9000/STATS`).
  - **SEARCH** word1 word2 word3 ... word10: crawler returns the number of words that are requested (ie `http://127.0.0.1:9000/SEARCH/word1/word2/.../word10`). If the crawler still has pages in the queue then it returns a message indicating that the crawling is in-progress. If the crawler has finished downloading all web pages, then this command returns the search results over the socket. More specifically it runs the job executor code inside the crawler with the appropriate parameters for the files, and executes the search command and returns the result over the socket for the given query. Workers start only once. HTML tags are ignored when searching/indexing. 
  - **SHUTDOWN**: The crawler stops asking for additional pages, releases any memory (shared or not) it has cached, and stops executing it (ie `http://127.0.0.1:9000/SHUTDOWN`).
  
## Telnet
Command requests can be sent via Telnet, by typing: `telnet localhost command_port`. Connection closes after sending a request.
  
## Compile
You can type:
  - `make` to compile all the code for this project
  - `make cleanAll` to delete all .o files, logs and pipes
  - `make clean/cleanLogFile/cleanPipes` to delete the above files respectively



