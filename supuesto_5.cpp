// === Supuesto 5: Calculadora estadística genérica ===

#include <iostream>
#include <vector>
#include <cmath>     // trae std::sqrt, equivalente a math.sqrt() de Python
#include <numeric>   // trae std::accumulate, equivalente a sum() de Python

// "template <typename T>" declara que lo que sigue (una función, en este caso)
// no está atado a un tipo concreto: T es un "tipo comodín" que el compilador
// rellenará según con qué tipo lo llames. Es el mecanismo estático que en Python
// no necesitas porque las funciones ya aceptan cualquier tipo por defecto (duck typing).
template <typename T>
T media(const std::vector<T>& datos) {
    // std::accumulate(inicio, fin, valorInicial) sirve para sumar todos los
    // elementos del rango, equivalente a sum(datos) en Python, pero también
    // sirve para otras operaciones de reducción si le pasas una función extra.
    T suma = std::accumulate(datos.begin(), datos.end(), T(0));
    // "T(0)" construye un cero del tipo T (funciona igual para int(0) que double(0)).
    // Dividimos por la cantidad de elementos; ".size()" devuelve std::size_t,
    // por eso lo convertimos a T explícitamente con "static_cast<T>(...)"
    // para que la división se haga en el tipo correcto (evita división entera
    // accidental si T fuera int y el resultado debiera ser decimal, por ejemplo).
    return suma / static_cast<T>(datos.size());
}

template <typename T>
double desviacionEstandar(const std::vector<T>& datos) {
    // Reutilizamos la función media<T> ya definida arriba.
    T m = media(datos);

    double sumaCuadrados = 0.0;
    for (const T& valor : datos) {
        double diferencia = static_cast<double>(valor) - static_cast<double>(m);
        sumaCuadrados += diferencia * diferencia; // equivalente a diferencia ** 2 en Python
    }

    // std::sqrt calcula raíz cuadrada. Siempre devolvemos double aquí porque
    // la desviación estándar rara vez tiene sentido como entero, aunque
    // el dataset original sea de enteros.
    return std::sqrt(sumaCuadrados / datos.size());
}

// Struct pequeño para agrupar un resultado con nombre, evitando devolver
// una tupla sin nombre y tener que recordar "el primero es la media, el
// segundo la desviación" en cada sitio donde se use.
struct ResumenEstadistico {
    double media;
    double desviacion;
};

// Sobrecarga del operador "<<" para que std::cout sepa cómo imprimir
// directamente un ResumenEstadistico con "std::cout << resumen".
// Esto es el equivalente a definir "__str__" o "__repr__" en una clase de Python.
// "std::ostream&" es el tipo del stream de salida (lo que es std::cout);
// se recibe y se devuelve por referencia para poder encadenar "<<" varias veces seguidas.
std::ostream& operator<<(std::ostream& salida, const ResumenEstadistico& r) {
    salida << "media=" << r.media << ", desviacion=" << r.desviacion;
    return salida; // se devuelve el mismo stream para permitir "cout << a << b"
}

// Función genérica que combina las dos anteriores para cualquier tipo T.
template <typename T>
ResumenEstadistico resumir(const std::vector<T>& datos) {
    // "static_cast<double>(...)" convierte explícitamente el resultado de media<T>
    // (que es de tipo T) a double, para que encaje en el struct ResumenEstadistico,
    // que siempre guarda doubles independientemente del tipo original de entrada.
    return ResumenEstadistico{
        static_cast<double>(media(datos)),
        desviacionEstandar(datos)
    };
}

int main() {
    // Dataset de enteros: notas de examen de un grupo de alumnos.
    std::vector<int> notas = {7, 8, 6, 9, 5, 10, 7, 8};

    // Dataset de doubles: rendimientos diarios (%) de una acción, por ejemplo.
    std::vector<double> rendimientos = {1.2, -0.5, 0.8, -1.1, 2.3, 0.1, -0.9};

    // Llamamos a "resumir" con dos tipos distintos SIN escribir dos funciones
    // distintas. El compilador genera internamente dos versiones especializadas
    // (resumir<int> y resumir<double>) a partir de la única definición de arriba;
    // esto se llama "instanciación de template" y ocurre en tiempo de compilación.
    ResumenEstadistico resumenNotas = resumir(notas);
    ResumenEstadistico resumenRendimientos = resumir(rendimientos);

    // Gracias a la sobrecarga de "<<" de más arriba, podemos imprimir el
    // struct directamente, sin tener que acceder campo por campo aquí.
    std::cout << "Notas: " << resumenNotas << "\n";
    std::cout << "Rendimientos: " << resumenRendimientos << "\n";

    return 0;
}