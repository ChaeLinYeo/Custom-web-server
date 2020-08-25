# Network-Simple-web-server
How to run <br>
```
make
```
First, run makefile.
```
./server 8080
```
Run the server with ./server 8080 in cmd.<br>
It is recommended to use 1024 or higher port numbers after ./server.<br>
```
http://127.0.0.1:8080
```
Open a browser and enter the address as above. <br>
If the server and the client are running on the same computer, use localhost or 127.0.0.1 as the address for access in the browser.<br>
Check that HTML, GIF, JPEG, MP3 and PDF files are recognized.<br>
You can check by clicking each link in the html page, or you can check by entering the address as follows.<br>
```
http://127.0.0.1:8080/index.html  //html file
```
```
http://127.0.0.1:8080/sample.jpg  //jpg image file
```
```
http://127.0.0.1:8080/sample.gif  //gif motion picture file 
```
```
http://127.0.0.1:8080/sample.pdf  //pdf document file
```
```
http://127.0.0.1:8080/sample.mp3  //mp3 music file
```
For more details, see 2017012197.pdf in the myTCPWebServer file.<br>
The ClientServer_Example file is an example on which this project is based.<br>
