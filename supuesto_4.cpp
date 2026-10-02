// === Supuesto 4: Sensor de temperatura embebido ===

#include <iostream>
#include <memory>  // trae std::unique_ptr, std::make_unique
#include <vector>

// Clase que representa un buffer de lecturas de un sensor.
// Simulamos "memoria cara/limitada" (típico en embebidos) reservando
// el array de muestras dinámicamente en vez de como array fijo.
class SensorTemperatura {
private:
    // Puntero crudo a un array de doubles reservado en el HEAP (memoria dinámica).
    // A diferencia de una variable normal (que vive en el stack y se destruye
    // sola al salir de scope), esta memoria NO se libera sola: hay que hacerlo
    // explícitamente con "delete[]" o el programa tiene una fuga de memoria (leak).
    double* muestras_;
    int capacidad_;
    int cantidadActual_;

public:
    // Constructor: reserva memoria dinámica con "new[]".
    // "new double[capacidad]" pide al sistema operativo un bloque de memoria
    // para "capacidad" doubles seguidos, y devuelve un puntero al primero.
    explicit SensorTemperatura(int capacidad)
        : capacidad_(capacidad), cantidadActual_(0) {
        // "explicit" delante del constructor evita conversiones implícitas
        // accidentales (por ejemplo, que "SensorTemperatura sensor = 10;"
        // compile creando un sensor de capacidad 10 sin que quede claro
        // en el código que eso es lo que está pasando).
        muestras_ = new double[capacidad_];
        std::cout << "[new] Memoria reservada para " << capacidad_ << " muestras\n";
    }

    // DESTRUCTOR: función especial (nombre de la clase precedido de "~")
    // que el compilador llama AUTOMÁTICAMENTE cuando el objeto muere
    // (sale de scope, o se hace "delete" sobre un puntero a él).
    // Aquí es donde liberamos manualmente lo que reservamos en el constructor.
    // Esto es exactamente el patrón RAII mencionado en la documentación general:
    // "el recurso se libera en el destructor, de forma determinista".
    ~SensorTemperatura() {
        // "delete[]" libera memoria reservada con "new[]" (con corchetes,
        // porque era un array). Usar "delete" a secas sobre memoria de "new[]"
        // es un error clásico de C++ (comportamiento indefinido).
        delete[] muestras_;
        std::cout << "[delete] Memoria liberada\n";
    }

    void agregarMuestra(double temperatura) {
        if (cantidadActual_ < capacidad_) {
            // Acceso a través de puntero con "[]", igual de sintaxis que un array normal.
            muestras_[cantidadActual_] = temperatura;
            cantidadActual_++;
        } else {
            std::cerr << "Buffer del sensor lleno, muestra descartada\n";
        }
    }

    double promedio() const {
        if (cantidadActual_ == 0) return 0.0;
        double suma = 0.0;
        for (int i = 0; i < cantidadActual_; i++) {
            suma += muestras_[i];
        }
        return suma / cantidadActual_;
    }

    // IMPORTANTE — regla de las 3/5: si una clase gestiona memoria manualmente
    // (tiene destructor propio), también debería definir (o prohibir) el
    // constructor de copia y el operador de asignación por copia. Si no lo haces,
    // C++ genera una copia "superficial" por defecto que copiaría el puntero
    // muestras_ (no el array al que apunta), y cuando los dos objetos mueran,
    // ambos destructores intentarían liberar la MISMA memoria -> crash (double free).
    // Para no complicar el ejemplo, prohibimos la copia explícitamente:
    SensorTemperatura(const SensorTemperatura&) = delete;
    SensorTemperatura& operator=(const SensorTemperatura&) = delete;
    // "= delete" le dice al compilador: "esta operación no existe, si alguien
    // intenta copiar un SensorTemperatura, que falle en tiempo de compilación".
    // Esto es justo el tipo de problema que los smart pointers evitan solos,
    // como se ve más abajo.
};

// Ahora la MISMA idea, pero con std::unique_ptr, el estilo recomendado en C++ moderno.
// Ya no necesitamos escribir destructor, ni prohibir la copia a mano: unique_ptr
// ya se comporta correctamente por diseño (no se puede copiar, solo mover).
class SensorTemperaturaModerno {
private:
    // std::unique_ptr<double[]> es un puntero inteligente que gestiona
    // un array reservado dinámicamente, y lo libera automáticamente
    // cuando el propio unique_ptr sale de scope. No hace falta destructor manual.
    std::unique_ptr<double[]> muestras_;
    int capacidad_;
    int cantidadActual_;

public:
    explicit SensorTemperaturaModerno(int capacidad)
        : muestras_(std::make_unique<double[]>(capacidad)), // reserva memoria de forma segura
          capacidad_(capacidad),
          cantidadActual_(0) {
        std::cout << "[unique_ptr] Memoria reservada para " << capacidad_ << " muestras\n";
    }

    // No hay destructor escrito por nosotros: el compilador genera uno
    // que automáticamente destruye muestras_, y como muestras_ es un
    // unique_ptr, SU destructor ya libera el array. Cero riesgo de olvidar un delete.

    void agregarMuestra(double temperatura) {
        if (cantidadActual_ < capacidad_) {
            muestras_[cantidadActual_] = temperatura;
            cantidadActual_++;
        }
    }

    double promedio() const {
        if (cantidadActual_ == 0) return 0.0;
        double suma = 0.0;
        for (int i = 0; i < cantidadActual_; i++) {
            suma += muestras_[i];
        }
        return suma / cantidadActual_;
    }
};

int main() {
    std::cout << "--- Version con memoria manual (new/delete) ---\n";
    {
        // Este bloque "{ }" suelto crea un scope artificial solo para forzar
        // que veas cuándo se destruye el objeto (al llegar a la llave de cierre).
        SensorTemperatura sensorAntiguo(5);
        sensorAntiguo.agregarMuestra(21.5);
        sensorAntiguo.agregarMuestra(22.1);
        sensorAntiguo.agregarMuestra(20.8);
        std::cout << "Promedio: " << sensorAntiguo.promedio() << "\n";
        // Al llegar aqui, cierra el scope -> se llama al destructor automaticamente
        // -> se imprime "[delete] Memoria liberada" SIN que hayamos escrito
        // "delete" explicitamente en este main(). El destructor ya lo hizo.
    }

    std::cout << "\n--- Version moderna (unique_ptr) ---\n";
    {
        SensorTemperaturaModerno sensorModerno(5);
        sensorModerno.agregarMuestra(19.9);
        sensorModerno.agregarMuestra(23.4);
        std::cout << "Promedio: " << sensorModerno.promedio() << "\n";
        // Aqui tambien se libera la memoria automaticamente al cerrar el scope,
        // pero no vemos un mensaje de "[delete]" porque no lo programamos:
        // el propio unique_ptr lo hace internamente sin que tengamos que saberlo.
    }

    return 0;
}