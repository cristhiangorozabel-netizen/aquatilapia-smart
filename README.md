# AquaTilapia Smart — Sistema Acuapónico Automatizado IoT

**Empresa Ficticia:** BioAquaTech Ecuador  
**Producto:** AquaTilapia Smart  
**Institución:** Universidad Técnica de Machala (UTMACH) — Nivelación de Tecnologías de la Información (Periodo 2026-1)

## Descripción del Proyecto

**AquaTilapia Smart** es un sistema acuapónico inteligente de tres niveles (estanque de tilapias, cama de cultivo hidropónico y biofiltro purificador) automatizado mediante un microcontrolador **ESP32**. Garantiza oxigenación continua 24/7 para las tilapias, regula la recirculación de agua mediante temporización cíclica y sensores de seguridad (nivel ultrasónico y humedad de sustrato), y transmite telemetría en tiempo real hacia una aplicación web pública.

## Integrantes del Equipo (8 Integrantes)

* **Líder del Grupo:** Cristhian Joao Gorozabel Roldan — *Arquitectura IoT, Firmware ESP32 y Publicación Web*
* **Integrante 1:** Justin Leonardo — *Desarrollo de Algoritmo en PSeInt*
* **Integrante 2:** Mathias Andres  — *Traducción y Validación en Java*
* **Integrante 3:** Darwin Alexis Ojeda Tamayo  — *Simulación Electrónica en Wokwi y Esquemas*
* **Integrante 4:** Brithany Dayanara Vega Cueva — *Montaje Físico, Sensores y Relés*
* **Integrante 5:** Brithany Suleiny Freire Rios — *Construcción de Maqueta Acuapónica y Biofiltro*
* **Integrante 6:** Jean Lucas Tituana Calderon  — *Diseño Comercial e Identidad Visual*
* **Integrante 7:** Robin Alejandro Salas Pedreros — *Documentación Técnica, Pruebas y Evidencias*

## Enlaces del Proyecto

* **Sitio Web Público:** `\[Pegar aquí URL pública de Netlify / GitHub Pages]`
* **Simulación Electrónica (Wokwi):** `\[https://wokwi.com/projects/481694232724767745]`

## Estructura del Repositorio

```text
aquatilapia-smart/
├── README.md                                      # Documentación general, integrantes y enlaces
├── PSeInt/
│   └── AquaTilapia\_Control.psc                    # Algoritmo y pseudocódigo del sistema
├── Java/
│   └── AquaTilapiaSistema.java                    # Implementación orientada a objetos en Java
└── Electronica/
    └── AquaTilapia\_ESP32\_WebCloud.ino             # Firmware ESP32 con telemetría IoT y LCD I2C
```

