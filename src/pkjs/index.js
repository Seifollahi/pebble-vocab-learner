var MAX_PINS = 48;
var TIMELINE_API = 'https://timeline-api.rebble.io/v1/user/pins/';

var configHTML = `
<!DOCTYPE html>
<html>
<head>
  <title>Vocab Learner Settings</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; padding: 20px; background-color: #f0f0f0; }
    h1 { color: #333; }
    .card { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); margin-bottom: 20px; }
    .item { margin-bottom: 20px; }
    label { display: block; margin-bottom: 5px; font-weight: bold; }
    .sub-label { display: block; margin-bottom: 8px; font-weight: normal; }
    select, input[type="checkbox"] { padding: 10px; }
    button { background: #FF4500; color: white; border: none; padding: 15px 20px; border-radius: 5px; width: 100%; font-size: 18px; }
  </style>
</head>
<body>
  <h1>Vocab Learner Settings</h1>
  <div class="card">
    <div class="item">
      <label>
        <input type="checkbox" id="enabledCheckbox">
        Enable Notifications (Timeline Pins)
      </label>
    </div>
    <div class="item">
      <label>Notification Frequency</label>
      <select id="frequencySelect">
        <option value="1">Every 1 Minute (Testing)</option>
        <option value="60">Every 1 Hour</option>
        <option value="240">Every 4 Hours</option>
        <option value="1440">Every 24 Hours</option>
      </select>
    </div>
    <div class="item">
      <label>
        <input type="checkbox" id="vibrationCheckbox">
        Enable App Vibrations
      </label>
    </div>
  </div>
  <div class="card">
    <div class="item">
      <label>Difficulty Levels</label>
      <label class="sub-label"><input type="checkbox" id="diffBasic"> Basic (Level 1)</label>
      <label class="sub-label"><input type="checkbox" id="diffIntermediate"> Intermediate (Level 2)</label>
      <label class="sub-label"><input type="checkbox" id="diffAdvanced"> Advanced (Level 3)</label>
    </div>
    <button id="saveBtn">Save Settings</button>
  </div>

  <script>
    // Current settings are injected by the JS component so the page
    // reflects what is actually saved (v1 always showed the defaults).
    var saved = __SAVED_CONFIG__;
    document.getElementById('enabledCheckbox').checked = saved.enabled;
    document.getElementById('frequencySelect').value = String(saved.frequency);
    document.getElementById('vibrationCheckbox').checked = saved.vibration;
    document.getElementById('diffBasic').checked = saved.diffBasic;
    document.getElementById('diffIntermediate').checked = saved.diffIntermediate;
    document.getElementById('diffAdvanced').checked = saved.diffAdvanced;

    function getQueryParam(variable, defaultValue) {
      var query = location.search.substring(1);
      var vars = query.split('&');
      for (var i = 0; i < vars.length; i++) {
        var pair = vars[i].split('=');
        if (pair[0] === variable) {
          return decodeURIComponent(pair[1]);
        }
      }
      return defaultValue || false;
    }

    document.getElementById('saveBtn').addEventListener('click', function() {
      var config = {
        enabled: document.getElementById('enabledCheckbox').checked,
        frequency: document.getElementById('frequencySelect').value,
        vibration: document.getElementById('vibrationCheckbox').checked,
        diffBasic: document.getElementById('diffBasic').checked,
        diffIntermediate: document.getElementById('diffIntermediate').checked,
        diffAdvanced: document.getElementById('diffAdvanced').checked
      };
      var return_to = getQueryParam('return_to', 'pebblejs://close#');
      document.location = return_to + encodeURIComponent(JSON.stringify(config));
    });
  </script>
</body>
</html>
`;

var DEFAULT_CONFIG = {
  enabled: true,
  frequency: 60,
  vibration: true,
  diffBasic: true,
  diffIntermediate: true,
  diffAdvanced: true
};

