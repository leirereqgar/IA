#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>

struct estado {
   int fila;
   int columna;
   int orientacion;
};

struct nodo{
	estado st;
	list<Action> secuencia;
	int costo;
};

struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};

struct ComparaCosto{
	bool operator()(const nodo &a, const nodo &b) const {
		bool menor = false;

		if (a.costo <= b.costo)
			menor = true;

		return menor;
	}
};

class ComportamientoJugador : public Comportamiento {
   public:
      ComportamientoJugador(unsigned int size) : Comportamiento(size) {
         // Inicializar Variables de Estado
         fil = col = 99;
         brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
         destino.fila = -1;
         destino.columna = -1;
         destino.orientacion = -1;
         hayplan=false;
      }

      ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
         // Inicializar Variables de Estado
         fil = col = 99;
         brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
         destino.fila = -1;
         destino.columna = -1;
         destino.orientacion = -1;
         hayplan=false;
      }

      ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}

      ~ComportamientoJugador(){}

      Action think(Sensores sensores);
      int interact(Action accion, int valor);
      void VisualizaPlan(const estado &st, const list<Action> &plan);
      ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

   private:
      // Declarar Variables de Estado
      int fil, col, brujula;
      estado actual, destino;
      list<Action> plan;
      bool hayplan;
      bool bikini, zapatillas;

      // Métodos privados de la clase
      void asignarCosto(nodo &nodo);
      bool pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan);
      bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
      bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
      bool pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan);

      void PintaPlan(list<Action> plan);
      bool HayObstaculoDelante(estado &st);
};

#endif
