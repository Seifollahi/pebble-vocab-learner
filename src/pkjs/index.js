var configHTML = `
<!DOCTYPE html>
<html>
<head>
  <title>Vocab Learner Settings</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; padding: 20px; background-color: #f0f0f0; }
    h1 { color: #333; }
    .card { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    .item { margin-bottom: 20px; }
    label { display: block; margin-bottom: 5px; font-weight: bold; }
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
        Enable Notifications
      </label>
    </div>
    <div class="item">
      <label>Notification Frequency</label>
      <select id="frequencySelect">
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
        vibration: document.getElementById('vibrationCheckbox').checked
      };
      var return_to = getQueryParam('return_to', 'pebblejs://close#');
      document.location = return_to + encodeURIComponent(JSON.stringify(config));
    });
  </script>
</body>
</html>
`;

Pebble.addEventListener('showConfiguration', function(e) {
  var url = 'data:text/html;charset=utf-8,' + encodeURIComponent(configHTML);
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e.response && e.response !== 'CANCELLED') {
    var configData = JSON.parse(decodeURIComponent(e.response));
    var dict = {
      'NOTIFICATIONS_ENABLED': configData.enabled ? 1 : 0,
      'NOTIFICATIONS_FREQUENCY': parseInt(configData.frequency, 10),
      'VIBRATION_ENABLED': configData.vibration ? 1 : 0
    };
    Pebble.sendAppMessage(dict, function() {
      console.log('Settings sent successfully');
    }, function(e) {
      console.log('Settings send failed: ' + JSON.stringify(e));
    });
  }
});
