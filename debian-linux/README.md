A couple of useful Linux commands for when using the UnoQ


```
sudo apt update
```
```
ls
```

```
hostname
```
```
hostname -I
```
```
hostnamectl
```

Check all COM ports
```
ls /dev/tty*
```

```
sudo usermod -aG dialout arduino
```


```
minicom -b 9600 -D /dev/ttyS0

```



Check GUI

```
echo $XDG_CURRENT_DESKTOP
```

answer is XXFCE

So when searching for commands tell it you are using XFCE



## Power and sleep

Super irritating that it shuts down after 10 minutes and needs a login password.

Applications Menu > Settings > Power Manager.

or Application finder --> Power Manager

--> display eitherr turn it off or crank them to 60 minutes, both sleep and switch-off




## Installing software



Synaptic Package Manager is installed just find it, but int=stalling is a bit confusing, try this website for help

https://www.debian.org/distrib/packages



#W# obs-studio

randomly works. does not liek the graphics adaptor but reducing resolution etc helps

```
sudo apt update
sudo apt install obs-studio
env LIBGL_ALWAYS_SOFTWARE=1 obs
```

then reduce resolution and fps


The xfce4-terminal is horrible for copy paste lets try installing terminator

```
sudo apt install terminator

```

## To load x11vnc server

```
sudo apt update
sudo apt install x11vnc
```


```
x11vnc -storepasswd
```

Make a bash file to run it
```
nano run-x11vnc.sh
```
and paste this and save it cntl-x, y,
```
#1/bin/bash
x11vnc -display :0 -auth guess -loop -forever -multiptr -rfbauth ~/.vnc/passwd -noxdamage -xkb -shared
```
Then make that file runable by 
```
chmod +x run-x11vnc.sh
```


To find the vnc IP address is a bit tricky try
```
hostname -I
```

or

```
ip route get 1.1.1.1 | awk '{print $7; exit}'
```




I am trying a different editor
```
sudo apt update
```
```
sudo app install geany
```



# IPv4 errors

there may be a better way to do this.
```
docker network prune

```

