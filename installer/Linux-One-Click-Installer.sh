#!/bin/bash

# Configuration
PY_VER="3.13"
PY_DIR="/usr/local/python313"
SCRIPT_URL="https://raw.githubusercontent.com/apple050620312/NCUT-Internet-Auto-Login/refs/heads/main/src/python/NCUT_Internet_Auto_Login.py"
STARTUP_DIR="/etc/profile.d/NCUT_Internet_Auto_Login.py"

# Check for root privileges
if [ "$(id -u)" -ne 0 ]; then
    echo "#######################################################"
    echo "# PLEASE RUN THIS SCRIPT AS ROOT!                      #"
    echo "#######################################################"
    sleep 5
    exit 1
fi

# Step 1: Install Python 3.13 if not present
echo "Checking Python $PY_VER installation..."
if command -v python3.13 &>/dev/null; then
    echo "Python $PY_VER is already installed."
else
    echo "Installing dependencies..."
    apt-get update && apt-get install -y build-essential zlib1g-dev libncurses5-dev libgdbm-dev libnss3-dev libssl-dev libreadline-dev libffi-dev libsqlite3-dev wget
    
    echo "Downloading Python $PY_VER..."
    wget https://www.python.org/ftp/python/3.13.0/Python-3.13.0.tgz
    
    echo "Extracting and installing Python..."
    tar -xzf Python-3.13.0.tgz
    cd Python-3.13.0
    ./configure --prefix=$PY_DIR --enable-optimizations
    make -j $(nproc)
    make install
    cd ..
    rm -rf Python-3.13.0.tgz Python-3.13.0
    
    # Add to PATH
    echo "export PATH=\"$PY_DIR/bin:\$PATH\"" >> /etc/profile
    source /etc/profile
fi

# Verify installation
if ! command -v python3.13 &>/dev/null; then
    echo "Failed to install Python $PY_VER"
    echo "Please try again until it works"
    sleep 5
    exit 1
fi

# Step 2: Install requests package
echo "Installing required packages..."
python3.13 -m pip install requests --quiet

# Step 3: Download auto-login script
echo "Downloading NCUT login script..."
wget -O $STARTUP_DIR $SCRIPT_URL || {
    echo "Failed to download login script"
    exit 1
}

# Make script executable
chmod +x $STARTUP_DIR

# Step 4: Run the script
echo "Starting login service..."
python3.13 $STARTUP_DIR &

# Create systemd service for reliable startup
SERVICE_FILE="/etc/systemd/system/ncut-autologin.service"
cat > $SERVICE_FILE <<EOL
[Unit]
Description=NCUT Internet Auto Login
After=network.target

[Service]
ExecStart=$PY_DIR/bin/python3.13 $STARTUP_DIR
Restart=always
User=root

[Install]
WantedBy=multi-user.target
EOL

systemctl daemon-reload
systemctl enable ncut-autologin.service
systemctl start ncut-autologin.service

echo "#######################################################"
echo "# INSTALLATION COMPLETED SUCCESSFULLY!                #"
echo "#                                                     #"
echo "# The login service will run:                         #"
echo "#   - Immediately now                                 #"
echo "#   - On every system startup                         #"
echo "#######################################################"

sleep 5
exit 0
