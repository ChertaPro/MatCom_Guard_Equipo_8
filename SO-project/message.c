#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Estructuras para almacenar mensajes
typedef struct {
    char message[256];
    int is_alert;
} ConsoleMessage;

typedef struct {
    ConsoleMessage* messages;
    int count;
    int capacity;
    pthread_mutex_t mutex;
} MessageStore;

// Almacén global de mensajes (inicializar en main)
MessageStore message_store = {0};

// Función para agregar mensajes al almacén
void add_message(const char* msg, int is_alert) {
    pthread_mutex_lock(&message_store.mutex);
    
    // Redimensionar si es necesario
    if (message_store.count >= message_store.capacity) {
        int new_capacity = message_store.capacity == 0 ? 10 : message_store.capacity * 2;
        ConsoleMessage* new_messages = realloc(message_store.messages, new_capacity * sizeof(ConsoleMessage));
        if (!new_messages) {
            pthread_mutex_unlock(&message_store.mutex);
            return;
        }
        message_store.messages = new_messages;
        message_store.capacity = new_capacity;
    }
    
    // Agregar nuevo mensaje
    strncpy(message_store.messages[message_store.count].message, msg, 255);
    message_store.messages[message_store.count].message[255] = '\0';
    message_store.messages[message_store.count].is_alert = is_alert;
    message_store.count++;
    
    pthread_mutex_unlock(&message_store.mutex);
}

// Función para obtener mensajes (filtrados por tipo)
char** get_messages(int* count, int get_alerts) {
    pthread_mutex_lock(&message_store.mutex);
    
    // Contar mensajes relevantes
    int relevant_count = 0;
    for (int i = 0; i < message_store.count; i++) {
        if (get_alerts == message_store.messages[i].is_alert) {
            relevant_count++;
        }
    }
    
    // Crear array de resultados
    char** result = malloc(relevant_count * sizeof(char*));
    int index = 0;
    for (int i = 0; i < message_store.count; i++) {
        if (get_alerts == message_store.messages[i].is_alert) {
            result[index] = strdup(message_store.messages[i].message);
            index++;
        }
    }
    
    *count = relevant_count;
    pthread_mutex_unlock(&message_store.mutex);
    return result;
}

// Ejemplo de cómo modificar tus funciones existentes
void verificar_puerto(int port) {
    // Lógica existente...
    
    // En lugar de printf:
    char buffer[256];
    if (port > 1024 /* && condición de alerta */) {
        snprintf(buffer, sizeof(buffer), "[!] Alerta: Servicio en puerto no estándar %d", port);
        add_message(buffer, 1);  // 1 = alerta
    } else {
        snprintf(buffer, sizeof(buffer), "Puerto %d verificado", port);
        add_message(buffer, 0);  // 0 = mensaje normal
    }
}

// Función para escanear puertos (ejemplo)
void scan_port(int port) {
    // Simulamos diferentes resultados
    char buffer[256];
    
    if (port % 3 == 0) {
        snprintf(buffer, sizeof(buffer), "[+] Puerto %d abierto", port);
        add_message(buffer, 0);
        verificar_puerto(port);
    } else if (port % 5 == 0) {
        snprintf(buffer, sizeof(buffer), "[!] Error al escanear puerto %d", port);
        add_message(buffer, 1);
    }
}

// ========== Interfaz GTK ========== (igual que antes pero con funciones modificadas)

// Función para mostrar alertas
void mostrar_alertas(GtkWidget* widget, gpointer user_data) {
    // ... (código existente)
    
    // Obtener alertas
    int alert_count;
    char** alerts = get_messages(&alert_count, 1);  // 1 = obtener alertas
    
    // ... (resto del código para mostrar en la interfaz)
    
    // Liberar memoria
    for (int i = 0; i < alert_count; i++) {
        free(alerts[i]);
    }
    free(alerts);
}

// Función para mostrar puertos
void mostrar_puertos(GtkWidget* widget, gpointer user_data) {
    // ... (código existente)
    
    // Obtener mensajes de puertos
    int message_count;
    char** messages = get_messages(&message_count, 0);  // 0 = obtener mensajes normales
    
    // ... (resto del código para mostrar en la interfaz)
    
    // Liberar memoria
    for (int i = 0; i < message_count; i++) {
        free(messages[i]);
    }
    free(messages);
}

// Función para simular el escaneo en segundo plano
gboolean background_scan(gpointer data) {
    static int port = 1;
    if (port < 100) {
        scan_port(port++);
        return TRUE;  // Continuar escaneo
    }
    return FALSE;  // Detener escaneo
}

int main(int argc, char* argv[]) {
    // Inicializar mutex
    pthread_mutex_init(&message_store.mutex, NULL);
    
    gtk_init(&argc, &argv);
    
    // ... (código de interfaz existente)
    
    // Iniciar escaneo en segundo plano
    g_timeout_add(100, background_scan, NULL);
    
    gtk_main();
    
    // Limpieza final
    pthread_mutex_destroy(&message_store.mutex);
    free(message_store.messages);
    
    return 0;
}