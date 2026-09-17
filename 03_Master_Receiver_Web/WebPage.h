#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Arduino.h>

const char PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Energy Monitor Dashboard</title>
  <style>
    :root {
      --bg-primary: #0b0f19;
      --bg-card: rgba(23, 32, 54, 0.75);
      --border-color: rgba(255, 255, 255, 0.08);
      --text-main: #f8fafc;
      --text-muted: #94a3b8;
      --accent-cyan: #06b6d4;
      --accent-purple: #8b5cf6;
      --accent-emerald: #10b981;
      --accent-amber: #f59e0b;
      --accent-rose: #f43f5e;
      --glow-cyan: rgba(6, 182, 212, 0.25);
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
    }

    body {
      background: radial-gradient(circle at 10% 20%, #172554 0%, var(--bg-primary) 85%);
      color: var(--text-main);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 24px 16px;
    }

    .container {
      width: 100%;
      max-width: 1050px;
    }

    /* HEADER */
    header {
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
      margin-bottom: 28px;
      padding-bottom: 20px;
      border-bottom: 1px solid var(--border-color);
    }

    .brand h1 {
      font-size: 1.65rem;
      font-weight: 700;
      letter-spacing: -0.5px;
      background: linear-gradient(135deg, #38bdf8 0%, #c084fc 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }

    .brand p {
      font-size: 0.85rem;
      color: var(--text-muted);
      margin-top: 4px;
    }

    .header-badges {
      display: flex;
      align-items: center;
      gap: 12px;
    }

    .badge {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 6px 14px;
      border-radius: 9999px;
      font-size: 0.82rem;
      font-weight: 600;
      background: rgba(255, 255, 255, 0.05);
      border: 1px solid var(--border-color);
    }

    .status-dot {
      width: 9px;
      height: 9px;
      border-radius: 50%;
      background: #64748b;
    }

    .status-dot.online {
      background: var(--accent-emerald);
      box-shadow: 0 0 10px var(--accent-emerald);
      animation: pulse 1.8s infinite;
    }

    .status-dot.offline {
      background: var(--accent-rose);
      box-shadow: 0 0 10px var(--accent-rose);
    }

    @keyframes pulse {
      0%, 100% { transform: scale(1); opacity: 1; }
      50% { transform: scale(1.25); opacity: 0.7; }
    }

    /* GRID METRIK */
    .metrics-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 20px;
      margin-bottom: 28px;
    }

    .metric-card {
      background: var(--bg-card);
      backdrop-filter: blur(12px);
      -webkit-backdrop-filter: blur(12px);
      border: 1px solid var(--border-color);
      border-radius: 18px;
      padding: 22px;
      position: relative;
      overflow: hidden;
      transition: transform 0.2s ease, border-color 0.2s ease, box-shadow 0.2s ease;
    }

    .metric-card:hover {
      transform: translateY(-3px);
      border-color: rgba(255, 255, 255, 0.18);
      box-shadow: 0 12px 30px rgba(0, 0, 0, 0.35);
    }

    .card-top {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 14px;
    }

    .card-title {
      font-size: 0.85rem;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.5px;
      color: var(--text-muted);
    }

    .card-icon {
      width: 36px;
      height: 36px;
      border-radius: 10px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 1.1rem;
    }

    .metric-value {
      font-size: 2.3rem;
      font-weight: 700;
      letter-spacing: -1px;
      line-height: 1.1;
      display: flex;
      align-items: baseline;
      gap: 6px;
    }

    .metric-unit {
      font-size: 1.05rem;
      font-weight: 500;
      color: var(--text-muted);
    }

    .card-footer-info {
      margin-top: 10px;
      font-size: 0.78rem;
      color: var(--text-muted);
    }

    /* THEME COLORS FOR CARDS */
    .theme-voltage .card-icon { background: rgba(6, 182, 212, 0.15); color: var(--accent-cyan); }
    .theme-voltage .metric-value { color: #e0f2fe; }

    .theme-current .card-icon { background: rgba(245, 158, 11, 0.15); color: var(--accent-amber); }
    .theme-current .metric-value { color: #fef3c7; }

    .theme-power .card-icon { background: rgba(139, 92, 246, 0.15); color: var(--accent-purple); }
    .theme-power .metric-value { color: #ede9fe; }

    .theme-energy .card-icon { background: rgba(16, 185, 129, 0.15); color: var(--accent-emerald); }
    .theme-energy .metric-value { color: #d1fae5; }

    .theme-freq .card-icon { background: rgba(56, 189, 248, 0.15); color: #38bdf8; }
    .theme-freq .metric-value { color: #e0f2fe; }

    .theme-pf .card-icon { background: rgba(244, 63, 94, 0.15); color: var(--accent-rose); }
    .theme-pf .metric-value { color: #ffe4e6; }

    /* SYSTEM STATUS BAR */
    .system-bar {
      background: var(--bg-card);
      border: 1px solid var(--border-color);
      border-radius: 14px;
      padding: 14px 20px;
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;
      gap: 12px;
      font-size: 0.82rem;
      color: var(--text-muted);
    }

    .system-bar span b {
      color: var(--text-main);
    }

    footer {
      margin-top: 36px;
      text-align: center;
      font-size: 0.8rem;
      color: #64748b;
    }
  </style>
</head>
<body>

  <div class="container">
    <header>
      <div class="brand">
        <h1>ESP32 Energy Monitor</h1>
        <p>PZEM-004T v3.0 AC Power Meter &bull; Wireless ESP-NOW Protocol</p>
      </div>
      <div class="header-badges">
        <div class="badge">
          <span class="status-dot" id="statusDot"></span>
          <span id="statusText">Menghubungkan...</span>
        </div>
      </div>
    </header>

    <main class="metrics-grid">
      <!-- 1. VOLTAGE -->
      <div class="metric-card theme-voltage">
        <div class="card-top">
          <span class="card-title">Tegangan Listrik</span>
          <div class="card-icon">&#9889;</div>
        </div>
        <div class="metric-value">
          <span id="valVoltage">0.0</span>
          <span class="metric-unit">V</span>
        </div>
        <div class="card-footer-info">Standar PLN: ~220 Volt AC</div>
      </div>

      <!-- 2. CURRENT -->
      <div class="metric-card theme-current">
        <div class="card-top">
          <span class="card-title">Arus Listrik</span>
          <div class="card-icon">&#8776;</div>
        </div>
        <div class="metric-value">
          <span id="valCurrent">0.000</span>
          <span class="metric-unit">A</span>
        </div>
        <div class="card-footer-info">Beban arus yang sedang aktif</div>
      </div>

      <!-- 3. ACTIVE POWER -->
      <div class="metric-card theme-power">
        <div class="card-top">
          <span class="card-title">Daya Aktif</span>
          <div class="card-icon">&#9733;</div>
        </div>
        <div class="metric-value">
          <span id="valPower">0.0</span>
          <span class="metric-unit">W</span>
        </div>
        <div class="card-footer-info">Konsumsi daya real-time (Watt)</div>
      </div>

      <!-- 4. TOTAL ENERGY -->
      <div class="metric-card theme-energy">
        <div class="card-top">
          <span class="card-title">Total Energi</span>
          <div class="card-icon">&#9670;</div>
        </div>
        <div class="metric-value">
          <span id="valEnergy">0.000</span>
          <span class="metric-unit">kWh</span>
        </div>
        <div class="card-footer-info">Akumulasi pemakaian listrik</div>
      </div>

      <!-- 5. FREQUENCY -->
      <div class="metric-card theme-freq">
        <div class="card-top">
          <span class="card-title">Frekuensi Jaringan</span>
          <div class="card-icon">&#12316;</div>
        </div>
        <div class="metric-value">
          <span id="valFrequency">0.0</span>
          <span class="metric-unit">Hz</span>
        </div>
        <div class="card-footer-info">Frekuensi AC PLN (50 Hz)</div>
      </div>

      <!-- 6. POWER FACTOR -->
      <div class="metric-card theme-pf">
        <div class="card-top">
          <span class="card-title">Faktor Daya (Cos &phi;)</span>
          <div class="card-icon">&#9680;</div>
        </div>
        <div class="metric-value">
          <span id="valPf">0.00</span>
          <span class="metric-unit">PF</span>
        </div>
        <div class="card-footer-info">Efisiensi daya beban (0.00 - 1.00)</div>
      </div>
    </main>

    <div class="system-bar">
      <div>Total Paket Masuk: <b id="valPackets">0</b></div>
      <div>Terakhir Diterima: <b id="valLastSeen">-</b></div>
      <div>Protokol: <b>ESP-NOW (Channel 1)</b></div>
    </div>

    <footer>
      ESP32 Dual-Core Architecture &bull; ESP-NOW + PZEM-004T Monitoring System
    </footer>
  </div>

  <script>
    let packetCount = 0;
    let lastReceivedTimestamp = 0;

    async function fetchData() {
      try {
        const response = await fetch('/api/data', { cache: "no-store" });
        if (!response.ok) throw new Error('HTTP error ' + response.status);
        const data = await response.json();

        const statusDot = document.getElementById('statusDot');
        const statusText = document.getElementById('statusText');

        if (data.valid) {
          statusDot.className = 'status-dot online';
          statusText.textContent = 'TERHUBUNG (ONLINE)';
          
          document.getElementById('valVoltage').textContent = data.voltage.toFixed(1);
          document.getElementById('valCurrent').textContent = data.current.toFixed(3);
          document.getElementById('valPower').textContent = data.power.toFixed(1);
          document.getElementById('valEnergy').textContent = data.energy.toFixed(3);
          document.getElementById('valFrequency').textContent = data.frequency.toFixed(1);
          document.getElementById('valPf').textContent = data.pf.toFixed(2);

          packetCount = data.packets;
          document.getElementById('valPackets').textContent = packetCount;
          document.getElementById('valLastSeen').textContent = new Date().toLocaleTimeString();
        } else {
          statusDot.className = 'status-dot offline';
          statusText.textContent = 'SENSOR TIDAK AKTIF (CEK AC 220V)';
        }
      } catch (err) {
        console.warn('Koneksi WebServer terputus:', err);
        document.getElementById('statusDot').className = 'status-dot offline';
        document.getElementById('statusText').textContent = 'SERVER DISCONNECTED';
      }
    }

    // Ambil data pertama kali dan jalankan polling berkala setiap 1 detik
    fetchData();
    setInterval(fetchData, 1000);
  </script>
</body>
</html>
)rawliteral";

#endif // WEBPAGE_H
