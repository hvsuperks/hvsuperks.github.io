import { initializeApp } from "https://www.gstatic.com/firebasejs/10.11.0/firebase-app.js";
import { getDatabase, ref, onValue } from "https://www.gstatic.com/firebasejs/10.11.0/firebase-database.js";


  const keys = [
    "co2_off_1", "co2_off_2", "co2_on_1", "co2_on_2",
    "delay_fillter", "fillter_pause", "set_upload",
    "temp_diff", "temp_offset", "temp_set", "test",
    "water_day_1", "water_day_2", "water_speed", "water_volume",
    "x_offset", "y_offset"
  ];

  // Gán giá trị từ Firebase vào input
  db.ref("Setting").once("value").then(snapshot => {
    const data = snapshot.val();
    if (data) {
      keys.forEach(key => {
        const input = document.getElementById(key);
        if (input && data[key] !== undefined) {
          input.value = data[key];
        }
      });
      status.innerText = "Đã tải dữ liệu từ Firebase";
    }
  });

  // Ghi lại khi dừng gõ 5s
  const debounceTimers = {};
  keys.forEach(key => {
    const input = document.getElementById(key);
    if (input) {
      input.addEventListener("input", () => {
        clearTimeout(debounceTimers[key]);
        debounceTimers[key] = setTimeout(() => {
          let val = input.value;
          if (val === "") return;
          if (!isNaN(val)) {
            val = parseFloat(val); // tự động hiểu cả int và float
          }
          db.ref("Setting/" + key).set(val)
            .then(() => status.innerText = `Đã cập nhật ${key}: ${val}`)
            .catch(err => status.innerText = `Lỗi cập nhật ${key}: ${err.message}`);
        }, 5000);
      });
    }
  });
