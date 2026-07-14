var TIMELINE_API = 'https://timeline-api.rebble.io/v1/user/pins/';
var DAY_PIN_SLOTS = 2;        // today + tomorrow
var LEGACY_PIN_SLOTS = 48;    // v2.0/2.1 used vocab-pin-0..47

// ---------------------------------------------------------------------------
// Config + stats storage (phone side)
// ---------------------------------------------------------------------------

var DEFAULT_CONFIG = {
  enabled: true,
  remindersPerDay: 2,   // 1..3 (timeline allows max 3 reminders per pin)
  windowStart: 9,       // reminders only between these hours
  windowEnd: 21,
  vibration: false, // haptics off by default
  diffBasic: true,
  diffIntermediate: true,
  diffAdvanced: true
};

function loadConfig() {
  try {
    var raw = localStorage.getItem('vocab_config_v2');
    if (raw) {
      var parsed = JSON.parse(raw);
      for (var key in DEFAULT_CONFIG) {
        if (parsed[key] === undefined) parsed[key] = DEFAULT_CONFIG[key];
      }
      return parsed;
    }
  } catch (err) {
    console.log('Could not load saved config: ' + err);
  }
  return JSON.parse(JSON.stringify(DEFAULT_CONFIG));
}

function saveConfig(config) {
  try {
    localStorage.setItem('vocab_config_v2', JSON.stringify(config));
  } catch (err) {
    console.log('Could not save config: ' + err);
  }
}

function loadStats() {
  try {
    var raw = localStorage.getItem('vocab_stats');
    if (raw) return JSON.parse(raw);
  } catch (err) {}
  return null;
}

// ---------------------------------------------------------------------------
// Timeline pins: ONE pin per day carrying up to 3 reminders, instead of one
// pin per interval (v2.1 with hourly reminders put 24+ pins in the timeline).
// ---------------------------------------------------------------------------

function timelineRequest(method, pinId, pin, token) {
  var xhr = new XMLHttpRequest();
  xhr.open(method, TIMELINE_API + pinId, true);
  xhr.onload = function() {
    if (xhr.status < 200 || xhr.status >= 300 && xhr.status !== 404) {
      console.log('Timeline ' + method + ' ' + pinId + ': HTTP ' + xhr.status);
    }
  };
  xhr.onerror = function() {
    console.log('Timeline ' + method + ' ' + pinId + ' network error');
  };
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.setRequestHeader('X-User-Token', token);
  xhr.send(pin ? JSON.stringify(pin) : null);
}

// Reminder times spread evenly across the waking window, e.g. 9-21 with
// 2/day -> 12:00 and 18:00.
function reminderHoursFor(config) {
  var n = Math.max(1, Math.min(3, config.remindersPerDay | 0));
  var start = config.windowStart, end = config.windowEnd;
  if (end <= start) { start = 9; end = 21; }
  var span = end - start;
  var hours = [];
  for (var k = 0; k < n; k++) {
    hours.push(start + (k + 0.5) * span / n);
  }
  return hours;
}

function pushDayPins(config) {
  Pebble.getTimelineToken(function(token) {
    var now = new Date();
    var hours = reminderHoursFor(config);

    for (var day = 0; day < DAY_PIN_SLOTS; day++) {
      var pinId = 'vocab-day-' + day;
      var reminders = [];
      var firstTime = null;

      for (var k = 0; k < hours.length; k++) {
        var t = new Date(now.getFullYear(), now.getMonth(), now.getDate() + day);
        t.setMinutes(Math.round(hours[k] * 60));
        if (t <= now) continue; // don't schedule reminders in the past
        if (!firstTime) firstTime = t;
        reminders.push({
          "time": t.toISOString(),
          "layout": {
            "type": "genericReminder",
            "title": "Vocab review",
            "tinyIcon": "system://images/NOTIFICATION_REMINDER",
            "body": "A few words are due. Open the app for a 30-second session."
          }
        });
      }

      if (reminders.length === 0) {
        // All of today's reminder times have passed: clear the slot.
        timelineRequest('DELETE', pinId, null, token);
        continue;
      }

      var pin = {
        "id": pinId,
        "time": firstTime.toISOString(),
        "layout": {
          "type": "genericPin",
          "title": "Vocab review",
          "subtitle": reminders.length + (reminders.length === 1 ? " reminder" : " reminders") + " today",
          "tinyIcon": "system://images/TIMELINE_CALENDAR",
          "body": "Review your due words. Hold SELECT if you know a word, hold DOWN if you forgot it."
        },
        "reminders": reminders,
        "actions": [
          {
            "title": "Open App",
            "type": "openWatchApp",
            "launchCode": 1
          }
        ]
      };
      timelineRequest('PUT', pinId, pin, token);
    }
    console.log('Refreshed ' + DAY_PIN_SLOTS + ' day pins (' + hours.length + ' reminders/day).');
  }, function(error) {
    console.log('Error getting timeline token: ' + error);
  });
}

