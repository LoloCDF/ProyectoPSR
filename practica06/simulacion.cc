/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/******************************************************************* 
 * simulacion.cc                                                   *
 *                                                                 *
 * Descripción: función principal encargada de montar el escenario *
 * y simular. Genera dos gráficas:                                 *         
 *  - Porcentaje de paquetes correctamente transmitidos, 		   *
 *	  práctica06-01.plt.                 						   *
 *  - Retardo medio de los paquetes, práctica06-02.plt.            *
 * Fecha: 15/12/2016                                               *
 * Autores: Manuel Aragón Añino y Juan Miguel García Díaz          *
 *******************************************************************/

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
#include "Observador.h" 

// Definiciones de constantes
// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0

#define NSIMULACIONES 10	// numero de simulaciones
#define NVALORES 5			
#define NCURVAS 5
#define PASO 0.05

#define DNI_0 9
#define DNI_1 8

#define IC 0.95				
#define T_STUDENT 2.2622


using namespace ns3;

// Definiciones de funciones
NS_LOG_COMPONENT_DEFINE ("Practica06");

void simulacion(uint32_t nCsma,Ptr<ExponentialRandomVariable> ton,
                Ptr<ExponentialRandomVariable> toff, DataRate dataRate,
                uint32_t sizePkt, uint32_t tamcola, 
                Observador * observaClientes);

