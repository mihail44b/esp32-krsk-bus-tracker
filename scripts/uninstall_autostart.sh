#!/bin/bash

PLIST_FILE="$HOME/Library/LaunchAgents/com.valera.esp32tracker.monitor.plist"
DESKTOP_SCRIPT="$HOME/Desktop/Табло Автобусов.command"

launchctl unload "$PLIST_FILE" 2>/dev/null || true
rm -f "$PLIST_FILE"
rm -f "$DESKTOP_SCRIPT"

echo "✅ Фоновый сервис авто-открытия монитора отключен и удален."

