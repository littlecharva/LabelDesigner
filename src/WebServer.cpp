#include "WebServer.h"
#include <sstream>

const char *html_index=
    #include "HtmlIndex.h"
  ;
const char *html_editor=
    #include "HtmlEditor.h"
  ;
const char *js_label = 
    #include "JsLabel.h"
  ;
const char *js_letter = 
    #include "JsLetter.h"
  ;
const char *json_original = 
    #include "JsonOriginal.h"
  ;
const char *json_curves = 
    #include "JsonCurves.h"
  ;

WebServer::WebServer(std::string ssid, std::string password)
  : m_Ssid{ ssid }
  , m_Password{ password }
  , m_Server{80}
  , m_ClientConnected{ false }
{

}

void WebServer::Setup()
{
    Serial.print("Connecting to ");
    Serial.println(m_Ssid.c_str());
    WiFi.begin(m_Ssid.c_str(), m_Password.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    m_Server.begin();
}

IPAddress WebServer::GetIPAddress()
{
  return WiFi.localIP();
}

bool WebServer::ClientConnected()
{
  return m_ClientConnected;
}

bool WebServer::HaveShapeToPrint() {
  return m_Shape.type != "";
}

const char* WebServer::GetShapeType() {
  return m_Shape.type.c_str();
}

size_t WebServer::GetCoordsCount() {
  return m_Shape.coords.size();
}

std::vector<Coord> WebServer::GetCoords() {
  return m_Shape.coords;
}
void WebServer::PrintingComplete() {
  m_Shape.type = "";
  m_Shape.coords.clear();
}


void WebServer::Loop()
{
  WiFiClient client = m_Server.available();

  if (client)
  {
    std::string header = "";
    std::string body = "";
    bool inBody = false;
    
    int timeout = 5000; // 5 seconds
    while (client.connected() && !client.available())
    {
      if (timeout-- <= 0) {
        client.stop();
        return;
      }
      delay(1);
    }
    m_ClientConnected = true;

    // Read HTTP Request
    while (client.available())
    {
      char c = client.read();

      if (inBody) {
        body += c;
      } else {
        header += c;
        if (header.size() >= 4 && header.substr(header.size() - 4) == "\r\n\r\n") {
          inBody = true;
        }
      }
    }

    std::string method = GetHttpMethod(header);
    std::string path = GetHttpPath(header);


    if (method == "GET")
    {
        if ((path == "/"))
        {
            SendResponse(client, html_index, "text/html");
        }
        else if ((path == "/editor"))
        {
            SendResponse(client, html_editor, "text/html");
        }
        else if(path == "/label-designer.js") 
        {
            SendResponse(client, js_label, "text/javascript");
        }
        else if(path == "/letter-editor.js") 
        {
            SendResponse(client, js_letter, "text/javascript");
        }
        else if(path == "/curves.json") 
        {
            SendResponse(client, json_curves, "application/json");
        }
        else if(path == "/original.json") 
        {
            SendResponse(client, json_original, "application/json");
        }
        else if(path == "/status") 
        {
            if (m_Shape.type != "") {
                SendResponse(client, "printing", "text/plain");
            } else {
                SendResponse(client, "waiting", "text/plain");
            }
        }
        else
        {
            SendNotFoundResponse(client);
        }
    }
    else if (method == "POST")
    {
      if(path == "/print")
      {
        if(m_Shape.type == "") {
          ParseShapeData(body);
          Serial.println("Received shape data:");
          Serial.println(body.c_str());
          SendResponse(client, "done", "text/plain");
        } 
        else {
          Serial.println("Shape data already received.");
          SendResponse(client, "fail", "text/plain");
        }
      }
      else
      {
        SendNotFoundResponse(client);
      }
    }
    else
    {
        SendNotFoundResponse(client);
    }
  }
}

void WebServer::SendResponse(WiFiClient &client, const char *body, const char *contentType)
{
    client.println("HTTP/1.1 200 OK");
    client.print("Content-type: ");
    client.println(contentType);
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close");
    client.println();  
    client.println(body);
}

void WebServer::SendNotFoundResponse(WiFiClient &client)
{
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
}

std::string WebServer::GetHttpMethod(const std::string& header) {
    size_t lineEnd = header.find("\r\n");
    if (lineEnd == std::string::npos) return "";
  
    std::string requestLine = header.substr(0, lineEnd);
    size_t spacePos = requestLine.find(' ');
    if (spacePos == std::string::npos) return "";
  
    return requestLine.substr(0, spacePos);
  }
  
  std::string WebServer::GetHttpPath(const std::string& header) {
    size_t lineEnd = header.find("\r\n");
    if (lineEnd == std::string::npos) return "";
  
    std::string requestLine = header.substr(0, lineEnd);
    size_t firstSpace = requestLine.find(' ');
    size_t secondSpace = requestLine.find(' ', firstSpace + 1);
  
    if (firstSpace == std::string::npos || secondSpace == std::string::npos) return "";
  
    return requestLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
  }

  void WebServer::ParseShapeData(const std::string& input) {
    Shape shape;
    std::stringstream ss(input);
    std::string item;
    std::vector<int> values;
  
    // Get the command
    if (std::getline(ss, item, ',')) {
      shape.type = item;
    }
  
    // Get the remaining numeric values
    while (std::getline(ss, item, ',')) {
      values.push_back(std::stoi(item));
    }
  
    // Group into coordinate pairs
    for (size_t i = 0; i + 1 < values.size(); i += 2) {
      shape.coords.push_back({ values[i], values[i + 1] });
    }

    m_Shape = shape;
  }