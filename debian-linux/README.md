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
BGL_ALWAYS_SOFTWARE=1 obs
```

then reduce resolution and fps
