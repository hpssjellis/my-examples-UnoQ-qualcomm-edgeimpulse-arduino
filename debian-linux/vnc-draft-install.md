



## VNC install is not gooing well

```
sudo apt update
```

```
sudo apt install x11vnc
```

```
x11vnc -storepasswd
```


saves passwrd to ```~/.vnc/passwd```


test manually

```
x11vnc -display :0 -rfbauth ~/.vnc/passwd -forever -noxdamage
```

Option  Purpose  
-display :0Specifies the main X display to share (usually :0).  
-rfbauthPoints to the password file for authentication.    
-foreverKeeps the server running after a client disconnects.   
-noxdamageA speed/stability option for some setups, often helpful.   

:5900


## as a service for startup

### Set a new, VNC-specific password and save it to /etc/x11vnc.pass


```

sudo x11vnc -storepasswd /etc/x11vnc.pass
```

```
sudo nano /etc/systemd/system/x11vnc.service
```
```
[Unit]
Description=VNC Server for X11
After=multi-user.target

[Service]
Type=simple
ExecStart=/usr/bin/x11vnc -auth guess -forever -loop -noxdamage -repeat -rfbauth /etc/x11vnc.pass -rfbport 5900 -shared -o /var/log/x11vnc.log
Restart=on-failure
RestartSec=10
User=root

[Install]
WantedBy=multi-user.target
```

then load it

```
# Reload the systemd manager configuration
sudo systemctl daemon-reload

# Enable the service to start automatically on boot
sudo systemctl enable x11vnc.service

# Start the service immediately
sudo systemctl start x11vnc.service
```

## verify

```
sudo systemctl status x11vnc.service
```

You should see Active: active (running). You can now try connecting with your VNC client.

