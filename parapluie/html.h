R"rawText(
<!DOCTYPE html>
<html lang="en" dir="ltr">

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <meta http-equiv="X-UA-Compatible" content="ie=edge" />
  <link rel="icon" type="image/png" href="data:image/png;base64,iVBORw0KGgo=">
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.0.1/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-+0n0xVW2eSR5OomGNYDnhzAbDsOXxcvSN1TPprVMTNDbiYZCxYbOOl7+AMvyTG2x" crossorigin="anonymous">
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.0.1/dist/js/bootstrap.bundle.min.js" integrity="sha384-gtEjrD/SeCtmISkJkNUaaKMoLD0//ElJ19smozuHV6z3Iehds+3Ulb9Bn9Plx0x4" crossorigin="anonymous"></script>
  <script type="text/javascript">

    window.addEventListener('load', init, true);
    var timeout = 250 ;
    var state = 2;

    function init() {
        setTimeout(webSocketConnection, Math.min(10000,timeout+=timeout));
        document.getElementById('testButton').addEventListener('click', RunTestButtonWasClicked);
        document.getElementById('townButton').addEventListener('click', RunTownButtonWasClicked);
        document.getElementById('keyButton').addEventListener('click', RunAPIButtonWasClicked);
    }

    var xmlHttp = createXmlHttpObject();

    function createXmlHttpObject() { // seems to handle web browsers compatibility
      if (window.XMLHttpRequest) {
        xmlHttp = new XMLHttpRequest();
      } else {
        xmlHttp = new ActiveXObject('Microsoft.XMLHTTP'); // for internet explorer compatibility ? not used by edge
      }
      return xmlHttp;
    }

    function RunTestButtonWasClicked() {
      test = (state == 1) ? 0 : 1;
      xmlHttp.open('SET', 'test?Test=' + test, true);
      xmlHttp.send(null);
    }

    function RunTownButtonWasClicked() {
      town = document.getElementById("town").value
      xmlHttp.open("SET", "town?Town=" + town, true);
      xmlHttp.send(null);
    }

    function RunAPIButtonWasClicked() {
      key = document.getElementById("key").value
      xmlHttp.open("SET", "apikey?Key=" + key, true);
      xmlHttp.send(null);
    }

    function webSocketConnection() {
        if (!navigator.onLine) {
            console.log('You are offline.');
        } else {
            console.log('You are online.');
            console.log('Attempting to reconnect to the server.');
            var webSocket = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
            webSocket.onopen = function (e) {
                webSocket.send('Client connected ' + new Date());
                timeout = 250;
            };
            webSocket.onmessage = function (e) {
                console.log('Server sent : ', e.data);
                if (typeof e.data === 'string'){
                    //create a JSON object
                    var jsonObject = JSON.parse(e.data);
                    switch (jsonObject.event) {
                        case 'state':
                            console.log('JSON event "state".');
                            umbrellaStateChange(jsonObject);
                            break;
                        default:
                            console.log('JSON event '+jsonObject.event+' is not defined.');
                    }
                }
            };
            webSocket.onclose = function (e) {
                console.log('' + e.data);
                if (e.code != 1000) {
                    setTimeout(webSocketConnection, Math.min(10000,timeout+=timeout));
                }
            };
            webSocket.onerror = function (error) {
                console.log('WebSocket Error ', error);
            };
    }
    }

    function umbrellaStateChange(jsonObject) {
        var oldState = state;
        state = jsonObject.state;
        if (oldState != state){
            if (state == '1') {
                message = 'ouvert';
                document.getElementById('umbrellaState').className = document.getElementById('umbrellaState').className.replace(/(?:^|\s)bg-danger(?!\S)/g, '');
                document.getElementById('umbrellaState').className += ' bg-success';
            }
            if (state == '0') {
                message = 'ferm&eacute;';
                document.getElementById('umbrellaState').className = document.getElementById('umbrellaState').className.replace(/(?:^|\s)bg-success(?!\S)/g, '');
                document.getElementById('umbrellaState').className += ' bg-danger';
            }
            document.getElementById('umbrellaState').innerHTML = message;
        }
    }

  </script>
  <title>POCL-Parapluie</title>
</head>

<!-- <body bgcolor='#E6E6FA' onload='process()'> -->
<body bgcolor='#E6E6FA'>
  <header id="main-header" class="py-2 bg-success text-white">
    <div class="container">
      <div class="row justify-content-md-center">
        <div class="col-md-6 text-center">
          <h1>Controle du POCL - Paraluie</h1>
        </div>
      </div>
    </div>
  </header>
  <section class="py-5 bg-white">
    <div class="container">
      <div class="row">
        <div class="col">
          <div class="card bg-light m-2">
            <div class="card-header">Etat du parapluie</div>
            <div class="card-body">
              <h1><span id="umbrellaState" class="badge bg-secondary"></span></h1>
            </div>
          </div>
        </div>
        <div class="col">
          <div class="card bg-light m-2">
            <div class="card-header">Test du parapluie</div>
            <div class="card-body">
              <p class="card-text">Pressez le boutton pour ouvrir/fermer le parapluie.</p>
              <button type="button" id="testButton" class="btn btn-lg btn-warning btn-block">
                Test
              </button>
            </div>
          </div>
        </div>
      </div>
      <div class="row">
        <div class="col">
          <div class="card bg-light m-2" style="min-height: 15rem;">
            <div class="card-header">
              Choix de la ville
            </div>
            <div class="card-body">
              <form class="was-validated">
                <label for="town" class="form-label">Ville</label>
                <input type="text" id="town" class="form-control" name="town" list="townNames" placeholder="Entrez la ville" required>
                <datalist id="townNames">
                  <option value="3030300">Brest</option>
                  <option value="6431033">Quimper</option>
                  <option value="6430976">Morlaix</option>
                  <option value="6453798">Lannion</option>
                  <option value="6453805">Saint-Brieuc</option>
                  <option value="6432801">Rennes</option>
                  <option value="6437298">Lorient</option>
                  <option value="2970777">Vannes</option>
                  <option value="6434483">Nantes</option>
                  <option value="6456407">Le Mans</option>
                  <option value="6427109">Caen</option>
                  <option value="6452361">Angers</option>
                  <option value="6456577">La Roche sur Yon</option>
                  <option value="3021411">Dieppe</option>
                </datalist>
                <div class="valid-feedback">Valide.</div>
                <div class="invalid-feedback">Veuillez séléctionner une ville valide.</div>
                <button type="button" id="townButton" class="btn btn-lg btn-warning btn-block">OK</button>
              </form>
            </div>
          </div>
        </div>
        <div class="col">
          <div class="card bg-light m-2" style="min-height: 15rem;">
            <div class="card-header">
                Parametrage de l'API
            </div>
            <div class="card-body">
                <form class="was-validated">
                    <label for="key" class="form-label">Clef de l'API</label>
                    <input type="text" id="key" class="form-control" name="key" placeholder="API Key" required>
                    <div class="valid-feedback">Valide.</div>
                    <div class="invalid-feedback">Veuillez séléctionner une clé valide.</div>
                    <button type="button" id="keyButton" class="btn btn-lg btn-warning btn-block">OK</button>
                </form>
            </div>
          </div>
        </div>
      </div>
    </div>
  </section>
</body>

</html>
)rawText"
