///////////////
// PARAPLUIE //
///////////////

/*  Un programme pour récupérer les données météo du site http://openweathermap.org/
 *  puis les afficher sur le moniteur série (partie de programme inspiré de http://educ8s.tv/esp8266-weather-display/)
 *  et ouvre ou ferme un parapluie chinois suivant les précipitations d'un lieu choisis (inspiré de http://julienlevesque.net/little-umbrella/ )
 *  Cette version intègre une page web de test du parapluie en hackant des bout de codes trouvés ici : https://www.ulasdikme.com/projects/esp8266/esp8266_ajax_websocket.php
 *  Antony Le Goïc-Auffret
 *  sous licence CC-By-SA - dimanche 1er août 2020 - Brest- Bretagne - France - Europe - Terre - Système solaire - Voie Lactée.
 *
 *                                                                                        _
 *                                                                                      _| |_
 *                                                                                   _/ /  \  \_
 *                                                                                _/   /    \    \_
 *                                                                             _/     /      \      \_
 *                                                                          _/  _._  /  _._   \ _._    \_
 *                                                                        / \_/     \_/  |_|\_/     \_/  \
 *                                                                                      /|#|
 *                                                                                     | | |
 *                                                                                     | | |
 *                                                                                     | | |
 *                                                                                     | | |
 *                                                                                     | | |
 *                                                                                     | | |
 *                                                                                 ____|_\_/_
 *                                                                                |    |     |
 *                                     BROCHAGE                                   |    |     |
 *                               _________________                          ______|____|_____|__________
 *                              /     D1 mini     \                        |           |               |
 *              Non Attribué - |[ ]RST        TX[ ]| - Non Attribué        |   ___     |               |
 *              Non Attribué - |[ ]A0  -GPIO  RX[ ]| - Non Attribué        |  |_°_|    |               |
 *              Non Attribué - |[ ]D0-16    5-D1[ ]| - Non Attribué        | |     |   |               |
 *              Non Attribué - |[ ]D5-14    4-D2[ ]| - Non Attribué        | |     |   |               |
 *              Non Attribué - |[ ]D6-12    0-D3[X]| - gestion servo < _   | |   __|___|               |
 *              Non Attribué - |[ ]D7-13    2-D4[X]| - LED_BUILTIN       \ | | (o)_____/               |
 *              Non Attribué - |[ ]D8-15     GND[X]| - alim servo <-      || |Servo|                   |
 *              Non Attribué - |[ ]3V3 .      5V[X]| - alim servo <_|_    || |_____|                   |
 *                             |       +---+       |                |  \   \  |_°_|                    |
 *                             |_______|USB|_______|                  \  \ |\__/||                     |
 *              Le wemos est connecté en USB à votre ordinateur,        \ \|___/ |                     |
 *                        le moniteur série ouvert                       \_|____/                      |
 *                                                                         |___________________________|
 *
 */

#include <ArduinoJson.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets

// id unique du POCL à paramétrer :
const char *POCL_ID = "1";
// nom du POCL
const char *POCL_NAME = "POCL-Parapluie";
// mot de pas AP
const char *AP_PW = "lpdgo2021";
/***************************** START-WebServer ********************************
* Début de la gestion du server web par ESP8266WebServer                      *
******************************************************************************/

ESP8266WebServer server(80);  //déclaration du serveur web sur le port
String webSite,xml,data,town,key; //déclaration de variables
bool testType; // Doit on tester l'ouverture (true) ou la fermeture (false);
bool test = false;

void buildWebsite(){ // Fonction qui écrit le code html du site web à partir du fichier html.h
        static const char* html =
  #include "html.h"
        ;
        webSite = String((const char*) html);
}

void handleWebsite(){ // génère le site web
        buildWebsite(); // écriture du html
        server.send(200,"text/html",webSite); // mise en ligne du site
}

void handleTest(){
        test = true;
        testType = server.arg("Test").toFloat();
        server.send(200,"text/plain","200: OK");
}

void handleTown(){
        town = server.arg("Town");
        server.send(200,"text/plain","200: OK");
}

void handleAPIKey(){
        key = server.arg("Key");
        server.send(200,"text/plain","200: OK");
}

void handleNotFound(){
    server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
/***************************** START-WebSocket ********************************
* Début de la gestion du server web par ESP8266WebServer                      *
******************************************************************************/
WebSocketsServer webSocket(81);    // create a websocket server on port 81

void startWebSocket() { // Start a WebSocket server
        webSocket.begin();                    // start the websocket server
        webSocket.onEvent(webSocketEvent);    // if there's an incomming websocket message, go to function 'webSocketEvent'
        Serial.println("WebSocket server started.");
}

bool status = 1;   // enregistre si le parapluie est ouvert ou fermé.
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
        switch (type) {
        case WStype_DISCONNECTED:                 // if the websocket is disconnected
                Serial.printf("[%u] Disconnected!\n", num);
                break;
        case WStype_CONNECTED: {                    // if a new websocket connection is established
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                StaticJsonDocument<64> doc;                 // Create JsonDocument of fixed size
                JsonObject root = doc.to<JsonObject>();                  // create JsonObject from JsonDocument
                root["event"] = "state";                 // populate JsonObject
                root["state"] = status;                 // populate JsonObject
                char buffer[64];                 // create temp buffer
                serializeJson(root, buffer);                 // serialize to buffer
                webSocket.broadcastTXT(buffer);                 // send buffer to web socket
        }
        break;
        case WStype_TEXT:                         // if new text data is received
                Serial.printf("[%u] get Text: %s\n", num, payload);
                break;
        case WStype_ERROR:                        // if an erro happen
                Serial.printf("[%u] get Error: %s\n", num, payload);
                break;
        }
}

