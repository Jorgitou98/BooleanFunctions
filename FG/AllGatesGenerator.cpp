#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <set>
#include <unordered_set>
#include <iterator>
#include <ctime>
#include <mutex>
#include <thread>
#include <map>

unsigned int maxcircuitos;

std::mutex mtxGenerador;
unsigned int circuitosGenerados = 0;
bool pararAlgoritmo = false;
//String con las letras correspondientes a las puertas del repertorio
std::string puertasBinarias = "aonOxXlrLR";
std::ofstream fs;
size_t computadasSobrantes = 0;


#pragma pack (1)
struct circuito {
	circuito* padreIzq;
	circuito* padreDer;
	short tam;
	unsigned int eval;
	char puerta;
	circuito() {}
	circuito(circuito* padreIzq, circuito* padreDer, unsigned int eval, short tam, char puerta) {
		this->padreIzq = padreIzq;
		this->padreDer = padreDer;
		this->eval = eval;
		this->tam = tam;
		this->puerta = puerta;
	}
};


// Mapa de tamaños a circuitos de ese tamaño calculados que deben ser anotados
std::map<short, std::vector<circuito*>> espera;

// Mapa de funciones computadas a vector de circuitos que la computan (de izquierda a derecha creciente de tamaño)
std::unordered_map<unsigned int, std::vector<circuito*>> almacen;


unsigned int evaluacion(circuito* circ1, circuito* circ2, char puerta) {
	// Puerta AND
	if (puerta == 'a') return circ1->eval & circ2->eval;
	// Puerta OR
	else if (puerta == 'o') return circ1->eval | circ2->eval;
	// Puerta NOT
	else if (puerta == 'N') return ~circ1->eval;
	// Puerta NAND
	else if (puerta == 'n') return ~(circ1->eval & circ2->eval);
	// Puerta NOR
	else if (puerta == 'O') return  ~(circ1->eval | circ2->eval);
	// Puerta XOR
	else if (puerta == 'x') return circ1->eval ^ circ2->eval;
	// Puerta XNOR
	else if (puerta == 'X') return  ~(circ1->eval ^ circ2->eval);
	// Puerta LONLY
	else if (puerta == 'l') return  circ1->eval & (~circ2->eval);
	// Puerta RONLY
	else if (puerta == 'r') return  (~circ1->eval) & circ2->eval;
	// Puerta NLONLY
	else if (puerta == 'L') return  ~(circ1->eval & (~circ2->eval));
	// Puerta NRONLY
	else if (puerta == 'R') return  ~((~circ1->eval) & circ2->eval);
}

// Pinta el circuito como árbol binario
void printBT(std::ostream& o, const std::string& prefix, circuito const* circ, bool isRight)
{
	o << prefix;

	o << "|--";

	// Mostramos el cable o la puerta que es
	if (circ->padreIzq == nullptr && circ->padreDer == nullptr) {
		if (circ->eval == 2863311530) o << "X0\n";
		else if (circ->eval == 3435973836) o << "X1\n";
		else if (circ->eval == 4042322160) o << "X2\n";
		else if (circ->eval == 4278255360) o << "X3\n";
		else if (circ->eval == 4294901760) o << "X4\n";
	}
	// Puerta AND
	else if (circ->puerta == 'a') {
		o << "AND\n";
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreDer, true);
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreIzq, false);
	}
	// Puerta OR
	else if (circ->puerta == 'o') {
		o << "OR\n";
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreDer, true);
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreIzq, false);
	}
	// Puerta NOT
	else if (circ->puerta == 'N') {
		o << "NOT\n";
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreDer, true);
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreIzq, false);
	}
	// Puerta NAND
	else if (circ->puerta == 'n') {
		o << "NAND\n";
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreDer, true);
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreIzq, false);
	}
	// Puerta NOR
	else if (circ->puerta == 'O') {
		o << "NOR\n";
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreDer, true);
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreIzq, false);
	}
	// Puerta XOR
	else if (circ->puerta == 'x') {
		o << "XOR\n";
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreDer, true);
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreIzq, false);
	}
	// Puerta XNOR
	else if (circ->puerta == 'X') {
		o << "XNOR\n";
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreDer, true);
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreIzq, false);
	}
	// Puerta LONLY
	else if (circ->puerta == 'l') {
		o << "LONLY\n";
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreDer, true);
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreIzq, false);
	}
	// Puerta RONLY
	else if (circ->puerta == 'r') {
		o << "RONLY\n";
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreDer, true);
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreIzq, false);
	}
	// Puerta NLONLY
	else if (circ->puerta == 'L') {
		o << "NLONLY\n";
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreDer, true);
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreIzq, false);
	}
	// Puerta NRONLY
	else if (circ->puerta == 'R') {
		o << "NRONLY\n";
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreDer, true);
		printBT(o, prefix + (isRight ? "|   " : "    "), circ->padreIzq, false);
	}

}

void bin(std::ostream& o, unsigned int n)
{
	unsigned int num = n;
	for (int i = 0; i < 32; ++i) {
		o << (num & 1);
		num = num >> 1;
	}
}

