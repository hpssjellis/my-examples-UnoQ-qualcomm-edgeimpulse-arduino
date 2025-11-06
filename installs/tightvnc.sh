#!/bin/bash
################################################################################
# TIGHTVNC AUTO-START INSTALLER FOR DEBIAN (UNO Q)
#
# USAGE EXAMPLE:
#
#   1. Make the script executable:
#      chmod +x install_vnc.sh
#
#   2. Run the script with sudo, passing required variables:
#      sudo ./install_vnc.sh <username> <display_num> <vnc_password>
#
#   Example:
#      sudo ./install_vnc.sh myuser 1 MySecureVNCpass123
#
# Note: The VNC password is passed directly and will be visible in history.
################################################################################

# --- VARIABLES ---
VNC_USER="$1"
VNC_DISPLAY_NUM="$2"
VNC_PASSWORD="$3"
GEOMETRY="1280x800"
DEPTH="24"
SERVICE_FILE="/etc/systemd/system/vncserver@.service"

# --- CHECK FOR REQUIRED ARGUMENTS ---
if [ -z "$VNC_USER" ] || [ -z "$VNC_DISPLAY_NUM" ] || [ -z "$VNC_PASSWORD" ]; then
    echo "Usage: sudo ./install_vnc.sh <username> <display_num> <vnc_password>"
    echo "Example: sudo ./install_vnc.sh debianuser 1 MySecurePass123"
    exit 1
fi

# --- 1. INSTALL TIGHTVNC AND UPDATE SYSTEM ---
echo "--- Step 1: Updating system and installing TightVNC ---"
sudo apt update -y
sudo apt install tightvncserver xfce4 xfce4-goodies -y

# --- 2. SETUP USER-SPECIFIC CONFIGURATION ---
USER_HOME=$(eval echo "~$VNC_USER")
VNC_DIR="$USER_HOME/.vnc"
XSTARTUP_FILE="$VNC_DIR/xstartup"
VNC_PASSWD_FILE="$VNC_DIR/passwd"

echo "--- Step 2: Configuring VNC for user $VNC_USER in $USER_HOME ---"

# Create .vnc directory and set permissions
sudo mkdir -p "$VNC_DIR"
sudo chown "$VNC_USER":"$VNC_USER" "$VNC_DIR"
sudo chmod 0700 "$VNC_DIR"

# Write VNC password hash non-interactively
# tightvncserver's 'vncpasswd -f' is used to write a one-way password file
# 'su - $VNC_USER -c' runs this command as the target user, which handles permissions
echo -n "$VNC_PASSWORD" | sudo su - "$VNC_USER" -c "vncpasswd -f > $VNC_PASSWD_FILE"
sudo chown "$VNC_USER":"$VNC_USER" "$VNC_PASSWD_FILE"
sudo chmod 0600 "$VNC_PASSWD_FILE"

# Configure xstartup file for XFCE
echo "--- Creating $XSTARTUP_FILE ---"
# Note: Using cat with EOT to write multi-line content
cat > /tmp/my_xstartup << EOT
#!/bin/bash
unset SESSION_MANAGER
unset DBUS_SESSION_BUS_ADDRESS
startxfce4 &
EOT

sudo mv /tmp/my_xstartup "$XSTARTUP_FILE"
sudo chown "$VNC_USER":"$VNC_USER" "$XSTARTUP_FILE"
sudo chmod +x "$XSTARTUP_FILE"

# Kill any running VNC server for this display to ensure a clean start
sudo /usr/bin/vncserver -kill :"$VNC_DISPLAY_NUM" > /dev/null 2>&1

# --- 3. CREATE SYSTEMD SERVICE FILE ---
echo "--- Step 3: Creating Systemd Service File $SERVICE_FILE ---"
# Note: Using cat with EOT for the service template, written to a temporary file, then moved with sudo
cat > /tmp/my_vncservice << EOT
[Unit]
Description=TightVNC Server for %i
After=syslog.target network.target

[Service]
Type=forking
User=$VNC_USER
PAMName=login
PIDFile=$USER_HOME/.vnc/%H:%i.pid
ExecStartPre=-/usr/bin/vncserver -kill :%i > /dev/null 2>&1
ExecStart=/usr/bin/vncserver :%i -geometry $GEOMETRY -depth $DEPTH
ExecStop=/usr/bin/vncserver -kill :%i

[Install]
WantedBy=multi-user.target
EOT

sudo mv /tmp/my_vncservice "$SERVICE_FILE"

# --- 4. ENABLE AND START SERVICE ---
echo "--- Step 4: Enabling and Starting VNC Service (Display :$VNC_DISPLAY_NUM) ---"
sudo systemctl daemon-reload
sudo systemctl enable vncserver@"$VNC_DISPLAY_NUM".service
sudo systemctl start vncserver@"$VNC_DISPLAY_NUM".service

# --- 5. VERIFICATION ---
echo "--- Step 5: Verification ---"
sudo systemctl status vncserver@"$VNC_DISPLAY_NUM".service --no-pager
echo ""
echo "VNC Setup Complete for user: $VNC_USER on display :$VNC_DISPLAY_NUM (Port 590$VNC_DISPLAY_NUM)"


# --- 6. manually delete password in history file ---
sudo nano ~/.bash_history





