// ============================================================================
// PROYECTO INTEGRADOR DE NIVELACION - TECNOLOGIAS DE LA INFORMACION (UTMACH)
// Empresa Ficticia: BioAquaTech Ecuador | Producto: AquaTilapia Smart
// Algoritmo: Control Automatizado de Oxigenacion, Recirculacion y Seguridad
// ============================================================================

SubProceso MostrarEstadoGeneral(o2, temp, hum, distAgua, aireOn, bombaOn, modoManual)
    Escribir "------------------------------------------------------------"
    Si modoManual Entonces
        Escribir "MODO DE OPERACION: [MANUAL]"
    SiNo
        Escribir "MODO DE OPERACION: [AUTOMATICO]"
    FinSi
    Escribir "Oxigeno Disuelto (Tilapias): ", o2, " mg/L  (Minimo: 5.0 mg/L)"
    Escribir "Temperatura del Agua:        ", temp, " C     (Ideal: 25.0 - 30.0 C)"
    Escribir "Humedad Sustrato Cultivo:    ", hum, " %     (Limite riego: 75%)"
    Escribir "Distancia Sensor-Agua:       ", distAgua, " cm   (Maximo: 15.0 cm)"
    Si aireOn Entonces
        Escribir "Actuador 1 - Aireador 24/7:  ENCENDIDO (Oxigenacion activa)"
    SiNo
        Escribir "Actuador 1 - Aireador 24/7:  APAGADO"
    FinSi
    Si bombaOn Entonces
        Escribir "Actuador 2 - Mini Bomba:     ENCENDIDA (Enviando agua al cultivo)"
    SiNo
        Escribir "Actuador 2 - Mini Bomba:     APAGADA"
    FinSi
    Escribir "------------------------------------------------------------"
FinSubProceso

Proceso AquaTilapiaSmart_Control
    Definir o2, temp, hum, distAgua Como Real
    Definir tiempoRiego, tiempoEspera, contadorCiclo, opcion Como Entero
    Definir aireOn, bombaOn, modoManual, faseRiego, salir Como Logico
    Definir alertaO2, alertaTemp, alertaAgua Como Logico

    // Parametros iniciales del sistema
    tiempoRiego <- 10
    tiempoEspera <- 20
    contadorCiclo <- 0
    aireOn <- Verdadero
    bombaOn <- Falso
    modoManual <- Falso
    faseRiego <- Verdadero
    salir <- Falso

    Escribir "============================================================"
    Escribir "   BIOAQUATECH ECUADOR - SISTEMA AQUATILAPIA SMART IoT"
    Escribir "============================================================"

    Mientras No salir Hacer
        Escribir ""
        Escribir "Seleccione una opcion de simulacion de sensores:"
        Escribir "1. Ingresar lectura de sensores y evaluar ciclo"
        Escribir "2. Alternar Modo (Automatico / Manual)"
        Escribir "3. Configurar tiempos de riego y drenaje"
        Escribir "4. Salir del sistema"
        Leer opcion

        Segun opcion Hacer
            1:
                Escribir "Ingrese Oxigeno Disuelto actual (mg/L, ej. 6.5):"
                Leer o2
                Escribir "Ingrese Temperatura del agua (C, ej. 27.5):"
                Leer temp
                Escribir "Ingrese Humedad del sustrato (%, ej. 45):"
                Leer hum
                Escribir "Ingrese Distancia sensor ultrasonico al agua (cm, ej. 9.0):"
                Leer distAgua

                // Evaluacion de umbrales biologicos y operativos
                alertaO2 <- (o2 < 5.0)
                alertaTemp <- (temp < 25.0 O temp > 30.0)
                alertaAgua <- (distAgua > 15.0)

                // El aireador permanece encendido 24/7 para preservar las tilapias
                aireOn <- Verdadero

                Si modoManual Entonces
                    bombaOn <- Verdadero
                    Escribir "[INFO] Modo Manual activo: bomba en flujo continuo y alarmas silenciadas."
                SiNo
                    // Alertas visuales y sonoras
                    Si alertaO2 Entonces
                        Escribir "[ALERTA CRITICA] Oxigeno disuelto bajo (< 5.0 mg/L) -> Buzzer O2 ON + LED Rojo"
                    FinSi
                    Si alertaTemp Entonces
                        Escribir "[ALERTA TERMICA] Temperatura fuera de rango (25-30 C) -> LED Rojo ON"
                    FinSi

                    // Jerarquia de control de la mini bomba
                    Si alertaAgua Entonces
                        // Prioridad 1: Proteccion contra trabajo en seco
                        bombaOn <- Falso
                        Escribir "[SEGURIDAD] Nivel de agua bajo (> 15 cm) -> Bomba BLOQUEADA + Buzzer Nivel ON"
                    SiNo
                        Si hum >= 75.0 Entonces
                            // Prioridad 2: Control por saturacion del sustrato
                            bombaOn <- Falso
                            Escribir "[PAUSA RIEGO] Sustrato suficientemente humedo (>= 75%) -> Bomba en espera"
                        SiNo
                            // Prioridad 3: Temporizador ciclico de irrigacion
                            Si faseRiego Entonces
                                bombaOn <- Verdadero
                                Escribir "[RIEGO ACTIVO] Bombeando agua hacia cama de cultivo (", tiempoRiego, " seg)"
                            SiNo
                                bombaOn <- Falso
                                Escribir "[DRENAJE / REPOSO] Filtrando agua en biofiltro (", tiempoEspera, " seg)"
                            FinSi
                            faseRiego <- No faseRiego
                        FinSi
                    FinSi

                    Si No alertaO2 Y No alertaTemp Y No alertaAgua Entonces
                        Escribir "[OPTIMO] Todos los parametros biologicos en rango ideal -> LED Verde ON"
                    FinSi
                FinSi

                MostrarEstadoGeneral(o2, temp, hum, distAgua, aireOn, bombaOn, modoManual)

            2:
                modoManual <- No modoManual
                Si modoManual Entonces
                    Escribir "Sistema cambiado a MODO MANUAL."
                SiNo
                    Escribir "Sistema cambiado a MODO AUTOMATICO."
                FinSi

            3:
                Escribir "Ingrese tiempo de envio de agua (segundos, min 3):"
                Leer tiempoRiego
                Escribir "Ingrese tiempo de espera/drenaje (segundos, min 3):"
                Leer tiempoEspera
                Escribir "Temporizador actualizado: ", tiempoRiego, "s ON / ", tiempoEspera, "s OFF."

            4:
                salir <- Verdadero
                Escribir "Finalizando monitoreo de AquaTilapia Smart..."

            De Otro Modo:
                Escribir "Opcion no valida. Intente nuevamente."
        FinSegun
    FinMientras
FinProceso
