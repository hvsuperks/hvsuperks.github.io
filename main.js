import { initializeApp } from "https://www.gstatic.com/firebasejs/10.11.0/firebase-app.js";
import { getDatabase, ref, onValue } from "https://www.gstatic.com/firebasejs/10.11.0/firebase-database.js";

// Đệ quy gán dữ liệu vào DOM
function updateDOMFromFirebase(data, prefix = '') {
  for (let key in data) {
    const value = data[key];
    const path = prefix ? `${prefix}-${key}` : key;

    if (typeof value === 'object' && value !== null) {
      updateDOMFromFirebase(value, path);
    } else {
      const el = document.getElementById(path);
      if (el) {
        el.textContent = value;
      }
    }
  }
}

// Load config và khởi tạo Firebase
async function start() {
  try {
    const res = await fetch('./firebaseConfig.json');
    const config = await res.json();

    const app = initializeApp(config);
    const db = getDatabase(app);

    const rootRef = ref(db, '/');
    onValue(rootRef, (snapshot) => {
      const data = snapshot.val();
      if (data) updateDOMFromFirebase(data);
    });

  } catch (err) {
    console.error("Lỗi tải config hoặc Firebase:", err);
  }
}

start();
