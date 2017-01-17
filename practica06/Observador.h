/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/******************************************************************* 
 * Observador.h                                                    *
 *                                                                 *
 * Descripción: definición de la clase Observador, que se dedica   *
 * a recoger datos y generar las estadísticas de la simulación.    *
 * Fecha: 15/12/2016                                               *
 * Autores: Manuel Aragón Añino y Juan Miguel García Díaz          *
 *******************************************************************/

// Includes del sistema
#include <ns3/core-module.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include <ns3/application.h>
#include <ns3/simulator.h>
#include <ns3/data-rate.h>
#include <ns3/average.h>

// Includes del programa
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/application.h"
#include "ns3/error-model.h"

// Definiciones
using namespace ns3;

class Observador : public Application
{
public:
  /*
    ===================================================================================
    simulacion: constructor de la clase Observador que inicializa todos los parámetros.
    RECIBE:
    DEVUELVE:
    ===================================================================================
  */
  Observador  ();
  
  /*
    ===================================================================================
    envioPqt: captura la traza "MacTx" para contabilizar el tiempo en el que se ha
    enviado un pqt.
    RECIBE:
    ->paquete: paquete enviado.
    DEVUELVE:
    ===================================================================================
  */
  void envioPqt (Ptr<const Packet> paquete);
  
  /*
    ===================================================================================
    reciboPqt: captura la traza "MacRx" para contabilizar el tiempo en el que se ha
    recibido un pqt. Aprovechamos, para calcular el retardo del pqt y añadirlo a las
    estadísticas del cliente.
    RECIBE:
    ->paquete: paquete enviado.
    DEVUELVE:
    ===================================================================================
  */
  void reciboPqt (Ptr<const Packet> paquete);

  /*
    ===================================================================================
    getMediaRetardo: gracias a las estadísticas recolectadas, obtenemos el valor medio
    del retardo de pqts en este cliente.
    RECIBE:
    DEVUELVE:
    <- Un "double" que contiene el valor medio del retardo de pqts de este cliente.
    ===================================================================================
  */
  double getMediaRetardo();

  /*
    ===================================================================================
    getTamanoMapa: devuelve el tamaño del mapa paquetes-tiempo de envío.
    RECIBE:
    DEVUELVE:
    <- devuelve el tamaño del mapa paquetes-tiempo de envío.
    ===================================================================================
  */
  uint64_t getTamanoMapa();
  
  /*
    ===================================================================================
    getCorrectosPorcentaje: devuelve el porcentaje de pqts correctos.
    RECIBE:
    DEVUELVE:
    <- el porcentaje de pqts correctos.
    ===================================================================================
  */
  double getCorrectosPorcentaje();

private:
  // Guardamos el tiempo en el que se mandó un paquete
  Time tEnvio;

  // Obtenemos estadísticas acerca del retardo de los pqts de este cliente
  Average<double> estRetardo;

  // Para mapear los pqts y poder distinguirlos
  std::map<uint64_t,Time> mapeo;

  // Para guardar los pqts totales
  uint64_t pqtsTotales;
  
  // Para guardar los pqts correctos
  uint64_t pqtsCorrectos;
};

