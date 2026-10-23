---

# 🚀 TNB - Nave Espacial en Vivo (Simulador de Cálculo Vectorial)

Herramienta interactiva 3D desarrollada con **Three.js** y **WebSockets** para visualizar el **marco de Frenet-Serret ($T, N, B$)**, la curvatura ($\kappa$), el radio de curvatura ($R$) y la torsión ($\tau$) de una trayectoria espacial en tiempo real, utilizando telemetría de hardware (ESP32 + MPU6050) o control por teclado.

Creado con un enfoque pedagógico de **aprendizaje encarnado** para las clases de Cálculo Vectorial y Ciencias Básicas.

---

## 🛠️ Características Principales

* **Visualización 3D en Tiempo Real:** Renderizado fluido mediante Three.js de la nave, su estela de trayectoria y los vectores del marco de Frenet-Serret.


* **Control Dual:**
* 🎮 **Modo MPU6050:** Recepción de datos inerciales en vivo vía WebSocket desde un microcontrolador ESP32.


* ⌨️ **Modo Teclado:** Pilotaje autónomo e intuitivo directamente desde la computadora.




* **Análisis Polinómico por Mínimos Cuadrados:** Ajuste de curvas espaciales ($r(t)$) de grado 2 a 6 con resolución matricial y cálculo de error RMS.


* **Gestión de Datos:** Capacidad de guardar y cargar trayectorias en formato `JSON` para análisis posterior.


* **Temas Dinámicos:** Soporte para modo oscuro y claro integrado.



---

## ⌨️ Controles por Teclado (Modo Teclado)

Cuando el simulador se conmuta al modo teclado, la nave se puede pilotar directamente usando las siguientes teclas:

| Acciones / Comandos | Teclas |
| --- | --- |
| **Yaw (Cabeceo horizontal):** Guiñada izquierda / derecha | `←` / `A`   e   `→` / `D` |
| **Pitch (Inclinación vertical):** Nariz arriba / abajo | `↑` / `W`   e   `↓` / `S` |
| **Roll (Rape / Alabeo):** Rotación sobre el eje longitudinal | `Q` (izq)   y   `E` (der) |
| **Avanzar / Detener:** Alterna el motor de crucero | `Barra Espaciadora` |
| **Reiniciar Posición:** Vuelve al origen y limpia la traza | `R` |
| **Cambiar Tema:** Alterna entre modo claro y oscuro | `T` |

---

## 🔌 Conexiones de Hardware (ESP32 + MPU6050)

Para el funcionamiento en tiempo real con el sensor inercial físico, el módulo MPU6050 se conecta al microcontrolador ESP32 mediante el bus **I2C** estándar de la siguiente manera:

| MPU6050 Pin | ESP32 Pin | Descripción |
| --- | --- | --- |
| **VCC** | **3.3V** | Alimentación del módulo |
| **GND** | **GND** | Tierra común |
| **SDA** | **GPIO 21** | Línea de datos I2C |
| **SCL** | **GPIO 22** | Línea de reloj I2C |

> **Nota:** El código fuente se encuentra disponible en este repositorio.

---

## 👨‍🏫 Contexto Académico

* **Institución:** Tecnológico Nacional de México (TecNM) – Instituto Tecnológico de Tuxtepec (ITTUX).


* **Asignatura:** Cálculo Vectorial / Ciencias Básicas.


* **Autor:** Rodrigo Compañ Sarmiento (2026).



---

## 📄 Licencia y Protección de Autoría

Este proyecto está protegido bajo la **Licencia General Pública de GNU v3.0 (GPLv3)**. Esto garantiza que cualquier uso, modificación o distribución académica de este material deba preservar la autoría original y mantenerse bajo los mismos principios de apertura de código libre.

Consulta el archivo [LICENSE](https://www.google.com/search?q=LICENSE) para más detalles.
