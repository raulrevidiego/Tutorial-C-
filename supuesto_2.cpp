// === Supuesto 2: Analizador de logs de servidor ===

#include <iostream>
#include <fstream>   // trae std::ifstream, para leer archivos (equivalente a open() de Python)
#include <sstream>   // trae std::istringstream, para trocear una línea de texto en partes
#include <string>
#include <unordered_map> // trae std::unordered_map, el equivalente a dict de Python (hash map)
#include <vector>
#include <algorithm> // trae std::sort, equivalente a sorted()/list.sort() de Python

// Estructura simple para representar una línea de log ya parseada.
// "struct" es prácticamente lo mismo que "class" en C++: la única diferencia
// real es que en struct los miembros son públicos por defecto, y en class
// son privados por defecto. Se usa struct por convención para "bolsas de datos"
// sin lógica compleja, y class cuando hay comportamiento encapsulado.
struct EntradaLog {
    std::string ip;
    int codigoEstado; // ej: 200, 404, 500
};

// Función libre (no pertenece a ninguna clase) que intenta parsear una línea.
// Formato esperado de línea: "192.168.1.10 GET /api/users 500"
// "std::optional<EntradaLog>" representa "puede que haya un EntradaLog, o puede
// que no" — es el equivalente tipado a devolver None en Python cuando el parseo falla,
// pero sin usar un puntero ni un valor mágico como -1.
#include <optional>

std::optional<EntradaLog> parsearLinea(const std::string& linea) {
    // std::istringstream permite tratar un std::string como si fuera un archivo
    // del que vas leyendo "palabras" separadas por espacio, con el operador >>.
    std::istringstream stream(linea);

    std::string ip, metodo, ruta, codigoTexto;

    // El operador ">>" extrae tokens separados por espacios en blanco,
    // de forma parecida a línea.split() en Python pero uno a uno.
    // Si alguna extracción falla (línea mal formada), el stream queda en
    // estado de error y las siguientes extracciones también fallan.
    stream >> ip >> metodo >> ruta >> codigoTexto;

    // ".fail()" comprueba si el stream entró en estado de error en algún punto.
    if (stream.fail()) {
        // "return {}" con un std::optional significa "devuelve el estado vacío",
        // equivalente a "return None" en Python.
        return {};
    }

    // Intentamos convertir el código de texto ("500") a entero.
    // Usamos try/catch porque std::stoi lanza una excepción si el texto
    // no es un número válido (equivalente a que int("abc") lance ValueError en Python).
    try {
        int codigo = std::stoi(codigoTexto);
        // Construimos y devolvemos el struct envuelto en el optional.
        return EntradaLog{ip, codigo};
    } catch (const std::exception&) {
        // "catch (const std::exception&)" captura cualquier excepción derivada
        // de std::exception sin necesitar usar la variable (por eso no le damos nombre).
        return {};
    }
}

int main() {
    // Simulamos el contenido de un archivo de log directamente en memoria
    // como si fueran líneas ya leídas, para que el ejemplo sea autocontenido
    // y no dependa de que exista un archivo en disco.
    std::vector<std::string> lineasLog = {
        "192.168.1.10 GET /api/users 200",
        "192.168.1.10 GET /api/orders 500",
        "10.0.0.5 POST /api/login 401",
        "192.168.1.10 GET /api/orders 503",
        "10.0.0.5 GET /api/status 200",
        "10.0.0.5 GET /api/status 502",
        "linea completamente corrupta sin sentido",
    };

    // std::unordered_map<K, V> es un diccionario hash: clave IP (string),
    // valor: contador de errores 5xx (int). Igual que dict[str, int] en Python,
    // pero el tipo va fijado en la declaración.
    std::unordered_map<std::string, int> errores5xxPorIp;

    // Recorremos cada línea del "archivo".
    for (const std::string& linea : lineasLog) {
        // Llamamos a la función que parsea la línea.
        auto resultado = parsearLinea(linea);

        // ".has_value()" comprueba si el optional contiene algo,
        // equivalente a "if resultado is not None:" en Python.
        if (!resultado.has_value()) {
            std::cerr << "Linea ignorada (formato invalido): " << linea << "\n";
            continue; // "continue" funciona exactamente igual que en Python.
        }

        // ".value()" extrae el EntradaLog de dentro del optional.
        EntradaLog entrada = resultado.value();

        // Comprobamos si el código de estado es un error de servidor (5xx).
        if (entrada.codigoEstado >= 500 && entrada.codigoEstado < 600) {
            // Esta línea es el equivalente a:
            //   errores5xxPorIp[entrada.ip] = errores5xxPorIp.get(entrada.ip, 0) + 1
            // en Python. std::unordered_map, si la clave no existe todavía,
            // la crea automáticamente con el valor por defecto de "int" (que es 0)
            // la primera vez que se accede con "[]".
            errores5xxPorIp[entrada.ip]++;
        }
    }

    // Queremos mostrar el resultado ordenado de más errores a menos.
    // std::unordered_map NO garantiza ningún orden al recorrerlo, así que
    // primero volcamos su contenido a un vector de pares para poder ordenarlo.
    // std::pair<std::string, int> es una tupla de 2 elementos con tipos fijos,
    // similar a Tuple[str, int] en Python.
    std::vector<std::pair<std::string, int>> ranking;

    // Recorrer un unordered_map con range-for da pares (clave, valor) en cada vuelta.
    for (const auto& [ip, conteo] : errores5xxPorIp) {
        // La sintaxis "const auto& [ip, conteo]" es "structured bindings" (C++17):
        // desempaqueta el par directamente en dos variables, igual que
        // "for ip, conteo in dict.items():" en Python.
        ranking.push_back({ip, conteo});
    }

    // std::sort ordena un rango [inicio, fin). Le pasamos una lambda como
    // criterio de comparación: "mayor conteo primero".
    // "[](const auto& a, const auto& b) { ... }" es una función anónima
    // (lambda), equivalente a "lambda a, b: ..." pero usada como comparador,
    // más parecido a key=... en sorted() de Python, solo que aquí compara
    // dos elementos directamente en vez de extraer una clave.
    std::sort(ranking.begin(), ranking.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second; // ".second" es el segundo elemento del pair
              });

    std::cout << "Ranking de IPs con mas errores 5xx:\n";
    for (const auto& [ip, conteo] : ranking) {
        std::cout << " - " << ip << ": " << conteo << " errores\n";
    }

    return 0;
}