/********************************* START-WiFi *********************************
* Début de la gestion du wifi par WiFiManager                                 *
******************************************************************************/

void connexion() {

        WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

        // WiFi.mode(WiFi_STA); // it is a good practice to make sure your code sets wifi mode how you want it.

        //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
        WiFiManager wm;

        //reset settings - wipe credentials for testing
        //wm.resetSettings();

        // Automatically connect using saved credentials,
        // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
        // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
        // then goes into a blocking loop awaiting configuration and will return success result

        bool res;
        // res = wm.autoConnect(); // auto generated AP name from chipid
        // res = wm.autoConnect("AutoConnectAP"); // anonymous ap

        char * apSSID = new char[strlen(POCL_NAME)+1+strlen(POCL_ID)+1];
        strcpy(apSSID, POCL_NAME);
        strcat(apSSID, "-");
        strcat(apSSID, POCL_ID);
        strcat(apSSID, "\0");
        res = wm.autoConnect(apSSID,AP_PW); // password protected ap

        if(!res) {
                Serial.println("Failed to connect");
                // ESP.restart();
        }
        else {
                //if you get here you have connected to the WiFi
                Serial.println("connected...yeey :)");
        }
}

/*************************** START-APIComSetup *******************************S
* Début de la gestion de la communication avec OpenWeatherMap API            *
******************************************************************************/

WiFiClient client;

char nomDuServeur[] = "api.openweathermap.org";  // Serveur distant auquel on va se connecter
String cledAPI = "";  // clé de l'API du site http://openweathermap.org/
// il faudra vous créer un compte et récupérer votre clé d'API, une formalité !
// Sur le site, vous trouvez les identifiants de toutes les villes du monde.
String identifiantVille = "3030300";   // indiquez l'identifiant d'une ville (CityID), ici Caen en France
/*Quelques identifiants d'autres villes françaises :
 * 3030300, Brest
 * 6431033, Quimper
 * 6430976, Morlaix
 * 6453798, Lannion
 * 6453805, Saint-Brieuc
 * 6432801, Rennes
 * 6437298, Lorient
 * 2970777, Vannes
 * 6434483, Nantes
 * 6456407, Le Mans
 * 6427109, Caen
 * 6452361, Angers
 * 6456577, La Roche sur Yon
 * 3021411, Dieppe
 */

String resultat;

String descriptionMeteo = "";
String lieu = "";
String pays = "";
float temperature = 0;
float humidite = 0;
float pression = 0;

int para = 100;

void prendDonneesMeteo() //Fonction qui utilise le client web du D1 mini pour envoyer/recevoir des données de requêtes GET.
{
        if (client.connect(nomDuServeur, 80)) { // Démarre la connexion du client, recherche les connexions
                client.println("GET /data/2.5/weather?id=" + identifiantVille + "&units=metric&lang=fr&APPID=" + cledAPI);
                client.println("Host: api.openweathermap.org");
                client.println("User-Agent: ArduinoWiFi/1.1");
                client.println("Connection: close");
                client.println();
        }
        else {
                Serial.println("Echec de la connexion"); //message d'erreur si la connexion échoue
                Serial.println();
        }
        resultat = "";
        while (client.connected() && !client.available()) delay(1); // attend les données
        while (client.connected() || client.available()) {    // soit le client est connecté, soit il a des données disponibles
                char c = client.read(); // récupère les données
                resultat = resultat + c; // les agrège dans la chaine de caractère resultat
        }

        client.stop(); // stoppe le client
        resultat.replace('[', ' ');
        resultat.replace(']', ' ');
        Serial.println(resultat); // écrit la chaine de caractère en entier sur le moniteur série

        char tableauJson[resultat.length() + 1];
        resultat.toCharArray(tableauJson, sizeof(tableauJson));
        tableauJson[resultat.length() + 1] = '\0';

        StaticJsonDocument<1024> doc;

        DeserializationError error = deserializeJson(doc, tableauJson);

        if (error) {
                Serial.print(F("deserializeJson() failed with code "));
                Serial.println(error.c_str());
                return;
        }

        lieu = String((const char*) doc["name"]);
        pays = String((const char*) doc["sys"]["country"]);
        String meteo = doc["weather"]["main"];
        descriptionMeteo = String((const char*) doc["weather"]["description"]);
        String id = doc["weather"]["id"];   //récupère le chiffre identifiant "id" de l'état météo sous forme de texte.
        temperature = doc["main"]["temp"];
        humidite = doc["main"]["humidity"];
        pression = doc["main"]["pressure"];

        para =id.toInt(); //transforme le texte "id" en entier.

}