void muestraCircuitoDetalle(std::ostream& o, circuito* const& circ) {
	o << "Size: " << circ->tam;
	o << "\n";
	printBT(o, "", circ, false);
	o << " Evaluacion: ";
	bin(o, circ->eval);
	o << "\n\n";
}

void preordenRecursivo(std::ostream& o, circuito* const& circ) {
	if (circ->padreIzq == nullptr && circ->padreDer == nullptr) {
		if (circ->eval == 2863311530) o << '0';
		else if (circ->eval == 3435973836) o << '1';
		else if (circ->eval == 4042322160) o << '2';
		else if (circ->eval == 4278255360) o << '3';
		else if (circ->eval == 4294901760) o << '4';
	}
	else if (circ->puerta == 'N') {
		o << circ->puerta;
		preordenRecursivo(o, circ->padreIzq);
	}
	else {
		o << circ->puerta;
		preordenRecursivo(o, circ->padreIzq);
		preordenRecursivo(o, circ->padreDer);
	}
}


std::ostream& operator<<(std::ostream& o, circuito* circ) {
	preordenRecursivo(o, circ);
	o << " " << circ->tam << " " << circ->eval << '\n';
	return o;
}


void introduceEnSet(std::set<circuito*>& set, circuito* circ) {
	if (circ != nullptr) {
		set.insert(circ);
		introduceEnSet(set, circ->padreIzq);
		introduceEnSet(set, circ->padreDer);
	}
}

void mezclar(circuito* circuito1, circuito* circuito2, int& tam) {

	std::set<circuito*> set1, set2;
	introduceEnSet(set1, circuito1);
	introduceEnSet(set2, circuito2);

	std::set<circuito*> intersect;
	std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), std::inserter(intersect, intersect.begin()));

	for (circuito* circ : intersect) {
		if (circ->tam > 0) tam--;
	}

}

// Funcion que totalmente ad-hoc pone los circuitos detamaño 0 (las entradas) con sus correspondientes evaluaciones 
void inicializa() {
	espera[0] = std::vector<circuito*>();

	circuito* circ = new circuito(nullptr, nullptr, 2863311530, 0, 'C');
	espera[0].push_back(circ);

	circ = new circuito(nullptr, nullptr, 3435973836, 0, 'C');
	espera[0].push_back(circ);

	circ = new circuito(nullptr, nullptr, 4042322160, 0, 'C');
	espera[0].push_back(circ);

	circ = new circuito(nullptr, nullptr, 4278255360, 0, 'C');
	espera[0].push_back(circ);

	circ = new circuito(nullptr, nullptr, 4294901760, 0, 'C');
	espera[0].push_back(circ);

}



void tareaGeneraCircuitos(short n, size_t ini, size_t fin) {
	for (char puerta : puertasBinarias) {
		for (size_t i = ini; i < fin; ++i) {
			circuito* circuito1 = espera[n][i];

			// Cruzo el circuito con todos los de su tamaño con los que no se haya cruzado ya
			for (size_t j = i + 1; j < espera[n].size(); ++j) {
				circuito* circuito2 = espera[n][j];
				int tam = circuito1->tam + circuito2->tam + 1;
				mezclar(circuito1, circuito2, tam);
				mtxGenerador.lock();
				if (!espera.count(tam)) espera[tam] = std::vector<circuito*>();
				espera[tam].push_back(new circuito(circuito1, circuito2, evaluacion(circuito1,circuito2, puerta), tam, puerta));
				circuitosGenerados++;
				if (pararAlgoritmo) {
					mtxGenerador.unlock();
					return;
				}
				else if (circuitosGenerados == maxcircuitos) {
					pararAlgoritmo = true;
					mtxGenerador.unlock();
					return;
				}
				else if (circuitosGenerados % 5000000 == 0) {
					std::cout << circuitosGenerados / 1000000 << '\n';
					mtxGenerador.unlock();
				}
				else mtxGenerador.unlock();
			}

			// Cruzo el circuito con todos los de tamaños menores
			for (auto par2 : almacen) {
				for (circuito* circuito2 : par2.second) {
					if (circuito1 == circuito2) break;
					int tam = circuito1->tam + circuito2->tam + 1;
					mezclar(circuito1, circuito2, tam);
					mtxGenerador.lock();
					if (!espera.count(tam)) espera[tam] = std::vector<circuito*>();
					espera[tam].push_back(new circuito(circuito1, circuito2, evaluacion(circuito1, circuito2, puerta), tam, puerta));
					circuitosGenerados++;
					if (pararAlgoritmo) {
						mtxGenerador.unlock();
						return;
					}
					else if (circuitosGenerados == maxcircuitos) {
						pararAlgoritmo = true;
						mtxGenerador.unlock();
						return;
					}
					else if (circuitosGenerados % 5000000 == 0) {
						std::cout << circuitosGenerados / 1000000 << '\n';
						mtxGenerador.unlock();
					}
					else mtxGenerador.unlock();
				}
			}
		}
	}
	// Puerta NOT unaria
	for (circuito* circuito1 : espera[n]) {
		int tam = circuito1->tam + 1;
		mtxGenerador.lock();
		if (!espera.count(tam)) espera[tam] = std::vector<circuito*>();
		espera[tam].push_back(new circuito(circuito1, nullptr, evaluacion(circuito1, nullptr, 'N'), tam, 'N'));
		circuitosGenerados++;
		if (pararAlgoritmo) {
			mtxGenerador.unlock();
			return;
		}
		else if (circuitosGenerados == maxcircuitos) {
			pararAlgoritmo = true;
			mtxGenerador.unlock();
			return;
		}
		else if (circuitosGenerados % 5000000 == 0) {
			std::cout << circuitosGenerados / 1000000 << '\n';
			mtxGenerador.unlock();
		}
		else mtxGenerador.unlock();
	}
}

