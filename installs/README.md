

Some things that might make the UNOQ run a bit smoother or be a bit more powerful.


## Suggestions to make the UnoQ more fun

```
sudo apt update
```

Terminal program that does easier copy and paste
```
sudo install terminator
```

Editor that does easier copy and paste and just works like notepad++

```
sodo install geany

```






## Arduino IDE v2x  for ARM64 systems 
WARNING. ONLY HAS ENOUGH SPACE FOR ONE BOARD. 

Will crash if you try to load mulitple boards


https://github.com/matu6968/arduino-ide-arm64

download and 

```
sudo apt update
```

```
cd ~/Downloads

```

```
sudo dpkg -i arduino-ide_2.3.7_arm64.deb
```

possibly need
```
sudo apt install --fix-broken
```

then go to the programs and load it 


# for webcam acces

```
groups
```

```
sudo usermod -aG video $USER
```

Then reboot the system and run groups again to see
if video has been added.

