var http = require('http');
var fs = require('fs');
var path = require('path');
const WebSocket = require('ws');
const PORT = process.env.PORT || 5000;

const COOKIE_HEADER = "STODOLA_LOGIN";

function parseCookies (request) {
    const list = {};
    const cookieHeader = request.headers?.cookie;
    if (!cookieHeader) return list;

    cookieHeader.split(`;`).forEach(function(cookie) {
        let [ name, ...rest] = cookie.split(`=`);
        name = name?.trim();
        if (!name) return;
        const value = rest.join(`=`).trim();
        if (!value) return;
        list[name] = decodeURIComponent(value);
    });

    return list;
}

var server = http.createServer(function (request, response) {
    console.log('request ', request.url);
	
	if(request.method == 'POST'){
		var body = '';
        request.on('data', function (data) {
            body += data;
        });
        request.on('end', function () {
            try {
				console.log(body);
				var post = JSON.parse(body);
				console.log(post);
                if("user" in post && "psw" in post){
				    if((post["user"] == "kuba" || post["user"] == "pawel") && post["psw"] == "admin"){
					    response.writeHead(200, {
							"Set-Cookie": COOKIE_HEADER+"=1",
							"Content-Type": "text/plain"
						});
					    response.end();
				    } 
				    else{
					    response.writeHead(200, {
							"Set-Cookie": COOKIE_HEADER+"=0",
							"Content-Type": "text/plain"
						});
						response.write('failed');
					    response.end();
				    }
			    } 
                return;
            }catch (err){
				console.log(err);
                response.writeHead(500, {"Content-Type": "text/plain"});
                response.write("Bad Post Data.  Is your data a proper JSON?\n");
                response.end();
                return;
            }
        });
	}
	
	if(request.url == "/resetwifi"){
		console.log("in reste");
		//filePath = './index.html';
		sockets.forEach(s => s.send("wifi"));
	}
	
	var filePath = '.' + request.url;
    if (filePath == './') {
        filePath = './loginPage.html';
    }
	
	const cookies = parseCookies(request);
	console.log(cookies);
	if( COOKIE_HEADER in cookies){
		if(cookies[COOKIE_HEADER] == "1"){
			filePath = './index.html';
		}
	}

    var extname = String(path.extname(filePath)).toLowerCase();
    var mimeTypes = {
        '.html': 'text/html',
        '.js': 'text/javascript',
        '.css': 'text/css',
        '.json': 'application/json',
        '.png': 'image/png',
        '.jpg': 'image/jpg',
        '.gif': 'image/gif',
        '.svg': 'image/svg+xml',
        '.wav': 'audio/wav',
        '.mp4': 'video/mp4',
        '.woff': 'application/font-woff',
        '.ttf': 'application/font-ttf',
        '.eot': 'application/vnd.ms-fontobject',
        '.otf': 'application/font-otf',
        '.wasm': 'application/wasm'
    };

    var contentType = mimeTypes[extname] || 'application/octet-stream';

    fs.readFile(filePath, function(error, content) {
        if (error) {
            if(error.code == 'ENOENT') {
                fs.readFile('./404.html', function(error, content) {
                    response.writeHead(404, { 'Content-Type': 'text/html' });
                    response.end(content, 'utf-8');
                });
            }
            else {
                response.writeHead(500);
                response.end('Sorry, check with the site admin for error: '+error.code+' ..\n');
            }
        }
        else {
            response.writeHead(200, { 'Content-Type': contentType });
            response.end(content, 'utf-8');
        }
    });

}).listen(PORT);

//server.listen(PORT);
console.log('Server running at http://127.0.0.1:80/');



var values = {
	msgType: "default",
	temp: 22,
	humi: 10,
	heat: 24,
	heating: "OFF",
	time: 13123,
};


const wsServer = new WebSocket.Server({
  //port: 3000
  server: server
});

let sockets = [];
wsServer.on('connection', function(socket) {
  sockets.push(socket);

  // When you receive a message, send that message to every socket.
  socket.on('message', function(msg) {
	  //console.log(`msg: ${msg}`)
	  if(msg == "getValues"){
		values.msgType = "getValues";
		socket.send(JSON.stringify(values));
	  }
	  else if(msg == "Connected"){}
	  else if(msg == "turnOn"){
		  sockets.forEach(s => s.send("ON"));
	  }
	  else if(msg == "turnOff"){
		  sockets.forEach(s => s.send("OFF"));
	  }
	  else{
		  try{
			  console.log(`msg: ${msg}`);
		  const data = JSON.parse(msg);
		  if(data.msgType == "update"){
					values.temp = data.temp
					values.humi = data.humi
					values.heat = data.heat
					values.heating = data.heating
					values.time = data.time
				}
			values.msgType = "getValues";
			sockets.forEach(s => s.send(JSON.stringify(values)));
		  }
		  catch (error){
				console.error(error);
		  }
	  }
    //sockets.forEach(s => s.send(temp));
  });

  // When a socket closes, or disconnects, remove it from the array.
  socket.on('close', function() {
    sockets = sockets.filter(s => s !== socket);
  });
});
