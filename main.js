<script type="module">
  import { firebaseConfig } from './firebaseconfig.js';
  import { initializeApp } from 'https://www.gstatic.com/firebasejs/10.11.0/firebase-app.js';
  import { getDatabase, ref, get, child, set } from 'https://www.gstatic.com/firebasejs/10.11.0/firebase-database.js';

  const app = initializeApp(firebaseConfig);
  const db = getDatabase(app);

  const status = document.getElementById("status");

  const keys = [
    "co2_off_1", "co2_off_2", "co2_on_1", "co2_on_2",
    "delay_fillter", "fillter_pause", "set_upload",
    "temp_diff", "temp_offset", "temp_set", "test",
    "water_day_1", "water_day_2", "water_speed", "water_volume",
    "x_offset", "y_offset"
  ];

  // Load dữ liệu từ Firebase
  get(child(ref(db), "Setting")).then(snapshot => {
    if (snapshot.exists()) {
      const data = snapshot.val();
      keys.forEach(key => {
        const el = document.getElementById(key);
        if (el && data[key] !== undefined) {
          el.value = data[key];
        }
      });
      status.innerText = "Đã tải dữ liệu từ Firebase";
    }
  });

  // Ghi lại sau 5s không gõ
  const debounceTimers = {};
  keys.forEach(key => {
    const input = document.getElementById(key);
    if (input) {
      input.addEventListener("input", () => {
        clearTimeout(debounceTimers[key]);
        debounceTimers[key] = setTimeout(() => {
          let val = input.value;
          if (!isNaN(val)) val = parseFloat(val);
          set(ref(db, "Setting/" + key), val)
            .then(() => status.innerText = `Đã cập nhật ${key}: ${val}`)
            .catch(err => status.innerText = `Lỗi cập nhật ${key}: ${err.message}`);
        }, 5000);
      });
    }
  });
</script>
