// === Supuesto 1: Inventario de ferretería ===

// "#include" le dice al PREPROCESADOR (el paso previo a la compilación real)
// que pegue aquí el contenido de esos archivos de cabecera (.h).
// <iostream> trae la funcionalidad de entrada/salida por consola (std::cout, std::cin).
// <vector>   trae std::vector, el contenedor dinámico equivalente a una "list" de Python.
// <string>   trae std::string, el tipo de texto (en C++ puro, un char* no es "texto seguro").
// <iomanip>  trae utilidades para formatear la salida (p.ej. decimales fijos).
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

// "class" define un nuevo tipo. Es el equivalente a "class" en Python,
// pero aquí hay que declarar explícitamente qué es público y qué es privado.
class Producto {
private:
    // Miembros privados: solo accesibles desde dentro de la propia clase.
    // La convención de terminar en "_" (no obligatoria por el lenguaje,
    // solo una convención de estilo) ayuda a distinguir miembros de variables locales.
    std::string nombre_;
    double precioUnitario_;
    int stock_;
    int stockMinimo_;

public:
    // Esto es el CONSTRUCTOR: una función especial que se llama automáticamente
    // al crear un objeto. Tiene el mismo nombre que la clase y no devuelve nada.
    // La sintaxis "nombre_(nombre)" después de los ":" es la "lista de inicialización":
    // inicializa los miembros directamente, en vez de asignarlos dentro del cuerpo.
    // Es la forma preferida en C++ moderno porque es más eficiente para tipos
    // complejos como std::string (evita crear el objeto vacío y luego copiarlo).
    Producto(std::string nombre, double precio, int stock, int stockMinimo)
        : nombre_(nombre), precioUnitario_(precio), stock_(stock), stockMinimo_(stockMinimo) {
        // Cuerpo del constructor vacío: ya hicimos todo en la lista de inicialización.
    }

    // "const" al final de una función miembro significa: "esta función promete
    // no modificar el objeto sobre el que se llama". Es una garantía verificada
    // por el compilador, no un comentario. Si dentro intentas modificar stock_,
    // no compila. Es buena práctica marcar así todo lo que solo lee datos.
    double valorTotal() const {
        // "return" con una expresión: aquí no hay nada raro respecto a Python,
        // salvo que el tipo de retorno (double) ya quedó fijado en la firma de arriba.
        return precioUnitario_ * stock_;
    }

    bool necesitaReposicion() const {
        return stock_ < stockMinimo_;
    }

    // Función miembro que SÍ modifica el objeto (no lleva "const").
    void venderUnidades(int cantidad) {
        // "if" funciona igual que en Python en cuanto a lógica, pero sin ":"
        // y con el cuerpo entre llaves. No hay "elif": se usa "else if".
        if (cantidad > stock_) {
            // std::cerr es como std::cout pero para el canal de errores (stderr),
            // igual que sys.stderr en Python. "\n" es el salto de línea.
            std::cerr << "Error: no hay stock suficiente de " << nombre_ << "\n";
            return; // "return;" sin valor: sale de la función porque es "void".
        }
        stock_ -= cantidad; // equivalente a stock_ = stock_ - cantidad
    }

    // Getters: funciones que solo exponen datos privados de forma controlada.
    // Esto es más estricto que Python, donde normalmente accedes a los atributos
    // directamente (o usas @property si quieres control, pero es opcional).
    std::string nombre() const { return nombre_; }
    int stock() const { return stock_; }
    double precioUnitario() const { return precioUnitario_; }
};

// "int main()" es el PUNTO DE ENTRADA obligatorio de cualquier programa C++.
// El sistema operativo llama a esta función al ejecutar el programa.
// Devuelve un "int": por convención, 0 significa "terminó sin errores".
int main() {
    // std::vector<Producto> es un array dinámico (crece solo) de objetos Producto.
    // El "<Producto>" es un TEMPLATE: le dices al compilador de qué tipo
    // va a ser cada elemento. Es obligatorio especificarlo (a diferencia de
    // una lista de Python, que acepta cualquier tipo mezclado).
    std::vector<Producto> inventario;

    // ".push_back(...)" añade un elemento al final. Equivalente a .append() en Python.
    // Aquí se construye un Producto "al vuelo" pasando los argumentos del constructor.
    inventario.push_back(Producto("Martillo", 12.50, 40, 10));
    inventario.push_back(Producto("Tornillos (caja 100u)", 3.20, 5, 15));
    inventario.push_back(Producto("Taladro", 89.90, 8, 3));
    inventario.push_back(Producto("Cinta metrica", 6.75, 2, 5));

    // Simulamos una venta: sacamos 35 martillos del primer producto (índice 0).
    // Los índices en C++ también empiezan en 0, igual que en Python.
    inventario[0].venderUnidades(35);

    // "double" para acumular dinero; se inicializa en 0.0 explícitamente
    // (en C++ una variable local no inicializada NO vale 0 por defecto,
    // a diferencia de Python donde una variable no existe hasta que se asigna).
    double valorTotalInventario = 0.0;

    // "range-for" (bucle "para cada"), equivalente a "for x in lista" de Python.
    // "const Producto&" significa: "dame una REFERENCIA de solo lectura a cada
    // elemento", para no copiar cada Producto en cada vuelta del bucle
    // (una copia sería más lenta e innecesaria si solo vamos a leer).
    for (const Producto& p : inventario) {
        valorTotalInventario += p.valorTotal();
    }

    // std::fixed y std::setprecision(2) fuerzan a mostrar siempre 2 decimales,
    // igual que f"{valor:.2f}" en Python. Vienen de <iomanip>.
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Valor total del inventario: " << valorTotalInventario << " EUR\n";

    std::cout << "\nProductos que necesitan reposicion:\n";
    for (const Producto& p : inventario) {
        // ".necesitaReposicion()" es una llamada a método, igual que en Python.
        if (p.necesitaReposicion()) {
            std::cout << " - " << p.nombre() << " (stock actual: " << p.stock() << ")\n";
        }
    }

    // Devolvemos 0 al sistema operativo indicando que todo fue bien.
    return 0;
}