/*********************** START-parapluie() variables **************************
* Début de la définition des variables globales de la fonction parapluie()    *
******************************************************************************/

int ferme = 0;   // angle pour fermer le parapluie
int ouvre = 170;  // angle pour ouvrir le parapluie

Servo monservo;    // créer un objet "monservo" pour le contrôler

/************************** START-loop() variables ****************************
* Debut de la définition des variables globales de la fonction parapluie()    *
******************************************************************************/
// permet de vérifier le temps écoulé
unsigned long dateDernierChangement = 0;
unsigned long dateCourante;
unsigned long intervalle;
// permet de savoir quel ping envoyer
bool ping = false;
/********************************* START-Setup *********************************
* Code qui doit s'exécuter qu'une fois                                         *
*******************************************************************************/
void setup() {
        Serial.begin(9600);
        delay(100);
        Serial.println("/r/nStart Setup()");

        connexion();

        delay(100);

        server.on("/",handleWebsite);
        server.on("/test",handleTest);
        server.on("/town",handleTown);
        server.on("/apikey",handleAPIKey);
        server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
        server.begin();

        startWebSocket(); // Start a WebSocket server

        prendDonneesMeteo();
        parapluie ();

        town = identifiantVille;
        key = cledAPI;

        Serial.println("End Setup() && Start Loop()");
}

/********************************* START-Loop **********************************
* Code qui doit s'exécuter en continu                                          *
*******************************************************************************/
void loop() {
        dateCourante = millis();
        intervalle = dateCourante - dateDernierChangement; // intervalle de temps depuis la dernière mise à jour du parapluie

        if (intervalle >= (600 * 1000) || identifiantVille != town || cledAPI != key) // Récupère de nouvelles données toutes les 10 minutes ou immediatement si la ville a été changé
        {
                identifiantVille = town;
                cledAPI = key;
                dateDernierChangement = millis();
                prendDonneesMeteo(); // récupère les données météo
                parapluie(); // met à jour le parapluie
        }

        if (test) // si le boutton test à été cliqué
        {
                int parastock = para; // stock la valeur "para" dans "parastock"
                if (!testType)
                {
                        para = 900; // triche sur la valeur "para" pour un test status
                        parapluie(); // met à jour le parapluie
                        Serial.println("parapluie fermé ");
                        delay (100);
                }
                if (testType)
                {
                        para = 100; // triche sur la valeur "para" pour un test status
                        parapluie(); // met à jour le parapluie
                        Serial.println("parapluie ouvert ");
                        delay (100);
                }
                para = parastock; // redonne à para sa valeur initiale
                test = false;
        }

        webSocket.loop();

        server.handleClient();

        if (intervalle%(60*1000)==0) { // toutes les minutes
                int i = webSocket.connectedClients(ping);
                Serial.printf("%d Connected websocket clients ping: %d\n", i, ping);
                ping = !ping;
                ecritMeteoGeneral();
}
}

void ecritMeteoGeneral()
{
        Serial.println("------------------");
        Serial.print(lieu);
        Serial.print(", ");
        Serial.print(pays);
        Serial.print(", ");
        Serial.print(descriptionMeteo);
        Serial.print(", ");
        Serial.println(para);
}

void parapluie ()
{

        if (para<600) {    // Si la valeur de l'indicateur météo est inférieur à 600 c'est qu'il pleut.
                if (status == 0) { // Si avant ça le parapluie était fermé
                        monservo.attach(0); // brancher le servomoteur sur la broche D3 (GPIO 0)
                        monservo.write(ouvre); // ouvre le parapluie
                        Serial.print("ouvre à : "); Serial.println(ouvre);
                        status = 1; // note que le parapluie est maintenant ouvert
                        Serial.print("status à : "); Serial.println(status);
                }
        }
        else {             // si il ne pleut pas
                if (status == 1) { // Si avant ça le parapluie était ouvert.
                        monservo.attach(0); // brancher le servomoteur sur la broche D3 (GPIO 0)
                        monservo.write(ferme); // ferme le parapluie
                        Serial.print("ferme à : "); Serial.println(ferme);
                        status = 0; // note que le parapluie est maintenant fermé
                        Serial.print("status à : "); Serial.println(status);

                }
        }
        delay (200);
        monservo.detach(); // débrancher le servomoteur de la broche D3 (GPIO 0)
        StaticJsonDocument<64> doc; // Create JsonDocument of fixed size
        JsonObject root = doc.to<JsonObject>();  // create JsonObject from JsonDocument
        root["event"] = "state"; // populate JsonObject
        root["state"] = status; // populate JsonObject
        char buffer[64]; // create temp buffer
        serializeJson(root, buffer); // serialize to buffer
        webSocket.broadcastTXT(buffer); // send buffer to web socket
}
