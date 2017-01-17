/********************************************************
 * Fichero: simulacion.cc                               *
 *                                                      *
 * Autores: Carlos Ramos León                           *
 *          Manuel Aragón Añino                         *
 *          Juan Miguel García Díaz                     *
 *          Luis Martínez Ruíz                          *
 *                                                      *
 * Fecha: 10/01/2017                                    *
 *                                                      *
 * Descripción: proceso principal de simulación.        *
 ********************************************************/

// Includes del sistema
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <ns3/random-variable-stream.h>
#include <ns3/gnuplot.h>

// Includes del programa

// Definiciones
NS_LOG_COMPONENT_DEFINE ("trabajo07");

void simulacion ();

using namespace ns3;

// Funciones

int main (int argc, char * argv[]){
  Time::SetResolution (Time::US);
  
  // Número de nodos de la red en anillo
  int nodos = 3;

  simulacion();
  
  return 0;
}

void simulacion (int nodos) {
  NS_LOG_FUNCTION_NOARGS();
  // Nodos independintes
  NodeContainer nodos;
  nodos.Create(nodos);
  
  // Creamos los distintos enlaces punto a punto
  NodeContainer p2pNodes[nodos];


  // Instalamos el dispositivo en los nodos punto a punto
  PointToPointHelper pointToPoint[nodos];
  NetDeviceContainer p2pDevices;

  for (int enlace = 0; enlace < nodos; enlace ++){
    pointToPoint[enlace].SetDeviceAttribute ("DataRate",
				     DataRateValue (DataRqqqqqw22wate("2Mbps")));
    pointToPoint[enlace].SetChannelAttribute ("Delay",
				      TimeValue (Time ("2ms")));
    
    p2pNodes[enlace].Add (nodos.Get(enlace));
    
    if (enlace==nodos-1) 
      p2pNodes[enlace].Add (p2pNodes.Get(0));

    else
      p2pNodes[enlace].Add (p2pNodes.Get(enlace+1));
    
    p2pDevices[enlace] = pointToPoint.Install (p2pNodes[enlace]);
  }

  // Instalamos la pila TCP/IP en todos los nodos
  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (csmaNodes);

  
}
