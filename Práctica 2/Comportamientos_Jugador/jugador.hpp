#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>

struct estado {
	int fila;
	int columna;
	int orientacion;
	bool bikini;
	bool zapatillas;
};

struct nodo{
	estado st;
	list<Action> secuencia;
	int costog;
	int costoh;
};

struct ComparaEstados {
	bool operator () (const estado & a, const estado & n) const {
		if (((a.fila > n.fila)
		||   (a.fila == n.fila && a.columna > n.columna)
		||   (a.fila == n.fila && a.columna == n.columna && a.orientacion > n.orientacion)
		||   (a.fila == n.fila && a.columna == n.columna && a.orientacion == n.orientacion && a.bikini != n.bikini )
		||   (a.fila == n.fila && a.columna == n.columna && a.orientacion == n.orientacion && a.bikini == n.bikini && a.zapatillas != n.zapatillas ))
		) {
			return true;
		}
		else {
			return false;
		}
	}
};


struct ComparaEstadosS {
	bool operator () (const estado & a, const estado & n) const {
		if ((a.fila > n.fila)
		||  (a.fila == n.fila && a.columna > n.columna)
		||  (a.fila == n.fila && a.columna == n.columna && a.orientacion > n.orientacion)
		) {
			return true;
		}
		else {
			return false;
		}
	}
};

struct ComparaCosto{
	bool operator()(const nodo &a, const nodo &b) const {
		return (a.costog + a.costoh <= b.costog + b.costoh);
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

	// Métodos privados de la clase
	void asignarCosto(nodo &nodo);
	bool pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan);
	bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
	bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
	bool pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan);
	bool pathFinding_A_Estrella(const estado & origen, const estado & destino, list<Action> &plan);

	void DescubreMapa (Sensores sensores);

	void PintaPlan(list<Action> plan);
	bool HayObstaculoDelante(estado &st);
};

#endif
