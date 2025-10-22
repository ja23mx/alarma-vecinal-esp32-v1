#include "MonitorRAM.h"
#include "LogSistema.h" // Para imprimir logs si es necesario

/**
 * @brief Obtiene el porcentaje de uso de la RAM.
 */
void RAMStatus(bool flash_info)
{
    if (flash_info)
    {
        LOG("\r\n\r\n\r\n\r\n=== INFORMACION DEL ESP32 ===");
        LOGF("\r\nChip Model: %s\n", ESP.getChipModel());
        LOGF("\r\nChip Cores: %d\n", ESP.getChipCores());
        LOGF("\r\nCPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
        LOGF("\r\nMemoria Libre (Heap): %d bytes\n", ESP.getFreeHeap());
        LOGF("\r\nMemoria Flash Total: %d KB\n", ESP.getFlashChipSize() / 1024);
        LOGF("\r\nMemoria Flash Usada: %d KB\n", ESP.getSketchSize() / 1024);
        LOGF("\r\nMemoria Flash Libre: %d KB\n", ESP.getFreeSketchSpace() / 1024);
    }

    uint32_t totalRAM = ESP.getHeapSize() / 1024;
    uint32_t libreRAM = ESP.getFreeHeap() / 1024;
    uint32_t usadoRAM = totalRAM - libreRAM;
    uint8_t porcentajeUso = ((totalRAM - libreRAM) * 100) / totalRAM; // Calculamos el porcentaje de uso

    if (!flash_info)
        LOG("\r\n\r\n");

    LOG("\r\n\r\n=== INFORMACION DE LA RAM ===");
    LOGF("\r\nRAM usada: %dkB/%dkB | Uso: %d%%", usadoRAM, totalRAM, porcentajeUso);
}
