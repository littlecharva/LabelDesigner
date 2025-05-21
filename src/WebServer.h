#include <WiFi.h>
#include <string>
#include <vector>

struct Coord {
  int x;
  int y;
};

struct Shape {
  std::string type;
  std::vector<Coord> coords;
};

class WebServer
{
public:
  WebServer(std::string ssid, std::string password);

  void Setup();
  void Loop();

  IPAddress GetIPAddress();
  bool ClientConnected();

  bool HaveShapeToPrint();
  const char* GetShapeType();  
  size_t GetCoordsCount();
  std::vector<Coord> GetCoords();
  void PrintingComplete();


private:
  void SendResponse(WiFiClient &client, const char *body, const char *contentType);
  void SendNotFoundResponse(WiFiClient &client);

  std::string GetHttpMethod(const std::string& header);  
  std::string GetHttpPath(const std::string& header);
  void ParseShapeData(const std::string& input);

private:
  std::string m_Ssid;
  std::string m_Password;
  WiFiServer m_Server;
  bool m_ClientConnected;
  Shape m_Shape;

};