#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>

Action ComportamientoJugador::think(Sensores sensores) {
	if(sensores.nivel == 4) {
		// Por si se cambia el destino manualmente en medio de la ejecución del plan
		if(destino.fila != sensores.destinoF || destino.columna != sensores.destinoC)
			hayplan = false;

		if(sensores.terreno[0] == 'K')
			actual.bikini = true;

		if(sensores.terreno[0] == 'D')
			actual.zapatillas = true;

	// Obligar a recalcular si no se tienen las zapatillas o el bikini y va
	// a entrar en agua o bosque
		if(sensores.terreno[2] == 'A' && !actual.bikini
		|| sensores.terreno[2] == 'B' && !actual.zapatillas)
			hayplan = false;

	}
	if(!hayplan) {
		actual.fila = sensores.posF;
		actual.columna = sensores.posC;
		actual.orientacion = sensores.sentido;
		destino.fila = sensores.destinoF;
		destino.columna = sensores.destinoC;
		hayplan = pathFinding(sensores.nivel, actual, destino, plan);
	}

	Action sigAction;

	if(sensores.nivel != 4) {
		if(hayplan && plan.size() > 0) { // Hay un plan no vacío
			sigAction = plan.front(); //tomamos la siguiente accion del hayplan
			plan.erase(plan.begin()); // eliminamos la acción de la lista de acciones
		}
		else {
			//Aquí solo entra cuando no es posible encontrar un comportamiento
			// o está mal implementado el método de búsqueda
		}
	}
	else {
		DescubreMapa(sensores);

		if(hayplan && plan.size() > 0) {
			// Esperar si hay un aldeano justo delante
			if(sensores.superficie[2] == 'a'){
				sigAction = actIDLE;
			}
			else if ((sensores.terreno[2] == 'M' || sensores.terreno[2] == 'P')
			&& (*plan.begin()) == actFORWARD) {
						if(sensores.destinoF > sensores.posF || sensores.destinoC > sensores.posC)
							sigAction = actTURN_R;
						else
							sigAction = actTURN_L;

						hayplan = false;
			}
			else {
				sigAction = plan.front();

				plan.erase(plan.begin());
			}
		}
		else if (plan.empty())
			hayplan = false;
	}

	return sigAction;
}


// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 1: cout << "Busqueda en profundad\n";
			return pathFinding_Profundidad(origen,destino,plan);
			break;
		case 2: cout << "Busqueda en Anchura\n";
			return pathFinding_Anchura(origen,destino,plan);
			break;
		case 3: cout << "Busqueda Costo Uniforme\n";
			return pathFinding_CostoUniforme(origen,destino,plan);
			break;
		case 4: cout << "Busqueda para el reto\n";
			return pathFinding_A_Estrella(origen, destino, plan);
			break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M')
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
		st.fila = fil;
		st.columna = col;

		// Hay que modificar el estado del bikini y las zapatillas antes de comprobar
		// si el estado está en cerrados para que no los tomo por un nodo ya visitado
		if(mapaResultado[fil][col] == 'K'){
			st.bikini = true;
		}

		if(mapaResultado[fil][col] == 'D') {
			st.zapatillas = true;
		}
		return false;
	}
	else{
		return true;
	}
}

void ComportamientoJugador::asignarCosto(nodo &n){
	int fil = n.st.fila;
	int col = n.st.columna;
	char casilla = mapaResultado[fil][col];
	switch (casilla){
		case 'A':
			if(n.st.bikini)
				n.costog += 10;
			else
				n.costog += 100;
			break;
		case 'B':
			if(n.st.zapatillas)
				n.costog += 5;
			else
				n.costog += 50;
			break;
		case 'T':
			n.costog += 2;
			break;
		case 'S':
			n.costog += 1;
			break;
		case 'K':
			n.costog += 1;
			n.st.bikini    = true;
			break;
		case 'D':
			n.costog  += 1;
			n.st.zapatillas = true;
			break;
		case 'X':
			n.costog += 1;
			break;
		case '?':
			n.costog += 3;
			break;
	}
}


// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstadosS> generados; // Lista de Cerrados
	stack<nodo> pila;											// Lista de Abiertos

	nodo current;
	current.st = origen;
	current.secuencia.clear();

	pila.push(current);

	while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

// Búsqueda por anchura
bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan){
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();

	set<estado,ComparaEstadosS> generados; // Lista de Cerrados
	queue<nodo> cola;

	nodo current;
	current.st = origen;
	current.secuencia.empty();

	cola.push(current);

	while (!cola.empty() && (current.st.fila!=destino.fila || current.st.columna != destino.columna)){
		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!cola.empty()){
			current = cola.front();
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila && current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
		cout << "No encontrado plan\n";

	return false;
}

bool MismoEstado (const estado & a, const estado & b) {
	return (a.fila == b.fila && a.columna == b.columna
		&&   a.orientacion == b.orientacion
		&&   a.bikini == b.bikini && a.zapatillas == b.zapatillas);
}

multiset<nodo, ComparaCosto>::const_iterator BuscaNodo(const multiset<nodo, ComparaCosto> & abiertos, const nodo &n){
	for (auto it = abiertos.begin(); it != abiertos.end(); it++) {
		if(MismoEstado((*it).st, n.st))
			return it;
	}

	return abiertos.end();
}

//Costo uniforme
bool ComportamientoJugador::pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan){
	//Borrar la lista
	cout << "Calculando plan" << endl;
	plan.clear();

	set<estado,ComparaEstados> cerrados; // Lista de Cerrados
	multiset<nodo,ComparaCosto> abiertos;

	nodo current;
	current.st = origen;
	current.secuencia.clear();
	current.costog = 0;
	current.costoh = 0;
	current.st.bikini     = false;
	current.st.zapatillas = false;

	abiertos.insert(current);

	while (!abiertos.empty() && (current.st.fila!=destino.fila || current.st.columna != destino.columna)){
		abiertos.erase(abiertos.begin());
		cerrados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1) % 4;
		if (cerrados.find(hijoTurnR.st) == cerrados.end()){
			asignarCosto(hijoTurnR);
			hijoTurnR.secuencia.push_back(actTURN_R);

			auto it = BuscaNodo(abiertos, hijoTurnR);

			if (it == abiertos.end())
				abiertos.insert(hijoTurnR);
			else if (hijoTurnR.costog < (*it).costog) {
				abiertos.erase(it);
				abiertos.insert(hijoTurnR);
			}
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (cerrados.find(hijoTurnL.st) == cerrados.end()){
			asignarCosto(hijoTurnL);
			hijoTurnL.secuencia.push_back(actTURN_L);

			auto it = BuscaNodo(abiertos, hijoTurnL);

			if (it == abiertos.end())
				abiertos.insert(hijoTurnL);
			else if (hijoTurnL.costog < (*it).costog) {
				abiertos.erase(it);
				abiertos.insert(hijoTurnL);
			}
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)
		&& (cerrados.find(hijoForward.st) == cerrados.end())){
			hijoForward.secuencia.push_back(actFORWARD);
			asignarCosto(hijoForward);

			auto it = BuscaNodo(abiertos, hijoForward);

			if (it == abiertos.end()) {
				abiertos.insert(hijoForward);
			}
			else if (hijoForward.costog < (*it).costog) {
				abiertos.erase(it);
				abiertos.insert(hijoForward);
			}
		}

		if (!abiertos.empty()){
			current = *abiertos.begin();
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila && current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		cout << "Coste del plan: " << current.costog << endl;
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
		cout << "No encontrado plan\n";

	return false;
}

void distanciaManhatan (nodo & origen, const estado & destino) {
	origen.costoh = abs(origen.st.fila - destino.fila) + abs(origen.st.columna - destino.columna);
}

//A Estrella
bool ComportamientoJugador::pathFinding_A_Estrella(const estado & origen, const estado & destino, list<Action> &plan) {
	//Borrar la lista
	cout << "Calculando plan" << endl;
	plan.clear();

	set<estado,ComparaEstados> cerrados; // Lista de Cerrados
	multiset<nodo,ComparaCosto> abiertos;

	nodo current;
	current.st = origen;
	current.secuencia.clear();
	current.costog = 0;
	distanciaManhatan(current, destino);
	current.st.bikini     = origen.bikini;
	current.st.zapatillas = origen.zapatillas;

	abiertos.insert(current);

	while (!abiertos.empty() && (current.st.fila!=destino.fila || current.st.columna != destino.columna)){
		abiertos.erase(abiertos.begin());
		cerrados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1) % 4;
		if (cerrados.find(hijoTurnR.st) == cerrados.end()){
			asignarCosto(hijoTurnR);
			distanciaManhatan(hijoTurnR, destino);
			hijoTurnR.secuencia.push_back(actTURN_R);

			auto it = BuscaNodo(abiertos, hijoTurnR);

			if (it == abiertos.end())
				abiertos.insert(hijoTurnR);
			else if (hijoTurnR.costog + hijoTurnR.costoh < (*it).costog + (*it).costoh) {
				abiertos.erase(it);
				abiertos.insert(hijoTurnR);
			}
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (cerrados.find(hijoTurnL.st) == cerrados.end()){
			asignarCosto(hijoTurnL);
			distanciaManhatan(hijoTurnL, destino);
			hijoTurnL.secuencia.push_back(actTURN_L);

			auto it = BuscaNodo(abiertos, hijoTurnL);

			if (it == abiertos.end())
				abiertos.insert(hijoTurnL);
			else if (hijoTurnL.costog + hijoTurnL.costoh < (*it).costog + (*it).costoh) {
				abiertos.erase(it);
				abiertos.insert(hijoTurnL);
			}
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		distanciaManhatan(hijoForward, destino);
		asignarCosto(hijoForward);

		if (!HayObstaculoDelante(hijoForward.st)
		&& (cerrados.find(hijoForward.st) == cerrados.end())){
			hijoForward.secuencia.push_back(actFORWARD);

			auto it = BuscaNodo(abiertos, hijoForward);

			if (it == abiertos.end()) {
				abiertos.insert(hijoForward);
			}
			else if (hijoForward.costog + hijoForward.costoh < (*it).costog + (*it).costoh) {
				abiertos.erase(it);
				abiertos.insert(hijoForward);
			}
		}

		if (!abiertos.empty()){
			current = *abiertos.begin();
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila && current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
		cout << "No encontrado plan\n";

	return false;
}

void ComportamientoJugador::DescubreMapa (Sensores sensores) {
	int k = 0;

	mapaResultado[sensores.posF][sensores.posC] = sensores.terreno[0];

	for (unsigned i=0; i<4; i++) {
		for (unsigned j=0; j<=i*2; j++) {
			switch (sensores.sentido) {
				case norte:
					if (mapaResultado[sensores.posF - i][sensores.posC - i + j] == '?')
						 mapaResultado[sensores.posF - i][sensores.posC - i + j] = sensores.terreno[k];
						k++;
						break;

				case sur:
					if (mapaResultado[sensores.posF + i][sensores.posC + i - j] == '?')
						 mapaResultado[sensores.posF + i][sensores.posC + i - j] = sensores.terreno[k];
					k++;
					break;

				case este:
					if (mapaResultado[sensores.posF - i + j][sensores.posC + i] == '?')
						 mapaResultado[sensores.posF - i + j][sensores.posC + i] = sensores.terreno[k];
					k++;
					break;

				case oeste:
					if (mapaResultado[sensores.posF + i - j][sensores.posC - i] == '?')
						 mapaResultado[sensores.posF + i - j][sensores.posC - i] = sensores.terreno[k];
					k++;
					break;
			}
		}
	}
}

// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}

void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}

// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}

int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
