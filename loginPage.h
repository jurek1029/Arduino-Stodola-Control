const char LOGIN_page[] PROGMEM = R"=====(
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
			}
			label{
				font-size:3vh; 
			}
			.bigT{
				text-align: center;
				font-size:min(5vw,5vh); 	
			}
			.box{
				background:linear-gradient(to bottom, RGBA(45, 171, 249, 0.3) 5%, RGBA(6, 136, 250, 0.3) 100%);
				border-radius:6px;
				display:inline-block;
				color:#ffffff;
				font-family:Arial;
				font-size:40px;
				padding:10px 20px;
				text-shadow:0px 1px 0px #263666;				
				min-width: 800px;
				position: absolute; 	
				left: 50%;
				top:50%;
				transform: translate(-50%, -50%);
				}
			
			input[type=text], input[type=password] {
				width: 100%;
				padding: 12px 20px;
				margin: 8px 0;
				display: inline-block;
				border: 1px solid #ccc;
				box-sizing: border-box;
				min-height:6vh;
				font-size: 1.5vh;
			}

			button {
				background-color: #04AA6D;
				color: white;
				padding: 14px 20px;
				margin: 1vh 0;
				border: none;
				cursor: pointer;
				width: 100%;
				min-height:6vh;
				font-size: 2vh;
			}

			/* Add a hover effect for buttons */
			button:hover {
			  opacity: 0.8;
			}
		</style>
	</head>
<body>
		<div class="box">
			<form id="LoginForm" onsubmit="return false">
			
				<div class="bigT"><b>Login in to Stodo&#322;a Center</b><br> </div>

				<label for="user"><b>User</b></label>
				<input type="text" placeholder="Enter User" name="user" required>

				<label for="psw"><b>Password</b></label>
				<input type="password" placeholder="Enter Password" name="psw" required>

				<button type="submit" class="button">Login</button>
				<div id="msg" style="text-align: center;color:red;display:none;">Wrong password or login</div><br>
			</form>
		</div>

<script>
	window.addEventListener("load", function() {
		var loginForm = document.getElementById("LoginForm");
		loginForm.addEventListener("submit", function() {
			 login(loginForm);
		 });
	 });
	 
	 function login(form) {
		var xhr = new XMLHttpRequest();
		xhr.open("POST", "loginValidate", true);
		xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
		xhr.onreadystatechange = function () {
			if (xhr.readyState == 4 && xhr.status == 200) {
				var msg = document.getElementById("msg");
				if (xhr.responseText.indexOf("failed") != -1) {
					msg.style.display = "block";
					form.user.select();
					form.user.className = "Highlighted";
					setTimeout(function() {
						msg.style.display = 'none';
					}, 3000);
				}
				else{
					window.location.replace("/");
				}
			}
		}
		xhr.send("user="+form.user.value+"&psw="+form.psw.value);
	}
</script>

</body>
</html>
)=====";