void generaCircuitos(short n) {
	if (!espera.count(n)) return;
	size_t numProcesadores = std::thread::hardware_concurrency();
	size_t tamProceso = (espera[n].size() / numProcesadores);
	std::vector<std::thread> hilos(numProcesadores);
	for (size_t i = 0; i < numProcesadores; ++i) {
		if (i < numProcesadores - 1) hilos[i] = std::thread(tareaGeneraCircuitos, n, i * tamProceso, (i + 1) * tamProceso);
		else hilos[i] = std::thread(tareaGeneraCircuitos, n, i * tamProceso, espera[n].size());
	}

	for (size_t i = 0; i < numProcesadores; ++i) {
		hilos[i].join();
	}

}

void anotaCircuitos(short n) {
	if (!espera.count(n)) return;
	for (circuito* circ : espera[n]) {
		unsigned int eval = circ->eval;
		if (!almacen.count(eval)) {
			almacen[eval] = std::vector<circuito*>();
			fs << circ;
		}
		almacen[eval].push_back(circ);
	}
	std::cout << "Size " << n << " anotado\n";

}

inline void quitaCircuitosEspera(short n) {
	espera[n].clear();
	espera.erase(n);
}


void tareaAnotaSobrantes(std::vector<circuito*> const& v, int ini, int fin) {
	for (int i = ini; i < fin; ++i) {
		circuito* circ = v[i];
		mtxGenerador.lock();
		if (!almacen.count(circ->eval)) {
			almacen[circ->eval] = std::vector<circuito*>();
			computadasSobrantes++;
			fs << circ;
		}
		mtxGenerador.unlock();
		delete circ;
	}
}


void anotaSobrantes() {
	std::vector<std::pair<int, std::vector<circuito*>>> sobrantes(espera.size());
	std::transform(espera.begin(), espera.end(), sobrantes.begin(), [](auto pair) {return std::move(pair); });
	espera.clear();
	sort(sobrantes.begin(), sobrantes.end(), [](auto pair1, auto pair2) {return pair1.first < pair2.first; });
	for (auto par : sobrantes) {

		int numProcesadores = std::thread::hardware_concurrency();
		int tamProceso = par.second.size() / numProcesadores;
		std::vector<std::thread> hilos(numProcesadores);
		for (int i = 0; i < numProcesadores; ++i) {
			if (i < numProcesadores - 1) hilos[i] = std::thread(tareaAnotaSobrantes, par.second, i * tamProceso, (i + 1) * tamProceso);
			else hilos[i] = std::thread(tareaAnotaSobrantes, par.second, i * tamProceso, par.second.size());
		}
		for (int i = 0; i < numProcesadores; ++i) {
			hilos[i].join();
		}
		std::cout << "Size " << par.first << " anotado\n";
	}
}



inline void muestraSobrantes() {
	for (auto par : espera) {
		std::cout << par.second.size() << " circuitos de tamano " << par.first << " por procesar\n";
	}
}

inline void liberaAlmacen() {
	for (auto par : almacen) {
		for (circuito* circ : par.second) {
			delete circ;
		}
		par.second.clear();
	}
	almacen.clear();
}

int main() {
	std::cout << "Numero de circuitos a generar:\n";
	std::cin >> maxcircuitos;
	unsigned t0, t1;
	t0 = clock();
	std::string nombreFichero;
	std::cout << "Nombre del fichero de salida:\n";
	std::cin >> nombreFichero;
	fs = std::ofstream(nombreFichero);
	inicializa();
	int tamano = 0;
	while (!pararAlgoritmo) {
		generaCircuitos(tamano);
		anotaCircuitos(tamano);
		quitaCircuitosEspera(tamano);
		tamano++;
	}
	std::cout << "Circuitos generados exhaustivamente hasta size " << tamano - 1 << '\n';
	muestraSobrantes();
	std::cout << "\n";
	std::cout << "Procesando... \n";
	anotaSobrantes();
	std::cout << "Total de funciones distintas computadas " << almacen.size() + computadasSobrantes << '\n';
	fs << "Total de funciones distintas computadas: " << almacen.size() + computadasSobrantes << '\n';
	liberaAlmacen();
	t1 = clock();
	double time = (double(t1 - t0) / CLOCKS_PER_SEC);
	fs << "Tiempo de ejecucion: " << time << "\n";
	fs.close();
	return 0;
}