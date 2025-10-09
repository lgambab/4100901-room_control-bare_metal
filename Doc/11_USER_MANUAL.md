# 11. USER_MANUAL.md

## Manual de Usuario: Sistema de Control de Sala

**Universidad Nacional de Colombia - Sede Manizales**  
**Curso:** Estructuras Computacionales (4100901)

### 1. Introducción

Escribe una introducción al Sistema de Control de Sala, explicando su propósito como controlador de iluminación para una habitación, utilizando LEDs que simulan bombillas controladas por PWM.

### 2. Hardware Utilizado

Describe el hardware utilizado, incluyendo la placa STM32L476RG, LEDs (heartbeat, bombilla principal, bombilla PWM), botón de usuario, y comunicación UART.

### 3. Funcionalidades

#### 3.1 Heartbeat LED
Explica el propósito del LED de heartbeat.

#### 3.2 Control de Bombilla por Botón
Describe cómo funciona el control de la bombilla principal mediante el botón, incluyendo el tiempo de encendido y mensajes UART.

#### 3.3 Comunicación UART
Detalla los comandos UART disponibles para controlar el sistema, incluyendo eco de caracteres y comandos específicos para PWM.

#### 3.4 Control PWM de Bombilla
Explica cómo el PWM controla el brillo de la bombilla secundaria, incluyendo frecuencia y duty cycle.

### 4. Arquitectura del Sistema

#### 4.1 Módulos
Lista y describe brevemente los módulos de software implementados.

#### 4.2 Flujo de Ejecución
Describe el flujo de ejecución del sistema, desde inicialización hasta procesamiento de eventos.

### 5. Uso del Sistema

1. **Conexión:**
   Instrucciones para conectar el hardware y configurar la comunicación.

2. **Inicio:**
   Qué sucede cuando se enciende el sistema.

3. **Interacción:**
   Cómo interactuar con el sistema mediante botón y UART.

### 6. Diagramas

#### Diagrama de Estados
Incluye un diagrama de estados que muestre las transiciones del sistema.

#### Diagrama de Componentes
Incluye un diagrama de componentes mostrando la arquitectura del software.

---

**Fin del Manual**