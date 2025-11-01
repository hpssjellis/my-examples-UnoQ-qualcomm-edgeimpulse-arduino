

Try booting the board and having a hotspot. the applab version is 0.1.12

If you have an active hotspot then reboot several times until it loads applab 0.2.0

Chromium will not work, but the rest seems sort of OK.

To kill the error of the chromium process

rm -rf ~/.config/chromium/Singleton*

Then the webpages start to work again. EdgeImpulse vision models shoud run. may have to reload chromium with ```localhost:7000```
