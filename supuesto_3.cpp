// === Supuesto 3: Simulador de cartera de inversión ===

#include <iostream>
#include <vector>
#include <stdexcept> // trae std::runtime_error e std::invalid_argument, excepciones estándar
#include <string>

// "enum class" define un tipo enumerado: un conjunto cerrado de valores con nombre.
// Es más seguro que usar strings sueltos como "compra"/"venta" porque el compilador
// verifica que solo uses estos valores exactos; es el equivalente tipado
// a un Enum de Python (from enum import Enum).
enum class TipoOperacion {
    Compra,
    Venta
};

// Struct para representar una operación de mercado.
struct Operacion {
    TipoOperacion tipo;
    double cantidadAcciones;
    double precioPorAccion;
};

// Esta clase modela la cartera y lanza excepciones cuando una operación es inválida,
// en vez de devolver un código de error silencioso — así ningún error pasa desapercibido.
class Cartera {
private:
    double efectivo_;
    double accionesEnCartera_;

public:
    // Constructor con un valor por defecto: si no le pasas accionesIniciales,
    // usa 0.0. Es parecido a "def __init__(self, capital, acciones=0.0)" en Python.
    Cartera(double capitalInicial, double accionesIniciales = 0.0)
        : efectivo_(capitalInicial), accionesEnCartera_(accionesIniciales) {
        // "throw" lanza una excepción inmediatamente, igual que "raise" en Python.
        // Aquí validamos que no nos den un capital negativo al construir la cartera.
        if (capitalInicial < 0) {
            // std::invalid_argument es una excepción estándar para "me diste
            // un argumento que no tiene sentido". El texto entre paréntesis
            // es el mensaje, recuperable luego con .what().
            throw std::invalid_argument("El capital inicial no puede ser negativo");
        }
    }

    void ejecutar(const Operacion& op) {
        // "switch" es el equivalente a comparar op.tipo contra varios valores
        // con una cadena de if/elif, pero más legible cuando hay pocos casos
        // fijos como aquí. No existe match/case en C++ salvo con std::visit
        // para std::variant (mecanismo distinto, más avanzado).
        switch (op.tipo) {
            case TipoOperacion::Compra: {
                double costeTotal = op.cantidadAcciones * op.precioPorAccion;
                if (costeTotal > efectivo_) {
                    // std::runtime_error: excepción estándar genérica para errores
                    // detectados en tiempo de ejecución (no de programación).
                    throw std::runtime_error(
                        "Fondos insuficientes para comprar " +
                        std::to_string(op.cantidadAcciones) + " acciones"
                        // std::to_string convierte un número a std::string,
                        // equivalente a str(numero) en Python. El "+" entre
                        // std::string concatena, igual que en Python.
                    );
                }
                efectivo_ -= costeTotal;
                accionesEnCartera_ += op.cantidadAcciones;
                break; // "break" es obligatorio en cada case, si no, C++ sigue
                       // ejecutando el siguiente case ("fallthrough"). Esto es
                       // distinto a Python, que no tiene switch y no tiene este problema.
            }
            case TipoOperacion::Venta: {
                if (op.cantidadAcciones > accionesEnCartera_) {
                    throw std::runtime_error(
                        "No se pueden vender " + std::to_string(op.cantidadAcciones) +
                        " acciones: solo hay " + std::to_string(accionesEnCartera_)
                    );
                }
                accionesEnCartera_ -= op.cantidadAcciones;
                efectivo_ += op.cantidadAcciones * op.precioPorAccion;
                break;
            }
        }
    }

    double valorTotal(double precioActualAccion) const {
        return efectivo_ + accionesEnCartera_ * precioActualAccion;
    }

    double efectivo() const { return efectivo_; }
    double acciones() const { return accionesEnCartera_; }
};

int main() {
    // Empezamos con 1000 EUR en efectivo y 0 acciones.
    Cartera cartera(1000.0);

    // Lista de operaciones a simular, en orden cronológico.
    std::vector<Operacion> operaciones = {
        {TipoOperacion::Compra, 10, 50.0},  // compra 10 acciones a 50 EUR -> gasta 500
        {TipoOperacion::Compra, 5, 60.0},   // compra 5 acciones a 60 EUR -> gasta 300
        {TipoOperacion::Venta, 8, 65.0},    // vende 8 acciones a 65 EUR -> ingresa 520
        {TipoOperacion::Venta, 100, 70.0},  // ¡ERROR! solo tenemos 7 acciones en este punto
    };

    // Recorremos las operaciones. Usamos índice (en vez de range-for) porque
    // queremos mostrar en qué paso concreto falla si algo va mal.
    for (std::size_t i = 0; i < operaciones.size(); ++i) {
        // "std::size_t" es el tipo estándar sin signo usado para tamaños e índices
        // de contenedores (equivalente aproximado a int, pero nunca negativo).
        // "++i" incrementa i ANTES de usarla en la siguiente vuelta del bucle
        // (aquí, con un for clásico, da igual usar i++ o ++i; la diferencia
        // importa más cuando el resultado del incremento se usa en la misma expresión).
        try {
            cartera.ejecutar(operaciones[i]);
            std::cout << "Operacion " << i << " ejecutada correctamente. "
                      << "Efectivo: " << cartera.efectivo()
                      << ", Acciones: " << cartera.acciones() << "\n";
        } catch (const std::exception& e) {
            // ".what()" devuelve el mensaje de la excepción como const char*,
            // equivalente a str(excepcion) en Python.
            std::cout << "Operacion " << i << " FALLO: " << e.what() << "\n";
            // No hacemos "break": seguimos procesando el resto de operaciones,
            // igual que harías con un try/except dentro de un bucle en Python.
        }
    }

    std::cout << "\nValor final de la cartera (accion a 70 EUR): "
              << cartera.valorTotal(70.0) << " EUR\n";

    return 0;
}