function deleteDayPins() {
  Pebble.getTimelineToken(function(token) {
    for (var day = 0; day < DAY_PIN_SLOTS; day++) {
      timelineRequest('DELETE', 'vocab-day-' + day, null, token);
    }
  }, function(error) {
    console.log('Error getting timeline token: ' + error);
  });
}

// One-time cleanup of the per-interval pins from v2.0/v2.1.
function cleanupLegacyPins() {
  if (localStorage.getItem('legacy_pins_cleaned')) return;
  Pebble.getTimelineToken(function(token) {
    for (var slot = 0; slot < LEGACY_PIN_SLOTS; slot++) {
      timelineRequest('DELETE', 'vocab-pin-' + slot, null, token);
    }
    localStorage.setItem('legacy_pins_cleaned', '1');
    console.log('Cleaned up legacy interval pins.');
  }, function(error) {});
}

// ---------------------------------------------------------------------------
// Configuration page
// ---------------------------------------------------------------------------

var configHTML = `
<!DOCTYPE html>
<html>
<head>
  <title>Vocab Learner Settings</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: -apple-system, sans-serif; padding: 16px; background-color: #f0f0f0; margin: 0; }
    h1 { color: #333; font-size: 22px; }
    h2 { color: #666; font-size: 14px; text-transform: uppercase; letter-spacing: 0.5px; margin: 24px 4px 8px; }
    .card { background: white; padding: 16px; border-radius: 10px; box-shadow: 0 1px 3px rgba(0,0,0,0.1); }
    .item { margin-bottom: 16px; }
    .item:last-child { margin-bottom: 0; }
    label { display: block; margin-bottom: 4px; font-weight: 600; color: #222; }
    .sub-label { display: block; margin-bottom: 6px; font-weight: normal; }
    select { padding: 8px; width: 100%; font-size: 16px; border-radius: 6px; border: 1px solid #ccc; background: white; }
    .row { display: flex; gap: 10px; }
    .row > div { flex: 1; }
    button { background: #FF4500; color: white; border: none; padding: 14px 20px; border-radius: 8px; width: 100%; font-size: 17px; font-weight: 600; margin-top: 20px; }
    .stats { display: flex; justify-content: space-between; text-align: center; }
    .stat b { display: block; font-size: 22px; color: #FF4500; }
    .stat span { font-size: 12px; color: #777; }
    .stale { font-size: 12px; color: #999; margin-top: 8px; text-align: center; }
    .danger label { color: #c0392b; }
    .hint { font-size: 12px; color: #888; margin-top: 4px; }
  </style>
</head>
<body>
  <h1>Vocab Learner</h1>

  <h2>Reminders</h2>
  <div class="card">
    <div class="item">
      <label><input type="checkbox" id="enabledCheckbox"> Timeline reminders</label>
      <div class="hint">One quiet pin per day in your timeline, with reminder pop-ups at the times below.</div>
    </div>
    <div class="item">
      <label>Reminders per day</label>
      <select id="perDaySelect">
        <option value="1">1 (midday)</option>
        <option value="2">2</option>
        <option value="3">3</option>
      </select>
    </div>
    <div class="item row">
      <div>
        <label>Not before</label>
        <select id="windowStart">
          <option value="6">6:00</option><option value="7">7:00</option>
          <option value="8">8:00</option><option value="9">9:00</option>
          <option value="10">10:00</option><option value="11">11:00</option>
          <option value="12">12:00</option>
        </select>
      </div>
      <div>
        <label>Not after</label>
        <select id="windowEnd">
          <option value="17">17:00</option><option value="18">18:00</option>
          <option value="19">19:00</option><option value="20">20:00</option>
          <option value="21">21:00</option><option value="22">22:00</option>
          <option value="23">23:00</option>
        </select>
      </div>
    </div>
  </div>

  <h2>Study</h2>
  <div class="card">
    <div class="item">
      <label>Difficulty levels</label>
      <label class="sub-label"><input type="checkbox" id="diffBasic"> Basic</label>
      <label class="sub-label"><input type="checkbox" id="diffIntermediate"> Intermediate</label>
      <label class="sub-label"><input type="checkbox" id="diffAdvanced"> Advanced</label>
    </div>
    <div class="item">
      <label><input type="checkbox" id="vibrationCheckbox"> Haptic vibrations</label>
    </div>
  </div>

  <h2>Your progress</h2>
  <div class="card">
    <div class="stats">
      <div class="stat"><b id="statMastered">–</b><span>Mastered</span></div>
      <div class="stat"><b id="statReviewed">–</b><span>Reviewed</span></div>
      <div class="stat"><b id="statGraduated">–</b><span>Graduated</span></div>
    </div>
    <div class="stale" id="statsStale"></div>
  </div>

  <h2>Danger zone</h2>
  <div class="card danger">
    <label><input type="checkbox" id="resetCheckbox"> Reset all learning progress</label>
    <div class="hint">Erases every word's spaced-repetition level and your lifetime stats on the watch. Applied when you save.</div>
  </div>

  <button id="saveBtn">Save Settings</button>

  <script>
    var saved = __SAVED_CONFIG__;
    var stats = __SAVED_STATS__;

    document.getElementById('enabledCheckbox').checked = saved.enabled;
    document.getElementById('perDaySelect').value = String(saved.remindersPerDay);
    document.getElementById('windowStart').value = String(saved.windowStart);
    document.getElementById('windowEnd').value = String(saved.windowEnd);
    document.getElementById('vibrationCheckbox').checked = saved.vibration;
    document.getElementById('diffBasic').checked = saved.diffBasic;
    document.getElementById('diffIntermediate').checked = saved.diffIntermediate;
    document.getElementById('diffAdvanced').checked = saved.diffAdvanced;

    if (stats) {
      document.getElementById('statMastered').textContent = stats.learned;
      document.getElementById('statReviewed').textContent = stats.reviewed;
      document.getElementById('statGraduated').textContent = stats.graduated + '/' + stats.total;
      var mins = Math.round((Date.now() - stats.at) / 60000);
      document.getElementById('statsStale').textContent =
        'Synced from watch ' + (mins < 2 ? 'just now' : mins < 90 ? mins + ' min ago' : Math.round(mins/60) + ' h ago');
    } else {
      document.getElementById('statsStale').textContent = 'Open the app on your watch to sync stats.';
    }

    function getQueryParam(variable, defaultValue) {
      var query = location.search.substring(1);
      var vars = query.split('&');
      for (var i = 0; i < vars.length; i++) {
        var pair = vars[i].split('=');
        if (pair[0] === variable) return decodeURIComponent(pair[1]);
      }
      return defaultValue || false;
    }

    document.getElementById('saveBtn').addEventListener('click', function() {
      if (document.getElementById('resetCheckbox').checked &&
          !confirm('Really erase all learning progress on the watch? This cannot be undone.')) {
        return;
      }
      var config = {
        enabled: document.getElementById('enabledCheckbox').checked,
        remindersPerDay: parseInt(document.getElementById('perDaySelect').value, 10),
        windowStart: parseInt(document.getElementById('windowStart').value, 10),
        windowEnd: parseInt(document.getElementById('windowEnd').value, 10),
        vibration: document.getElementById('vibrationCheckbox').checked,
        diffBasic: document.getElementById('diffBasic').checked,
        diffIntermediate: document.getElementById('diffIntermediate').checked,
        diffAdvanced: document.getElementById('diffAdvanced').checked,
        resetProgress: document.getElementById('resetCheckbox').checked
      };
      var return_to = getQueryParam('return_to', 'pebblejs://close#');
      document.location = return_to + encodeURIComponent(JSON.stringify(config));
    });
  </script>
</body>
</html>
`;

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

