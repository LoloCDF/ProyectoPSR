/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/******************************************************************* 
 * Observador.cc                                                   *
 *                                                                 *
 * Descripción: implementación de la clase Observador.             *
 * Fecha: 15/12/2016                                               *
 * Autores: Manuel Aragón Añino y Juan Miguel García Díaz          *
 *******************************************************************/

// Includes del sistema
#include <ns3/core-module.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include <ns3/application.h>
#include <ns3/simulator.h>
#include <ns3/average.h>
#include <ns3/ethernet-header.h>

// Includes del programa
#include "Observador.h"

// Definiciones
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Observador");

// Implementación del constructor
Observador::Observador ()
{
  NS_LOG_FUNCTION_NOARGS ();
  // Inicializamos las variables
  tEnvio=Time("0ms");
  estRetardo.Reset();
  pqtsTotales=0;
  pqtsCorrectos=0;
}

// Implementación de "envioPqt"
void
Observador::envioPqt(Ptr<const Packet> paquete){
  NS_LOG_FUNCTION_NOARGS ();
  // Debemos descartar los pqts de difusión
  Ptr<Packet> pqtaux = paquete->Copy ();
  EthernetHeader  cabecera;  
  pqtaux->RemoveHeader (cabecera);
  Mac48Address 	destino = cabecera.GetDestination();

  if(!destino.IsBroadcast())
  {
    // Guardamos el instante en el que se envió el pqt mapeado
    mapeo[paquete->GetUid()]=Simulator::Now();
    
    NS_LOG_DEBUG("Nuevo paquete enviado.");
    
    // Aumentamos el número de pqts totales
    pqtsTotales++;
  } else {
    NS_LOG_DEBUG("Trama ARP enviado.");
  }
}

// Implementación de "reciboPqt"
void
Observador::reciboPqt(Ptr<const Packet> paquete){
  NS_LOG_FUNCTION_NOARGS ();
  // Debemos descartar los pqts de difusión
  Ptr<Packet> pqtaux = paquete->Copy ();
  EthernetHeader  cabecera;  
  pqtaux->RemoveHeader (cabecera);
  Mac48Address 	destino = cabecera.GetDestination();

  if(!destino.IsBroadcast())
  {
    std::map<uint64_t, Time>::iterator iterador;

    // Debemos distinguir el origen de los pqts que nos llegan
    if((iterador = mapeo.find(paquete->GetUid()))!=mapeo.end()){
      // Calculamos el retardo de este pqt y lo guardamos en las
      // estadísticas
      NS_LOG_DEBUG("El retardo del paquete con id: " << iterador->first << " es: " << 
                   Simulator::Now().GetMilliSeconds() - mapeo[paquete->GetUid()].GetMilliSeconds() << "ms");
      estRetardo.Update( Simulator::Now().GetMilliSeconds() - mapeo[paquete->GetUid()].GetMilliSeconds());
    
      // Aumentamos el número de pqts correctos
      pqtsCorrectos++;
    
      // Borramos el paquete del mapeo
      mapeo.erase(iterador);
    } else {
      NS_LOG_WARN("El paquete con id: "<< paquete->GetUid()<< " no se puede mapear.");
    }
  } else {
    NS_LOG_DEBUG("Trama ARP recibida.");
  }
}

// Implementación de "getMediaRetardo"
double
Observador::getMediaRetardo(){
  NS_LOG_FUNCTION_NOARGS ();

  return estRetardo.Mean();
}

// Implementación de "getTamanoMapa"
uint64_t
Observador::getTamanoMapa(){
  return mapeo.size();
}

// Implementacion de "getCorrectosPorcentaje"
double
Observador::getCorrectosPorcentaje(){
  return (((double) pqtsCorrectos)/((double) pqtsTotales)) * 100;
}
