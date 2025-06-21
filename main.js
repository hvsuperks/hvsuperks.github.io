
   const firebaseConfig = {
    apiKey: "AIzaSyAOOtP5Va7zCa9lH62H20piGYCvdACt9DE",
    authDomain: "be-ca-ad623.firebaseapp.com",
    databaseURL: "https://be-ca-ad623-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "be-ca-ad623",
    storageBucket: "be-ca-ad623.firebasestorage.app",
    messagingSenderId: "648232770855",
    appId: "1:648232770855:web:8457f90c7a5bc0f5a3dd02"
  };
import { initializeApp } from 'https://www.gstatic.com/firebasejs/10.11.0/firebase-app.js';
  import { getDatabase, ref, get, child, set } from 'https://www.gstatic.com/firebasejs/10.11.0/firebase-database.js';
  import { getAuth, signInWithEmailAndPassword } from "https://www.gstatic.com/firebasejs/10.11.0/firebase-auth.js";
document.addEventListener("DOMContentLoaded", () => {
  const app = initializeApp(firebaseConfig);
  const db = getDatabase(app);
  const auth = getAuth(app);
  const status = document.getElementById("status");

  const settingKeys = [
    "co2_off_1", "co2_off_2", "co2_on_1", "co2_on_2", "temp_set"
    // 👉 thêm các key khác trong Setting nếu muốn
  ];

  const relayKeys = ["fillter", "co2", "chiller"];

  // Đăng nhập
  signInWithEmailAndPassword(auth, "hvsuperks@gmail.com", "SAObang!((#")
    .then(() => {
      status.innerText = "✅ Đăng nhập Firebase thành công";
      loadSettingsFromFirebase();
      startRelaySync();
      setupSettingAutoSave();
      setupRelayButtons();
    })
    .catch((err) => {
      status.innerText = "❌ Lỗi đăng nhập: " + err.message;
    });

  function loadSettingsFromFirebase() {
    get(child(ref(db), "Setting")).then(snapshot => {
      if (snapshot.exists()) {
        const data = snapshot.val();
        settingKeys.forEach(key => {
          const el = document.getElementById(key);
          if (el && data[key] !== undefined) {
            el.value = data[key];
          }
        });
        status.innerText = "✅ Đã tải Setting";
      } else {
        status.innerText = "⚠️ Không có dữ liệu Setting";
      }
    }).catch(err => {
      status.innerText = "❌ Lỗi đọc Setting: " + err.message;
    });
  }

  function setupSettingAutoSave() {
    settingKeys.forEach(key => {
      const el = document.getElementById(key);
      if (el) {
        let timer;
        el.addEventListener("input", () => {
          clearTimeout(timer);
          timer = setTimeout(() => {
            const val = parseFloat(el.value);
            if (!isNaN(val)) {
              set(ref(db, `Setting/${key}`), val)
                .then(() => status.innerText = `💾 Cập nhật ${key}: ${val}`)
                .catch(err => status.innerText = "❌ Lỗi ghi: " + err.message);
            }
          }, 5000);
        });
      }
    });
  }

  function updateRelayButton(key, value) {
    const btn = document.getElementById(key);
    if (btn) {
      btn.innerText = value ? "ON" : "OFF";
      btn.style.backgroundColor = value ? "lightgreen" : "lightcoral";
    }
  }

  function setupRelayButtons() {
    relayKeys.forEach(key => {
      const btn = document.getElementById(key);
      if (btn) {
        btn.addEventListener("click", () => {
          const newValue = btn.innerText === "ON" ? 0 : 1;
          set(ref(db, `Relay/${key}`), newValue)
            .then(() => {
              status.innerText = `🔁 Đã bật/tắt relay ${key}`;
              updateRelayButton(key, newValue);
            })
            .catch(err => status.innerText = "❌ Lỗi ghi relay: " + err.message);
        });
      }
    });
  }

  function startRelaySync() {
    setInterval(() => {
      get(child(ref(db), "Relay")).then(snapshot => {
        if (snapshot.exists()) {
          const relays = snapshot.val();
          relayKeys.forEach(key => {
            updateRelayButton(key, relays[key]);
          });
          status.innerText = "🔄 Đã đồng bộ trạng thái relay";
        }
      }).catch(err => {
        status.innerText = "❌ Lỗi đọc Relay: " + err.message;
      });
    }, 5000);
  }
});
