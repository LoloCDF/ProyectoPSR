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

void simulacion (uint32_t num_nodos);

using namespace ns3;

// Funciones

int main (int argc, char * argv[]){
  Time::SetResolution (Time::US);
  
  // Número de nodos de la red en anillo
  uint32_t num_nodos = 10;

  simulacion(num_nodos);
  
  return 0;
}

void simulacion (uint32_t num_nodos) {
  NS_LOG_FUNCTION_NOARGS();
  
  // Cremaos los nodos que van a formar la red
  Ptr<Node> nodos[num_nodos];
  

  // Creamos un contenedor con todos los nodos para instalar las aplicaciones
  NodeContainer todosNodos;

  for (uint32_t nodo = 0; nodo < num_nodos; nodo++){
    nodos[nodo]=CreateObject <Node> ();
    todosNodos.Add(nodos[nodo]);
  }


  // Creamos los distintos enlaces punto a punto
  NodeContainer p2pNodes[num_nodos];

  // Puerto de las aplicaciones
  uint16_t port = 9;

  // Instalamos el dispositivo en los nodos punto a punto
  PointToPointHelper pointToPoint[num_nodos];
  NetDeviceContainer p2pDevices[num_nodos];
  Ipv4InterfaceContainer p2pInterfaces[num_nodos];
  ApplicationContainer clientApps;

  // Instalamos los clientes con destinos aleatorios
  Ptr<UniformRandomVariable> enlaceAleat = CreateObjectWithAttributes<UniformRandomVariable> ("Min",DoubleValue(0),"Max",DoubleValue(num_nodos));
  Ptr<UniformRandomVariable> interfazAleat = CreateObjectWithAttributes<UniformRandomVariable> ("Min",DoubleValue(0),"Max",DoubleValue(1));

  // Configuramos los nodos
  for (uint32_t enlace = 0; enlace < num_nodos; enlace ++){
    pointToPoint[enlace].SetDeviceAttribute ("DataRate",
					     DataRateValue (DataRate("2Mbps")));
    pointToPoint[enlace].SetChannelAttribute ("Delay",
					      TimeValue (Time ("2ms")));
    
    // Añadimos el nodo
    p2pNodes[enlace].Add (nodos[enlace]);
    
    // Si es el último nodo, lo enlazamos con el primero
    if (enlace==num_nodos-1) 
      p2pNodes[enlace].Add (nodos[0]);

    else
      p2pNodes[enlace].Add (nodos[enlace+1]);
    
    // Instalamos las interfaces
    p2pDevices[enlace] = pointToPoint[enlace].Install (p2pNodes[enlace]);

  }
         
  // Instalamos la pila TCP/IP en todos los nodos
  InternetStackHelper stack;
  stack.Install(todosNodos);
  
  // Rellenamos los destinos
  uint32_t destinos[num_nodos];
  uint32_t valida = 0;

  // Inicializamos la tabla de destinos aleatorios
  for (uint32_t indice = 0; indice < num_nodos; indice++){
    destinos[indice] = 123456; 
  }
  
  // Rellenamos la tabla de destinos aleatorios
  for (uint32_t indice = 0; indice < num_nodos; indice++){

    while (valida==0){
      uint32_t value = (uint32_t)enlaceAleat->GetValue();
      valida=1;

      for (uint32_t indice2 = 0; indice2 < num_nodos; indice2++){
	if (destinos[indice2]==value)
	  valida=0;
      }

      if (valida==1)
	destinos[indice]=value;

    }

    valida=0;
  }
  
  for (uint32_t enlace = 0; enlace < num_nodos; enlace ++){    
    // Asignamos direcciones a cada una de las interfaces
    Ipv4AddressHelper address;
    std::ostringstream cadena;
    cadena << "10.0." << enlace << ".0";
    address.SetBase (cadena.str().c_str(), "255.255.255.0");
    cadena.str("");

    p2pInterfaces[enlace] = address.Assign (p2pDevices[enlace]);

    // Instalamos un cliente OnOff en la interfaz del nodo en esta red
    OnOffHelper cliente ("ns3::UdpSocketFactory",
			 Address (InetSocketAddress (p2pInterfaces[enlace].GetAddress (interfazAleat->GetInteger()),
						      port)));

    clientApps.Add(cliente.Install (p2pNodes[destinos[enlace]].Get(0)));
    
    // Activamos las trazas pcap
    pointToPoint[enlace].EnablePcap ("practica06", p2pDevices[enlace].Get (0));
    pointToPoint[enlace].EnablePcap ("practica06", p2pDevices[enlace].Get (1));
  }
  
  // Calculamos las rutas del escenario. Con este comando, los
  //     nodos de la red de área local definen que para acceder
  //     al nodo del otro extremo del enlace punto a punto deben
  //     utilizar el primer nodo como ruta por defecto.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  
  // Establecemos un sumidero para los paquetes en el puerto 9 del nodo
  //     aislado del enlace punto a punto
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
			 Address (InetSocketAddress (Ipv4Address::GetAny (),
						     port)));
  ApplicationContainer app = sink.Install (todosNodos);    

 
  clientApps.Start (Time ("2s"));
  clientApps.Stop (Time ("10s"));

  // Lanzamos la simulación
  Simulator::Run ();
  Simulator::Destroy ();
    
}