function loadSavedConfig() {
  try {
    var raw = localStorage.getItem('vocab_config');
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
  return DEFAULT_CONFIG;
}

function saveConfig(config) {
  try {
    localStorage.setItem('vocab_config', JSON.stringify(config));
  } catch (err) {
    console.log('Could not save config: ' + err);
  }
}

function timelineRequest(method, pinId, pin, token, callback) {
  var xhr = new XMLHttpRequest();
  xhr.open(method, TIMELINE_API + pinId, true);
  xhr.onload = function() {
    if (xhr.status < 200 || xhr.status >= 300) {
      console.log('Timeline ' + method + ' ' + pinId + ' failed: HTTP ' + xhr.status);
    }
    if (callback) callback(xhr.status);
  };
  xhr.onerror = function() {
    console.log('Timeline ' + method + ' ' + pinId + ' network error');
    if (callback) callback(0);
  };
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.setRequestHeader('X-User-Token', token);
  xhr.send(pin ? JSON.stringify(pin) : null);
}

// Slot-based pin IDs (vocab-pin-0..47): a PUT to the same slot overwrites the
// old pin instead of accumulating new ones (v1 used timestamp IDs and never
// deleted anything, so the timeline filled up with stale pins).
function pushTimelinePins(frequencyMins) {
  Pebble.getTimelineToken(function(token) {
    var numPins = Math.min(Math.floor(48 * 60 / frequencyMins), MAX_PINS);
    var now = new Date();

    for (var slot = 0; slot < MAX_PINS; slot++) {
      var pinId = 'vocab-pin-' + slot;

      if (slot >= numPins) {
        // Slot not needed at this frequency: remove any stale pin in it.
        timelineRequest('DELETE', pinId, null, token, null);
        continue;
      }

      var pinTime = new Date(now.getTime() + (slot + 1) * frequencyMins * 60000);
      var pin = {
        "id": pinId,
        "time": pinTime.toISOString(),
        "layout": {
          "type": "genericPin",
          "title": "Vocab Review Time",
          "tinyIcon": "system://images/NOTIFICATION_FLAG",
          "body": "Tap to review a new word!"
        },
        "reminders": [
          {
            "time": pinTime.toISOString(),
            "layout": {
              "type": "genericReminder",
              "title": "Vocab Review Time",
              "tinyIcon": "system://images/NOTIFICATION_FLAG",
              "body": "Tap to review a new word!"
            }
          }
        ],
        "actions": [
          {
            "title": "Open App",
            "type": "openWatchApp",
            "launchCode": 1
          }
        ]
      };
      timelineRequest('PUT', pinId, pin, token, null);
    }
    console.log('Refreshed ' + numPins + ' timeline pin slots.');
  }, function(error) {
    console.log('Error getting timeline token: ' + error);
  });
}

function deleteAllTimelinePins() {
  Pebble.getTimelineToken(function(token) {
    for (var slot = 0; slot < MAX_PINS; slot++) {
      timelineRequest('DELETE', 'vocab-pin-' + slot, null, token, null);
    }
    console.log('Deleted all timeline pin slots.');
  }, function(error) {
    console.log('Error getting timeline token: ' + error);
  });
}

Pebble.addEventListener('showConfiguration', function(e) {
  var html = configHTML.replace('__SAVED_CONFIG__', JSON.stringify(loadSavedConfig()));
  var url = 'data:text/html;charset=utf-8,' + encodeURIComponent(html);
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e.response && e.response !== 'CANCELLED' && e.response !== '{}') {
    var configData;
    try {
      configData = JSON.parse(decodeURIComponent(e.response));
    } catch (err) {
      console.log('Could not parse config response: ' + err);
      return;
    }
    configData.frequency = parseInt(configData.frequency, 10) || 60;
    saveConfig(configData);

    var dict = {
      'NOTIFICATIONS_ENABLED': configData.enabled ? 1 : 0,
      'NOTIFICATIONS_FREQUENCY': configData.frequency,
      'VIBRATION_ENABLED': configData.vibration ? 1 : 0,
      'DIFF_BASIC': configData.diffBasic ? 1 : 0,
      'DIFF_INTERMEDIATE': configData.diffIntermediate ? 1 : 0,
      'DIFF_ADVANCED': configData.diffAdvanced ? 1 : 0
    };
    Pebble.sendAppMessage(dict, function() {
      console.log('Settings sent successfully');
    }, function(err) {
      console.log('Settings send failed: ' + JSON.stringify(err));
    });

    // Pins are only refreshed here, on an explicit settings save.
    // (v1 also pushed a fresh batch on every app launch.)
    if (configData.enabled) {
      pushTimelinePins(configData.frequency);
    } else {
      deleteAllTimelinePins();
    }
  }
});
