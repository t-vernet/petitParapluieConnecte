R"rawText(
<!DOCTYPE html>
<html lang="en" dir="ltr">

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <meta http-equiv="X-UA-Compatible" content="ie=edge" />
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.0.1/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-+0n0xVW2eSR5OomGNYDnhzAbDsOXxcvSN1TPprVMTNDbiYZCxYbOOl7+AMvyTG2x" crossorigin="anonymous">
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.0.1/dist/js/bootstrap.bundle.min.js" integrity="sha384-gtEjrD/SeCtmISkJkNUaaKMoLD0//ElJ19smozuHV6z3Iehds+3Ulb9Bn9Plx0x4" crossorigin="anonymous"></script>
  <script type="text/javascript">
    var xmlHttp = createXmlHttpObject();

    function createXmlHttpObject() { // seems to handle web browsers compatibility
      if (window.XMLHttpRequest) {
        xmlHttp = new XMLHttpRequest();
      } else {
        xmlHttp = new ActiveXObject('Microsoft.XMLHTTP'); // for internet explorer compatibility ? not used by edge
      }
      return xmlHttp;
    }

    var click;
    var town;

    function handleServerResponse() {
      xmlResponse = xmlHttp.responseXML;
      xmldoc = xmlResponse.getElementsByTagName('response');
      message = xmldoc[0].firstChild.nodeValue;
      if (message == 1) {
        click = 1;
        message = 'ouvert';
        document.getElementById("umbrellaState").className = document.getElementById("umbrellaState").className.replace(/(?:^|\s)bg-danger(?!\S)/g, '');
        document.getElementById("umbrellaState").className += " bg-success";
      } else {
        click = 0;
        message = 'ferm&eacute;';
        document.getElementById("umbrellaState").className = document.getElementById("umbrellaState").className.replace(/(?:^|\s)bg-success(?!\S)/g, '');
        document.getElementById("umbrellaState").className += " bg-danger";
      }
      document.getElementById('umbrellaState').innerHTML = message;
    }

    function process() {
      xmlHttp.open('PUT', 'xml', true);
      xmlHttp.onreadystatechange = handleServerResponse; // no brackets?????
      xmlHttp.send(null);
      setTimeout('process()', 200);
    }

    // function process2() {
    // xmlHttp.open('SET', 'set1ESPval?Start=' + click, true);
    // xmlHttp.send(null);
    // setTimeout('process2()', 400);
    // }

    function RunTestButtonWasClicked() {
      click = (click == 1) ? 0 : 1;
      xmlHttp.open('SET', 'set1ESPval?Start=' + click, true);
      xmlHttp.send(null);
    }

    function RunTownButtonWasClicked() {
      town = document.getElementById("town").value
      xmlHttp.open("SET", "set1ESPval?Town=" + town, true);
      xmlHttp.send(null);
    }

  </script>
  <title>POCL-Parapluie</title>
</head>

<body bgcolor='#E6E6FA' onload='process()'>
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
              <button type="button" id="buttonTestUmbrella" class="btn btn-lg btn-warning btn-block" onClick="RunTestButtonWasClicked()">
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
                <div class="invalid-feedback">Veuillez sélétionner une ville valide.</div>
                <button type="button" id="townButton" class="btn btn-lg btn-warning btn-block" onClick="RunTownButtonWasClicked()">OK</button>
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
