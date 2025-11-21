const API_BASE = "http://192.168.1.46:8000";

const shelfSelect = document.getElementById("shelf-select");
const ctx = document.getElementById("soilChart").getContext("2d");

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
}

shelfSelect.addEventListener("change", updateChart);

// 주기적으로 새 데이터 반영 (30초마다)
setInterval(updateChart, 30000);

loadShelves();