Pebble.addEventListener('ready', function(e) {
  cleanupLegacyPins();
  // Keep the two day pins rolling forward on every launch. Slot IDs make
  // this idempotent (PUT overwrites, nothing accumulates).
  var config = loadConfig();
  if (config.enabled) pushDayPins(config);
});

Pebble.addEventListener('showConfiguration', function(e) {
  var html = configHTML
    .replace('__SAVED_CONFIG__', JSON.stringify(loadConfig()))
    .replace('__SAVED_STATS__', JSON.stringify(loadStats()));
  Pebble.openURL('data:text/html;charset=utf-8,' + encodeURIComponent(html));
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e.response || e.response === 'CANCELLED' || e.response === '{}') return;
  var configData;
  try {
    configData = JSON.parse(decodeURIComponent(e.response));
  } catch (err) {
    console.log('Could not parse config response: ' + err);
    return;
  }
  var doReset = !!configData.resetProgress;
  delete configData.resetProgress;
  saveConfig(configData);

  var dict = {
    'NOTIFICATIONS_ENABLED': configData.enabled ? 1 : 0,
    'NOTIFICATIONS_FREQUENCY': configData.remindersPerDay,
    'VIBRATION_ENABLED': configData.vibration ? 1 : 0,
    'DIFF_BASIC': configData.diffBasic ? 1 : 0,
    'DIFF_INTERMEDIATE': configData.diffIntermediate ? 1 : 0,
    'DIFF_ADVANCED': configData.diffAdvanced ? 1 : 0
  };
  if (doReset) dict['RESET_PROGRESS'] = 1;

  Pebble.sendAppMessage(dict, function() {
    console.log('Settings sent successfully' + (doReset ? ' (with progress reset)' : ''));
  }, function(err) {
    console.log('Settings send failed: ' + JSON.stringify(err));
  });

  if (configData.enabled) {
    pushDayPins(configData);
  } else {
    deleteDayPins();
  }
});

// Watch pushes its stats on every launch; cache them for the config page.
Pebble.addEventListener('appmessage', function(e) {
  var d = e.payload;
  if (d['STAT_LEARNED'] !== undefined) {
    try {
      localStorage.setItem('vocab_stats', JSON.stringify({
        learned: d['STAT_LEARNED'],
        reviewed: d['STAT_REVIEWED'],
        graduated: d['STAT_GRADUATED'],
        total: d['STAT_TOTAL'],
        at: Date.now()
      }));
    } catch (err) {}
  }
});