int 
main (int argc, char *argv[])
{
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
  Time::SetResolution (Time::US);

  // Parámetros de la simulación 
  
  // número de nodos
  uint32_t nCsma = 150 + 5*DNI_0;
  Ptr<ExponentialRandomVariable> ton = CreateObject<ExponentialRandomVariable>();
  Ptr<ExponentialRandomVariable> toff = CreateObject<ExponentialRandomVariable>();
  
  // valor inicial del tiempo medio de permanencia en el estado On
  double vton  = 0.15;
  // tiempo medio de permanencia en el estado Off
  double vtoff = 0.65;
  // tamaño del paquete
  uint32_t sizePkt = 40 - DNI_1/2;
  // tasa de bit en el estado activo
  DataRate dataRate = DataRate ("64kbps");

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Dispositivos CSMA adicionales", nCsma);
  cmd.AddValue ("ton", "Tiempo de actividad de los dispositivos.", vton);
  cmd.AddValue ("toff", "Tiempo de silencio de los dispositivos.", vtoff);
  cmd.AddValue ("dataRate", "Tasa de bit del estado activo.", dataRate);
  cmd.AddValue ("sizePkt", "Tamaño de los paquetes enviados en el estado activo.", sizePkt);
  cmd.Parse (argc,argv);

  nCsma = nCsma == 0 ? 1 : nCsma;
  ton->SetAttribute("Mean",DoubleValue(vton));
  toff->SetAttribute("Mean",DoubleValue(vtoff));

  /*************** AHORA PREPARAMOS LAS GRÁFICAS ***************/
  // Configuramos las gráficas
  Gnuplot practica06_1 ("practica06-1");
  Gnuplot practica06_2 ("practica06-2");
  practica06_1.SetTitle ("Porcentaje de pqts correctamente transmitidos variando Ton");
  practica06_2.SetTitle ("Retardo medio de los pqts");
  
  // Etiquetamos los ejes
  practica06_1.SetLegend ("T. medio de permanencia en el estado ON", "% de pqts correctos");
  practica06_2.SetLegend ("T. medio de permanencia en el estado ON", "Retardo medio de los pqts (ms)");

  // Establecemos el rango
  practica06_1.AppendExtra("set yrange [0:+100]");

  // Para establecer títulos
  std::ostringstream rotulo;
  
  /*************************************************************/

  // Obtenemos distintas curvas variando el tam. máximo de cola
  for(uint32_t curva=0; curva<NCURVAS; curva++){
    uint32_t tamcola = curva+1;
    NS_LOG_DEBUG("////////////// CURVA CON TAM. COLA "<<
                 tamcola << " //////////////");

    // Creamos los dataset para generar las curvas
    rotulo<<"Con tam. cola="<<tamcola;
    Gnuplot2dDataset dP06_1 (rotulo.str());
    rotulo.str("");
    rotulo<<"Con tam. cola="<<tamcola;
    Gnuplot2dDataset dP06_2 (rotulo.str());
    rotulo.str("");
    
    dP06_1.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    dP06_1.SetErrorBars(Gnuplot2dDataset::Y);
    dP06_2.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    dP06_2.SetErrorBars(Gnuplot2dDataset::Y);

    // Realizamos varias simulaciones variando el Ton
    for(uint32_t valor=0; valor<NVALORES; valor++){
    	// Incrementamos el Ton
      double tonActual=vton+valor*PASO;
      ton->SetAttribute("Mean",DoubleValue(tonActual));
      NS_LOG_DEBUG("============= SIMULANDO CON TON "<< 
                   tonActual << " ============="); 
   
      // Variables para guardar las estadísticas
      Average<double> estRetardo;
      Average<double> estCorrectos;

      // Realizamos varías simulaciones
      for (uint32_t numSimula=0; numSimula < NSIMULACIONES; numSimula++) {
        // Creamos los observadores, estadísticas de las simulaciones y simulamos
        Observador observaClientes;
        simulacion (nCsma,ton,toff,dataRate,sizePkt,tamcola,&observaClientes);
        estRetardo.Update(observaClientes.getMediaRetardo());
        estCorrectos.Update(observaClientes.getCorrectosPorcentaje());
        NS_LOG_DEBUG("Simulación " << numSimula+1 << " de " << NSIMULACIONES);

        // Si el mapa no está vacío lo advertimos
        if(observaClientes.getTamanoMapa()>0)
          NS_LOG_WARN("El mapa no se ha vaciado, hay " << observaClientes.getTamanoMapa()
                      << " elementos en él.");
      }
      // Imprimimos información
      NS_LOG_INFO("Porcentaje de pqts correctos: " << estCorrectos.Mean() << "%");
      NS_LOG_INFO("Retardo general de los clientes: " << estRetardo.Mean() << "ms");
      
      // Datos para la gráfica 01
      double s = estCorrectos.Var();
      double z = T_STUDENT*sqrt(s/NSIMULACIONES);
      dP06_1.Add(tonActual,estCorrectos.Mean(),z);

      // Datos para la gráfica 02
      s = estRetardo.Var();
      z = T_STUDENT*sqrt(s/NSIMULACIONES);
      dP06_2.Add(tonActual,estRetardo.Mean(),z);

      // Reseteamos acumuladores
      estCorrectos.Reset();
      estRetardo.Reset();

    }
    // Guardamos la curva
    practica06_1.AddDataset(dP06_1);
    practica06_2.AddDataset(dP06_2);
  }

  // Escribimos el archivo de gráficas
  // Abrimos los archivos de las gráficas
  for(uint32_t ngrafi=0;ngrafi<2;ngrafi++){
    std::ostringstream cadena;
    cadena << "practica06-"<<ngrafi+1<<".plt";
    std::ofstream archivoPlot (cadena.str().c_str());
    cadena.str("");
    // Escribimos...
    if(ngrafi==0)
      practica06_1.GenerateOutput (archivoPlot);
    else if(ngrafi==1)
      practica06_2.GenerateOutput (archivoPlot);
    
    archivoPlot << "pause -1" << std::endl;
    
    // Y por último cerramos el archivo
    archivoPlot.close ();
  }

  return 0;
}

