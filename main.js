import { initializeApp } from "https://www.gstatic.com/firebasejs/10.11.0/firebase-app.js";
import { getDatabase, ref, onValue } from "https://www.gstatic.com/firebasejs/10.11.0/firebase-database.js";

async function start() {
  const res = await fetch('./firebaseConfig.json');
  const config = await res.json();
  const app = initializeApp(config);
  const db = getDatabase(app);

  // Sensor temps
  onValue(ref(db, "sensor/temp_tank"), snap => {
    document.getElementById("temp_tank").innerText = snap.val() + " °C";
  });

  onValue(ref(db, "sensor/temp_room"), snap => {
    document.getElementById("temp_room").innerText = snap.val() + " °C";
  });

  // Temp settings
  onValue(ref(db, "setting/temp_set"), snap => {
    document.getElementById("temp_set").innerText = snap.val() + " °C";
  });

  onValue(ref(db, "setting/temp_diff"), snap => {
    document.getElementById("temp_diff").innerText = snap.val() + " °C";
  });

  onValue(ref(db, "setting/temp_offset"), snap => {
    document.getElementById("temp_offset").innerText = snap.val() + " °C";
  });

  // Relay states
  onValue(ref(db, "relay/chiller"), snap => {
    const on = snap.val();
    const el = document.getElementById("chiller_btn");
    el.innerText = on ? "ON" : "OFF";
    el.className = on ? "onoff-btn on" : "onoff-btn off";
  });

  onValue(ref(db, "relay/filter"), snap => {
    const on = snap.val();
    const el = document.getElementById("filter_btn");
    el.innerText = on ? "ON" : "OFF";
    el.className = on ? "onoff-btn on" : "onoff-btn off";
  });

  onValue(ref(db, "timer/filter_pause"), snap => {
    document.getElementById("filter_pause").innerText = snap.val();
  });

  onValue(ref(db, "relay/co2"), snap => {
    const on = snap.val();
    const el = document.getElementById("co2_btn");
    el.innerText = on ? "ON" : "OFF";
    el.className = on ? "onoff-btn on" : "onoff-btn off";
  });

  onValue(ref(db, "timer/co2_on"), snap => {
    document.getElementById("co2_on").innerText = snap.val();
  });

  onValue(ref(db, "timer/co2_off"), snap => {
    document.getElementById("co2_off").innerText = snap.val();
  });

  onValue(ref(db, "relay/valve"), snap => {
    const on = snap.val();
    const el = document.getElementById("valve_btn");
    el.innerText = on ? "ON" : "OFF";
    el.className = on ? "onoff-btn on" : "onoff-btn off";
  });

  onValue(ref(db, "timer/water_day"), snap => {
    document.getElementById("water_day").innerText = snap.val();
  });

  onValue(ref(db, "timer/water_volume"), snap => {
    document.getElementById("water_volume").innerText = snap.val() + "L";
  });

  onValue(ref(db, "timer/water_speed"), snap => {
    document.getElementById("water_speed").innerText = snap.val() + "L/h";
  });

  onValue(ref(db, "timer/water_today"), snap => {
    document.getElementById("water_today").innerText = snap.val() + "L";
  });
}

start();
