#include "web_server_manager.h"
#include "../storage/settings_manager.h"
#include "../network/wifi_manager.h"

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>Красноярскгортранс | Онлайн-табло</title>
    <!-- PWA & Mobile Web App settings -->
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
    <meta name="apple-mobile-web-app-title" content="Автобусы">
    <meta name="theme-color" content="#0f172a">
    <link rel="manifest" href="/manifest.json">
    <link rel="icon" type="image/svg+xml" href="/icon.svg">
    <link rel="apple-touch-icon" href="/icon.svg">
    <style>
        :root {
            --bg-color: #0f172a;
            --card-bg: #1e293b;
            --accent: #f59e0b;
            --accent-bus: #3b82f6;
            --accent-trol: #10b981;
            --accent-tram: #ef4444;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --border: #334155;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-main);
            min-height: 100vh;
            padding: 16px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        .container {
            width: 100%;
            max-width: 650px;
        }
        header {
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 14px;
            padding: 16px 20px;
            margin-bottom: 16px;
            box-shadow: 0 4px 6px -1px rgba(0,0,0,0.3);
        }
        .header-top {
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .title {
            font-size: 1.25rem;
            font-weight: 700;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        .live-dot {
            width: 10px;
            height: 10px;
            background-color: #22c55e;
            border-radius: 50%;
            box-shadow: 0 0 8px #22c55e;
            animation: pulse 2s infinite;
        }
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.4; }
            100% { opacity: 1; }
        }
        .subtitle {
            color: var(--text-muted);
            font-size: 0.9rem;
            margin-top: 6px;
        }
        .pwa-banner {
            background: rgba(59, 130, 246, 0.14);
            border: 1px solid rgba(59, 130, 246, 0.35);
            border-radius: 12px;
            padding: 10px 14px;
            margin-bottom: 14px;
            font-size: 0.85rem;
            color: #93c5fd;
            display: flex;
            align-items: center;
            justify-content: space-between;
            gap: 10px;
            line-height: 1.4;
        }
        @media (display-mode: standalone) {
            .pwa-banner { display: none !important; }
        }
        .board {
            display: flex;
            flex-direction: column;
            gap: 10px;
        }
        .item-card {
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 14px 16px;
            display: grid;
            grid-template-columns: auto 1fr auto;
            align-items: center;
            gap: 14px;
            transition: transform 0.15s ease;
        }
        .item-card:hover {
            border-color: #475569;
        }
        .route-badge {
            background-color: #f59e0b;
            color: #000;
            font-size: 1.3rem;
            font-weight: 800;
            padding: 6px 12px;
            border-radius: 8px;
            min-width: 52px;
            text-align: center;
        }
        .route-info {
            display: flex;
            flex-direction: column;
            gap: 4px;
            overflow: hidden;
        }
        .dest-title {
            font-size: 1.05rem;
            font-weight: 600;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
        }
        .type-tag {
            font-size: 0.8rem;
            color: var(--text-muted);
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        .arrival-col {
            text-align: right;
            display: flex;
            flex-direction: column;
            align-items: flex-end;
            gap: 2px;
        }
        .time-label {
            font-size: 1.2rem;
            font-weight: 700;
            color: var(--accent);
            font-variant-numeric: tabular-nums;
        }
        .wait-pill {
            font-size: 0.8rem;
            background: #334155;
            padding: 2px 8px;
            border-radius: 10px;
            color: #e2e8f0;
            font-weight: 600;
        }
        .wait-pill.soon {
            background: #15803d;
            color: #dcfce7;
        }
        .empty-state {
            background: var(--card-bg);
            border: 1px dashed var(--border);
            border-radius: 12px;
            padding: 40px 20px;
            text-align: center;
            color: var(--text-muted);
        }
        footer {
            margin-top: 24px;
            text-align: center;
            font-size: 0.8rem;
            color: #64748b;
        }
        .settings-btn {
            text-decoration: none;
            font-size: 1.25rem;
            padding: 4px;
            border-radius: 8px;
            transition: background 0.15s;
        }
        .settings-btn:hover { background: #334155; }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <div class="header-top">
                <div class="title">
                    <span class="live-dot"></span>
                    <span>Табло транспорта</span>
                </div>
                <div style="display:flex; align-items:center; gap:12px;">
                    <div id="clock" style="font-size:1.1rem; font-weight:700; color:var(--accent);">--:--:--</div>
                    <a href="/settings" class="settings-btn" title="Настройки Wi-Fi и остановки">⚙️</a>
                </div>
            </div>
            <div class="subtitle">
                Красноярск | Остановка ID: <strong id="stopId" style="color:#f8fafc;">...</strong> | <span id="status">Загрузка...</span>
            </div>
        </header>

        <div class="pwa-banner" id="pwaBanner">
            <span>📲 <strong>Совет:</strong> Нажмите «Поделиться» → «На экран Домой», чтобы табло открывалось как приложение.</span>
            <button onclick="document.getElementById('pwaBanner').style.display='none'" style="background:none;border:none;color:#93c5fd;font-size:1.2rem;cursor:pointer;padding:0 4px;">✕</button>
        </div>

        <div id="board" class="board">
            <div class="empty-state">Получение данных от Гортранса...</div>
        </div>

        <footer>
            ESP32 Bus Tracker • Доступно по адресу: <code>http://bus.local</code> • <a href="/settings" style="color:#94a3b8;">Настройки</a>
        </footer>
    </div>

    <script>
        async function fetchArrivals() {
            try {
                const res = await fetch('/api/arrivals');
                if (!res.ok) throw new Error('HTTP ' + res.status);
                const data = await res.json();

                document.getElementById('stopId').textContent = data.stopId || '-';
                if (data.serverTime) {
                    document.getElementById('clock').textContent = data.serverTime;
                }
                document.getElementById('status').textContent = 'Обновлено ' + (data.predictions ? data.predictions.length : 0) + ' рейсов';

                const board = document.getElementById('board');
                if (!data.predictions || data.predictions.length === 0) {
                    board.innerHTML = '<div class="empty-state">На ближайшее время рейсов нет или смена завершена</div>';
                    return;
                }

                let html = '';
                for (const item of data.predictions) {
                    const isSoon = item.waitMin >= 0 && item.waitMin <= 5;
                    const waitClass = isSoon ? 'wait-pill soon' : 'wait-pill';
                    const waitText = item.waitMin >= 0 ? `~${item.waitMin} мин` : 'скоро';
                    
                    html += `
                    <div class="item-card">
                        <div class="route-badge">${item.route}</div>
                        <div class="route-info">
                            <div class="dest-title">${item.lastStation || 'По маршруту'}</div>
                            <div class="type-tag">${item.type} • через ${item.firstStation ? item.firstStation : 'город'}</div>
                        </div>
                        <div class="arrival-col">
                            <div class="time-label">${item.arriveTime}</div>
                            <div class="${waitClass}">${waitText}</div>
                        </div>
                    </div>`;
                }
                board.innerHTML = html;
            } catch (err) {
                document.getElementById('status').textContent = 'Ожидание сети или обновления...';
            }
        }

        fetchArrivals();
        setInterval(fetchArrivals, 5000);
    </script>
</body>
</html>
)rawliteral";

static const char SETTINGS_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>Настройки | Табло Красноярск</title>
    <style>
        :root {
            --bg-color: #0f172a;
            --card-bg: #1e293b;
            --accent: #f59e0b;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --border: #334155;
            --primary: #3b82f6;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            background: var(--bg-color);
            color: var(--text-main);
            min-height: 100vh;
            padding: 16px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        .container { width: 100%; max-width: 480px; }
        .card {
            background: var(--card-bg);
            border: 1px solid var(--border);
            border-radius: 16px;
            padding: 24px;
            box-shadow: 0 4px 6px -1px rgba(0,0,0,0.3);
        }
        .header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 20px;
        }
        .header h1 { font-size: 1.3rem; display: flex; align-items: center; gap: 8px; }
        .back-link {
            color: var(--text-muted);
            text-decoration: none;
            font-size: 0.9rem;
            padding: 6px 12px;
            border-radius: 8px;
            border: 1px solid var(--border);
            transition: background 0.15s;
        }
        .back-link:hover { background: #334155; }
        .form-group {
            margin-bottom: 18px;
        }
        label {
            display: block;
            margin-bottom: 6px;
            font-size: 0.9rem;
            font-weight: 600;
            color: #cbd5e1;
        }
        select, input {
            width: 100%;
            padding: 12px 14px;
            background: #0f172a;
            border: 1px solid var(--border);
            border-radius: 10px;
            color: #f8fafc;
            font-size: 1rem;
            outline: none;
            transition: border-color 0.2s;
        }
        select:focus, input:focus {
            border-color: var(--primary);
        }
        .hint {
            font-size: 0.8rem;
            color: var(--text-muted);
            margin-top: 5px;
        }
        .btn {
            width: 100%;
            padding: 14px;
            background: var(--primary);
            color: #fff;
            border: none;
            border-radius: 10px;
            font-size: 1.05rem;
            font-weight: 700;
            cursor: pointer;
            transition: all 0.2s;
            margin-top: 10px;
        }
        .btn:hover { background: #2563eb; }
        .btn-danger {
            background: transparent;
            border: 1px solid #ef4444;
            color: #ef4444;
            font-size: 0.95rem;
            margin-top: 14px;
        }
        .btn-danger:hover {
            background: rgba(239, 68, 68, 0.15);
        }
        .status-msg {
            margin-top: 14px;
            padding: 12px;
            border-radius: 8px;
            font-size: 0.9rem;
            text-align: center;
            display: none;
        }
        .status-msg.success {
            background: rgba(34, 197, 94, 0.15);
            border: 1px solid #16a34a;
            color: #86efac;
            display: block;
        }
        .current-connection-card {
            background: #0f172a;
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 14px 16px;
            margin-bottom: 20px;
        }
        .conn-status-row {
            display: flex;
            align-items: center;
            gap: 10px;
            font-size: 0.95rem;
            font-weight: 600;
        }
        .conn-dot {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background: #94a3b8;
            flex-shrink: 0;
        }
        .conn-dot.online {
            background: #22c55e;
            box-shadow: 0 0 8px #22c55e;
        }
        .conn-dot.offline {
            background: #f59e0b;
            box-shadow: 0 0 8px #f59e0b;
        }
        .conn-details {
            margin-top: 6px;
            font-size: 0.85rem;
            color: var(--text-muted);
            line-height: 1.4;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="card">
            <div class="header">
                <h1>⚙️ Настройки табло</h1>
                <a href="/" class="back-link">← Табло</a>
            </div>

            <div class="current-connection-card" id="connectionInfo">
                <div class="conn-status-row">
                    <span class="conn-dot" id="connDot"></span>
                    <span id="connStatusText">Проверка подключения платы...</span>
                </div>
                <div class="conn-details" id="connDetails">Загрузка информации...</div>
            </div>

            <form id="settingsForm" onsubmit="saveSettings(event)">
                <div class="form-group">
                    <label for="ssid">Сеть Wi-Fi (2.4 ГГц):</label>
                    <select id="ssid" onchange="onSsidChange(this.value)">
                        <option value="">Поиск сетей...</option>
                    </select>
                    <input type="text" id="manualSsid" placeholder="Имя скрытой сети" style="display:none; margin-top:8px;">
                    <div class="hint">ESP32 подключается к Wi-Fi сетям 2.4 ГГц</div>
                </div>

                <div class="form-group">
                    <label for="password">Пароль от Wi-Fi:</label>
                    <input type="password" id="password" placeholder="Пароль сети">
                </div>

                <div class="form-group">
                    <label for="stopId">ID остановки Гортранса:</label>
                    <input type="text" id="stopId" placeholder="Например: 646">
                    <div class="hint">646 — Академгородок, 480 — Вокзал, 638 — Театр Оперы</div>
                </div>

                <button type="submit" id="saveBtn" class="btn">💾 Сохранить и подключиться</button>
                <button type="button" class="btn btn-danger" onclick="resetSettings()">🗑 Сбросить сохраненный Wi-Fi</button>
                <div id="statusMsg" class="status-msg"></div>
            </form>
        </div>
    </div>

    <script>
        async function loadData() {
            try {
                // Загружаем текущие сохраненные данные
                const currentRes = await fetch('/api/settings');
                let currentSsid = '';
                if (currentRes.ok) {
                    const cur = await currentRes.json();
                    currentSsid = cur.ssid || '';
                    if (cur.stopId) document.getElementById('stopId').value = cur.stopId;

                    const connDot = document.getElementById('connDot');
                    const connStatusText = document.getElementById('connStatusText');
                    const connDetails = document.getElementById('connDetails');

                    if (cur.connected && cur.currentSsid) {
                        connDot.className = 'conn-dot online';
                        connStatusText.innerHTML = `Подключено к: <strong style="color:#22c55e;">${cur.currentSsid}</strong>`;
                        connDetails.innerHTML = `IP в сети: <strong style="color:#f8fafc;">${cur.ip}</strong> • Сигнал: <strong style="color:#f8fafc;">${cur.rssi} dBm</strong>`;
                    } else {
                        connDot.className = 'conn-dot offline';
                        connStatusText.innerHTML = `<span style="color:#f59e0b;">Плата не подключена к Wi-Fi</span>`;
                        connDetails.innerHTML = `Точка доступа для настройки: <strong>KRSK-Bus-Setup</strong> (IP: ${cur.ip || '192.168.4.1'})`;
                    }
                }

                // Сканируем эфир
                const scanRes = await fetch('/api/scan');
                const select = document.getElementById('ssid');
                select.innerHTML = '<option value="">-- Выберите сеть --</option>';

                let foundCurrent = false;
                if (scanRes.ok) {
                    const networks = await scanRes.json();
                    for (const net of networks) {
                        const opt = document.createElement('option');
                        const ssidName = typeof net === 'string' ? net : net.ssid;
                        const rssi = (net && net.rssi !== undefined) ? ` (${net.rssi} dBm)` : '';
                        const lock = (net && net.secure) ? '🔒 ' : '🔓 ';
                        opt.value = ssidName;
                        opt.textContent = `${lock}${ssidName}${rssi}`;
                        if (ssidName === currentSsid) {
                            opt.selected = true;
                            foundCurrent = true;
                        }
                        select.appendChild(opt);
                    }
                }

                if (currentSsid && !foundCurrent) {
                    const curOpt = document.createElement('option');
                    curOpt.value = currentSsid;
                    curOpt.textContent = currentSsid + ' (сохраненная)';
                    curOpt.selected = true;
                    select.insertBefore(curOpt, select.firstChild.nextSibling);
                }

                const manualOpt = document.createElement('option');
                manualOpt.value = "__manual__";
                manualOpt.textContent = "+ Ввести имя сети вручную...";
                select.appendChild(manualOpt);
            } catch (err) {
                console.error(err);
            }
        }

        function onSsidChange(val) {
            const manual = document.getElementById('manualSsid');
            if (val === '__manual__') {
                manual.style.display = 'block';
                manual.focus();
            } else {
                manual.style.display = 'none';
            }
        }

        async function saveSettings(e) {
            e.preventDefault();
            const btn = document.getElementById('saveBtn');
            const statusMsg = document.getElementById('statusMsg');
            btn.disabled = true;
            btn.textContent = 'Сохранение...';

            let ssid = document.getElementById('ssid').value;
            if (ssid === '__manual__') {
                ssid = document.getElementById('manualSsid').value.trim();
            }
            const password = document.getElementById('password').value;
            const stopId = document.getElementById('stopId').value.trim();

            if (!ssid) {
                alert('Пожалуйста, выберите сеть Wi-Fi!');
                btn.disabled = false;
                btn.textContent = '💾 Сохранить и подключиться';
                return;
            }

            try {
                const res = await fetch('/api/settings', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ ssid, password, stopId })
                });

                if (res.ok) {
                    statusMsg.className = 'status-msg success';
                    statusMsg.innerHTML = '✅ Настройки сохранены!<br>Плата перезагружается...';
                    setTimeout(() => {
                        window.location.href = '/';
                    }, 4000);
                } else {
                    throw new Error('Ошибка при сохранении');
                }
            } catch (err) {
                alert('Ошибка: ' + err.message);
                btn.disabled = false;
                btn.textContent = '💾 Сохранить и подключиться';
            }
        }

        async function resetSettings() {
            if (!confirm('Вы уверены, что хотите сбросить настройки Wi-Fi? Плата перезагрузится в режим точки доступа KRSK-Bus-Setup.')) {
                return;
            }
            const statusMsg = document.getElementById('statusMsg');
            try {
                const res = await fetch('/api/reset', { method: 'POST' });
                if (res.ok) {
                    statusMsg.className = 'status-msg success';
                    statusMsg.innerHTML = '⚠️ Настройки сброшены!<br>Плата перезагружается...';
                }
            } catch (err) {
                alert('Ошибка: ' + err.message);
            }
        }

        loadData();
    </script>
</body>
</html>
)rawliteral";

WebServerManager::WebServerManager() 
    : _server(80), _settings(nullptr), _wifi(nullptr), _lastUpdateTimestamp(0) {}

bool WebServerManager::begin(SettingsManager* settings, WiFiManager* wifi, uint16_t port) {
    _settings = settings;
    _wifi = wifi;

    // Запуск DNS-сервера для перехвата запросов (Captive Portal)
    IPAddress apIP = WiFi.softAPIP();
    if (apIP[0] != 0) {
        _dnsServer.start(53, "*", apIP);
        Serial.printf("[CAPTIVE] DNS-перехватчик запущен на %s:53\n", apIP.toString().c_str());
    }

    if (MDNS.begin("bus")) {
        MDNS.addService("http", "tcp", port);
        Serial.println("[WEB] mDNS запущен: http://bus.local");
    } else {
        Serial.println("[WEB] Ошибка запуска mDNS");
    }

    // Маршруты страниц
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/settings", HTTP_GET, [this]() { handleSettings(); });

    // REST API
    _server.on("/api/arrivals", HTTP_GET, [this]() { handleApiArrivals(); });
    _server.on("/api/scan", HTTP_GET, [this]() { handleApiScan(); });
    _server.on("/api/settings", HTTP_GET, [this]() { handleApiSaveSettings(); });
    _server.on("/api/settings", HTTP_POST, [this]() { handleApiSaveSettings(); });
    _server.on("/api/reset", HTTP_POST, [this]() { handleApiReset(); });

    // PWA манифест и иконка
    _server.on("/manifest.json", HTTP_GET, [this]() {
        const char manifest[] PROGMEM = "{\"name\":\"Табло Автобусов Красноярск\",\"short_name\":\"Автобусы\",\"start_url\":\"/\",\"display\":\"standalone\",\"background_color\":\"#0f172a\",\"theme_color\":\"#0f172a\",\"icons\":[{\"src\":\"/icon.svg\",\"sizes\":\"any\",\"type\":\"image/svg+xml\",\"purpose\":\"any maskable\"}]}";
        _server.send_P(200, "application/manifest+json; charset=utf-8", manifest);
    });

    _server.on("/icon.svg", HTTP_GET, [this]() {
        const char icon[] PROGMEM = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\"><rect width=\"100\" height=\"100\" rx=\"22\" fill=\"#0f172a\"/><rect x=\"5\" y=\"5\" width=\"90\" height=\"90\" rx=\"18\" fill=\"#1e293b\" stroke=\"#f59e0b\" stroke-width=\"4\"/><text x=\"50\" y=\"66\" font-size=\"50\" text-anchor=\"middle\">🚎</text></svg>";
        _server.send_P(200, "image/svg+xml; charset=utf-8", icon);
    });

    // Эндпоинты проверки Captive Portal для мгновенного всплывания окна (iOS, Android, Windows)
    auto captiveHandler = [this]() {
        if (_wifi && !_wifi->isConnected()) {
            handleSettings();
        } else {
            handleRoot();
        }
    };

    _server.on("/hotspot-detect.html", HTTP_GET, captiveHandler);
    _server.on("/generate_204", HTTP_GET, captiveHandler);
    _server.on("/gen_204", HTTP_GET, captiveHandler);
    _server.on("/ncsi.txt", HTTP_GET, captiveHandler);
    _server.on("/connecttest.txt", HTTP_GET, captiveHandler);
    _server.on("/canonical.html", HTTP_GET, captiveHandler);
    _server.on("/success.txt", HTTP_GET, captiveHandler);

    _server.onNotFound([this]() { handleNotFound(); });

    _server.begin();
    Serial.printf("[WEB] Веб-сервер запущен на порту %d. Доступ: http://bus.local или через Wi-Fi 'KRSK-Bus-Setup'\n", port);
    return true;
}

void WebServerManager::update() {
    _dnsServer.processNextRequest();
    _server.handleClient();
}

void WebServerManager::setArrivals(const StopArrivalsResult& result) {
    _latestResult = result;
    _lastUpdateTimestamp = millis();
}

void WebServerManager::handleRoot() {
    // Если Wi-Fi еще не настроен / нет подключения, сразу открываем страницу настроек
    if (_wifi && !_wifi->isConnected()) {
        handleSettings();
        return;
    }
    _server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void WebServerManager::handleSettings() {
    _server.send_P(200, "text/html; charset=utf-8", SETTINGS_HTML);
}

void WebServerManager::handleApiArrivals() {
    JsonDocument doc;
    doc["stopId"] = _settings ? _settings->getStopId() : "646";
    doc["serverTime"] = _latestResult.systemTime;
    doc["success"] = _latestResult.success;

    JsonArray preds = doc["predictions"].to<JsonArray>();
    for (const auto& item : _latestResult.predictions) {
        JsonObject obj = preds.add<JsonObject>();
        obj["route"] = item.routeNumber;
        obj["type"] = getVehicleTypeBadge(item.vehicleType);
        obj["arriveTime"] = item.arrivalTime;
        obj["waitMin"] = item.minutesRemaining;
        obj["firstStation"] = item.firstStation;
        obj["lastStation"] = item.lastStation;
    }

    String response;
    serializeJson(doc, response);
    _server.send(200, "application/json; charset=utf-8", response);
}

void WebServerManager::handleApiScan() {
    auto networks = _wifi ? _wifi->scanNetworks() : std::vector<ScannedNetwork>();

    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (const auto& net : networks) {
        JsonObject obj = arr.add<JsonObject>();
        obj["ssid"] = net.ssid;
        obj["rssi"] = net.rssi;
        obj["secure"] = net.isSecure;
    }

    String response;
    serializeJson(doc, response);
    _server.send(200, "application/json; charset=utf-8", response);
}

void WebServerManager::handleApiSaveSettings() {
    if (_server.method() == HTTP_GET) {
        JsonDocument doc;
        doc["ssid"] = _settings ? _settings->getSSID() : "";
        doc["stopId"] = _settings ? _settings->getStopId() : "646";
        bool isConnected = _wifi ? _wifi->isConnected() : false;
        doc["connected"] = isConnected;
        doc["currentSsid"] = isConnected ? WiFi.SSID() : "";
        doc["ip"] = isConnected ? _wifi->getIP() : (_wifi ? _wifi->getApIP() : "");
        doc["rssi"] = isConnected ? _wifi->getRSSI() : 0;
        String response;
        serializeJson(doc, response);
        _server.send(200, "application/json; charset=utf-8", response);
        return;
    }

    // Обработка POST запроса
    String body = _server.arg("plain");
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);

    if (err) {
        _server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    String ssid = doc["ssid"] | "";
    String pass = doc["password"] | "";
    String stopId = doc["stopId"] | "";

    if (ssid.length() == 0) {
        _server.send(400, "application/json", "{\"error\":\"SSID cannot be empty\"}");
        return;
    }

    if (_settings) {
        _settings->save(ssid, pass, stopId);
    }

    _server.send(200, "application/json", "{\"status\":\"ok\"}");

    // Перезагрузка через 1 секунду для применения настроек
    delay(1000);
    ESP.restart();
}

void WebServerManager::handleApiReset() {
    if (_settings) {
        _settings->clear();
    }
    _server.send(200, "application/json; charset=utf-8", "{\"status\":\"ok\"}");
    delay(1000);
    ESP.restart();
}

void WebServerManager::handleNotFound() {
    String host = _server.hostHeader();
    // Если запрос пришел на внешний домен через DNS Captive Portal — перенаправляем на корень или настройки
    if (host != "bus.local" && host != WiFi.localIP().toString() && host != "192.168.4.1") {
        String target = (_wifi && !_wifi->isConnected()) ? "http://192.168.4.1/settings" : "http://192.168.4.1/";
        _server.sendHeader("Location", target, true);
        _server.send(302, "text/plain", "");
        return;
    }
    _server.send(404, "text/plain", "Not Found");
}
