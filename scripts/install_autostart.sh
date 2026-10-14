#!/bin/bash
set -e

PLIST_DIR="$HOME/Library/LaunchAgents"
PLIST_FILE="$PLIST_DIR/com.valera.esp32tracker.monitor.plist"
SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd)/auto_monitor.py"
DESKTOP_SCRIPT="$HOME/Desktop/Табло Автобусов.command"

mkdir -p "$PLIST_DIR"

cat <<EOF > "$PLIST_FILE"
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.valera.esp32tracker.monitor</string>
    <key>ProgramArguments</key>
    <array>
        <string>/usr/bin/python3</string>
        <string>$SCRIPT_PATH</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
    <key>StandardOutPath</key>
    <string>$HOME/Library/Logs/esp32_tracker_monitor.log</string>
    <key>StandardErrorPath</key>
    <string>$HOME/Library/Logs/esp32_tracker_monitor.log</string>
</dict>
</plist>
EOF

launchctl unload "$PLIST_FILE" 2>/dev/null || true
launchctl load "$PLIST_FILE"

cat <<EOF > "$DESKTOP_SCRIPT"
#!/bin/bash
export PATH="\$HOME/Library/Python/3.9/bin:\$PATH"
cd "$(cd "$(dirname "$0")/.." && pwd)" 2>/dev/null || cd "/Users/valera/study/semestr7/MPS/esp32-krsk-bus-tracker"
echo "=================================================="
echo "    Подключение к табло автобусов ESP32..."
echo "=================================================="
pio device monitor
EOF
chmod +x "$DESKTOP_SCRIPT"

echo "✅ Фоновый сервис авто-открытия монитора успешно установлен и запущен!"
echo "✅ На Рабочем столе также создан ярлык: 'Табло Автобусов.command'"