void simulacion (uint32_t nCsma,Ptr<ExponentialRandomVariable> ton,Ptr<ExponentialRandomVariable> toff,
                 DataRate dataRate,uint32_t sizePkt,uint32_t tamcola, Observador * observaClientes) {
  NS_LOG_FUNCTION_NOARGS ();

  // Nodos que pertenecen al enlace punto a punto
  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  // Nodos que pertenecen a la red de área local
  // Como primer nodo añadimos el encaminador que proporciona acceso
  //      al enlace punto a punto.
  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

  // Instalamos el dispositivo en los nodos punto a punto
  PointToPointHelper pointToPoint;
  NetDeviceContainer p2pDevices;
  pointToPoint.SetDeviceAttribute ("DataRate",
                                   DataRateValue (DataRate("2Mbps")));
  pointToPoint.SetChannelAttribute ("Delay",
                                    TimeValue (Time ("2ms")));
  p2pDevices = pointToPoint.Install (p2pNodes);

  // Instalamos el dispositivo de red en los nodos de la LAN
  CsmaHelper csma;
  NetDeviceContainer csmaDevices;
  csma.SetChannelAttribute ("DataRate",
                            DataRateValue (DataRate ("100Mbps")));
  csma.SetChannelAttribute ("Delay",
                            TimeValue (Time ("6560ns")));
  csmaDevices = csma.Install (csmaNodes);

  // Instalamos la pila TCP/IP en todos los nodos
  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (csmaNodes);

  // Asignamos direcciones a cada una de las interfaces
  // Utilizamos dos rangos de direcciones diferentes:
  //    - un rango para los dos nodos del enlace
  //      punto a punto
  //    - un rango para los nodos de la red de área local.
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer p2pInterfaces;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  p2pInterfaces = address.Assign (p2pDevices);
  Ipv4InterfaceContainer csmaInterfaces;
  address.SetBase ("10.1.2.0", "255.255.255.0");
  csmaInterfaces = address.Assign (csmaDevices);

  // Calculamos las rutas del escenario. Con este comando, los
  //     nodos de la red de área local definen que para acceder
  //     al nodo del otro extremo del enlace punto a punto deben
  //     utilizar el primer nodo como ruta por defecto.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  
  // Establecemos un sumidero para los paquetes en el puerto 9 del nodo
  //     aislado del enlace punto a punto
  uint16_t port = 9;
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (),
                                                     port)));
  ApplicationContainer app = sink.Install (p2pNodes.Get (0));

  // Instalamos un cliente OnOff en uno de los equipos de la red de área local
  OnOffHelper clientes ("ns3::UdpSocketFactory",
                        Address (InetSocketAddress (p2pInterfaces.GetAddress (0),
                                                    port)));
  // Asignamos el ton
  clientes.SetAttribute("OnTime",PointerValue(ton));
  // Asignamos el toff
  clientes.SetAttribute("OffTime",PointerValue(toff));
  // Asignamos el tamaño de paquetes
  clientes.SetAttribute("PacketSize",UintegerValue(sizePkt));
  // Asignamos la tasa de transmisión en estado activo Ton
  clientes.SetAttribute("DataRate",DataRateValue(dataRate));

  ApplicationContainer clientApps = clientes.Install (csmaNodes);

  clientApps.Start (Time ("2s"));
  clientApps.Stop (Time ("10s"));

  // Suscribimos las trazas que nos convienen a "Observador"
  p2pDevices.Get(1)->TraceConnectWithoutContext("MacRx",MakeCallback(&Observador::reciboPqt,observaClientes));
  // Cambiamos el tamaño de la cola
  p2pDevices.Get(1)->GetObject<PointToPointNetDevice>()->GetQueue()->GetObject<DropTailQueue>()->
    SetAttribute("MaxPackets",UintegerValue(tamcola));

  p2pDevices.Get(0)->TraceConnectWithoutContext("MacRx",MakeCallback(&Observador::reciboPqt,observaClientes));

  for(uint32_t cliente=0; cliente<nCsma; cliente++){
      csmaDevices.Get(cliente)->TraceConnectWithoutContext("MacTx",MakeCallback(&Observador::envioPqt,observaClientes));    
  }
  
  // Activamos las trazas pcap
  pointToPoint.EnablePcap ("practica06", p2pDevices.Get (1));
  csma.EnablePcap ("practica06", csmaDevices.Get (0), true);

  // Lanzamos la simulación
  Simulator::Run ();
  Simulator::Destroy ();
}
