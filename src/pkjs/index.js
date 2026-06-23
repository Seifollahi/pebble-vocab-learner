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
        <input type="checkbox" id="enabledCheckbox" checked>
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
        <input type="checkbox" id="vibrationCheckbox" checked>
        Enable App Vibrations
      </label>
    </div>
  </div>
  <div class="card">
    <div class="item">
      <label>Difficulty Levels</label>
      <label class="sub-label"><input type="checkbox" id="diffBasic" checked> Basic (Level 1)</label>
      <label class="sub-label"><input type="checkbox" id="diffIntermediate" checked> Intermediate (Level 2)</label>
      <label class="sub-label"><input type="checkbox" id="diffAdvanced" checked> Advanced (Level 3)</label>
    </div>
    <button id="saveBtn">Save Settings</button>
  </div>

  <script>
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

function pushTimelinePins(frequencyMins) {
  Pebble.getTimelineToken(function(token) {
    // Generate a few pins for the next 48 hours
    var numPins = Math.min(48 * 60 / frequencyMins, 48); // max 48 pins
    var now = new Date();
    
    for (var i = 1; i <= numPins; i++) {
      var pinTime = new Date(now.getTime() + i * frequencyMins * 60000);
      var pinId = 'vocab-pin-' + pinTime.getTime();
      
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
      
      var xhr = new XMLHttpRequest();
      xhr.open('PUT', 'https://timeline-api.rebble.io/v1/user/pins/' + pinId, true);
      xhr.setRequestHeader('Content-Type', 'application/json');
      xhr.setRequestHeader('X-User-Token', token);
      xhr.send(JSON.stringify(pin));
    }
    console.log("Pushed " + numPins + " timeline pins.");
  }, function(error) {
    console.log("Error getting timeline token: " + error);
  });
}

Pebble.addEventListener('showConfiguration', function(e) {
  var url = 'data:text/html;charset=utf-8,' + encodeURIComponent(configHTML);
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e.response && e.response !== 'CANCELLED' && e.response !== '{}') {
    var configData = JSON.parse(decodeURIComponent(e.response));
    var dict = {
      'NOTIFICATIONS_ENABLED': configData.enabled ? 1 : 0,
      'NOTIFICATIONS_FREQUENCY': parseInt(configData.frequency, 10),
      'VIBRATION_ENABLED': configData.vibration ? 1 : 0,
      'DIFF_BASIC': configData.diffBasic ? 1 : 0,
      'DIFF_INTERMEDIATE': configData.diffIntermediate ? 1 : 0,
      'DIFF_ADVANCED': configData.diffAdvanced ? 1 : 0
    };
    Pebble.sendAppMessage(dict, function() {
      console.log('Settings sent successfully');
      if (configData.enabled) {
        pushTimelinePins(parseInt(configData.frequency, 10));
      }
    }, function(e) {
      console.log('Settings send failed: ' + JSON.stringify(e));
    });
  }
});

Pebble.addEventListener('appmessage', function(e) {
  var dict = e.payload;
  if (dict['NOTIFICATIONS_ENABLED'] !== undefined && dict['NOTIFICATIONS_FREQUENCY'] !== undefined) {
    if (dict['NOTIFICATIONS_ENABLED']) {
      pushTimelinePins(dict['NOTIFICATIONS_FREQUENCY']);
    }
  }
});
