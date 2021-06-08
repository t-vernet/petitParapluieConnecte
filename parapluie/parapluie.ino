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

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

/***************************** START-WebServer ********************************
* Début de la gestion du server web par ESP8266WebServer                     *
******************************************************************************/

ESP8266WebServer server(80);  //déclaration du serveur web sur le port
String webSite,xml,data,town,key; //déclaration de variables
int start = 0; // variable start

void buildWebsite(){ // Fonction qui écrit le code html du site web à partir du fichier html.h
        static const char* html =
  #include "html.h"
        ;
        webSite = String((const char*) html);
}
void buildXML(){
        xml="<?xml version='1.0'?>";
        xml+="<response>";
        xml+=data;
        xml+="</response>";
}

void handleWebsite(){ // génère le site web
        buildWebsite(); // écriture du html
        server.send(200,"text/html",webSite); // mise en ligne du site
}

void handleXML(){ // gère le xml (description de l'état du bouton)
        buildXML();
        server.send(200,"text/xml",xml);
}

void handle1ESPval(){
        start = server.arg("Start").toFloat();
}

void handleTown(){
        town = server.arg("Town");
}

void handleAPIKey(){
        key = server.arg("Key");
}

/********************************* START-WiFi *********************************
* Début de la gestion du wifi par WiFiManager                                *
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
        res = wm.autoConnect("test","testtest"); // password protected ap

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
* Début de la définition des variables globales de la fonction parapluie()   *
******************************************************************************/

int ferme = 90;   // angle pour fermer le parapluie
int ouvre = 170;  // angle pour ouvrir le parapluie
bool pluie = 1;   // enregistre si il pleut ou pas.

Servo monservo;    // créer un objet "monservo" pour le contrôler

/************************** START-loop() variables ****************************
* Debut de la définition des variables globales de la fonction parapluie()   *
******************************************************************************/

int webClic = 0;  // stock l'état du bouton de test pour vérifier si il à changé

// permet de vérifier le temps écoulé
unsigned long dateDernierChangement = 0;
unsigned long dateCourante;
unsigned long intervalle;

void setup() {
        Serial.begin(9600);
        Serial.println();

        connexion();

        delay(100);

        server.on("/",handleWebsite);
        server.on("/xml",handleXML);
        server.on("/set1ESPval",handle1ESPval);
        server.on("/town",handleTown);
        server.on("/apikey",handleAPIKey);
        server.begin();

        prendDonneesMeteo();
        parapluie ();

        town = identifiantVille;
        key = cledAPI;
}

void loop() {
        dateCourante = millis();
        intervalle = dateCourante - dateDernierChangement; // intervalle de temps depuis la dernière mise à jour du parapluie

        if (intervalle >= 600000 || identifiantVille != town || cledAPI != key) // Récupère de nouvelles données toutes les 10 minutes ou immediatement si la ville a été changé
        {
                identifiantVille = town;
                cledAPI = key;
                dateDernierChangement = millis();
                prendDonneesMeteo(); // récupère les données météo
                parapluie(); // met à jour le parapluie
        }

        if (start != webClic) // si l'état du bouton web à changé
        {
                int parastock = para; // stock la valeur "para" dans "parastock"
                if (!start)
                {
                        para = 900; // triche sur la valeur "para" pour un test pluie
                        parapluie(); // met à jour le parapluie
                        Serial.println("parapluie fermé ");
                        delay (200);
                }
                if (start)
                {
                        para = 100; // triche sur la valeur "para" pour un test pluie
                        parapluie(); // met à jour le parapluie
                        Serial.println("parapluie ouvert ");
                        delay (200);
                }
                webClic = start; // met à jour webClic
                para = parastock; // redonne à para sa valeur initiale
        }

        data =(String)start;
        server.handleClient();

        if ((intervalle%6000) == 0) { // toutes les 6 secondes, j'écris ces infos sur le moniteur série
                ecritMeteoGeneral();
                Serial.print("interval modulo 6000 : "); Serial.println((intervalle%60));
                Serial.print("interval : "); Serial.println(intervalle);
                Serial.print("date Courante : "); Serial.println(dateCourante);
                Serial.print("date du Dernier Changement : "); Serial.println(dateDernierChangement);
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
                if (pluie == 0) { // Si avant ça il ne pleuvait pas
                        monservo.attach(0); // brancher le servomoteur sur la broche D3 (GPIO 0)
                        monservo.write(ouvre); // ouvre le parapluie
                        Serial.print("ouvre à : "); Serial.println(ouvre);
                        pluie = 1; // note qu'il pleut
                        Serial.print("pluie à : "); Serial.println(pluie);
                }
        }
        else {             // si il ne pleut pas
                if (pluie == 1) { // et que juste avant il pleuvait (le parapluie était donc ouvert).
                        monservo.attach(0); // brancher le servomoteur sur la broche D3 (GPIO 0)
                        monservo.write(ferme); // ferme le parapluie
                        Serial.print("ferme à : "); Serial.println(ferme);
                        pluie = 0; // note bien qu'il ne pleut pas
                        Serial.print("pluie à : "); Serial.println(pluie);
                }
        }
        delay (200);
        monservo.detach(); // débrancher le servomoteur de la broche D3 (GPIO 0)
}
