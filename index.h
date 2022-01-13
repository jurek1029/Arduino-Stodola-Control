const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
	<head>
		<style> 
			body {
			   background-image: url('https://static-ca-cdn.eporner.com/gallery/VX/lQ/4xOAsfDlQVX/253003-janice-griffith-nude.jpg'); font-size:3vh 
			}
			html { 
				font-family: Helvetica;
				display: inline-block; 
				margin: 0px auto;
				text-align: center;
				color:#ffffff;
			}
			.button {
				box-shadow:inset 0px 1px 0px 0px #29bbff;
				background:linear-gradient(to bottom, #2dabf9 5%, #0688fa 100%);
				background-color:#2dabf9;
				border-radius:6px;
				border:1px solid #0b0e07;
				display:inline-block;
				cursor:pointer;
				color:#ffffff;
				font-family:Arial;
				font-size:40px;
				padding:10px 20px;
				text-decoration:none;
				text-shadow:0px 1px 0px #263666;
			}
			.button:hover {
				background:linear-gradient(to bottom, #0688fa 5%, #2dabf9 100%);
				background-color:#0688fa;
			}
			.button:active {
				position:relative;
				top:1px;
			}	
			.box {
				background:linear-gradient(to bottom, RGBA(45, 171, 249, 0.3) 5%, RGBA(6, 136, 250, 0.3) 100%);
				border-radius:6px;
				display:inline-block;
				color:#ffffff;
				font-family:Arial;
				font-size:40px;
				padding:10px 20px;
				text-shadow:0px 1px 0px #263666;
			}
			.video-background {
				background: #000;
				position: fixed;
				top: 0; right: 0; bottom: 0; left: 0;
				z-index: -99;
			}
			.video-foreground,
			.video-background iframe {
				position: absolute;
				top: 0;
				left: 0;
				width: 100%;
				height: 100%;
				pointer-events: none;
			}
			@media (min-aspect-ratio: 16/9) {
				.video-foreground { height: 300%; top: -100%; }
			}
			@media (max-aspect-ratio: 16/9) {
				.video-foreground { width: 300%; left: -100%; }
			}
			ul{
				padding-left: 0px;
				margin-top: 10px;
				margin-bottom: 10px;
			}		
			li {
				display: inline-block;
				font-size: 20px;
				list-style-type: none;
				padding: 1em;
				text-transform: uppercase;
			}
			li span {
				display: block;
				font-size: 25px;
			}
		</style>
	</head>
	<body>
		<div class="video-background">
			<div class="video-foreground">
				<div id="player"></div>
			</div>
		</div>
	
		<h1> <b>Stodo&#322;a Control Center</b></h1>
		<br><br>
		<div class="box">
		Temperatura: <span id="temp">0</span>&deg;C <br>
		Wilgotno&#347;&#263;: <span id="humi">0</span>% <br>
		Heat Index: <span id="heatIndex">0</span>&deg;C 
		<ul>
			<li><span id="minutes"></span>Minutes</li>
			<li><span id="seconds"></span>Seconds</li>
		</ul>
		Heating is <span id="heating">OFF</span>			
		</div>
		<br>
		<br>
		<button class='button' onclick="sendData(1)"> Turn ON </button>
		<br><br>
		<button class='button' onclick="sendData(0)"> Turn OFF </button>
		
		<script>
			var tag = document.createElement('script');
			tag.src = "https://www.youtube.com/iframe_api";
			var firstScriptTag = document.getElementsByTagName('script')[0];
			firstScriptTag.parentNode.insertBefore(tag, firstScriptTag);

			var player;
			function onYouTubeIframeAPIReady() {
			player = new YT.Player('player', {
			  videoId: '6ksVdlXP4BI',
			  playerVars:{
				"controls" : 0,
				"showinfo":0,
				"rel":0,
				"autoplay":1,
				"loop":1,
				"mute":1,
				"playlist":"6ksVdlXP4BI"
			  },
			  events: {
				'onReady': onPlayerReady
			  }
			});
			}
			function onPlayerReady(event) {
				event.target.playVideo();
			}
			
			function sendData(led) {
				var xhttp = new XMLHttpRequest();
				xhttp.onreadystatechange = function() {
					if (this.readyState == 4 && this.status == 200) {
					  document.getElementById("heating").innerHTML = this.responseText;
					}
				};
				xhttp.open("GET", "setHeat?state="+led, true);
				xhttp.send();
			}

			setInterval(function() { getData(); }, 1500); //1500mSeconds update rate

			function updateUI(id){
				var xhttp = new XMLHttpRequest();
				xhttp.onreadystatechange = function() {
					if (this.readyState == 4 && this.status == 200) {
					  document.getElementById(id).innerHTML = this.responseText;
					}
				};
				xhttp.open("GET", "read"+id, true);
				xhttp.send();
			}
			
			countDown = new Date().getTime()+100;
			function getTimer(){
				var xhttp = new XMLHttpRequest();
				xhttp.onreadystatechange = function() {
					if (this.readyState == 4 && this.status == 200) {
					  countDown = new Date().getTime() + parseInt(this.responseText);
					}
				};
				xhttp.open("GET", "readTimer", true);
				xhttp.send();
			}
			
			function getData() {
				updateUI("temp");
				updateUI("humi");
				updateUI("heatIndex");
				updateUI("heating");
				getTimer();
			}
			
			(function () {
				const second = 1000,
				minute = second * 60,
				hour = minute * 60;
			 
				x = setInterval(function () {
					let now = new Date().getTime(),
					distance = countDown - now;
					if (distance >= 0) {
					(document.getElementById("minutes").innerText = Math.floor(
						(distance % hour) / minute)),
					(document.getElementById("seconds").innerText = Math.floor(
						(distance % minute) / second));
					}
				}, 0);
			})();
		</script>
	</body>
</html>
)=====";