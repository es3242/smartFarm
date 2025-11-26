const API_BASE = "http://192.168.1.46:8000";

const shelfSelect = document.getElementById("shelf-select");
const ctx = document.getElementById("soilChart").getContext("2d");
const fanManualBtn = document.getElementById("fan-manual-btn");
const fanAutoBtn   = document.getElementById("fan-auto-btn");
const fanStatusEl  = document.getElementById("fan-status");

let soilChart = null;

async function loadShelves() {
  const res = await fetch(`${API_BASE}/api/shelves`);
  const json = await res.json();
  json.shelves.forEach((s) => {
    const opt = document.createElement("option");
    opt.value = opt.textContent = s;
    shelfSelect.appendChild(opt);
  });

  if (json.shelves.length > 0) {
    shelfSelect.value = json.shelves[0];
    await updateChart();
  }
}

async function updateChart() {
  const shelf = shelfSelect.value;
  const res = await fetch(`${API_BASE}/api/samples/${shelf}?limit=100`);
  const json = await res.json();
  const data = json.data;

  const labels = data.map((d) => d.ts);
  const values = data.map((d) => d.soil_pct);

  if (soilChart) soilChart.destroy();

  soilChart = new Chart(ctx, {
    type: "line",
    data: {
      labels,
      datasets: [
        {
          label: `Soil moisture (%) - ${shelf}`,
          data: values,
          fill: false,
        },
      ],
    },
    options: {
      scales: {
        x: { display: true },
        y: { beginAtZero: true, max: 100 },
      },
    },
  });
    if (data.length > 0) {
    const latest = data[data.length - 1];
    const on = latest.fan_state === 1; // DB에 INTEGER로 저장됨 
    fanStatusEl.textContent = on ? "Fan: ON" : "Fan: OFF (or AUTO OFF)";
  } else {
    fanStatusEl.textContent = "(no data yet)";
  }
}
async function sendFanMode(mode) {
  const shelf = shelfSelect.value;
  await fetch(`${API_BASE}/api/fan/${shelf}/mode`, {
    method: "POST",
    headers: {"Content-Type": "application/json"},
    body: JSON.stringify({ mode }),
  });
  // 명령 보낸 뒤에 최신 상태 확인
  setTimeout(updateChart, 500);
}

fanManualBtn.addEventListener("click", () => {
  sendFanMode("manual_on");
});

fanAutoBtn.addEventListener("click", () => {
  sendFanMode("auto");
});

shelfSelect.addEventListener("change", updateChart);

// 주기적으로 새 데이터 반영 (30초마다)
setInterval(updateChart, 30000);

loadShelves();
