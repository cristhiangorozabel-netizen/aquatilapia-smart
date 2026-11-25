import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

/**
 * PROYECTO INTEGRADOR DE NIVELACION - TECNOLOGIAS DE LA INFORMACION (UTMACH)
 * Empresa Ficticia: BioAquaTech Ecuador
 * Producto: AquaTilapia Smart - Sistema Acuaponico Automatizado IoT
 * Traduccion a Java del algoritmo de control de oxigenacion, recirculacion y seguridad.
 */
public class AquaTilapiaSistema {

    // Umbrales biologicos y operativos para cultivo de tilapia
    private static final double O2_MINIMO = 5.0;
    private static final double TEMP_MINIMA = 25.0;
    private static final double TEMP_MAXIMA = 30.0;
    private static final double DISTANCIA_MAX_AGUA_CM = 15.0;
    private static final double HUMEDAD_MAX_RIEGO = 75.0;

    // Estado del sistema
    private double oxigeno = 6.8;
    private double temperatura = 27.2;
    private double humedadTierra = 42.0;
    private double distanciaAgua = 8.5;

    private int tiempoRiegoSeg = 10;
    private int tiempoEsperaSeg = 20;
    private boolean faseRiegoActiva = true;
    private boolean modoManual = false;
    private boolean aireadorEncendido = true; // Continuo 24/7
    private boolean bombaEncendida = false;

    private final List<String> historial = new ArrayList<>();
    private final DateTimeFormatter fmt = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

    public void evaluarSistema() {
        boolean alertaO2 = oxigeno < O2_MINIMO;
        boolean alertaTemp = (temperatura < TEMP_MINIMA || temperatura > TEMP_MAXIMA);
        boolean alertaAgua = distanciaAgua > DISTANCIA_MAX_AGUA_CM;

        // El aireador opera de forma continua 24/7
        aireadorEncendido = true;

        String estadoOperativo;
        if (modoManual) {
            bombaEncendida = true;
            estadoOperativo = "MODO MANUAL - Flujo continuo activo";
        } else {
            if (alertaO2) {
                System.out.println("[ALERTA CRITICA] Oxigeno disuelto bajo (" + oxigeno + " mg/L) -> Buzzer O2 ACTIVADO");
            }
            if (alertaTemp) {
                System.out.println("[ALERTA TERMICA] Temperatura fuera de rango (" + temperatura + " C) -> LED Rojo ACTIVADO");
            }

            // Jerarquia de seguridad de 3 prioridades para la mini bomba
            if (alertaAgua) {
                bombaEncendida = false;
                estadoOperativo = "BLOQUEADA POR SEGURIDAD (Falta agua > 15 cm)";
                System.out.println("[SEGURIDAD] Nivel bajo en estanque -> Bomba detenida + Buzzer Nivel ACTIVADO");
            } else if (humedadTierra >= HUMEDAD_MAX_RIEGO) {
                bombaEncendida = false;
                estadoOperativo = "PAUSA POR SUSTRATO HUMEDO (>= 75%)";
            } else {
                if (faseRiegoActiva) {
                    bombaEncendida = true;
                    estadoOperativo = "REGANDO CULTIVO (" + tiempoRiegoSeg + "s ON)";
                } else {
                    bombaEncendida = false;
                    estadoOperativo = "REPOSO / DRENAJE BIOFILTRO (" + tiempoEsperaSeg + "s OFF)";
                }
                faseRiegoActiva = !faseRiegoActiva;
            }

            if (!alertaO2 && !alertaTemp && !alertaAgua) {
                System.out.println("[OPTIMO] Parametros biologicos ideales -> LED Verde ACTIVADO");
            }
        }

        String marcaTiempo = LocalDateTime.now().format(fmt);
        String registro = String.format("[%s] O2: %.1f mg/L | Temp: %.1f C | Hum: %.0f%% | Nivel: %.1f cm | Aire: %s | Bomba: %s | %s",
                marcaTiempo, oxigeno, temperatura, humedadTierra, distanciaAgua,
                aireadorEncendido ? "ON" : "OFF",
                bombaEncendida ? "ON" : "OFF",
                estadoOperativo);
        historial.add(registro);
        System.out.println("
" + registro);
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        AquaTilapiaSistema sistema = new AquaTilapiaSistema();
        boolean salir = false;

        System.out.println("============================================================");
        System.out.println("   BIOAQUATECH ECUADOR - AQUATILAPIA SMART (MODULO JAVA)");
        System.out.println("============================================================");

        while (!salir) {
            System.out.println("
1. Ingresar lecturas de sensores y ejecutar ciclo");
            System.out.println("2. Alternar Modo (Automatico / Manual)");
            System.out.println("3. Configurar temporizador de irrigacion");
            System.out.println("4. Ver historial de mediciones");
            System.out.println("5. Salir");
            System.out.print("Opcion: ");

            int op = sc.nextInt();
            switch (op) {
                case 1:
                    System.out.print("Oxigeno Disuelto (mg/L): ");
                    sistema.oxigeno = sc.nextDouble();
                    System.out.print("Temperatura del Agua (C): ");
                    sistema.temperatura = sc.nextDouble();
                    System.out.print("Humedad de Tierra (%): ");
                    sistema.humedadTierra = sc.nextDouble();
                    System.out.print("Distancia Ultrasonico al agua (cm): ");
                    sistema.distanciaAgua = sc.nextDouble();
                    sistema.evaluarSistema();
                    break;
                case 2:
                    sistema.modoManual = !sistema.modoManual;
                    System.out.println("Modo actual: " + (sistema.modoManual ? "MANUAL" : "AUTOMATICO"));
                    break;
                case 3:
                    System.out.print("Segundos enviando agua (min 3): ");
                    int on = sc.nextInt();
                    System.out.print("Segundos de espera/drenaje (min 3): ");
                    int off = sc.nextInt();
                    if (on >= 3 && off >= 3) {
                        sistema.tiempoRiegoSeg = on;
                        sistema.tiempoEsperaSeg = off;
                        System.out.println("Temporizador actualizado correctamente.");
                    }
                    break;
                case 4:
                    System.out.println("
--- HISTORIAL DE TELEMETRIA ---");
                    if (sistema.historial.isEmpty()) {
                        System.out.println("Aun no hay registros.");
                    } else {
                        for (String item : sistema.historial) {
                            System.out.println(item);
                        }
                    }
                    break;
                case 5:
                    salir = true;
                    break;
                default:
                    System.out.println("Opcion invalida.");
            }
        }
        sc.close();
    }
}
