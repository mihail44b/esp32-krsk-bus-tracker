#!/usr/bin/env python3
import glob
import os
import subprocess
import time

PROJECT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
PATTERNS = [
    "/dev/cu.usbserial*",
    "/dev/cu.wchusbserial*",
    "/dev/cu.SLAB_USBtoUART*",
]

def find_serial_ports():
    ports = []
    for pat in PATTERNS:
        ports.extend(glob.glob(pat))
    return sorted(ports)

def is_monitor_running():
    try:
        res = subprocess.run(
            ["pgrep", "-f", "pio device monitor|platformio device monitor"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        return res.returncode == 0
    except Exception:
        return False

def open_browser():
    subprocess.run(["open", "http://bus.local"], check=False)

def main():
    print(f"[ESP32 Auto-Monitor] Запущен мониторинг USB портов для {PROJECT_DIR}...")
    was_connected = bool(find_serial_ports())
    if was_connected:
        print("[ESP32 Auto-Monitor] Плата уже подключена при старте демона.")

    while True:
        ports = find_serial_ports()
        is_connected = len(ports) > 0

        if is_connected and not was_connected:
            print(f"[ESP32 Auto-Monitor] Обнаружена плата на порту: {ports[0]}")
            time.sleep(2.0)  # Даем плате секунду для запуска Wi-Fi и mDNS
            print("[ESP32 Auto-Monitor] Автоматически открываем сайт в браузере...")
            open_browser()
            was_connected = True
        elif not is_connected and was_connected:
            print("[ESP32 Auto-Monitor] Плата отключена от USB.")
            was_connected = False

        time.sleep(1.5)

if __name__ == "__main__":
    main()

